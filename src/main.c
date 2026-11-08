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
//---------------------- Funcoes do arquivo ----------------------

//Abre o arquivo e confere o tamanho. Retorna 1=ok (arq pode ser NULL se nao existe)
//0=erro (arquivo corrompido ou nao abre)
int abreLeitura(const char *nome, long tamReg, FILE **arq){
    long tam;

    *arq = fopen(nome, "rb");
    if (*arq == NULL){
        return (errno == ENOENT);   //arquivo ainda nao existe: lista comeca vazia
    }

    fseek(*arq, 0, SEEK_END);
    tam = ftell(*arq);
    rewind(*arq);

    //o tamanho precisa ser multiplo do registro, senao o arquivo esta corrompido
    if (tam < 0 || tam % tamReg != 0){
        fclose(*arq);
        *arq = NULL;
        return 0;
    }
    return 1;
}

//Le o arquivo de contas e monta a lista. Retorna 1=ok 0=erro
int carregaContas(ListaContas *L, const char *nome){
    reg_conta c;
    FILE *arq;

    if (!abreLeitura(nome, sizeof(reg_conta), &arq)) return 0;
    if (arq == NULL) return 1;

    while (fread(&c, sizeof(reg_conta), 1, arq) == 1){
        //so o conteudo vem do arquivo; os ponteiros sao refeitos na insercao
        if (incluiConta(L, c, 3, 0) != 0){
            fclose(arq);
            liberaContas(L);
            return 0;
        }
    }
    fclose(arq);
    return 1;
}

//Le o arquivo de movimentacoes. Retorna 1=ok 0=erro
int carregaMov(ListaMov *M, ListaContas *L, const char *nome){
    reg_mov m;
    FILE *arq;

    if (!abreLeitura(nome, sizeof(reg_mov), &arq)) return 0;
    if (arq == NULL) return 1;

    while (fread(&m, sizeof(reg_mov), 1, arq) == 1){
        //a movimentacao precisa pertencer a uma conta que existe
        if (pesquisaConta(L, m.codigo_conta) == NULL || !insereMovFinal(M, m)){
            fclose(arq);
            liberaMov(M);
            return 0;
        }
    }
    fclose(arq);
    return 1;
}

//Troca o arquivo temporario pelo definitivo (assim, se der erro no meio da
//gravacao, o arquivo antigo continua intacto)
int fechaGravacao(FILE *arq, int falhou, const char *tmp, const char *nome){
    if (fclose(arq) != 0) falhou = 1;

    if (falhou){
        remove(tmp);
        return 0;
    }
    remove(nome);
    return (rename(tmp, nome) == 0);
}

//Grava a lista de contas no arquivo. Retorna 1=ok 0=erro
int salvaContas(ListaContas *L, const char *nome){
    char tmp[300];
    FILE *arq;
    ContaApontador p;
    int falhou = 0;

    sprintf(tmp, "%s.tmp", nome);
    arq = fopen(tmp, "wb");
    if (arq == NULL) return 0;

    //Grava so o conteudo, o ponteiro nao vai para o arquivo
    p = L->Primeiro;
    while (p != NULL && !falhou){
        if (fwrite(&p->conteudo, sizeof(reg_conta), 1, arq) != 1) falhou = 1;
        p = p->proximo;
    }
    return fechaGravacao(arq, falhou, tmp, nome);
}

//Grava a lista de movimentacoes no arquivo. Retorna 1=ok 0=erro
int salvaMov(ListaMov *M, const char *nome){
    char tmp[300];
    FILE *arq;
    MovApontador p;
    int falhou = 0;

    sprintf(tmp, "%s.tmp", nome);
    arq = fopen(tmp, "wb");
    if (arq == NULL) return 0;

    p = M->Primeiro;
    while (p != NULL && !falhou){
        if (fwrite(&p->conteudo, sizeof(reg_mov), 1, arq) != 1) falhou = 1;
        p = p->proximo;
    }
    return fechaGravacao(arq, falhou, tmp, nome);
}
//---------------------- Leitura de campos na tela ----------------------

