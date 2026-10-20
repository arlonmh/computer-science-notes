#include <stdio.h>
#include <stdlib.h>

#include "arvoreb.h"
#include "arquivo.h"
#include "suporte.h"
#include "tempo.h"

// Compara se 'a' é maior que 'b', incrementa o contador de comparações e retorna verdadeiro/falso
static int maior(int a, int b, Medidas *medidas){
    medidas->comparacoes++; // Incrementa a contagem de comparações de chaves executadas pelo método
    return a > b;           // Retorna 1 se 'a' for maior que 'b', ou 0 caso contrário
}

// Compara se 'a' é menor que 'b', incrementa o contador de comparações e retorna verdadeiro/falso
static int menor(int a, int b, Medidas *medidas){
    medidas->comparacoes++; // Incrementa a contagem de comparações de chaves executadas pelo método
    return a < b;           // Retorna 1 se 'a' for menor que 'b', ou 0 caso contrário
}

// Compara se 'a' é igual a 'b', incrementa o contador de comparações e retorna verdadeiro/falso
static int igual(int a, int b, Medidas *medidas){
    medidas->comparacoes++; // Incrementa a contagem de comparações de chaves executadas pelo método
    return a == b;          // Retorna 1 se 'a' for igual a 'b', ou 0 caso contrário
}

// Compara se 'a' é maior ou igual a 'b', incrementa o contador e retorna verdadeiro/falso
static int maiorOuIgual(int a, int b, Medidas *medidas){
    medidas->comparacoes++; // Incrementa a contagem de comparações de chaves executadas pelo método
    return a >= b;          // Retorna 1 se 'a' for maior ou igual a 'b', ou 0 caso contrário
}

const Referencia *pesquisa(Pagina *pagina, int chave, Medidas *medidas){
    // Laço executado enquanto o ponteiro da página não for NULO (navegação no índice em RAM)
    while(pagina != NULL){
        int i = 1; // Inicializa o índice de navegação na página

        // Varre as chaves da página da esquerda para a direita enquanto a chave procurada for maior
        while(i < pagina->n && maior(chave, pagina->registro[i - 1].chave,
                                      medidas)){
            i++; // Avança para a próxima posição dentro da página atual
        }

        // Verifica se a chave na posição atual é a chave buscada
        if(igual(chave, pagina->registro[i - 1].chave, medidas)){
            return &pagina->registro[i - 1]; // Sucesso: retorna o ponteiro da Referencia encontrada
        }

        // Se a chave buscada for menor que a chave avaliada na página, ajusta o índice do filho
        if(menor(chave, pagina->registro[i - 1].chave, medidas)){
            i--; // Decrementa o índice para direcionar para o ponteiro do filho esquerdo correto
        }

        // Desce na árvore navegando para a página filha apontada no ponteiro 'filhos[i]'
        pagina = pagina->filhos[i];
    }
    return NULL; // Se percorreu o caminho e chegou em NULL, a chave não existe na árvore
}

void InsereNaPagina(Pagina *pagina, Referencia reg, Pagina *direita,
                    Medidas *medidas){
    int k = pagina->n; // Começa a verificação a partir da última posição preenchida na página

    // Varre a página de trás para frente abrindo espaço para manter as chaves ordenadas
    while(k > 0){
        // Se a nova chave for maior ou igual à chave da posição anterior, achou o ponto correto de inserção
        if(maiorOuIgual(reg.chave, pagina->registro[k - 1].chave, medidas)){
            break; // Interrompe o laço de deslocamento
        }

        // Desloca a chave e o ponteiro filho uma posição para a direita no vetor
        pagina->registro[k] = pagina->registro[k - 1];
        pagina->filhos[k + 1] = pagina->filhos[k];
        k--; // Passa para a posição anterior
    }

    // Insere a nova Referencia no local correto
    pagina->registro[k] = reg;

    // Associa o ponteiro da página à direita (gerado por uma cisão prévia) ao filho da chave
    pagina->filhos[k + 1] = direita;

    // Incrementa a contagem de elementos presentes na página
    pagina->n++;
}

