#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivo.h"
#include "acessoSequencial.h"
#include "arvorebinaria.h"
#include "arvoreb.h"
#include "arvorebestrela.h"

static void uso(const char *programa){
    fprintf(stderr, "Uso: %s <metodo> <quantidade> <situacao> <chave> [-P]\n", programa);
    fprintf(stderr, "Experimento: %s <metodo> <quantidade> <situacao> -t [-P]\n", programa);
    fprintf(stderr, "Metodos: 1 sequencial indexado, 2 binaria externa, 3 B, 4 B*.\n");
    fprintf(stderr, "Situacoes: 1 crescente, 2 decrescente, 3 aleatoria.\n");
}

int main(int argc, char *argv[]){
    int metodo, quantidade, situacao, chave = 0, modoTeste, sucesso = 0;
    char nome[80];
    if((argc != 5 && argc != 6) ||
        !converterInteiro(argv[1], &metodo) || !converterInteiro(argv[2], &quantidade) ||
        !converterInteiro(argv[3], &situacao)){
        uso(argv[0]);
        return 1;
    }
    modoTeste = strcmp(argv[4], "-t") == 0 || strcmp(argv[4], "-T") == 0;
    if((!modoTeste && !converterInteiro(argv[4], &chave)) ||
        (argc == 6 && strcmp(argv[5], "-P") != 0)){
        uso(argv[0]);
        return 1;
    }
    if(metodo < 1 || metodo > 4 || quantidade < 1 || quantidade > MAX_REGISTROS ||
        situacao < 1 || situacao > 3){
        fprintf(stderr, "Metodo deve ser 1..4, quantidade 1..%d e situacao 1..3.\n", MAX_REGISTROS);
        return 1;
    }
    if(metodo == 1 && situacao == 3){
        fprintf(stderr, "Acesso sequencial indexado nao se aplica a arquivo desordenado.\n");
        return 1;
    }
    if(modoTeste && quantidade < CHAVES_EXPERIMENTO){
        fprintf(stderr, "O experimento exige pelo menos 20 registros com chaves distintas.\n");
        return 1;
    }
    snprintf(nome, sizeof(nome), "dados_%d_%d.bin", quantidade, situacao);
    srand(1);
    if(!garantirArquivo(nome, quantidade, situacao)) return 1;
    printf("Metodo: %d | Registros: %d | Situacao: %d | Arquivo: %s\n",
           metodo, quantidade, situacao, nome);
    if(argc == 6 && !mostrarChaves(nome, quantidade)) return 1;
    switch(metodo){
        case 1: sucesso = executarSequencial(nome, quantidade, situacao, chave, modoTeste); break;
        case 2: sucesso = executarArvoreBinaria(nome, quantidade, chave, modoTeste); break;
        case 3: sucesso = executarArvoreB(nome, quantidade, chave, modoTeste); break;
        case 4: sucesso = executarArvoreBEstrela(nome, quantidade, chave, modoTeste); break;
    }
    return sucesso ? 0 : 1;
}
