/*
Autor.....: Guilherme Viana da Silva
Turma.....: 2C ESW
Data......: 07/11/2026
Objetivo..: Sistema de Controle Bancario - aplicar listas encadeadas
            Contas ............: lista DUPLAMENTE encadeada
            Movimentacoes .....: lista SIMPLESMENTE encadeada
*/
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//Nomes dos arquivos onde as listas sao gravadas
#define ARQ_CONTAS "contas.dat"
#define ARQ_MOV    "movimentacoes.dat"

#define COL        33    //coluna onde o usuario digita os campos
#define LINHA_INI  7     //primeira linha dos relatorios
#define LINHA_FIM  20    //ultima linha dos relatorios

const char *TIPOS[3] = {"corrente", "poupanca", "cartao credito"};

//---------------------- Estruturas de dados ----------------------

//Dados da conta bancaria
typedef struct
{
    int     codigo_conta;
    char    banco[50];
    char    agencia[10];
    char    numero_conta[20];
    char    tipo_conta[20];
    double  vl_saldo;
    double  vl_limite;      //limite negativo permitido (cheque especial)
    char    status[10];     //"ativo" ou "inativo"
} reg_conta;

//No da lista de contas: DUPLAMENTE encadeada (tem anterior e proximo)
typedef struct ContaItem *ContaApontador;

typedef struct ContaItem
{
    reg_conta conteudo;
    ContaApontador anterior;
    ContaApontador proximo;
} ContaItem;

typedef struct
{
    ContaApontador Primeiro;
    ContaApontador Ultimo;
} ListaContas;

//Dados da movimentacao financeira
typedef struct
{
    int     sequencial;         //1, 2, 3... dentro de cada conta
    int     codigo_conta;
    char    dt_movimento[11];   //dd/mm/aaaa
    char    tp_movimentacao[15];//"credito" ou "debito"
    char    ds_favorecido[50];
    double  vl_movimento;
    double  vl_saldo;           //saldo da conta depois da movimentacao
} reg_mov;

//No da lista de movimentacoes: SIMPLESMENTE encadeada (so proximo)
typedef struct MovItem *MovApontador;

typedef struct MovItem
{
    reg_mov conteudo;
    MovApontador proximo;
} MovItem;

typedef struct
{
    MovApontador Primeiro;
    MovApontador Ultimo;
} ListaMov;

//---------------------- Funcoes de tela ----------------------

//Função para posicionar o cursor
void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = (short)x;
    coord.Y = (short)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//Desenha a moldura da tela
void tela(){
    int y, i;

    for (y = 1; y <= 24; y++){
        gotoxy(1,y);
        if (y == 1 || y == 4 || y == 22 || y == 24){
            putchar('+');
            for (i = 0; i < 116; i++) putchar('-');
            putchar('+');
        }else{
            putchar('|');
            for (i = 0; i < 116; i++) putchar(' ');
            putchar('|');
        }
    }
    gotoxy(4,2);
    printf("UNICIVE");
    gotoxy(45,2);
    printf("SISTEMA DE CONTROLE BANCARIO");
    gotoxy(97,2);
    printf("ESTRUTURA DE DADOS");
    gotoxy(3,23);
    printf("MSG:");
}

//Escreve o titulo da tela
void titulo(const char *texto){
    gotoxy(40,3);
    printf("==== %s ====", texto);
}

//Escreve o nome de um campo com pontinhos ate a coluna dos dois pontos
void rotulo(int x, int y, const char *txt){
    int n = strlen(txt);

    gotoxy(x,y);
    printf("%s", txt);
    while (n < 22){
        putchar('.');
        n++;
    }
    putchar(':');
}

//Tela de cadastro de conta
void tela_conta(){
    rotulo(9,5,"Codigo da Conta");
    rotulo(9,7,"1- Banco");
    rotulo(9,9,"2- Agencia");
    rotulo(9,11,"3- Numero da Conta");
    rotulo(9,13,"4- Tipo da Conta");
    gotoxy(70,13);
    printf("(1-Corrente 2-Poupanca 3-Cartao Credito)");
    rotulo(9,15,"5- Limite (R$)");
    rotulo(9,17,"6- Saldo (R$)");
    rotulo(9,19,"7- Status");
    gotoxy(70,19);
    printf("(1-Ativo 2-Inativo)");
}

//Tela de movimentacao financeira
void tela_mov(){
    rotulo(9,5,"Codigo da Conta");
    rotulo(9,7,"Banco");
    rotulo(9,9,"Saldo Atual (R$)");
    rotulo(9,11,"Limite (R$)");
    rotulo(9,13,"Data (dd/mm/aaaa)");
    rotulo(9,15,"Favorecido/Origem");
    rotulo(9,17,"Valor (R$)");
    rotulo(9,19,"Saldo Apos (R$)");
}

