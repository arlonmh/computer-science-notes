#ifndef SUPORTE_H
#define SUPORTE_H

#include "registro.h"

typedef int (*FuncaoPesquisa)(void *contexto, int chave, Registro *resultado, Medidas *medidas);
int rodarPesquisas(const char *nome, int quantidade, int chave, int modoTeste,
                  const Medidas *construcao, FuncaoPesquisa pesquisar, void *contexto);
void imprimirMedidas(const char *etapa, const Medidas *medidas);
void imprimirRegistro(const Registro *registro);

#endif
