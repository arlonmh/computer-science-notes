## Algoritmo de Dijkstra

[Vídeo no YouTube](https://youtu.be/Z0RHwEn0RP4?si=C2q9xC44q_VueIEx)

### Resumo
O vídeo apresenta, passo a passo, o funcionamento do **Algoritmo de Dijkstra**, utilizado para encontrar o menor caminho entre vértices de um grafo com arestas de pesos não negativos.

Durante a resolução, são utilizadas duas tabelas:

* **DT (Distância Total):** armazena a menor distância conhecida entre o vértice inicial e cada um dos demais vértices.
* **ROT (Rótulo/Predecessor):** armazena o vértice predecessor utilizado para chegar até cada vértice pelo menor caminho encontrado.

O algoritmo realiza sucessivas iterações, analisando os vértices e atualizando os valores de DT e ROT sempre que um caminho de menor custo é encontrado.

Ao final da execução, é possível determinar tanto o **menor caminho** entre os vértices quanto o **custo total desse caminho**.

### Passo a Passo

1. Escolha o vértice de origem.

2. Inicialize sua distância como `0` e a distância dos demais vértices como infinito.

3. Analise os vértices vizinhos do vértice atual.

4. Para cada vizinho, calcule:

   `Distância atual + peso da aresta`

5. Compare o resultado com o valor atualmente armazenado em **DT**.

6. Caso o novo valor seja menor, atualize:

   * o valor de **DT**;
   * o valor de **ROT**, indicando o vértice predecessor.

7. Após analisar todos os vizinhos, marque o vértice atual como visitado.

8. Entre os vértices ainda não visitados, escolha aquele que possui o menor valor em **DT**.

9. Repita o processo até que todos os vértices necessários tenham sido analisados.

10. Para encontrar o caminho final, utilize a tabela **ROT**, partindo do destino e seguindo os predecessores até chegar ao vértice de origem.

## Ferramentas Utilizadas
[Excalidraw](https://excalidraw.com/) Para demonstração do algoritimo. <br>
[OBS](https://obsproject.com/pt-br) Para gravações.