void liberarArvoreB(Pagina *raiz){
    // Se a página for nula, não há nada a liberar
    if(raiz == NULL){
        return;
    }

    // Percorre recursivamente todos os ponteiros de filhos válidos da página
    for(int i = 0; i <= raiz->n; i++){
        // Libera as subárvores filhas de baixo para cima
        liberarArvoreB(raiz->filhos[i]);
    }
    free(raiz); // Libera o bloco de memória da página atual da RAM
}

int Ins(Referencia reg, Pagina *pagina, short *cresceu,
        Referencia *regRetorno, Pagina **paginaRetorno, Medidas *medidas){
    int i = 1; // Inicializa índice de busca

    // Caso Base da Recursão: Chegou na folha nula da árvore
    if(pagina == NULL){
        *cresceu = 1; // Sinaliza para o nível superior que uma chave precisa ser inserida
        *regRetorno = reg; // Passa a referência a ser inserida no nó acima
        *paginaRetorno = NULL; // Ponteiro do filho direito começa como nulo
        return 1; // Retorna 1 indicando execução bem-sucedida
    }

    // Procura a posição adequada dentro da página corrente para identificar qual filho descer
    while(i < pagina->n && maior(reg.chave, pagina->registro[i - 1].chave,
                                  medidas)){
        i++;
    }

    // Se a chave já existir na árvore, não insere duplicados
    if(igual(reg.chave, pagina->registro[i - 1].chave, medidas)){
        *cresceu = 0; // Sinaliza que a árvore não precisa crescer
        return 1; // Sucesso sem inserir
    }

    // Se a chave for menor que a posição testada, decrementa o ponteiro para pegar o filho à esquerda
    if(menor(reg.chave, pagina->registro[i - 1].chave, medidas)){
        i--;
    }

    // Chamada recursiva descendo no filho correspondente
    if(!Ins(reg, pagina->filhos[i], cresceu, regRetorno, paginaRetorno,
             medidas)){
        return 0; // Se a recursão falhar (ex: erro de memória), propaga a falha para cima
    }

    // Se o nível inferior não gerou propagação/subida de elemento, encerra esta chamada
    if(!*cresceu){
        return 1;
    }

    // Se a página corrente AINDA POSSUI ESPAÇO livre (n < MM), insere nela mesmo e resolve
    if(pagina->n < MM){
        InsereNaPagina(pagina, *regRetorno, *paginaRetorno, medidas); // Insere o item promovido
        *cresceu = 0; // Reseta a flag 'cresceu', pois a subida parou nesta página
        return 1; // Sucesso
    }

    // PROCESSO DE SPLIT (QUANDO A PÁGINA ESTÁ CHEIA: n == MM)

    // Aloca memória zerada (calloc) para a nova página irmã que será criada à direita
    Pagina *nova = calloc(1, sizeof(*nova));

    // Se faltar memória no sistema
    if(nova == NULL){
        liberarArvoreB(*paginaRetorno); // Limpa subárvores geradas
        *paginaRetorno = NULL;
        *cresceu = 0;
        return 0; // Erro de alocação
    }

    // Distribui os elementos entre a página antiga, a nova página irmã e o item que subirá
    if(i < ORDEM + 1){
        // Se a inserção nova for na metade esquerda, move o último item da antiga para a nova página
        InsereNaPagina(nova, pagina->registro[MM - 1], pagina->filhos[MM],
                       medidas);
        pagina->n--; // Decrementa a quantidade da página antiga

        // Insere o novo registro na antiga
        InsereNaPagina(pagina, *regRetorno, *paginaRetorno, medidas);
    }else{
        // Se a inserção for na metade direita, insere o elemento direto na nova página irmã
        InsereNaPagina(nova, *regRetorno, *paginaRetorno, medidas);
    }

    // Copia a metade superior das chaves (da posição ORDEM+1 até MM) para a nova página irmã
    for(int j = ORDEM + 1; j < MM; j++){
        InsereNaPagina(nova, pagina->registro[j], pagina->filhos[j + 1],
                       medidas);
    }

    // Reduz o número de elementos da página original para metade da ordem (ORDEM = 50)
    pagina->n = ORDEM;

    // O primeiro filho da nova página irmã passa a ser o filho que estava à direita da mediana
    nova->filhos[0] = pagina->filhos[ORDEM + 1];

    // A chave mediana (posição ORDEM) é selecionada para subir para o nó pai
    *regRetorno = pagina->registro[ORDEM];

    // O ponteiro da nova página criada passa a ser o filho direito da chave que subiu
    *paginaRetorno = nova;

    return 1; // Retorna 1 mantendo a flag 'cresceu = 1' para o pai tratar
}

