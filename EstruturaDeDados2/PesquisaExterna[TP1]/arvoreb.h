#ifndef ARVOREB_H
#define ARVOREB_H

#include "registro.h"

#define ORDEM 50 // Define 'd' (a Ordem da Árvore B). Cada página (exceto a raiz) deve ter no mínimo 50 chaves
#define MM (2 * ORDEM) // Define a capacidade máxima de chaves em uma página: MM = 2 * ORDEM = 100 chaves

// Estrutura que representa um Nó (Página) da Árvore B em memória RAM
typedef struct pagina{
    int n;                          // Quantidade atual de chaves/referências armazenadas dentro desta página
    Referencia registro[MM];        // Vetor de referências (chaves + posição no disco)
    struct pagina *filhos[MM + 1];  // Vetor de ponteiros para as páginas filhas. Sempre possui espaço para até MM + 1 (101) filhos
} Pagina;

// Procura por uma chave dentro da árvore B
const Referencia *pesquisa(Pagina *pagina, int chave, Medidas *medidas);

// Insere uma chave mantendo o vetor da página ordenado
void InsereNaPagina(Pagina *pagina, Referencia reg, Pagina *direita,
                    Medidas *medidas);

// Função auxiliar recursiva para navegação, inserção e tratamento de cisão (split)
int Ins(Referencia reg, Pagina *pagina, short *cresceu,
        Referencia *regRetorno, Pagina **paginaRetorno, Medidas *medidas);

// Função principal de inserção, cria nova raiz se a árvore crescer para cima
int Insere(Referencia reg, Pagina **raiz, Medidas *medidas);

// Lê os registros do arquivo binário no disco e constrói o índice da Árvore B em memória RAM
int lerArquivoArvoreB(const char *nomeArquivo, int numRegistros,
                     Pagina **raiz, Medidas *medidas);

// Libera toda a memória RAM alocada para os nós da árvore
void liberarArvoreB(Pagina *raiz);

// Função controladora da interface chamada pela main para rodar a Fase 1 ou Fase 2
int executarArvoreB(const char *nomeArquivo, int quantidade, int chave,
                    int modoTeste);

#endif
