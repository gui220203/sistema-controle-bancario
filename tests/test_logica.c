/*
Autor.....: Guilherme Viana da Silva
Turma.....: 2C ESW
Objetivo..: Testes automaticos das listas e das regras do Sistema de Controle Bancario.
            Este arquivo inclui o main.c (sem o main dele) e testa as funcoes direto.

Como rodar (na pasta do projeto):
    gcc tests/test_logica.c -o test_logica
    ./test_logica

Os testes usam arquivos temporarios proprios, NAO mexem em contas.dat / movimentacoes.dat.
Os testes 7 (menus) e 8 (relatorios) do roteiro sao feitos na mao, pelo programa.
*/
#define TESTE
#include "../src/main.c"

int total = 0;
int falhas = 0;

//Confere uma condicao e escreve [ok] ou [FALHOU]
void confere(int condicao, const char *descricao){
    total++;
    if (condicao){
        printf("  [ok]     %s\n", descricao);
    }else{
        falhas++;
        printf("  [FALHOU] %s\n", descricao);
    }
}

//Compara dois valores reais (diferenca menor que 1 centavo)
int igual(double a, double b){
    return (a - b) < 0.005 && (b - a) < 0.005;
}

reg_conta novaConta(int cod, double saldo, double limite, const char *status){
    reg_conta c;

    memset(&c, 0, sizeof(c));
    c.codigo_conta = cod;
    strcpy(c.banco, "Banco Central");
    strcpy(c.agencia, "1234");
    sprintf(c.numero_conta, "456789-%d", cod % 10);
    strcpy(c.tipo_conta, "corrente");
    strcpy(c.status, status);
    c.vl_saldo = saldo;
    c.vl_limite = limite;
    return c;
}

int inclui(ListaContas *L, int cod, int modo, int pos){
    return incluiConta(L, novaConta(cod, 0, 100, "ativo"), modo, pos);
}

//Percorre a lista de contas nos dois sentidos conferindo anterior/proximo
int integridadeContas(ListaContas *L){
    ContaApontador p;
    ContaApontador ultimo = NULL;
    int n = 0, volta = 0, total = tamanhoContas(L);

    if (L->Primeiro == NULL) return L->Ultimo == NULL;
    if (L->Ultimo == NULL) return 0;
    if (L->Primeiro->anterior != NULL || L->Ultimo->proximo != NULL) return 0;

    for (p = L->Primeiro; p != NULL; p = p->proximo){
        if (p->proximo != NULL && p->proximo->anterior != p) return 0;
        ultimo = p;
        if (++n > total) return 0;
    }
    if (ultimo != L->Ultimo) return 0;

    for (p = L->Ultimo; p != NULL; p = p->anterior){
        if (++volta > total) return 0;
    }
    return volta == total;
}

//Confere a ordem dos codigos nos dois sentidos
int ordemContas(ListaContas *L, int *esperado, int n){
    ContaApontador p = L->Primeiro;
    int i;

    if (tamanhoContas(L) != n) return 0;
    for (i = 0; i < n; i++){
        if (p == NULL || p->conteudo.codigo_conta != esperado[i]) return 0;
        p = p->proximo;
    }
    p = L->Ultimo;
    for (i = n - 1; i >= 0; i--){
        if (p == NULL || p->conteudo.codigo_conta != esperado[i]) return 0;
        p = p->anterior;
    }
    return 1;
}

int integridadeMov(ListaMov *M){
    MovApontador p;
    MovApontador ultimo = NULL;
    int n = 0;

    if (M->Primeiro == NULL) return M->Ultimo == NULL;
    for (p = M->Primeiro; p != NULL; p = p->proximo){
        ultimo = p;
        if (++n > 100000) return 0;   //ciclo
    }
    return ultimo == M->Ultimo && ultimo->proximo == NULL;
}

//---------------------------------------------------------------