//Função que limpa o buffer do teclado
void limparBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//Função que limpa a mensagem
void limpa_mensagem(){
    gotoxy(07,23);
    printf("                                                                        ");
}

//Função que tira o ENTER do final e limpa o que sobrou no buffer
void ajusta_string(char *str)
{
    int len = strlen(str);

    if (len > 0 && str[len-1] == '\n'){
        str[len-1] = '\0';
    }else{
        limparBuffer();
    }
}

//Mostra uma mensagem e espera uma tecla
void mensagem(const char *texto){
    limpa_mensagem();
    gotoxy(07,23);
    printf("%s", texto);
    getch();
    limpa_mensagem();
}

//Mostra uma pergunta na linha de mensagem e le um numero
int perguntaInt(const char *texto){
    int n = 0;

    limpa_mensagem();
    gotoxy(07,23);
    printf("%s", texto);
    scanf("%d", &n);
    limparBuffer();
    limpa_mensagem();

    return n;
}

//Pergunta a posicao da lista (0 para sair)
int perguntaPosicao(int maximo){
    int pos;

    do{
        pos = 0;
        limpa_mensagem();
        gotoxy(07,23);
        printf("Posicao (1 a %d, 0=sair).: ", maximo);
        scanf("%d", &pos);
        limparBuffer();

        if (pos < 0 || pos > maximo){
            mensagem("Posicao invalida..");
        }
    }while (pos < 0 || pos > maximo);

    limpa_mensagem();
    return pos;
}

//Le a opcao de um menu (retorna -1 se a entrada acabou)
int lerOpcao(){
    int n = 0;

    if (scanf("%d", &n) == EOF) return -1;
    limparBuffer();
    return n;
}

//Limpa o campo da tela e deixa o cursor no inicio dele
void limpaCampo(int y){
    gotoxy(COL,y);
    printf("                                                            ");
    gotoxy(COL,y);
}

//---------------------- Funcoes auxiliares (regras) ----------------------

//Transforma o valor em centavos (numero inteiro): evita erro de arredondamento
long long centavos(double v){
    if (v >= 0) return (long long)(v * 100.0 + 0.5);
    return (long long)(v * 100.0 - 0.5);
}

