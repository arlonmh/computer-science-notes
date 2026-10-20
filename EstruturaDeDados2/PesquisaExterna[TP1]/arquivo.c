#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64
#endif
#include "arquivo.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/types.h>
#endif

int converterInteiro(const char *texto, int *valor){
    char *fim;
    long numero;
    errno = 0;
    numero = strtol(texto, &fim, 10);
    if(errno != 0 || fim == texto || *fim != '\0' || numero < INT_MIN || numero > INT_MAX)
        return 0;
    *valor = (int)numero;
    return 1;
}

int sortearPosicao(int limite){
    unsigned long long base = (unsigned long long)RAND_MAX + 1ULL;
    unsigned long long total = base * base;
    unsigned long long corte = total - total % (unsigned long long)limite;
    unsigned long long valor;
    do{
        valor = (unsigned long long)rand() * base + (unsigned long long)rand();
    } while(valor >= corte);
    return (int)(valor % (unsigned long long)limite);
}

int posicionarArquivo(FILE *arquivo, int64_t deslocamento, int origem){
#ifdef _WIN32
    // Se compilar em Windows, usa _fseeki64 para suportar offsets de 64 bits (ficheiros maiores que 2 GB)
    return _fseeki64(arquivo, deslocamento, origem) == 0;
#else
    // Em sistemas Linux, usa fseeko com o tipo off_t de 64 bits
    return fseeko(arquivo, (off_t)deslocamento, origem) == 0;
#endif
}

int64_t tamanhoArquivo(FILE *arquivo){
#ifdef _WIN32
    int64_t atual = _ftelli64(arquivo);
#else
    int64_t atual = (int64_t)ftello(arquivo);
#endif
    int64_t tamanho;
    if(atual < 0 || !posicionarArquivo(arquivo, 0, SEEK_END)) return -1;
#ifdef _WIN32
    tamanho = _ftelli64(arquivo);
#else
    tamanho = (int64_t)ftello(arquivo);
#endif
    if(!posicionarArquivo(arquivo, atual, SEEK_SET)) return -1;
    return tamanho;
}

int lerRegistro(FILE *arquivo, int64_t posicao, Registro *registro, Medidas *medidas){
    // Valida se a posição é positiva e se o cálculo do byte offset não ultrapassa o limite de 64 bits
    if(posicao < 0 || posicao > INT64_MAX / (int64_t)sizeof(*registro) ||
        // Posiciona o ponteiro do ficheiro no byte exato: posicao * tamanho_do_registro
        !posicionarArquivo(arquivo, posicao * (int64_t)sizeof(*registro), SEEK_SET)){
        fprintf(stderr, "Erro ao posicionar a leitura do registro.\n");
        return 0; // Retorna 0 se falhar ao posicionar
    }

    // Tenta ler exatamente 1 registro do ficheiro físico para a memória RAM
    if(fread(registro, sizeof(*registro), 1, arquivo) != 1){
        fprintf(stderr, "Erro de leitura ou registro incompleto.\n");
        return 0; // Retorna 0 em caso de erro de I/O
    }

    // Se a struct 'medidas' tiver sido passada, contabiliza 1 leitura física de disco
    if(medidas != NULL) medidas->leituras++;
    return 1; // Retorna 1 indicando sucesso na leitura do registro
}

int criarArquivo(const char *nome, int quantidade, int situacao){
    int *chaves = NULL;
    FILE *arquivo;
    Registro registro;
    int sucesso = 1;
    if(quantidade < 1 || quantidade > MAX_REGISTROS || situacao < 1 || situacao > 3){
        fprintf(stderr, "Quantidade ou situacao invalida para gerar o arquivo.\n");
        return 0;
    }
    if(situacao == 3){
        chaves = malloc((size_t)quantidade * sizeof(*chaves));
        if(chaves == NULL){
            fprintf(stderr, "Memoria insuficiente para gerar as chaves.\n");
            return 0;
        }
        for(int i = 0; i < quantidade; i++) chaves[i] = i + 1;
        for(int i = quantidade - 1; i > 0; i--){
            int j = sortearPosicao(i + 1);
            int auxiliar = chaves[i];
            chaves[i] = chaves[j];
            chaves[j] = auxiliar;
        }
    }
    arquivo = fopen(nome, "wb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao criar %s.\n", nome);
        free(chaves);
        return 0;
    }
    for(int i = 0; i < quantidade; i++){
        memset(&registro, 0, sizeof(registro));
        registro.chave = situacao == 1 ? i + 1 : (situacao == 2 ? quantidade - i : chaves[i]);
        registro.dado1 = registro.chave * 10L;
        snprintf(registro.dado2, sizeof(registro.dado2), "Registro %d", registro.chave);
        if(fwrite(&registro, sizeof(registro), 1, arquivo) != 1){
            fprintf(stderr, "Erro ao gravar %s.\n", nome);
            sucesso = 0;
            break;
        }
    }
    free(chaves);
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s apos a gravacao.\n", nome);
        sucesso = 0;
    }
    if(!sucesso && remove(nome) != 0)
        fprintf(stderr, "Nao foi possivel remover o arquivo incompleto %s.\n", nome);
    return sucesso;
}

