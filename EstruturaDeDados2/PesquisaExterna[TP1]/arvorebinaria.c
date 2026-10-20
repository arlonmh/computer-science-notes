#include "arvorebinaria.h"
#include "arquivo.h"
#include "suporte.h"
#include "tempo.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mantém os dois arquivos abertos durante as pesquisas. A árvore fica no arquivo
   de índice, enquanto os registros completos continuam no arquivo de dados. */
typedef struct{
    FILE *dados;
    FILE *arvore;
    int64_t numeroRegistros;
    int64_t numeroNos;
} ContextoAB;

/* Lê um nó pela sua posição no arquivo de índice. */
static int lerNo(FILE *arquivo, int64_t posicao, int64_t numeroNos,
                 NoArquivo *no, Medidas *medidas){
    if(posicao < 0 || posicao >= numeroNos ||
        posicao > INT64_MAX / (int64_t)sizeof(*no) ||
        !posicionarArquivo(arquivo, posicao * (int64_t)sizeof(*no), SEEK_SET) ||
        fread(no, sizeof(*no), 1, arquivo) != 1){
        return 0;
    }
    medidas->leituras++;
    return 1;
}

/* Grava um nó na posição indicada, usada também pelos ponteiros dos filhos. */
static int escreverNo(FILE *arquivo, int64_t posicao, const NoArquivo *no){
    return posicao >= 0 && posicao <= INT64_MAX / (int64_t)sizeof(*no) &&
           posicionarArquivo(arquivo, posicao * (int64_t)sizeof(*no), SEEK_SET) &&
           fwrite(no, sizeof(*no), 1, arquivo) == 1;
}

/* Confere se as referências do nó apontam para posições existentes. */
static int noValido(const NoArquivo *no, int64_t posicao,
                    int64_t numeroNos, int64_t numeroRegistros){
    return no->registro.posicao >= 0 && no->registro.posicao < numeroRegistros &&
           no->esquerda >= -1 && no->esquerda < numeroNos &&
           no->direita >= -1 && no->direita < numeroNos &&
           no->esquerda != posicao && no->direita != posicao;
}

/* Insere uma chave na árvore armazenada em disco. Os nós são acrescentados
   ao fim do arquivo, e os campos esquerda/direita guardam suas posições. */
static int inserirEmArquivo(FILE *arvore, Referencia referencia,
                            int64_t *numeroNos, Medidas *medidas){
    NoArquivo novo;
    int64_t posicao = 0;
    int64_t passos = 0;

    memset(&novo, 0, sizeof(novo));
    novo.registro = referencia;
    novo.esquerda = -1;
    novo.direita = -1;
    /* O primeiro nó ocupa a posição zero e passa a ser a raiz. */
    if(*numeroNos == 0){
        if(!escreverNo(arvore, 0, &novo)) return 0;
        *numeroNos = 1;
        return 1;
    }

    /* Percorre a árvore até encontrar um ponteiro vazio. O limite de passos
       evita ficar preso caso o índice contenha referências inconsistentes. */
    while(posicao != -1 && passos < *numeroNos){
        NoArquivo atual;
        int direita;
        int64_t filho;
        passos++;
        if(!lerNo(arvore, posicao, *numeroNos, &atual, medidas)) return 0;

        medidas->comparacoes++;
        /* Decide em qual lado continuar de acordo com a chave. */
        if(referencia.chave > atual.registro.chave){
            direita = 1;
            filho = atual.direita;
        }else{
            medidas->comparacoes++;
            if(referencia.chave < atual.registro.chave){
                direita = 0;
                filho = atual.esquerda;
            }else{
                return 1;
            }
        }
        if(filho < -1 || filho >= *numeroNos) return 0;
        /* Ao achar um filho vazio, grava o novo nó e atualiza o pai. */
        if(filho == -1){
            if(!escreverNo(arvore, *numeroNos, &novo)) return 0;
            if(direita) atual.direita = *numeroNos;
            else atual.esquerda = *numeroNos;
            if(!escreverNo(arvore, posicao, &atual)) return 0;
            (*numeroNos)++;
            return 1;
        }
        posicao = filho;
    }
    return 0;
}

/* Constrói o índice externo lendo os registros do arquivo de dados e inserindo
   na árvore apenas a chave e a posição de cada registro. */
