#ifndef ACESSOSEQUENCIAL_H
#define ACESSOSEQUENCIAL_H

#include "registro.h"
#include "tempo.h"

#define ITENSPAGINA 4

typedef struct{
    int posicao;
    int chave;
} TipoIndice;

typedef Registro TipoItem;

int criarIndicePaginas(const char *nome, int quantidade, int situacao,
                      TipoIndice **tabela, int *numPaginas, Medidas *medidas);
int pesquisarSequencial(const char *nome, int quantidade, int situacao, int chave,
                        const TipoIndice *tabela, int numPaginas,
                        Registro *resultado, Medidas *medidas);
int executarSequencial(const char *nome, int quantidade, int situacao,
                      int chave, int modoTeste);

#endif