int garantirArquivo(const char *nome, int quantidade, int situacao){
    FILE *arquivo = fopen(nome, "rb");
    int64_t tamanho;
    int sucesso;
    if(arquivo == NULL){
        if(errno != ENOENT){
            fprintf(stderr, "Erro ao abrir %s.\n", nome);
            return 0;
        }
        if(!criarArquivo(nome, quantidade, situacao)) return 0;
        printf("Arquivo criado: %s\n", nome);
        return 1;
    }
    tamanho = tamanhoArquivo(arquivo);
    sucesso = tamanho == (int64_t)quantidade * (int64_t)sizeof(Registro);
    if(!sucesso)
        fprintf(stderr, "Arquivo %s tem tamanho incorreto: esperados %d registros de %zu bytes.\n",
                nome, quantidade, sizeof(Registro));
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        sucesso = 0;
    }
    return sucesso;
}

int mostrarChaves(const char *nome, int quantidade){
    FILE *arquivo = fopen(nome, "rb");
    Registro registro;
    int sucesso = 1;
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para imprimir chaves.\n", nome);
        return 0;
    }
    printf("Chaves do arquivo:\n");
    for(int i = 0; i < quantidade; i++){
        if(fread(&registro, sizeof(registro), 1, arquivo) != 1){
            fprintf(stderr, "Erro ao ler as chaves de %s.\n", nome);
            sucesso = 0;
            break;
        }
        printf("%d%s", registro.chave, (i + 1) % 20 == 0 ? "\n" : " ");
    }
    if(quantidade % 20 != 0) printf("\n");
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        sucesso = 0;
    }
    return sucesso;
}

static int chaveRepetida(int chave, const int *chaves, int quantidade){
    // Percorre o vetor de chaves já selecionadas
    for(int i = 0; i < quantidade; i++)
        // Retorna 1 se a chave já tiver sido sorteada
        if(chaves[i] == chave) return 1;
    return 0; // Retorna 0 se for uma chave nova/única
}

int selecionarChaves(const char *nome, int quantidade, int chaves[CHAVES_EXPERIMENTO]){
    FILE *arquivo;
    Registro registro;
    int selecionadas = 0, sucesso = 1;

    // Valida se o ficheiro possui pelo menos 20 registros
    if(quantidade < CHAVES_EXPERIMENTO){
        fprintf(stderr, "O experimento exige pelo menos 20 chaves distintas.\n");
        return 0;
    }

    // Abre o ficheiro binário para leitura
    arquivo = fopen(nome, "rb");
    if(arquivo == NULL){
        fprintf(stderr, "Erro ao abrir %s para selecionar chaves.\n", nome);
        return 0;
    }

    srand(1); // Fixa a semente do gerador aleatório para reprodutibilidade dos testes

    // Divide o ficheiro em 20 blocos contínuos iguais para garantir chaves bem distribuídas
    for(int i = 0; i < CHAVES_EXPERIMENTO; i++){
        // Calcula a posição inicial do bloco 'i'
        int inicio = (int)((int64_t)i * quantidade / CHAVES_EXPERIMENTO);

        // Calcula a posição final do bloco 'i'
        int fim = (int)((int64_t)(i + 1) * quantidade / CHAVES_EXPERIMENTO);

        // Sorteia uma posição aleatória dentro desse intervalo do bloco
        int posicao = inicio + sortearPosicao(fim - inicio);

        // Lê o registro da posição sorteada sem contabilizar na medição da Árvore B
        if(!lerRegistro(arquivo, posicao, &registro, NULL)){
            sucesso = 0;
            break;
        }

        // Se a chave lida ainda não tiver sido guardada, adiciona-a ao vetor de testes
        if(!chaveRepetida(registro.chave, chaves, selecionadas))
            chaves[selecionadas++] = registro.chave;
    }

    // Caso o sorteio por blocos não tenha atingido 20 chaves únicas (por repetição de chaves no ficheiro)
    if(sucesso && selecionadas < CHAVES_EXPERIMENTO){
        if(!posicionarArquivo(arquivo, 0, SEEK_SET)) sucesso = 0;
        // Varre o ficheiro do início preenchendo as chaves restantes
        for(int i = 0; sucesso && i < quantidade && selecionadas < CHAVES_EXPERIMENTO; i++){
            if(fread(&registro, sizeof(registro), 1, arquivo) != 1){
                sucesso = 0;
                break;
            }
            if(!chaveRepetida(registro.chave, chaves, selecionadas))
                chaves[selecionadas++] = registro.chave;
        }
        if(!sucesso) fprintf(stderr, "Erro ao completar a selecao de chaves.\n");
    }
    if(sucesso && selecionadas != CHAVES_EXPERIMENTO){
        fprintf(stderr, "O arquivo nao contem 20 chaves distintas.\n");
        sucesso = 0;
    }

    // Fecha o ficheiro após extrair as 20 chaves do teste
    if(fclose(arquivo) != 0){
        fprintf(stderr, "Erro ao fechar %s.\n", nome);
        sucesso = 0;
    }
    return sucesso; // Retorna 1 se selecionou com sucesso as 20 chaves
}
