# TP1 — Estrutura de Dados II: Pesquisa Externa

Este projeto implementa quatro métodos de pesquisa externa em C:

1. Acesso Sequencial Indexado
2. Árvore Binária de Pesquisa
3. Árvore B
4. Árvore B*

O programa principal recebe parâmetros pela linha de comando, cria o arquivo de dados quando necessário e executa a pesquisa escolhida.

## 1. Estrutura e requisitos

Arquivos-fonte utilizados na compilação do programa principal:

```text
main.c
acessoSequencial.c
arquivo.c
arvoreb.c
arvorebestrela.c
arvorebinaria.c
suporte.c
tempo.c
```

Os arquivos de cabeçalho (`.h`) também devem permanecer na pasta do projeto, conforme as referências `#include` existentes nos códigos.

O arquivo `gerador.c`, caso esteja presente, é um programa separado e **não deve ser incluído** na compilação de `pesquisa`. Não é necessário executá-lo antes da pesquisa, pois o programa principal já pode gerar os dados automaticamente.

Para compilar o projeto, é necessário instalar o **GCC**. Se você já possui um executável adequado ao seu sistema, não precisa compilar novamente.

## 2. Parâmetros de execução

### Pesquisa individual

```text
pesquisa <metodo> <quantidade> <situacao> <chave> [-P]
```

| Parâmetro | Descrição |
| --- | --- |
| `metodo` | `1` = Acesso Sequencial Indexado; `2` = Árvore Binária; `3` = Árvore B; `4` = Árvore B*. |
| `quantidade` | Quantidade de registros do arquivo de dados. |
| `situacao` | `1` = crescente; `2` = decrescente; `3` = aleatória. |
| `chave` | Chave que será pesquisada. |
| `-P` | Opcional: solicita a impressão dos registros na saída. |

**Atenção:** o método 1 (Acesso Sequencial Indexado) aceita apenas as situações `1` e `2`. Para esse método, não utilize a situação `3` (aleatória).

### Testes automáticos

É possível substituir a chave por `-t` para executar 20 pesquisas automáticas com chaves existentes no arquivo:

```text
pesquisa <metodo> <quantidade> <situacao> -t [-P]
```

## 3. Windows

### Opção A — Compilar e executar do zero

1. Instale o GCC para Windows (por exemplo, com o **MSYS2/MinGW-w64**) e configure o `PATH`, se necessário.
2. Abra o **PowerShell** na pasta que contém os arquivos do projeto.
3. Verifique a instalação do compilador:

   ```powershell
   gcc --version
   ```

4. Compile todos os módulos do programa principal:

   ```powershell
   gcc main.c acessoSequencial.c arquivo.c arvoreb.c arvorebestrela.c arvorebinaria.c suporte.c tempo.c -o pesquisa.exe
   ```

5. Execute uma pesquisa, por exemplo, usando o método 1, 100 registros crescentes e chave 50:

   ```powershell
   .\pesquisa.exe 1 100 1 50 -P
   ```

Para executar os 20 testes automáticos:

```powershell
.\pesquisa.exe 1 100 1 -t
```

### Opção B — Utilizar o `pesquisa.exe` já compilado

Se a pasta do projeto já contém `pesquisa.exe`, não é necessário instalar o GCC nem compilar o código para executá-lo.

1. Abra o PowerShell na pasta do executável.
2. Execute diretamente:

   ```powershell
   .\pesquisa.exe 3 1000 3 500
   ```

   Ou, para realizar os testes automáticos:

   ```powershell
   .\pesquisa.exe 4 1000 3 -t -P
   ```

**Observação:** o `.exe` precisa ser compatível com o Windows e com a arquitetura do computador. Dependendo de como foi compilado, podem ser necessárias bibliotecas de execução adicionais.

## 4. Linux

### Opção A — Compilar e executar do zero

