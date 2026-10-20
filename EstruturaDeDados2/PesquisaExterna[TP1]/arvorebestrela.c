#include <stdio.h>
#include <stdlib.h>

#include "arvorebestrela.h"
#include "arquivo.h"
#include "suporte.h"
#include "tempo.h"

// verifica se o primeiro valor e maior que o segundo e contabiliza a comparacao.
static int maior(int a, int b, Medidas *medidas){
    medidas->comparacoes++;
    return a > b;
}

// verifica se o primeiro valor e menor que o segundo e contabiliza a comparacao.
static int menor(int a, int b, Medidas *medidas){
    medidas->comparacoes++;
    return a < b;
}

// confere se os dois valores sao iguais e registra a comparacao nas medidas.
static int igual(int a, int b, Medidas *medidas){
    medidas->comparacoes++;
    return a == b;
}

// verifica se o primeiro valor e maior ou igual ao segundo, contando a comparacao.
static int maiorOuIgual(int a, int b, Medidas *medidas){
    medidas->comparacoes++;
    return a >= b;
}

// insere uma chave na pagina interna, mantendo as chaves em ordem.
// desloca as chaves e os ponteiros para abrir espaco para o novo filho.
void insereNaPaginaInterna(PaginaEstrela *pagina, int chave,
                          PaginaEstrela *direita, Medidas *medidas){
    int k = pagina->conteudo.interna.numChaves;
    while(k > 0){
        if(maiorOuIgual(chave, pagina->conteudo.interna.chaves[k - 1],
                         medidas)){
            break;
        }
        pagina->conteudo.interna.chaves[k] =
            pagina->conteudo.interna.chaves[k - 1];
        pagina->conteudo.interna.filhos[k + 1] =
            pagina->conteudo.interna.filhos[k];
        k--;
    }
    pagina->conteudo.interna.chaves[k] = chave;
    pagina->conteudo.interna.filhos[k + 1] = direita;
    pagina->conteudo.interna.numChaves++;
}

// adiciona uma referencia na pagina externa em sua posicao ordenada.
// move os registros necessarios antes de colocar a nova referencia.
void insereNaPaginaExterna(PaginaEstrela *pagina, Referencia reg,
                          Medidas *medidas){
    int k = pagina->conteudo.externa.numRegistros;
    while(k > 0){
        if(maiorOuIgual(reg.chave, pagina->conteudo.externa.registros[k - 1].chave,
                         medidas)){
            break;
        }
        pagina->conteudo.externa.registros[k] =
            pagina->conteudo.externa.registros[k - 1];
        k--;
    }
    pagina->conteudo.externa.registros[k] = reg;
    pagina->conteudo.externa.numRegistros++;
}

// percorre as paginas internas escolhendo o filho adequado para cada chave.
// ao chegar na pagina externa, procura e retorna a referencia, caso exista.
const Referencia *pesquisaBEstrela(PaginaEstrela *pagina, int chave,
                                  Medidas *medidas){
    while(pagina != NULL){
        int i = 1;
        if(pagina->tipo == Interna){
            while(i < pagina->conteudo.interna.numChaves &&
                   maior(chave, pagina->conteudo.interna.chaves[i - 1],
                         medidas)){
                i++;
            }
            if(menor(chave, pagina->conteudo.interna.chaves[i - 1], medidas)){
                i--;
            }
            pagina = pagina->conteudo.interna.filhos[i];
        }else{
            while(i < pagina->conteudo.externa.numRegistros &&
                   maior(chave, pagina->conteudo.externa.registros[i - 1].chave,
                         medidas)){
                i++;
            }
            if(igual(chave, pagina->conteudo.externa.registros[i - 1].chave,
                      medidas)){
                return &pagina->conteudo.externa.registros[i - 1];
            }
            return NULL;
        }
    }
    return NULL;
}

// libera a memoria ocupada pela arvore, passando primeiro pelos filhos.
// assim, cada pagina e liberada depois das suas subarvores.
void liberarArvoreBEstrela(PaginaEstrela *raiz){
    if(raiz == NULL){
        return;
    }
    if(raiz->tipo == Interna){
        for(int i = 0; i <= raiz->conteudo.interna.numChaves; i++){
            liberarArvoreBEstrela(raiz->conteudo.interna.filhos[i]);
        }
    }
    free(raiz);
}

