#ifndef ARVOREBESTRELA_H
#define ARVOREBESTRELA_H

#include "registro.h"

#define ORDEM_ESTRELA 50
#define MM_ESTRELA (2 * ORDEM_ESTRELA)

typedef enum{ Interna, Externa } TipoPaginaEstrela;

typedef struct PaginaEstrela{
    TipoPaginaEstrela tipo;
    union{
        struct{
            int numChaves;
            int chaves[MM_ESTRELA];
            struct PaginaEstrela *filhos[MM_ESTRELA + 1];
        } interna;
        struct{
            int numRegistros;
            Referencia registros[MM_ESTRELA];
        } externa;
    } conteudo;
} PaginaEstrela;

const Referencia *pesquisaBEstrela(PaginaEstrela *pagina, int chave,
                                  Medidas *medidas);
void insereNaPaginaInterna(PaginaEstrela *pagina, int chave,
                          PaginaEstrela *direita, Medidas *medidas);
void insereNaPaginaExterna(PaginaEstrela *pagina, Referencia reg,
                          Medidas *medidas);
int insBEstrela(Referencia reg, PaginaEstrela *pagina, short *cresceu,
               int *chaveRetorno, PaginaEstrela **paginaRetorno,
               Medidas *medidas);
int insereBEstrela(Referencia reg, PaginaEstrela **raiz, Medidas *medidas);
int lerArquivoArvoreBEstrela(const char *nomeArquivo, int numRegistros,
                            PaginaEstrela **raiz, Medidas *medidas);
void liberarArvoreBEstrela(PaginaEstrela *raiz);
int executarArvoreBEstrela(const char *nomeArquivo, int quantidade, int chave,
                           int modoTeste);

#endif