void teste1_cadastro(){
    ListaContas L;
    int esp[] = {5, 10, 15, 20};
    int antes;
    reg_conta ruim;

    printf("\nTeste 1 - Cadastro e consulta de contas\n");
    iniciaContas(&L);

    confere(pesquisaConta(&L, 1) == NULL, "consulta em lista vazia retorna NULL");
    confere(inclui(&L, 10, 3, 0) == 0, "inclusao no final com lista vazia");
    confere(integridadeContas(&L) && tamanhoContas(&L) == 1, "lista com uma unica conta integra");
    confere(inclui(&L, 20, 3, 0) == 0, "inclusao no final");
    confere(inclui(&L, 5, 1, 0) == 0, "inclusao no inicio");
    confere(inclui(&L, 15, 2, 3) == 0, "inclusao em posicao intermediaria (3)");
    confere(ordemContas(&L, esp, 4), "ordem 5,10,15,20 nos dois sentidos");
    confere(inclui(&L, 30, 2, 5) == 0, "inclusao na posicao qtd+1 (final)");
    confere(inclui(&L, 1, 2, 1) == 0, "inclusao na posicao 1 (inicio)");
    confere(integridadeContas(&L), "integridade depois das inclusoes");

    antes = tamanhoContas(&L);
    confere(inclui(&L, 40, 2, 0) == 2, "posicao 0 rejeitada");
    confere(inclui(&L, 40, 2, -3) == 2, "posicao negativa rejeitada");
    confere(inclui(&L, 40, 2, 99) == 2, "posicao alem do final rejeitada");
    confere(inclui(&L, 10, 3, 0) == 1, "codigo duplicado rejeitado");
    confere(inclui(&L, 0, 3, 0) == 1, "codigo zero rejeitado");
    confere(inclui(&L, -7, 3, 0) == 1, "codigo negativo rejeitado");
    confere(tamanhoContas(&L) == antes && integridadeContas(&L), "entradas invalidas nao alteram a lista");

    ruim = novaConta(50, 0, 100, "ativo");
    confere(pesquisaConta(&L, 15) != NULL && strcmp(pesquisaConta(&L, 15)->conteudo.agencia, "1234") == 0,
            "consulta confere os campos");
    liberaContas(&L);
    confere(L.Primeiro == NULL && L.Ultimo == NULL, "liberaContas deixa a lista vazia");
    (void)ruim;
}

void teste2_alteracao_exclusao(){
    ListaContas L;
    ListaMov M;
    ContaApontador p;
    int esp[] = {2, 4};

    printf("\nTeste 2 - Alteracao e exclusao\n");
    iniciaContas(&L);
    iniciaMov(&M);

    inclui(&L, 1, 3, 0); inclui(&L, 2, 3, 0); inclui(&L, 3, 3, 0);
    inclui(&L, 4, 3, 0); inclui(&L, 5, 3, 0);

    p = pesquisaConta(&L, 3);
    strcpy(p->conteudo.banco, "Banco Delta");
    p->conteudo.vl_limite = 777.0;
    confere(strcmp(pesquisaConta(&L, 3)->conteudo.banco, "Banco Delta") == 0 &&
            igual(pesquisaConta(&L, 3)->conteudo.vl_limite, 777.0), "alteracao aparece na consulta");
    confere(pesquisaConta(&L, 99) == NULL, "codigo inexistente nao e encontrado");

    removeConta(&L, pesquisaConta(&L, 1));
    removeConta(&L, pesquisaConta(&L, 3));
    removeConta(&L, pesquisaConta(&L, 5));
    confere(ordemContas(&L, esp, 2) && integridadeContas(&L), "remover inicio, meio e final: sobra 2,4 integra");
    removeConta(&L, pesquisaConta(&L, 2));
    removeConta(&L, pesquisaConta(&L, 4));
    confere(L.Primeiro == NULL && L.Ultimo == NULL && integridadeContas(&L), "remover ate esvaziar (inclui a unica conta)");

    //conta com historico nao pode ser removida (a tela confere contaMov antes de remover)
    inclui(&L, 7, 3, 0);
    registraMov(pesquisaConta(&L, 7), &M, 1, "01/01/2024", "Salario", 50.0);
    confere(contaMov(&M, 7) == 1, "conta 7 tem 1 movimentacao -> a tela bloqueia a remocao");
    confere(contaMov(&M, 8) == 0, "conta sem movimentacao pode ser removida");
    liberaContas(&L);
    liberaMov(&M);
}