// faz a insercao recursiva ate encontrar a pagina onde o registro deve ficar.
// quando uma pagina fica cheia, divide seu conteudo e repassa a chave ao pai.
int insBEstrela(Referencia reg, PaginaEstrela *pagina, short *cresceu,
               int *chaveRetorno, PaginaEstrela **paginaRetorno,
               Medidas *medidas){
    int i = 1;
    if(pagina->tipo == Interna){
        while(i < pagina->conteudo.interna.numChaves &&
               maior(reg.chave, pagina->conteudo.interna.chaves[i - 1],
                     medidas)){
            i++;
        }
        if(menor(reg.chave, pagina->conteudo.interna.chaves[i - 1], medidas)){
            i--;
        }
        if(!insBEstrela(reg, pagina->conteudo.interna.filhos[i], cresceu,
                        chaveRetorno, paginaRetorno, medidas)){
            return 0;
        }
        if(!*cresceu){
            return 1;
        }
        if(pagina->conteudo.interna.numChaves < MM_ESTRELA){
            insereNaPaginaInterna(pagina, *chaveRetorno, *paginaRetorno,
                                  medidas);
            *cresceu = 0;
            return 1;
        }

        PaginaEstrela *nova = calloc(1, sizeof(*nova));
        if(nova == NULL){
            liberarArvoreBEstrela(*paginaRetorno);
            *paginaRetorno = NULL;
            *cresceu = 0;
            return 0;
        }
        nova->tipo = Interna;
        if(i < ORDEM_ESTRELA + 1){
            insereNaPaginaInterna(nova,
                pagina->conteudo.interna.chaves[MM_ESTRELA - 1],
                pagina->conteudo.interna.filhos[MM_ESTRELA], medidas);
            pagina->conteudo.interna.numChaves--;
            insereNaPaginaInterna(pagina, *chaveRetorno, *paginaRetorno,
                                  medidas);
        }else{
            insereNaPaginaInterna(nova, *chaveRetorno, *paginaRetorno,
                                  medidas);
        }
        for(int j = ORDEM_ESTRELA + 1; j < MM_ESTRELA; j++){
            insereNaPaginaInterna(nova, pagina->conteudo.interna.chaves[j],
                pagina->conteudo.interna.filhos[j + 1], medidas);
        }
        pagina->conteudo.interna.numChaves = ORDEM_ESTRELA;
        nova->conteudo.interna.filhos[0] =
            pagina->conteudo.interna.filhos[ORDEM_ESTRELA + 1];
        *chaveRetorno = pagina->conteudo.interna.chaves[ORDEM_ESTRELA];
        *paginaRetorno = nova;
        return 1;
    }

    while(i < pagina->conteudo.externa.numRegistros &&
           maior(reg.chave, pagina->conteudo.externa.registros[i - 1].chave,
                 medidas)){
        i++;
    }
    if(igual(reg.chave, pagina->conteudo.externa.registros[i - 1].chave,
              medidas)){
        *cresceu = 0;
        return 1;
    }
    if(menor(reg.chave, pagina->conteudo.externa.registros[i - 1].chave,
              medidas)){
        i--;
    }
    if(pagina->conteudo.externa.numRegistros < MM_ESTRELA){
        insereNaPaginaExterna(pagina, reg, medidas);
        *cresceu = 0;
        return 1;
    }

    PaginaEstrela *nova = calloc(1, sizeof(*nova));
    if(nova == NULL){
        *cresceu = 0;
        return 0;
    }
    nova->tipo = Externa;
    if(i < ORDEM_ESTRELA + 1){
        insereNaPaginaExterna(nova,
                             pagina->conteudo.externa.registros[MM_ESTRELA - 1],
                             medidas);
        pagina->conteudo.externa.numRegistros--;
        insereNaPaginaExterna(pagina, reg, medidas);
    }else{
        insereNaPaginaExterna(nova, reg, medidas);
    }
    for(int j = ORDEM_ESTRELA; j < MM_ESTRELA; j++){
        insereNaPaginaExterna(nova, pagina->conteudo.externa.registros[j],
                             medidas);
    }
    pagina->conteudo.externa.numRegistros = ORDEM_ESTRELA;
    *chaveRetorno = nova->conteudo.externa.registros[0].chave;
    *paginaRetorno = nova;
    *cresceu = 1;
    return 1;
}