//Le um texto obrigatorio (nao aceita vazio)
void lerTexto(int y, char *dest, int tam){
    do{
        limpaCampo(y);
        if (fgets(dest, tam, stdin) == NULL){
            strcpy(dest, "-");          //fim da entrada
            return;
        }
        ajusta_string(dest);

        if (dest[0] == '\0'){
            mensagem("Campo obrigatorio..");
        }
    }while (dest[0] == '\0');
}

//Le um texto opcional (se ficar vazio vira "Nao informado")
void lerTextoOpcional(int y, char *dest, int tam){
    limpaCampo(y);
    if (fgets(dest, tam, stdin) == NULL) dest[0] = '\0';
    else ajusta_string(dest);

    if (dest[0] == '\0'){
        strcpy(dest, "Nao informado");
        gotoxy(COL,y);
        printf("%s", dest);
    }
}

//Le um numero real (aceita virgula). Pergunta de novo se for invalido
double lerDouble(int y){
    char buf[40];
    char *fim;
    char *p;
    double v;

    while (1){
        limpaCampo(y);
        if (fgets(buf, 40, stdin) == NULL) return 0;
        ajusta_string(buf);

        for (p = buf; *p; p++){
            if (*p == ',') *p = '.';
        }

        v = strtod(buf, &fim);

        if (buf[0] == '\0' || *fim != '\0' || !(v > -1e12 && v < 1e12)){
            mensagem("Valor invalido. Exemplo: 1500.50");
        }else{
            return v;
        }
    }
}

//Le um numero inteiro (0 se for invalido)
int lerInteiro(int y){
    int n = 0;

    limpaCampo(y);
    scanf("%d", &n);
    limparBuffer();
    return n;
}

//Le uma data valida (dd/mm/aaaa)
void lerData(int y, char *dest){
    char buf[40];

    while (1){
        limpaCampo(y);
        if (fgets(buf, 40, stdin) == NULL){
            strcpy(dest, "01/01/1900");
            return;
        }
        ajusta_string(buf);

        if (dataChave(buf) < 0){
            mensagem("Data invalida. Use dd/mm/aaaa (ex.: 15/10/2024)..");
        }else{
            strcpy(dest, buf);
            return;
        }
    }
}

//Le um campo da conta (campo de 1 a 7)
void lerCampoConta(reg_conta *c, int campo){
    int y = 5 + campo * 2;
    int v;
    double x;

    switch (campo)
    {
        case 1 :
            lerTexto(y, c->banco, 50);
            break;

        case 2 :
            lerTexto(y, c->agencia, 10);
            break;

        case 3 :
            lerTexto(y, c->numero_conta, 20);
            break;

        case 4 :
            do{
                v = lerInteiro(y);
                if (v < 1 || v > 3) mensagem("Digite 1, 2 ou 3..");
            }while (v < 1 || v > 3);
            strcpy(c->tipo_conta, TIPOS[v-1]);
            limpaCampo(y);
            printf("%s", c->tipo_conta);
            break;

        case 5 :
            do{
                x = lerDouble(y);
                if (x < 0){
                    mensagem("O limite nao pode ser negativo..");
                }else if (centavos(c->vl_saldo) + centavos(x) < 0){
                    mensagem("Esse limite deixa o saldo atual abaixo do permitido..");
                    x = -1;
                }
            }while (x < 0);
            c->vl_limite = x;
            break;

        case 6 :
            do{
                x = lerDouble(y);
                if (centavos(x) + centavos(c->vl_limite) < 0){
                    mensagem("O saldo nao pode ficar abaixo do limite negativo..");
                }
            }while (centavos(x) + centavos(c->vl_limite) < 0);
            c->vl_saldo = x;
            break;

        case 7 :
            do{
                v = lerInteiro(y);
                if (v < 1 || v > 2) mensagem("Digite 1 ou 2..");
            }while (v < 1 || v > 2);
            strcpy(c->status, v == 1 ? "ativo" : "inativo");
            limpaCampo(y);
            printf("%s", c->status);
            break;
    }
}