int Insere(Referencia reg, Pagina **raiz, Medidas *medidas){
    short cresceu = 0; // Flag para verificar se a raiz precisará sofrer divisão
    Referencia regRetorno; // Variável para armazenar a chave mediana que pode subir
    Pagina *paginaRetorno = NULL; // Ponteiro da página gerada no split

    // Tenta fazer a inserção recursiva a partir da raiz
    if(!Ins(reg, *raiz, &cresceu, &regRetorno, &paginaRetorno, medidas)){
        return 0; // Falha na inserção por falta de memória
    }

    // Se o sinalizador 'cresceu' voltou verdadeiro, a raiz antiga estourou e sofreu split!
    if(cresceu){
        // Aloca espaço na memória RAM para uma NOVA RAIZ (a árvore ganha um nível para cima)
        Pagina *novaRaiz = calloc(1, sizeof(*novaRaiz));
        if(novaRaiz == NULL){
            liberarArvoreB(paginaRetorno);
            return 0; // Erro de memória
        }

        // A nova raiz conterá exatamente 1 chave (a mediana promovida)
        novaRaiz->n = 1;

        // Atribui a chave mediana promovida
        novaRaiz->registro[0] = regRetorno;

        // O filho esquerdo da nova raiz vira a antiga raiz
        novaRaiz->filhos[0] = *raiz;

        // O filho direito vira a nova página irmã criada no split
        novaRaiz->filhos[1] = paginaRetorno;

        // Atualiza o ponteiro principal para a nova raiz
        *raiz = novaRaiz;
    }
    return 1; // Sucesso na inserção
}

int lerArquivoArvoreB(const char *nomeArquivo, int numRegistros,
                     Pagina **raiz, Medidas *medidas){
    double inicio = now_seconds(); // Marca o tempo de início do cronômetro de construção
    *medidas = (Medidas){0}; // Zera a estrutura de medidas (leituras, comparações, tempo)
    *raiz = NULL; // Inicializa o ponteiro da raiz como NULO (árvore vazia)
    
    // Abre o arquivo binário em modo de leitura binária ("rb")
    FILE *arquivo = fopen(nomeArquivo, "rb");

    // Se o arquivo não puder ser aberto
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para construir a arvore B.\n",
                nomeArquivo);
        medidas->tempo = now_seconds() - inicio; // Salva tempo decorrido até a falha
        return 0; // Falha
    }

    int sucesso = 1; // Controle de status de execução
    Registro reg; // Buffer temporário para carregar o registro lido do disco

    // Percorre o arquivo lendo sequencialmente os 'numRegistros' solicitados
    for(int pos = 0; pos < numRegistros; pos++){
        // Lê 1 registro do arquivo binário no disco
        if(fread(&reg, sizeof(reg), 1, arquivo) != 1){
            fprintf(stderr, "Erro: arquivo incompleto ou falha de leitura na arvore B.\n");
            sucesso = 0; // Marca falha
            break;
        }

        // Contabiliza 1 leitura (transferência) do disco para a memória
        medidas->leituras++;

        // Cria a estrutura leve 'Referencia' contendo apenas a chave e seu número de registro (offset)
        Referencia ref = {reg.chave, pos};

        // Insere a referência no índice da Árvore B alocado na memória RAM
        if(!Insere(ref, raiz, medidas)){
            fprintf(stderr, "Erro: memoria insuficiente para construir a arvore B.\n");
            sucesso = 0;
            break;
        }
    }

    // Fecha a ponte com o arquivo no disco
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar o arquivo de dados da arvore B.\n");
        sucesso = 0;
    }

    // Se houve alguma falha no meio do caminho, limpa toda a RAM para não haver vazamento
    if(!sucesso){
        liberarArvoreB(*raiz);
        *raiz = NULL;
    }

    // Registra o tempo final de construção do índice
    medidas->tempo = now_seconds() - inicio;

    return sucesso; // Retorna status de sucesso (1) ou falha (0)
}