void teste3_integridade(){
    ListaContas L;
    ListaMov M;
    int i;

    printf("\nTeste 3 - Integridade das listas encadeadas\n");
    iniciaContas(&L);
    iniciaMov(&M);

    for (i = 1; i <= 20; i++) inclui(&L, i, (i % 3 == 0) ? 1 : 3, 0);
    for (i = 2; i <= 20; i += 4) removeConta(&L, pesquisaConta(&L, i));
    inclui(&L, 100, 2, 4);
    confere(integridadeContas(&L), "contas: ida e volta, sem no perdido, sem ciclo, extremidades NULL");

    registraMov(pesquisaConta(&L, 3), &M, 1, "01/02/2024", "A", 10);
    registraMov(pesquisaConta(&L, 7), &M, 1, "02/02/2024", "B", 20);
    registraMov(pesquisaConta(&L, 3), &M, 2, "03/02/2024", "C", 5);
    confere(integridadeMov(&M), "movimentacoes: percurso por proximo, ultimo aponta para NULL");
    confere(M.Primeiro->conteudo.sequencial == 1 && M.Ultimo->conteudo.sequencial == 2,
            "sequencial e contado por conta");
    liberaContas(&L);
    liberaMov(&M);
}

void teste4_exemplo101(){
    ListaContas L;
    ListaMov M;
    ContaApontador c;
    MovApontador m;

    printf("\nTeste 4 - Creditos, debitos e historico (exemplo da conta 101)\n");
    iniciaContas(&L);
    iniciaMov(&M);
    incluiConta(&L, novaConta(101, 500.0, 200.0, "ativo"), 3, 0);
    c = pesquisaConta(&L, 101);

    confere(registraMov(c, &M, 1, "10/10/2024", "Deposito", 200.0) == 0 && igual(c->conteudo.vl_saldo, 700.0),
            "credito 200 -> saldo 700");
    confere(registraMov(c, &M, 2, "12/10/2024", "Mercado", 100.0) == 0 && igual(c->conteudo.vl_saldo, 600.0),
            "debito 100 -> saldo 600");
    confere(registraMov(c, &M, 2, "15/10/2024", "Posto Catanduva", 150.0) == 0 && igual(c->conteudo.vl_saldo, 450.0),
            "debito 150 -> saldo 450");

    m = M.Primeiro;
    confere(m != NULL && m->conteudo.sequencial == 1 && m->conteudo.codigo_conta == 101 &&
            strcmp(m->conteudo.dt_movimento, "10/10/2024") == 0 &&
            strcmp(m->conteudo.tp_movimentacao, "credito") == 0 &&
            igual(m->conteudo.vl_movimento, 200.0) && igual(m->conteudo.vl_saldo, 700.0), "registro 1 correto");
    m = m->proximo;
    confere(m != NULL && m->conteudo.sequencial == 2 && strcmp(m->conteudo.tp_movimentacao, "debito") == 0 &&
            igual(m->conteudo.vl_saldo, 600.0), "registro 2 correto");
    m = m->proximo;
    confere(m != NULL && m->conteudo.sequencial == 3 && igual(m->conteudo.vl_saldo, 450.0) &&
            m->proximo == NULL, "registro 3 correto e proximo == NULL");
    liberaContas(&L);
    liberaMov(&M);
}