//Mostra os dados da conta na tela
void mostraConta(reg_conta *c){
    gotoxy(COL,5);
    printf("%d", c->codigo_conta);
    gotoxy(COL,7);
    printf("%s", c->banco);
    gotoxy(COL,9);
    printf("%s", c->agencia);
    gotoxy(COL,11);
    printf("%s", c->numero_conta);
    gotoxy(COL,13);
    printf("%s", c->tipo_conta);
    gotoxy(COL,15);
    printf("%.2f", c->vl_limite);
    gotoxy(COL,17);
    printf("%.2f", c->vl_saldo);
    gotoxy(COL,19);
    printf("%s", c->status);
}

//Pede o codigo da conta (0 para sair). Se deveExistir=1 o codigo precisa estar na lista,
//se deveExistir=0 o codigo nao pode estar na lista
int pedeCodigoConta(ListaContas *L, int deveExistir){
    int codigo;
    int achou;
    int invalido;
    int r;

    do{
        codigo = 0;
        invalido = 0;

        limpa_mensagem();
        gotoxy(07,23);
        printf("Digite 0 para sair..");

        gotoxy(COL,05);
        printf("          ");
        gotoxy(COL,05);
        r = scanf("%d", &codigo);
        if (r == EOF) return 0;
        limparBuffer();

        if (r != 1 || codigo < 0){
            mensagem("Codigo invalido. Digite um numero inteiro..");
            invalido = 1;
        }else if (codigo != 0){
            achou = (pesquisaConta(L, codigo) != NULL);

            if (deveExistir && !achou){
                mensagem("Conta nao cadastrada..");
                invalido = 1;
            }
            if (!deveExistir && achou){
                mensagem("Codigo ja cadastrado..");
                invalido = 1;
            }
        }
    }while (invalido);

    limpa_mensagem();
    return codigo;
}

//---------------------- Cadastro de contas ----------------------

//Incluir conta: modo 1=inicio, 2=posicao, 3=final
void cadastrarConta(ListaContas *L, int modo){
    reg_conta c;
    int resp;
    int pos;
    int campo;
    int r;

    do{
        resp = 2;
        pos = 1;
        memset(&c, 0, sizeof(c));

        tela();
        tela_conta();

        if (modo == 1) titulo("Incluir Conta no Inicio da Lista");
        if (modo == 2) titulo("Incluir Conta em uma Posicao da Lista");
        if (modo == 3) titulo("Incluir Conta no Final da Lista");

        c.codigo_conta = pedeCodigoConta(L, 0);

        if (c.codigo_conta != 0){

            //No modo 2 pergunta a posicao
            if (modo == 2){
                pos = perguntaPosicao(tamanhoContas(L) + 1);
            }

            if (pos != 0){
                for (campo = 1; campo <= 7; campo++){
                    lerCampoConta(&c, campo);
                }

                resp = perguntaInt("Deseja gravar os dados (1=Sim; 2=Nao).: ");

                if (resp == 1){
                    r = incluiConta(L, c, modo, pos);

                    if (r == 0)      mensagem("Conta cadastrada com sucesso..");
                    else if (r == 3) mensagem("Erro ao alocar memoria..");
                    else             mensagem("Nao foi possivel incluir: codigo ou posicao invalidos..");
                }

                resp = perguntaInt("Deseja cadastrar outra? (1=Sim;2=Nao).: ");
            }
        }
    }while (c.codigo_conta != 0 && pos != 0 && resp == 1);
}
//Alteracao do cadastro da conta
void alterarConta(ListaContas *L){
    ContaApontador p;
    reg_conta c;
    int codigo;
    int campo;
    int resp;

    do{
        resp = 2;

        tela();
        tela_conta();
        titulo("Alteracao do Cadastro da Conta");

        if (L->Primeiro == NULL){
            mensagem("Lista vazia, nao ha contas para alterar..");
            return;
        }

        codigo = pedeCodigoConta(L, 1);

        if (codigo != 0){
            p = pesquisaConta(L, codigo);

            //Trabalha em uma copia e so grava se o usuario confirmar
            c = p->conteudo;
            mostraConta(&c);

            do{
                campo = perguntaInt("Digite o numero do campo a alterar (0=Fim).: ");

                if (campo == 6){
                    mensagem("O saldo so muda por movimentacao financeira..");
                }else if (campo >= 1 && campo <= 7){
                    lerCampoConta(&c, campo);
                }else if (campo != 0){
                    mensagem("Campo invalido, digite de 1 a 7..");
                }
            }while (campo != 0);

            resp = perguntaInt("Deseja gravar as alteracoes (1=Sim; 2=Nao).: ");

            if (resp == 1){
                p->conteudo = c;
                mensagem("Conta alterada com sucesso..");
            }

            resp = perguntaInt("Deseja alterar outra? (1=Sim;2=Nao).: ");
        }
    }while (codigo != 0 && resp == 1);
}

