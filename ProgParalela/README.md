# Programação Paralela — Seminários

Este material reúne dois estudos apresentados na disciplina de **Programação Paralela (BCC447)** da Universidade Federal de Ouro Preto (UFOP), abordando comunicação, segurança e tolerância a falhas em sistemas distribuídos.

## FastBFT

O **FastBFT** é um protocolo de consenso tolerante a falhas bizantinas que busca superar limitações de desempenho de soluções tradicionais, como o PBFT. Utiliza **ambientes de execução confiáveis (TEEs)** e **compartilhamento de segredos** para reduzir verificações criptográficas e alcançar comunicação de ordem **O(n)** no caminho rápido.

O estudo também aborda o tratamento de falhas, a troca de líder (*view change*) e limitações relacionadas à dependência de hardware confiável.

## Server-Based Architecture

A **arquitetura baseada em servidor** organiza o aprendizado distribuído por meio de um servidor central que coordena os *workers* e agrega seus gradientes para atualizar o modelo global.

O estudo destaca riscos como ponto único de falha, envio de atualizações maliciosas e vazamento de informações. Também apresenta mecanismos de agregação mais robustos, como **mediana**, **média aparada (*trimmed mean*)** e **Multi-Krum**, para reduzir a influência de contribuições incorretas.

## Objetivo

Apresentar os princípios, as vantagens e os desafios dessas abordagens, com foco em **desempenho, confiabilidade e segurança** de sistemas distribuídos.