// cria a primeira pagina se a arvore estiver vazia ou inicia a insercao recursiva.
// se a raiz for dividida, cria uma nova raiz para ligar as duas paginas.
int insereBEstrela(Referencia reg, PaginaEstrela **raiz, Medidas *medidas){
    if(*raiz == NULL){
        PaginaEstrela *nova = calloc(1, sizeof(*nova));
        if(nova == NULL){
            return 0;
        }
        nova->tipo = Externa;
        insereNaPaginaExterna(nova, reg, medidas);
        *raiz = nova;
        return 1;
    }
    short cresceu = 0;
    int chaveRetorno = 0;
    PaginaEstrela *paginaRetorno = NULL;
    if(!insBEstrela(reg, *raiz, &cresceu, &chaveRetorno, &paginaRetorno,
                    medidas)){
        return 0;
    }
    if(cresceu){
        PaginaEstrela *novaRaiz = calloc(1, sizeof(*novaRaiz));
        if(novaRaiz == NULL){
            liberarArvoreBEstrela(paginaRetorno);
            return 0;
        }
        novaRaiz->tipo = Interna;
        novaRaiz->conteudo.interna.numChaves = 1;
        novaRaiz->conteudo.interna.chaves[0] = chaveRetorno;
        novaRaiz->conteudo.interna.filhos[0] = *raiz;
        novaRaiz->conteudo.interna.filhos[1] = paginaRetorno;
        *raiz = novaRaiz;
    }
    return 1;
}

// le os registros do arquivo e monta a arvore com suas chaves e posicoes.
// tambem mede o custo da construcao e trata possiveis falhas nesse processo.
int lerArquivoArvoreBEstrela(const char *nomeArquivo, int numRegistros,
                            PaginaEstrela **raiz, Medidas *medidas){
    double inicio = now_seconds();
    *medidas = (Medidas){0};
    *raiz = NULL;
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para construir a arvore B*.\n",
                nomeArquivo);
        medidas->tempo = now_seconds() - inicio;
        return 0;
    }

    int sucesso = 1;
    Registro reg;
    for(int pos = 0; pos < numRegistros; pos++){
        if(fread(&reg, sizeof(reg), 1, arquivo) != 1){
            fprintf(stderr, "Erro: arquivo incompleto ou falha de leitura na arvore B*.\n");
            sucesso = 0;
            break;
        }
        medidas->leituras++;
        Referencia ref = {reg.chave, pos};
        if(!insereBEstrela(ref, raiz, medidas)){
            fprintf(stderr, "Erro: memoria insuficiente para construir a arvore B*.\n");
            sucesso = 0;
            break;
        }
    }
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar o arquivo de dados da arvore B*.\n");
        sucesso = 0;
    }
    if(!sucesso){
        liberarArvoreBEstrela(*raiz);
        *raiz = NULL;
    }
    medidas->tempo = now_seconds() - inicio;
    return sucesso;
}

typedef struct{
    PaginaEstrela *raiz;
    FILE *arquivo;
} ContextoBEstrela;

// localiza a chave na arvore e usa a posicao encontrada para ler o registro.
// contabiliza as operacoes e o tempo gasto durante a pesquisa.
static int buscarRegistroBEstrela(void *contexto, int chave, Registro *registro,
                                 Medidas *medidas){
    ContextoBEstrela *ctx = contexto;
    *medidas = (Medidas){0};
    double inicio = now_seconds();
    const Referencia *ref = pesquisaBEstrela(ctx->raiz, chave, medidas);
    int resultado = 0;
    if(ref != NULL){
        resultado = lerRegistro(ctx->arquivo, ref->posicao, registro, medidas)
                    ? 1 : -1;
    }
    medidas->tempo = now_seconds() - inicio;
    return resultado;
}

// organiza a execucao do metodo, construindo a arvore e abrindo o arquivo.
// realiza as pesquisas solicitadas e libera os recursos usados ao final.
int executarArvoreBEstrela(const char *nomeArquivo, int quantidade, int chave,
                           int modoTeste){
    PaginaEstrela *raiz = NULL;
    Medidas construcao;
    if(!lerArquivoArvoreBEstrela(nomeArquivo, quantidade, &raiz, &construcao)){
        return 0;
    }
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para pesquisar na arvore B*.\n",
                nomeArquivo);
        liberarArvoreBEstrela(raiz);
        return 0;
    }
    ContextoBEstrela ctx = {raiz, arquivo};
    int resultado = rodarPesquisas(nomeArquivo, quantidade, chave, modoTeste,
                                  &construcao, buscarRegistroBEstrela, &ctx);
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar o arquivo de pesquisa da arvore B*.\n");
        resultado = 0;
    }
    liberarArvoreBEstrela(raiz);
    return resultado;
}