//Consulta uma conta pelo codigo
void consultarConta(ListaContas *L, ListaMov *M){
    ContaApontador p;
    int codigo;
    int resp;

    do{
        resp = 2;

        tela();
        tela_conta();
        titulo("Consultar Conta");

        if (L->Primeiro == NULL){
            mensagem("Lista vazia, nao ha contas cadastradas..");
            return;
        }

        codigo = pedeCodigoConta(L, 1);

        if (codigo != 0){
            p = pesquisaConta(L, codigo);
            mostraConta(&p->conteudo);

            gotoxy(70,7);
            printf("Movimentacoes....: %d", contaMov(M, codigo));
            gotoxy(70,9);
            printf("Disponivel (R$)..: %.2f", p->conteudo.vl_saldo + p->conteudo.vl_limite);

            mensagem("Conta encontrada..");

            resp = perguntaInt("Deseja consultar outra? (1=Sim;2=Nao).: ");
        }
    }while (codigo != 0 && resp == 1);
}

//Remover conta (so se nao tiver movimentacoes)
void removerConta(ListaContas *L, ListaMov *M){
    ContaApontador p;
    int codigo;
    int resp;

    do{
        resp = 2;

        tela();
        tela_conta();
        titulo("Remover Conta");

        if (L->Primeiro == NULL){
            mensagem("Lista vazia, nao ha o que remover..");
            return;
        }

        codigo = pedeCodigoConta(L, 1);

        if (codigo != 0){
            p = pesquisaConta(L, codigo);
            mostraConta(&p->conteudo);

            if (contaMov(M, codigo) > 0){
                mensagem("Conta possui movimentacoes e nao pode ser removida..");
            }else{
                resp = perguntaInt("Confirma a remocao? (1=Sim;2=Nao).: ");

                if (resp == 1){
                    removeConta(L, p);
                    mensagem("Conta removida com sucesso..");
                }
            }

            resp = perguntaInt("Deseja remover outra? (1=Sim;2=Nao).: ");
        }
    }while (codigo != 0 && resp == 1);
}int main(){
    ListaContas L;
    ListaMov M;
    int op;

    iniciaContas(&L);
    iniciaMov(&M);

    do{
        system("cls");
        printf("1-Incluir 2-Alterar 3-Consultar 4-Remover 0-Sair: ");
        op = lerOpcao();

        if (op == 1) cadastrarConta(&L, 3);
        if (op == 2) alterarConta(&L);
        if (op == 3) consultarConta(&L, &M);
        if (op == 4) removerConta(&L, &M);
    }while (op != 0 && op != -1);

    liberaContas(&L);
    liberaMov(&M);
    return 0;
}