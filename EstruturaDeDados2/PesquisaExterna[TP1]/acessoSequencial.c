#include "acessoSequencial.h"
#include "arquivo.h"
#include "suporte.h"
#include <stdlib.h>

// Percorre o arquivo, verifica se os registros estão ordenados e constrói
// uma tabela de índices com a primeira chave e a posição de cada página.
// Também contabiliza as operações realizadas durante a construção.
int criarIndicePaginas(const char *nome, int quantidade, int situacao, TipoIndice **tabela, int *numPaginas, Medidas *medidas){
    // tempo atual em segundos
    double inicio = now_seconds();

    FILE *arquivo;
    Registro registro;

    int anterior = 0, sucesso = 1;

    *medidas = (Medidas){0}; 
    *tabela = NULL;
    *numPaginas = 0;
    if(quantidade < 1 || quantidade > MAX_REGISTROS || (situacao != 1 && situacao != 2)){
        fprintf(stderr, "O acesso sequencial indexado exige um arquivo ordenado.\n");
        return 0;
    }

    *numPaginas = (quantidade + ITENSPAGINA - 1) / ITENSPAGINA; //quant de paginas
    *tabela = malloc((size_t)*numPaginas * sizeof(**tabela)); // tabela de indices 
    //printf("%d\n", *numPaginas);

    // pouca memoria ou arquivo muito grande
    if(*tabela == NULL){
        fprintf(stderr, "Memoria insuficiente para criação do indice sequencial.\n");
        *numPaginas = 0;
        medidas->tempo = now_seconds() - inicio;
        return 0;
    }
    arquivo = fopen(nome, "rb");

    //arq nao pode ser aberto
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para construir o indice.\n", nome);
        free(*tabela);
        *tabela = NULL;
        *numPaginas = 0;
        medidas->tempo = now_seconds() - inicio;
        return 0;
    }

    //lê cada registro do arquivo
    for(int i = 0; i < quantidade; i++){
        if(fread(&registro, sizeof(registro), 1, arquivo) != 1){
            fprintf(stderr, "Erro de leitura ou arquivo incompleto: %s.\n", nome);
            sucesso = 0;
            break;
        }
        medidas->leituras++;
        if(i > 0){
            medidas->comparacoes++;
            //verifica se o arq esta em ordem certa: crescente ou decrescente
            if((situacao == 1 && registro.chave < anterior) || (situacao == 2 && registro.chave > anterior)){
                fprintf(stderr, "O arquivo %s nao esta na ordem informada.\n", nome);
                sucesso = 0;
                break;
            }
        }
        anterior = registro.chave;
        if(i % ITENSPAGINA == 0){ //primeiro regsitro de cada pagina

            //posição de cada pagina
            (*tabela)[i / ITENSPAGINA].posicao = i / ITENSPAGINA;
            (*tabela)[i / ITENSPAGINA].chave = registro.chave;
        }
    }
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        sucesso = 0;
    }
    if(!sucesso){
        free(*tabela);
        *tabela = NULL;
        *numPaginas = 0;
    }
    medidas->tempo = now_seconds() - inicio;
    return sucesso;
}