1. Abra o terminal na pasta do projeto.
2. Instale o GCC, caso ainda não esteja instalado. Em distribuições Ubuntu ou Debian:

   ```bash
   sudo apt update
   sudo apt install build-essential
   ```

3. Compile o programa para Linux:

   ```bash
   gcc main.c acessoSequencial.c arquivo.c arvoreb.c arvorebestrela.c arvorebinaria.c suporte.c tempo.c -o pesquisa
   ```

4. Execute o programa:

   ```bash
   ./pesquisa 1 100 1 50 -P
   ```

Para executar os testes automáticos:

```bash
./pesquisa 2 1000 3 -t
```

### Opção B — Já possuir apenas o `pesquisa.exe` do Windows

O arquivo `pesquisa.exe` foi compilado para Windows e **não é um executável Linux nativo**. Existem duas possibilidades:

- **Recomendado:** caso os arquivos `.c` e `.h` estejam disponíveis, compile-os no Linux usando o comando da opção A. O resultado será o executável nativo `pesquisa`.
- **Alternativa:** se houver somente o `pesquisa.exe`, tente executá-lo com o **Wine**, desde que esteja instalado e seja compatível com esse executável:

  ```bash
  wine pesquisa.exe 1 100 1 50 -P
  ```

O Wine não garante a execução de todos os programas Windows. Se você possuir um executável **Linux nativo** chamado `pesquisa`, execute-o com `./pesquisa`, sem necessidade de recompilar; caso falte permissão:

```bash
chmod +x pesquisa
./pesquisa 1 100 1 50
```

## 5. Exemplos de execução

Os exemplos abaixo usam o executável Windows. No Linux, substitua `.\pesquisa.exe` por `./pesquisa`.

| Objetivo | Comando |
| --- | --- |
| Acesso Sequencial Indexado, 100 registros crescentes, chave 50 | `.\pesquisa.exe 1 100 1 50` |
| Árvore Binária, 1.000 registros aleatórios, chave 500 | `.\pesquisa.exe 2 1000 3 500` |
| Árvore B, 10.000 registros decrescentes, chave 800 | `.\pesquisa.exe 3 10000 2 800` |
| Árvore B*, 10.000 registros aleatórios, chave 800 | `.\pesquisa.exe 4 10000 3 800` |
| Testar 20 chaves com Árvore B | `.\pesquisa.exe 3 10000 3 -t` |
| Imprimir registros e executar testes automáticos | `.\pesquisa.exe 4 1000 3 -t -P` |

## 6. Arquivos de dados

O programa pode gerar automaticamente os arquivos binários de dados quando necessário. Não é preciso criá-los manualmente ou executar `gerador.c` antes de iniciar a pesquisa.

Os arquivos gerados seguem o padrão:

```text
dados_<quantidade>_<situacao>.bin
```

Por exemplo, uma execução com `100` registros na situação `1` utiliza o arquivo `dados_100_1.bin`.

Os arquivos de dados são criados ou lidos em relação ao diretório de execução do programa. Portanto, execute os comandos preferencialmente a partir da pasta do projeto.

## 7. Problemas comuns

- **`gcc` não reconhecido:** instale o GCC e confira sua configuração no `PATH`.
- **Executável não encontrado:** abra o terminal na pasta correta e confira o nome do arquivo gerado.
- **`Permission denied` no Linux:** verifique as permissões do executável Linux (`chmod +x pesquisa`).
- **Erro ao executar `.exe` diretamente no Linux:** compile uma versão Linux ou utilize Wine.
- **Erro de múltiplas definições de `main`:** não inclua `gerador.c` na compilação do programa principal.
- **Erro de arquivo `.h` não encontrado no Linux:** confira os nomes dos arquivos e a capitalização das letras, pois Linux diferencia maiúsculas e minúsculas.
- **Situação inválida para o método 1:** utilize `1` (crescente) ou `2` (decrescente), nunca `3` (aleatória).