//Retorna a data como numero aaaammdd (para poder comparar) ou -1 se for invalida
long dataChave(const char *s){
    int dias[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int i, d, m, a, maxDia;

    if (strlen(s) != 10) return -1;

    for (i = 0; i < 10; i++){
        if (i == 2 || i == 5){
            if (s[i] != '/') return -1;
        }else if (s[i] < '0' || s[i] > '9'){
            return -1;
        }
    }

    d = (s[0]-'0')*10 + (s[1]-'0');
    m = (s[3]-'0')*10 + (s[4]-'0');
    a = (s[6]-'0')*1000 + (s[7]-'0')*100 + (s[8]-'0')*10 + (s[9]-'0');

    if (a < 1900 || a > 2200 || m < 1 || m > 12 || d < 1) return -1;

    maxDia = dias[m-1];
    //ano bissexto
    if (m == 2 && ((a % 4 == 0 && a % 100 != 0) || a % 400 == 0)) maxDia = 29;

    if (d > maxDia) return -1;

    return a * 10000L + m * 100L + d;
}

//---------------------- Lista duplamente encadeada: CONTAS ----------------------

//Deixa a lista vazia
void iniciaContas(ListaContas *L){
    L->Primeiro = NULL;
    L->Ultimo = NULL;
}

//Conta quantas contas tem na lista
int tamanhoContas(ListaContas *L){
    int n = 0;
    ContaApontador p = L->Primeiro;

    while (p != NULL){
        n++;
        p = p->proximo;
    }
    return n;
}

//Procura uma conta pelo codigo (retorna NULL se nao achar)
ContaApontador pesquisaConta(ListaContas *L, int codigo){
    ContaApontador p = L->Primeiro;

    while (p != NULL){
        if (p->conteudo.codigo_conta == codigo){
            return p;
        }
        p = p->proximo;
    }
    return NULL;
}

//Retorna a conta que esta na posicao (comeca em 1)
ContaApontador buscaPosicaoConta(ListaContas *L, int pos){
    int i;
    ContaApontador p = L->Primeiro;

    for (i = 1; i < pos && p != NULL; i++){
        p = p->proximo;
    }
    return p;
}

//Cria um no novo com os dados da conta
ContaApontador criaConta(reg_conta c){
    ContaApontador novo = (ContaApontador) malloc(sizeof(ContaItem));

    if (novo != NULL){
        novo->conteudo = c;
        novo->anterior = NULL;
        novo->proximo = NULL;
    }
    return novo;
}

//Insere no inicio da lista
int insereContaInicio(ListaContas *L, reg_conta c){
    ContaApontador novo = criaConta(c);

    if (novo == NULL) return 0;

    novo->proximo = L->Primeiro;

    if (L->Primeiro != NULL){
        L->Primeiro->anterior = novo;   //o antigo primeiro aponta de volta para o novo
    }else{
        L->Ultimo = novo;               //lista estava vazia
    }
    L->Primeiro = novo;
    return 1;
}

//Insere no final da lista
int insereContaFinal(ListaContas *L, reg_conta c){
    ContaApontador novo = criaConta(c);

    if (novo == NULL) return 0;

    novo->anterior = L->Ultimo;

    if (L->Ultimo != NULL){
        L->Ultimo->proximo = novo;
    }else{
        L->Primeiro = novo;             //lista estava vazia
    }
    L->Ultimo = novo;
    return 1;
}

//Insere em uma posicao da lista (comeca em 1)
int insereContaPosicao(ListaContas *L, reg_conta c, int pos){
    int n = tamanhoContas(L);
    ContaApontador novo;
    ContaApontador atual;

    if (pos < 1 || pos > n + 1) return 0;

    if (pos == 1) return insereContaInicio(L, c);
    if (pos == n + 1) return insereContaFinal(L, c);

    novo = criaConta(c);
    if (novo == NULL) return 0;

    //atual e quem esta hoje na posicao; o novo entra ANTES dele
    atual = buscaPosicaoConta(L, pos);

    novo->proximo = atual;
    novo->anterior = atual->anterior;
    atual->anterior->proximo = novo;
    atual->anterior = novo;
    return 1;
}

//Inclui uma conta validando os dados. modo: 1=inicio 2=posicao 3=final
//Retorna 0=ok 1=codigo invalido/duplicado 2=posicao invalida 3=sem memoria
int incluiConta(ListaContas *L, reg_conta c, int modo, int pos){
    int ok;

    if (c.codigo_conta <= 0) return 1;
    if (pesquisaConta(L, c.codigo_conta) != NULL) return 1;

    if (modo == 1) ok = insereContaInicio(L, c);
    else if (modo == 3) ok = insereContaFinal(L, c);
    else{
        if (pos < 1 || pos > tamanhoContas(L) + 1) return 2;
        ok = insereContaPosicao(L, c, pos);
    }
    return ok ? 0 : 3;
}

//Remove uma conta da lista (p e o no que sera removido)
void removeConta(ListaContas *L, ContaApontador p){
    if (p->anterior != NULL){
        p->anterior->proximo = p->proximo;
    }else{
        L->Primeiro = p->proximo;       //removeu o primeiro
    }

    if (p->proximo != NULL){
        p->proximo->anterior = p->anterior;
    }else{
        L->Ultimo = p->anterior;        //removeu o ultimo
    }
    free(p);
}

//Libera toda a memoria da lista de contas
void liberaContas(ListaContas *L){
    while (L->Primeiro != NULL){
        removeConta(L, L->Primeiro);
    }
}

//---------------------- Lista simplesmente encadeada: MOVIMENTACOES ----------------------

void iniciaMov(ListaMov *M){
    M->Primeiro = NULL;
    M->Ultimo = NULL;
}

//Insere no final da lista
int insereMovFinal(ListaMov *M, reg_mov m){
    MovApontador novo = (MovApontador) malloc(sizeof(MovItem));

    if (novo == NULL) return 0;

    novo->conteudo = m;
    novo->proximo = NULL;

    if (M->Primeiro == NULL){
        M->Primeiro = novo;
    }else{
        M->Ultimo->proximo = novo;
    }
    M->Ultimo = novo;
    return 1;
}

//Quantas movimentacoes uma conta tem
int contaMov(ListaMov *M, int codigo){
    int n = 0;
    MovApontador p = M->Primeiro;

    while (p != NULL){
        if (p->conteudo.codigo_conta == codigo) n++;
        p = p->proximo;
    }
    return n;
}

//Data (aaaammdd) da ultima movimentacao da conta; 0 se nao tem nenhuma
long ultimaData(ListaMov *M, int codigo){
    long maior = 0;
    long k;
    MovApontador p = M->Primeiro;

    while (p != NULL){
        if (p->conteudo.codigo_conta == codigo){
            k = dataChave(p->conteudo.dt_movimento);
            if (k > maior) maior = k;
        }
        p = p->proximo;
    }
    return maior;
}

//Libera toda a memoria da lista de movimentacoes
void liberaMov(ListaMov *M){
    MovApontador aux;

    while (M->Primeiro != NULL){
        aux = M->Primeiro;
        M->Primeiro = aux->proximo;
        free(aux);
    }
    M->Ultimo = NULL;
}

//Registra uma movimentacao e atualiza o saldo da conta. tipo: 1=credito 2=debito
//Retorna 0=ok 1=conta inativa 2=valor invalido 3=data anterior a ultima
//        4=limite excedido 5=sem memoria 6=data invalida
int registraMov(ContaApontador c, ListaMov *M, int tipo, const char *data,
                const char *favorecido, double valor){
    reg_mov m;
    long chave;
    long long saldo, v, lim, novo;

    if (strcmp(c->conteudo.status, "ativo") != 0) return 1;

    if (!(valor > -1e12 && valor < 1e12) || centavos(valor) <= 0) return 2;

    chave = dataChave(data);
    if (chave < 0) return 6;
    if (chave < ultimaData(M, c->conteudo.codigo_conta)) return 3;

    saldo = centavos(c->conteudo.vl_saldo);
    v     = centavos(valor);
    lim   = centavos(c->conteudo.vl_limite);

    if (tipo == 1) novo = saldo + v;
    else           novo = saldo - v;

    //Regra do limite: depois do debito o saldo nao pode ficar abaixo de -limite
    if (tipo == 2 && novo < -lim) return 4;

    memset(&m, 0, sizeof(m));
    m.sequencial = contaMov(M, c->conteudo.codigo_conta) + 1;
    m.codigo_conta = c->conteudo.codigo_conta;
    strcpy(m.dt_movimento, data);
    strcpy(m.tp_movimentacao, tipo == 1 ? "credito" : "debito");
    strncpy(m.ds_favorecido, favorecido, 49);
    m.vl_movimento = v / 100.0;
    m.vl_saldo = novo / 100.0;

    if (!insereMovFinal(M, m)) return 5;

    c->conteudo.vl_saldo = m.vl_saldo;   //so atualiza a conta se a movimentacao entrou
    return 0;
}

//Texto de cada codigo de erro do registraMov
const char *msgMov(int cod){
    switch (cod){
        case 1: return "Conta inativa nao pode receber movimentacoes..";
        case 2: return "Valor invalido: deve ser maior que zero..";
        case 3: return "A data nao pode ser anterior a ultima movimentacao da conta..";
        case 4: return "Saldo insuficiente: o debito ultrapassa o limite da conta..";
        case 5: return "Erro ao alocar memoria..";
        case 6: return "Data invalida..";
    }
    return "Operacao realizada..";
}


int main(){
    ListaContas L;
    ListaMov M;
    reg_conta c;
    ContaApontador p;
    int r;

    iniciaContas(&L);
    iniciaMov(&M);
    memset(&c, 0, sizeof(c));
    c.codigo_conta = 101;
    strcpy(c.status, "ativo");
    c.vl_saldo = 500;
    c.vl_limite = 200;
    incluiConta(&L, c, 3, 0);
    p = pesquisaConta(&L, 101);

    r = registraMov(p, &M, 1, "10/10/2024", "Deposito", 200);
    printf("credito 200   -> codigo %d, saldo %.2f\n", r, p->conteudo.vl_saldo);
    r = registraMov(p, &M, 2, "12/10/2024", "Mercado", 100);
    printf("debito 100    -> codigo %d, saldo %.2f\n", r, p->conteudo.vl_saldo);
    r = registraMov(p, &M, 2, "15/10/2024", "Posto", 150);
    printf("debito 150    -> codigo %d, saldo %.2f\n", r, p->conteudo.vl_saldo);
    r = registraMov(p, &M, 2, "16/10/2024", "X", 650.01);
    printf("debito 650.01 -> codigo %d, saldo %.2f\n", r, p->conteudo.vl_saldo);
    r = registraMov(p, &M, 2, "16/10/2024", "X", 650);
    printf("debito 650    -> codigo %d, saldo %.2f\n", r, p->conteudo.vl_saldo);
    r = registraMov(p, &M, 1, "01/01/2024", "X", 10);
    printf("data antiga   -> codigo %d\n", r);

    liberaContas(&L);
    liberaMov(&M);
    return 0;
}