typedef struct{
    Pagina *raiz; // Ponteiro para a raiz da árvore B construída na RAM
    FILE *arquivo; // Ponteiro para o arquivo binário aberto para buscar os dados completos
} ContextoB;

static int buscarRegistroB(void *contexto, int chave, Registro *registro,
                          Medidas *medidas){
    ContextoB *ctx = contexto; // Converte o ponteiro genérico void* para ContextoB*
    *medidas = (Medidas){0}; // Zera contadores de medição do teste atual
    double inicio = now_seconds(); // Inicia o cronômetro da pesquisa

    // Pesquisa a chave no índice da Árvore B em RAM
    const Referencia *ref = pesquisa(ctx->raiz, chave, medidas);
    int resultado = 0; // Resultado 0 = não encontrado

    // Se encontrou a referência no índice
    if(ref != NULL){
        // Usa o 'ref->posicao' obtido na árvore B para buscar 1 única vez o registro completo no disco
        resultado = lerRegistro(ctx->arquivo, ref->posicao, registro, medidas)
                    ? 1 : -1;
    }

    // Registra o tempo consumido pela pesquisa
    medidas->tempo = now_seconds() - inicio;
    return resultado; // Retorna 1 (encontrado), 0 (não encontrado) ou -1 (erro)
}

int executarArvoreB(const char *nomeArquivo, int quantidade, int chave,
                    int modoTeste){
    Pagina *raiz = NULL; // Ponteiro inicial para a raiz da árvore B
    Medidas construcao; // Variável para armazenar leituras, comparações e tempo de construção

    // Etapa 1: Lê o arquivo binário e constrói o índice da Árvore B em RAM
    if(!lerArquivoArvoreB(nomeArquivo, quantidade, &raiz, &construcao)){
        return 0; // Retorna 0 se a criação do índice falhar
    }

    // Abre o arquivo binário em modo de leitura para resgatar os registros encontrados
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para pesquisar na arvore B.\n",
                nomeArquivo);
        liberarArvoreB(raiz); // Libera o índice da RAM
        return 0;
    }

    // Prepara a struct de contexto passando a raiz da árvore e o arquivo aberto
    ContextoB ctx = {raiz, arquivo};

    // Etapa 2: Executa os testes da Fase 1 (1 chave) ou Fase 2 (20 chaves) usando 'rodarPesquisas'
    int resultado = rodarPesquisas(nomeArquivo, quantidade, chave, modoTeste,
                                  &construcao, buscarRegistroB, &ctx);

    // Fecha o arquivo binário no disco
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar o arquivo de pesquisa da arvore B.\n");
        resultado = 0;
    }

    // Libera da memória RAM toda a Árvore B construída
    liberarArvoreB(raiz);
    return resultado; // Retorna o status final de sucesso ou erro
}