int lerArquivoBinario(const char *nomeArquivoDados, const char *nomeArquivoArvore,
                     int numRegistros, Medidas *medidas){
    FILE *dados = NULL;
    FILE *arvore = NULL;
    int64_t numeroNos = 0;
    int sucesso = 0;
    int arvoreCriada = 0;
    double inicio;

    if(!medidas) return 0;
    memset(medidas, 0, sizeof(*medidas));
    inicio = now_seconds();
    /* Evita nomes inválidos, quantidade negativa e sobrescrita do próprio arquivo de dados. */
    if(!nomeArquivoDados || !nomeArquivoArvore || numRegistros < 0 ||
        strcmp(nomeArquivoDados, nomeArquivoArvore) == 0) goto finalizar;
    dados = fopen(nomeArquivoDados, "rb");
    if(!dados) goto finalizar;
    {
        int64_t tamanho = tamanhoArquivo(dados);
        if(tamanho < 0 || tamanho % (int64_t)sizeof(Registro) != 0 ||
            tamanho / (int64_t)sizeof(Registro) < numRegistros) goto finalizar;
    }
    arvore = fopen(nomeArquivoArvore, "w+b");
    if(!arvore) goto finalizar;
    arvoreCriada = 1;
    /* Cada nó guarda a posição do registro original, sem copiar seus campos maiores. */
    for(int i = 0; i < numRegistros; i++){
        Registro registro;
        Referencia referencia;
        if(!lerRegistro(dados, i, &registro, medidas)) goto finalizar;
        memset(&referencia, 0, sizeof(referencia));
        referencia.chave = registro.chave;
        referencia.posicao = i;
        if(!inserirEmArquivo(arvore, referencia, &numeroNos, medidas)) goto finalizar;
    }
    sucesso = 1;

finalizar:
    /* Fecha os arquivos mesmo quando ocorre uma falha durante a construção. */
    if(arvore && fclose(arvore) != 0) sucesso = 0;
    if(dados && fclose(dados) != 0) sucesso = 0;
    if(!sucesso && arvoreCriada && remove(nomeArquivoArvore) != 0){
        fprintf(stderr, "Erro ao remover o indice binario incompleto.\n");
    }
    medidas->tempo = now_seconds() - inicio;
    if(!sucesso) fprintf(stderr, "Erro ao construir a arvore binaria externa.\n");
    return sucesso;
}

/* Abre os arquivos usados nas buscas e calcula quantos registros e nós existem. */
static int abrirContexto(const char *nomeArquivoDados,
                         const char *nomeArquivoArvore, ContextoAB *contexto){
    int64_t tamanhoDados;
    int64_t tamanhoArvore;
    memset(contexto, 0, sizeof(*contexto));
    if(!nomeArquivoDados || !nomeArquivoArvore) return 0;
    contexto->dados = fopen(nomeArquivoDados, "rb");
    contexto->arvore = fopen(nomeArquivoArvore, "rb");
    if(!contexto->dados || !contexto->arvore) return 0;
    tamanhoDados = tamanhoArquivo(contexto->dados);
    tamanhoArvore = tamanhoArquivo(contexto->arvore);
    if(tamanhoDados < 0 || tamanhoArvore < 0 ||
        tamanhoDados % (int64_t)sizeof(Registro) != 0 ||
        tamanhoArvore % (int64_t)sizeof(NoArquivo) != 0) return 0;
    contexto->numeroRegistros = tamanhoDados / (int64_t)sizeof(Registro);
    contexto->numeroNos = tamanhoArvore / (int64_t)sizeof(NoArquivo);
    return 1;
}

/* Encerra os arquivos abertos pelo contexto. */
static int fecharContexto(ContextoAB *contexto){
    int sucesso = 1;
    if(contexto->arvore && fclose(contexto->arvore) != 0) sucesso = 0;
    if(contexto->dados && fclose(contexto->dados) != 0) sucesso = 0;
    contexto->arvore = NULL;
    contexto->dados = NULL;
    return sucesso;
}

/* Procura a chave começando pela raiz (posição zero). Quando encontra, usa
   a posição guardada no nó para ler o registro completo no arquivo de dados. */
