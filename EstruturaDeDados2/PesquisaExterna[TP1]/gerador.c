#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivo.h"

int main(int argc, char *argv[]){
    int quantidade, situacao;
    char nome[80];
    if((argc != 3 && argc != 4) || !converterInteiro(argv[1], &quantidade) ||
        !converterInteiro(argv[2], &situacao) || (argc == 4 && strcmp(argv[3], "-P") != 0)){
        fprintf(stderr, "Uso: %s <quantidade> <situacao> [-P]\n", argv[0]);
        return 1;
    }
    if(quantidade < 1 || quantidade > MAX_REGISTROS || situacao < 1 || situacao > 3){
        fprintf(stderr, "Quantidade deve ser 1..%d e situacao 1..3.\n", MAX_REGISTROS);
        return 1;
    }
    snprintf(nome, sizeof(nome), "dados_%d_%d.bin", quantidade, situacao);
    srand(1);
    if(!criarArquivo(nome, quantidade, situacao)) return 1;
    printf("Arquivo criado: %s | Registros: %d | Bytes por registro: %zu\n",
           nome, quantidade, sizeof(Registro));
    if(argc == 4 && !mostrarChaves(nome, quantidade)) return 1;
    return 0;
}
