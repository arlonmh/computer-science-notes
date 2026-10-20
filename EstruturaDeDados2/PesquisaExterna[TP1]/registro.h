#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdint.h>

typedef struct{
    int chave;
    long dado1;
    char dado2[5000];
} Registro;

typedef struct{
    int chave;
    int64_t posicao;
} Referencia;

typedef struct{
    unsigned long long leituras;
    unsigned long long comparacoes;
    double tempo;
} Medidas;

#define MAX_REGISTROS 2000000
#define CHAVES_EXPERIMENTO 20

#endif