/* localiza a página pelo índice e pesquisa a chave nos registros carregados */
static int pesquisarPagina(FILE *arquivo, int quantidade, int situacao, int chave,
                           const TipoIndice *tabela, int numPaginas,
                           Registro *resultado, Medidas *medidas){
    Registro pagina[ITENSPAGINA];
    int i = 0, primeiraPosicao, quantitens;
    size_t lidos;

    // percorre o índice considerando a ordem crescente ou decrescente
    while(i < numPaginas){
        medidas->comparacoes++;
        if(situacao == 1 ? tabela[i].chave <= chave : tabela[i].chave >= chave) i++;
        else break;
    }

    if(i == 0) return 0;

    // calcula a posição inicial e a quantidade de registros da página
    primeiraPosicao = tabela[i - 1].posicao * ITENSPAGINA;
    quantitens = quantidade - primeiraPosicao;
    if(quantitens > ITENSPAGINA) quantitens = ITENSPAGINA;

    // posiciona o arquivo no início da página, convertendo a posição em bytes
    if(quantitens <= 0 || !posicionarArquivo(arquivo,
            (int64_t)primeiraPosicao * (int64_t)sizeof(Registro), SEEK_SET)){
        fprintf(stderr, "Erro ao posicionar a pagina sequencial.\n");
        return -1;
    }

    // carrega a página do arquivo para a memória principal
    lidos = fread(pagina, sizeof(Registro), (size_t)quantitens, arquivo);
    medidas->leituras += (unsigned long long)lidos;

    if(lidos != (size_t)quantitens){
        fprintf(stderr, "Erro ao ler a pagina sequencial completa.\n");
        return -1;
    }

    // pesquisa a chave nos registros da página
    for(i = 0; i < quantitens; i++){
        medidas->comparacoes++;
        if(pagina[i].chave == chave){
            *resultado = pagina[i];
            return 1;
        }
    }

    return 0;
}

/* realiza uma pesquisa individual e contabiliza as operações e o tempo */
int pesquisarSequencial(const char *nome, int quantidade, int situacao, int chave,
                        const TipoIndice *tabela, int numPaginas,
                        Registro *resultado, Medidas *medidas){
    double inicio = now_seconds();
    FILE *arquivo;
    int encontrado;
    *medidas = (Medidas){0};

    // verifica os parâmetros recebidos
    if(tabela == NULL || numPaginas < 1 || quantidade < 1 || (situacao != 1 && situacao != 2)){
        fprintf(stderr, "Parametros invalidos para a pesquisa sequencial.\n");
        return -1;
    }

    arquivo = fopen(nome, "rb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para pesquisa.\n", nome);
        medidas->tempo = now_seconds() - inicio;
        return -1;
    }

    encontrado = pesquisarPagina(arquivo, quantidade, situacao, chave, tabela, numPaginas,
                                 resultado, medidas);

    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        encontrado = -1;
    }

    medidas->tempo = now_seconds() - inicio;
    return encontrado;
}

/* reúne os dados necessários para reutilizar o índice e o arquivo nas pesquisas */
typedef struct{
    FILE *arquivo;
    int quantidade, situacao, numPaginas;
    TipoIndice *tabela;
} ContextoSequencial;

/* executa uma pesquisa utilizando os dados armazenados no contexto */
static int pesquisar(void *contexto, int chave, Registro *resultado, Medidas *medidas){

    // converte o ponteiro genérico para o tipo do contexto
    ContextoSequencial *dados = contexto;
    double inicio = now_seconds();
    int encontrado;
    *medidas = (Medidas){0};

    encontrado = pesquisarPagina(dados->arquivo, dados->quantidade, dados->situacao, chave,
                                 dados->tabela, dados->numPaginas, resultado, medidas);

    medidas->tempo = now_seconds() - inicio;
    return encontrado;
}

/* constrói o índice, executa as pesquisas e libera os recursos utilizados */
int executarSequencial(const char *nome, int quantidade, int situacao,
                      int chave, int modoTeste){
    ContextoSequencial contexto = {0};
    Medidas construcao;
    int sucesso;

    // constrói o índice e interrompe em caso de falha
    if(!criarIndicePaginas(nome, quantidade, situacao, &contexto.tabela,
                           &contexto.numPaginas, &construcao)) return 0;

    contexto.arquivo = fopen(nome, "rb");
    if(contexto.arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para pesquisa.\n", nome);
        free(contexto.tabela);
        return 0;
    }

    contexto.quantidade = quantidade;
    contexto.situacao = situacao;

    // realiza as pesquisas reutilizando o índice e o arquivo aberto
    sucesso = rodarPesquisas(nome, quantidade, chave, modoTeste, &construcao, pesquisar, &contexto);

    if(fclose(contexto.arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        sucesso = 0;
    }

    free(contexto.tabela);
    return sucesso;
}