//Um caso do teste 5: conta nova (saldo 500, limite 200) e um unico debito
void casoLimite(double debito, int esperado, double saldoFinal, const char *desc){
    ListaContas L;
    ListaMov M;
    ContaApontador c;
    int r;

    iniciaContas(&L);
    iniciaMov(&M);
    incluiConta(&L, novaConta(1, 500.0, 200.0, "ativo"), 3, 0);
    c = pesquisaConta(&L, 1);

    r = registraMov(c, &M, 2, "01/01/2024", "X", debito);
    confere(r == esperado && igual(c->conteudo.vl_saldo, saldoFinal), desc);
    confere(esperado == 0 ? contaMov(&M, 1) == 1 : contaMov(&M, 1) == 0, "  historico coerente com o resultado");
    liberaContas(&L);
    liberaMov(&M);
}

void teste5_limite(){
    ListaContas L;
    ListaMov M;
    ContaApontador c;

    printf("\nTeste 5 - Limite de debito (saldo 500, limite 200, cada caso independente)\n");
    casoLimite(600.00, 0, -100.0, "debito 600,00 permitido (saldo -100)");
    casoLimite(700.00, 0, -200.0, "debito 700,00 permitido (saldo -200, exatamente o limite)");
    casoLimite(700.01, 4, 500.0, "debito 700,01 bloqueado, saldo inalterado");
    casoLimite(800.00, 4, 500.0, "debito 800,00 bloqueado (exemplo do enunciado)");

    iniciaContas(&L);
    iniciaMov(&M);
    incluiConta(&L, novaConta(1, 300.0, 0.0, "ativo"), 3, 0);
    c = pesquisaConta(&L, 1);
    confere(registraMov(c, &M, 2, "01/01/2024", "X", 300.01) == 4 && igual(c->conteudo.vl_saldo, 300.0),
            "limite zero: debito acima do saldo rejeitado");
    confere(registraMov(c, &M, 2, "01/01/2024", "X", 300.00) == 0 && igual(c->conteudo.vl_saldo, 0.0),
            "limite zero: debito igual ao saldo permitido");
    confere(registraMov(c, &M, 2, "01/01/2024", "X", 0.01) == 4, "limite zero: saldo zerado nao permite novo debito");
    liberaContas(&L);
    liberaMov(&M);
}

void teste6_datas(){
    ListaContas L;
    ListaMov M;
    ContaApontador c;

    printf("\nTeste 6 - Datas das movimentacoes\n");
    iniciaContas(&L);
    iniciaMov(&M);
    incluiConta(&L, novaConta(1, 1000.0, 0.0, "ativo"), 3, 0);
    c = pesquisaConta(&L, 1);

    confere(registraMov(c, &M, 1, "15/10/2024", "A", 10) == 0, "primeira movimentacao em 15/10/2024");
    confere(registraMov(c, &M, 1, "14/10/2024", "A", 10) == 3, "data anterior rejeitada");
    confere(registraMov(c, &M, 1, "15/09/2025", "A", 10) == 0, "data posterior aceita");
    confere(registraMov(c, &M, 1, "15/09/2025", "A", 10) == 0, "data igual a ultima aceita");
    confere(registraMov(c, &M, 1, "14/09/2025", "A", 10) == 3, "voltar depois de avancar e rejeitado");
    confere(registraMov(c, &M, 1, "31/02/2024", "A", 10) == 6, "31/02/2024 nao existe");
    confere(registraMov(c, &M, 1, "29/02/2023", "A", 10) == 6, "29/02/2023 (ano nao bissexto) nao existe");
    confere(dataChave("29/02/2024") == 20240229L, "29/02/2024 (bissexto) e valida");
    confere(dataChave("31/04/2024") == -1 && dataChave("00/01/2024") == -1 && dataChave("10/13/2024") == -1,
            "dia ou mes fora do intervalo rejeitados");
    confere(dataChave("1/1/2024") == -1 && dataChave("ab/cd/efgh") == -1 && dataChave("") == -1 &&
            dataChave("10-10-2024") == -1, "formato invalido rejeitado");
    confere(contaMov(&M, 1) == 3 && integridadeMov(&M), "rejeicoes nao criaram movimentacoes");
    liberaContas(&L);
    liberaMov(&M);
}

