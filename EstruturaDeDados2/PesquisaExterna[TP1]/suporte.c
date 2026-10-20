#include "suporte.h"
#include "arquivo.h"
#include <stdio.h>

void imprimirMedidas(const char *etapa, const Medidas *medidas){
    // Imprime o nome da etapa, número de leituras do disco, comparações de chaves e o tempo exato em segundos
    printf("%s | Leituras: %llu | Comparacoes: %llu | Tempo: %.9f s\n",
           etapa, medidas->leituras, medidas->comparacoes, medidas->tempo);
}

void imprimirRegistro(const Registro *registro){
    printf("Chave: %d | Dado1: %ld\n", registro->chave, registro->dado1);
    // Imprime o campo dado2 delimitado pelos 5000 caracteres definidos na struct
    printf("Dado2: %.*s\n", (int)sizeof(registro->dado2), registro->dado2);
}

int rodarPesquisas(const char *nome, int quantidade, int chave, int modoTeste,
                  const Medidas *construcao, FuncaoPesquisa pesquisar, void *contexto){
    Registro resultado; // Buffer para guardar o registro trazido do disco
    Medidas medidas = {0}, total = {0}; // Zera contadores locais e acumuladores
    int encontrado;

    // Imprime primeiro as estatísticas registradas na construção da Árvore B (tempo de criação e leituras em RAM)
    imprimirMedidas("Construcao", construcao);

    // Fase 1: Pesquisa de Apenas 1 Chave Específica
    if(!modoTeste){
        // Chama a função de busca da Árvore B ('buscarRegistroB' do arvoreb_2.c)
        encontrado = pesquisar(contexto, chave, &resultado, &medidas);
        if(encontrado < 0) return 0; // Erro de execução


        if(encontrado){
            printf(">>> CHAVE ENCONTRADA <<<\n");
            // Exibe os dados do registro trazido do disco
            imprimirRegistro(&resultado);
        }else{
            printf(">>> CHAVE NAO ENCONTRADA <<<\n");
        }

        // Exibe as leituras, comparações e tempo gasto EXCLUSIVAMENTE na busca dessa chave
        imprimirMedidas("Pesquisa", &medidas);
        return 1; // Finaliza Fase 1 com sucesso
    }

    // Fase 2: Experimento com 20 Chaves Distintas
    int chaves[CHAVES_EXPERIMENTO]; // Vetor para 20 chaves inteiras

    // Seleciona as 20 chaves do ficheiro sem cronometrar na pesquisa da Árvore B
    if(!selecionarChaves(nome, quantidade, chaves)) return 0;

    printf("Experimento: 20 chaves distintas existentes; selecao fora das medidas.\n");

    // Loop que executa a pesquisa da Árvore B exatamente 20 vezes
    for(int i = 0; i < CHAVES_EXPERIMENTO; i++){
        // Executa a busca da i-ésima chave na Árvore B e lê o registro no disco
        encontrado = pesquisar(contexto, chaves[i], &resultado, &medidas);
        if(encontrado < 0) return 0; // Cancela se der erro de I/O

        // Exibe o resultado individual de cada uma das 20 pesquisas
        printf("Chave %d | %s | Leituras: %llu | Comparacoes: %llu | Tempo: %.9f s\n",
               chaves[i], encontrado ? "Encontrada" : "Nao encontrada",
               medidas.leituras, medidas.comparacoes, medidas.tempo);

        
        if(!encontrado){
            fprintf(stderr, "Falha do metodo: a chave existente %d nao foi encontrada.\n", chaves[i]);
            return 0; // Alerta de falha de lógica na Árvore B
        }

        // Acumula os valores de cada execução para calcular a média ao final
        total.leituras += medidas.leituras;
        total.comparacoes += medidas.comparacoes;
        total.tempo += medidas.tempo;
    }

    // Imprime as MÉDIAS finais exigidas no enunciado dividindo os acumuladores por 20 (CHAVES_EXPERIMENTO)
    printf("Media | Leituras: %.6f | Comparacoes: %.6f | Tempo: %.9f s\n",
           (double)total.leituras / CHAVES_EXPERIMENTO,
           (double)total.comparacoes / CHAVES_EXPERIMENTO,
           total.tempo / CHAVES_EXPERIMENTO);
           
    return 1; // Finaliza a Fase 2 com sucesso
}
