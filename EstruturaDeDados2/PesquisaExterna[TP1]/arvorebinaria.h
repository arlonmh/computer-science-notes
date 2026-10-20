/* Declarações usadas pela implementação da árvore binária externa. */
#ifndef ARVOREBINARIA_H
#define ARVOREBINARIA_H

#include "registro.h"

/* Nó gravado no arquivo de índice. Os filhos são identificados pela posição
   que ocupam no arquivo; -1 indica que não existe filho naquele lado. */
typedef struct{
    Referencia registro;
    int64_t esquerda;
    int64_t direita;
} NoArquivo;

/* Cria o arquivo de índice a partir dos registros do arquivo de dados. */
int lerArquivoBinario(const char *nomeArquivoDados, const char *nomeArquivoArvore,
                     int numRegistros, Medidas *medidas);

/* Procura uma chave em um índice já construído. */
int buscarEmArquivo(const char *nomeArquivoDados, const char *nomeArquivoArvore,
                    int chave, Registro *resultado, Medidas *medidas);
/* Executa a construção e os testes de pesquisa da árvore binária externa. */
int executarArvoreBinaria(const char *nomeArquivo, int quantidade, int chave,
                         int modoTeste);

#endif