void teste7_validacoes(){
    ListaContas L;
    ListaMov M;
    ContaApontador a;
    ContaApontador i;

    printf("\nTeste 7 (parte logica) - Validacoes\n");
    iniciaContas(&L);
    iniciaMov(&M);
    incluiConta(&L, novaConta(1, 100.0, 0.0, "ativo"), 3, 0);
    incluiConta(&L, novaConta(2, 100.0, 0.0, "inativo"), 3, 0);
    a = pesquisaConta(&L, 1);
    i = pesquisaConta(&L, 2);

    confere(registraMov(a, &M, 1, "01/01/2024", "A", 0.0) == 2, "valor zero rejeitado");
    confere(registraMov(a, &M, 2, "01/01/2024", "A", -5.0) == 2, "valor negativo rejeitado");
    confere(registraMov(a, &M, 1, "01/01/2024", "A", 0.001) == 2, "valor menor que 1 centavo rejeitado");
    confere(registraMov(i, &M, 1, "01/01/2024", "A", 5.0) == 1, "conta inativa nao movimenta");
    confere(contaMov(&M, 1) == 0 && igual(a->conteudo.vl_saldo, 100.0), "nenhuma alteracao indevida");
    confere(registraMov(a, &M, 1, "01/01/2024", "A", 0.10) == 0 &&
            registraMov(a, &M, 1, "01/01/2024", "A", 0.20) == 0 && igual(a->conteudo.vl_saldo, 100.30),
            "centavos somam sem erro de ponto flutuante");
    liberaContas(&L);
    liberaMov(&M);
}