static int pesquisarContexto(ContextoAB *contexto, int chave,
                            Registro *resultado, Medidas *medidas){
    int64_t posicao = contexto->numeroNos == 0 ? -1 : 0;
    int64_t passos = 0;
    while(posicao != -1){
        NoArquivo no;
        if(passos >= contexto->numeroNos) return -1;
        passos++;
        if(!lerNo(contexto->arvore, posicao, contexto->numeroNos, &no, medidas) ||
            !noValido(&no, posicao, contexto->numeroNos, contexto->numeroRegistros)){
            return -1;
        }
        medidas->comparacoes++;
        /* Se a chave estiver no nó atual, recupera o registro original. */
        if(chave == no.registro.chave){
            if(!lerRegistro(contexto->dados, no.registro.posicao, resultado, medidas)){
                return -1;
            }
            return 1;
        }
        /* Chaves menores seguem para a esquerda; as maiores, para a direita. */
        medidas->comparacoes++;
        if(chave < no.registro.chave) posicao = no.esquerda;
        else posicao = no.direita;
    }
    return 0;
}

/* Adapta a busca ao formato de função esperado por rodarPesquisas e mede o tempo. */
static int pesquisarAB(void *contexto, int chave, Registro *resultado,
                       Medidas *medidas){
    double inicio;
    int retorno;
    if(!contexto || !resultado || !medidas) return -1;
    memset(medidas, 0, sizeof(*medidas));
    inicio = now_seconds();
    retorno = pesquisarContexto(contexto, chave, resultado, medidas);
    medidas->tempo = now_seconds() - inicio;
    if(retorno < 0) fprintf(stderr, "Erro na pesquisa da arvore binaria externa.\n");
    return retorno;
}

/* Interface para realizar uma única busca abrindo os arquivos nesta chamada. */
int buscarEmArquivo(const char *nomeArquivoDados, const char *nomeArquivoArvore,
                    int chave, Registro *resultado, Medidas *medidas){
    ContextoAB contexto;
    double inicio;
    int retorno = -1;
    if(!resultado || !medidas) return -1;
    memset(medidas, 0, sizeof(*medidas));
    inicio = now_seconds();
    if(abrirContexto(nomeArquivoDados, nomeArquivoArvore, &contexto)){
        retorno = pesquisarContexto(&contexto, chave, resultado, medidas);
    }
    if(!fecharContexto(&contexto)) retorno = -1;
    medidas->tempo = now_seconds() - inicio;
    if(retorno < 0) fprintf(stderr, "Erro na pesquisa da arvore binaria externa.\n");
    return retorno;
}

/* Fluxo principal deste método: cria o arquivo de índice, executa as pesquisas
   solicitadas e fecha os arquivos ao final. */
int executarArvoreBinaria(const char *nomeArquivo, int quantidade, int chave,
                         int modoTeste){
    char *nomeArvore;
    ContextoAB contexto;
    Medidas construcao;
    size_t tamanhoNome;
    int sucesso;
    if(!nomeArquivo) return 0;
    tamanhoNome = strlen(nomeArquivo);
    if(tamanhoNome > SIZE_MAX - sizeof(".ab")) return 0;
    nomeArvore = malloc(tamanhoNome + sizeof(".ab"));
    if(!nomeArvore){
        fprintf(stderr, "Memoria insuficiente para o nome do indice binario.\n");
        return 0;
    }
    /* O índice é salvo em um arquivo separado, com a extensão .ab. */
    memcpy(nomeArvore, nomeArquivo, tamanhoNome);
    memcpy(nomeArvore + tamanhoNome, ".ab", sizeof(".ab"));
    /* Primeiro monta a árvore externa; depois utiliza o índice nas buscas. */
    sucesso = lerArquivoBinario(nomeArquivo, nomeArvore, quantidade, &construcao);
    if(sucesso){
        if(abrirContexto(nomeArquivo, nomeArvore, &contexto)){
            sucesso = rodarPesquisas(nomeArquivo, quantidade, chave, modoTeste,
                                    &construcao, pesquisarAB, &contexto);
        }else{
            fprintf(stderr, "Erro ao abrir os arquivos da arvore binaria externa.\n");
            sucesso = 0;
        }
        if(!fecharContexto(&contexto)){
            fprintf(stderr, "Erro ao fechar os arquivos da arvore binaria externa.\n");
            sucesso = 0;
        }
    }
    free(nomeArvore);
    return sucesso;
}
