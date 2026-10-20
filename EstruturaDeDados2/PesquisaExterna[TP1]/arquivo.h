#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include "registro.h"

int converterInteiro(const char *texto, int *valor);
int sortearPosicao(int limite);
int posicionarArquivo(FILE *arquivo, int64_t deslocamento, int origem);
int64_t tamanhoArquivo(FILE *arquivo);
int lerRegistro(FILE *arquivo, int64_t posicao, Registro *registro, Medidas *medidas);
int criarArquivo(const char *nome, int quantidade, int situacao);
int garantirArquivo(const char *nome, int quantidade, int situacao);
int mostrarChaves(const char *nome, int quantidade);
int selecionarChaves(const char *nome, int quantidade, int chaves[CHAVES_EXPERIMENTO]);

#endif