void teste9_arquivos(){
    const char *fc = "teste_contas.tmp";
    const char *fm = "teste_mov.tmp";
    ListaContas A, B;
    ListaMov MA, MB;
    ContaApontador x, y;
    MovApontador p, q;
    int ok;
    FILE *f;

    printf("\nTeste 9 - Gravacao e leitura de arquivos\n");
    iniciaContas(&A); iniciaContas(&B);
    iniciaMov(&MA);   iniciaMov(&MB);

    confere(carregaContas(&B, "nao_existe_c.dat") == 1 && B.Primeiro == NULL, "arquivo ausente: comeca vazio sem erro");

    inclui(&A, 101, 3, 0); inclui(&A, 102, 3, 0); inclui(&A, 103, 1, 0);
    registraMov(pesquisaConta(&A, 101), &MA, 1, "10/10/2024", "Deposito", 200);
    registraMov(pesquisaConta(&A, 101), &MA, 2, "12/10/2024", "Posto Catanduva", 100);
    registraMov(pesquisaConta(&A, 102), &MA, 2, "01/11/2024", "Loja", 50);

    confere(salvaContas(&A, fc) == 1 && salvaMov(&MA, fm) == 1, "gravacao concluida");
    confere(carregaContas(&B, fc) == 1 && carregaMov(&MB, &B, fm) == 1, "leitura concluida");

    ok = (tamanhoContas(&A) == tamanhoContas(&B));
    x = A.Primeiro;
    y = B.Primeiro;
    while (ok && x != NULL && y != NULL){
        ok = x != y &&   //sao nos novos, nao o mesmo endereco
             x->conteudo.codigo_conta == y->conteudo.codigo_conta &&
             strcmp(x->conteudo.banco, y->conteudo.banco) == 0 &&
             strcmp(x->conteudo.status, y->conteudo.status) == 0 &&
             igual(x->conteudo.vl_saldo, y->conteudo.vl_saldo) &&
             igual(x->conteudo.vl_limite, y->conteudo.vl_limite);
        x = x->proximo;
        y = y->proximo;
    }
    confere(ok && x == NULL && y == NULL, "contas recuperadas (campos, saldos, limites, status) em nos novos");

    p = MA.Primeiro;
    q = MB.Primeiro;
    ok = 1;
    while (ok && p != NULL && q != NULL){
        ok = p != q && p->conteudo.sequencial == q->conteudo.sequencial &&
             p->conteudo.codigo_conta == q->conteudo.codigo_conta &&
             strcmp(p->conteudo.dt_movimento, q->conteudo.dt_movimento) == 0 &&
             strcmp(p->conteudo.ds_favorecido, q->conteudo.ds_favorecido) == 0 &&
             igual(p->conteudo.vl_saldo, q->conteudo.vl_saldo);
        p = p->proximo;
        q = q->proximo;
    }
    confere(ok && p == NULL && q == NULL, "historico de movimentacoes recuperado");
    confere(integridadeContas(&B) && integridadeMov(&MB), "ponteiros reconstruidos validos");

    //arquivo corrompido: tamanho que nao e multiplo do registro
    liberaContas(&B);
    f = fopen(fc, "wb");
    fwrite("lixo", 1, 5, f);
    fclose(f);
    confere(carregaContas(&B, fc) == 0 && B.Primeiro == NULL, "arquivo corrompido: erro tratado e memoria limpa");

    //arquivo vazio
    f = fopen(fc, "wb"); fclose(f);
    confere(carregaContas(&B, fc) == 1 && B.Primeiro == NULL, "arquivo vazio: lista vazia sem erro");

    //falha de gravacao: pasta que nao existe
    confere(salvaContas(&A, "pasta_inexistente/c.dat") == 0, "falha de gravacao devolve erro (nao finge sucesso)");

    //movimentacao de uma conta que nao existe
    liberaMov(&MB);
    confere(carregaMov(&MB, &B, fm) == 0 && MB.Primeiro == NULL, "movimentacao de conta inexistente e rejeitada");

    remove(fc);
    remove(fm);
    liberaContas(&A); liberaContas(&B);
    liberaMov(&MA);   liberaMov(&MB);
}

void teste10_memoria(){
    ListaContas L;
    ListaMov M;
    int ciclo, i, ok = 1;

    printf("\nTeste 10 - Ciclos de cadastro/movimentacao/remocao/encerramento\n");
    iniciaContas(&L);
    iniciaMov(&M);

    for (ciclo = 0; ciclo < 200; ciclo++){
        for (i = 1; i <= 25; i++) inclui(&L, i, 3, 0);
        for (i = 1; i <= 25; i += 2) registraMov(pesquisaConta(&L, i), &M, 1, "01/01/2024", "x", 1.0);
        for (i = 1; i <= 25; i++){
            if (contaMov(&M, i) == 0) removeConta(&L, pesquisaConta(&L, i));
        }
        if (tamanhoContas(&L) != 13 || !integridadeContas(&L) || !integridadeMov(&M)) ok = 0;
        liberaContas(&L);
        liberaMov(&M);
        if (L.Primeiro != NULL || M.Primeiro != NULL) ok = 0;
    }
    confere(ok, "200 ciclos mantem as listas integras e vazias ao liberar");
    printf("  (para achar vazamentos use: gcc -fsanitize=address tests/test_logica.c -o t && ./t)\n");
}

int main(){
    teste1_cadastro();
    teste2_alteracao_exclusao();
    teste3_integridade();
    teste4_exemplo101();
    teste5_limite();
    teste6_datas();
    teste7_validacoes();
    teste9_arquivos();
    teste10_memoria();

    printf("\nResultado: %d verificacoes, %d falha(s).\n", total, falhas);
    return falhas == 0 ? 0 : 1;
}
