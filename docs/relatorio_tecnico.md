# UNIVERSIDADE FEDERAL DO ABC — UFABC
**Centro de Engenharia, Modelagem e Ciências Sociais Aplicadas (CECS)**  
**Disciplina:** MCTA028-15 — Programação Estruturada | **Quadrimestre:** 2026.3  
**Docentes:** Prof. Roberto Sadao Yokoyama e Prof. Luneque Del Rio de Souza e Silva Junior  

---

# RELATÓRIO TÉCNICO DE IMPLEMENTAÇÃO
## GarchY: Compactador e Descompactador Huffman em ANSI C

### Integrantes da Equipe e Responsabilidades:
* **Integrante 1 (jrfeltran):** Implementação da fila de prioridade Min-Heap, construção da árvore de Huffman e geração dos códigos binários prefixados.
* **Integrante 2 (RAFAELMX30):** Implementação da manipulação de bits (BitStream), serialização do cabeçalho compacto com codificação varint e rotina de compressão em duas passadas.
* **Integrante 3 (CWAF-Renan):** Implementação da rotina de descompressão bit a bit e algoritmo de integridade por checksum CRC-32 (IEEE 802.3).
* **Integrante 4 (Rodrigo Araújo — Martter):** Implementação do menu interativo no terminal, tratamento e normalização de caminhos de arquivos, instrumentação de métricas temporais e espaciais, execução da suíte de testes comparativos contra o gzip, elaboração do leia-me.txt e redação deste Relatório Técnico.

---

## 1. Requisitos Implementados e Funcionais

O projeto **GarchY** foi implementado estritamente no padrão formal da linguagem C (ANSI C / C99), com compilação livre de erros e advertências sob os parâmetros `-Wall -Wextra` do compilador GCC.

1. **Interface de Terminal Amigável:** Menu interativo estruturado com opções de:
   * `1) Comprimir arquivo (.abcz)`
   * `2) Descomprimir arquivo (.abcz)`
   * `3) Visualizar tabela de códigos de Huffman e frequências`
   * `4) Executar teste de integridade rápido (Roundtrip)`
   * `5) Sobre o projeto e equipe`
   * `0) Sair`
2. **Tratamento Robusto de Caminhos:** Suporte pleno a caminhos relativos e absolutos, sanitização de quebras de linha residuais (`\r` e `\n`), remoção automática de aspas (permitindo arrastar arquivos para dentro do terminal) e validação de existência prévia dos arquivos.
3. **Extensão Personalizada:** O compressor gera por padrão arquivos com extensão `.abcz`. Na descompressão, o programa sugere o nome original expurgando a extensão e permite ao usuário especificar qualquer destino desejado.
4. **Preservação de Integridade e Arquivos Originais:** Os arquivos originais são estritamente preservados em modo somente-leitura. Em caso de qualquer erro de I/O ou alocação, arquivos parciais corrompidos são automaticamente eliminados pelo sistema.
5. **Métricas de Desempenho em Tempo Real:** Medição e exibição instantânea dos tamanhos em Bytes (e formatado em KB/MB), Razão de Compressão ($R = \frac{S_c}{S_o}$), percentual de economia de espaço, tempo cronometrado via `clock()` e velocidade de processamento em MB/s.

---

## 2. Fundamentação Conceitual dos Algoritmos

### 2.1 Teoria da Informação e Entropia de Shannon
A compressão estatística sem perdas encontra seu limite teórico na Entropia de Shannon:
$$H(X) = - \sum_{i=1}^{n} P(x_i) \log_2 P(x_i)$$
Em um arquivo comum, os bytes apresentam probabilidades desiguais de ocorrência. Em vez de utilizar 8 bits fixos para cada símbolo, a Codificação de Huffman (1952) gera **códigos binários de tamanho variável com propriedade de prefixo** (*prefix-free codes*). Símbolos mais frequentes recebem sequências menores de bits, minimizando o comprimento médio esperado da mensagem:
$$L = \sum_{i=1}^{n} P(x_i) \cdot l(x_i) \ge H(X)$$

### 2.2 Árvore de Huffman e Fila de Prioridade (Min-Heap)
A árvore de Huffman é uma árvore binária estrita construída de baixo para cima (*bottom-up*) em tempo $O(N \log N)$ com auxílio de uma Fila de Prioridade (Min-Heap):
1. Cada byte presente no arquivo é inicializado como um nó folha contendo sua frequência.
2. Todas as folhas são inseridas no Min-Heap.
3. A cada passo, os dois nós com as menores frequências são extraídos da fila.
4. Cria-se um novo nó pai cuja frequência é a soma das frequências dos dois filhos.
5. O novo nó é reinserido no heap.
6. O processo repete-se até restar apenas um nó no heap, que será a raiz da árvore.

Para garantir determinismo estrito na construção da árvore (permitindo reconstruí-la na descompressão a partir apenas das frequências, sem salvar ponteiros no disco), adotou-se um critério determinístico de desempate: nós com mesma frequência são desempatados pelo contador sequencial de criação (`ordem`).

### 2.3 Verificação de Integridade por Checksum CRC-32
Para detecção de corrupção ou adulteração de dados, implementou-se o algoritmo *Cyclic Redundancy Check* de 32 bits (CRC-32) padrão IEEE 802.3 com o polinômio `0xEDB88320` em representação invertida. O CRC-32 opera de forma incremental em tempo linear $O(N)$ utilizando uma tabela de dispersão de 256 entradas pré-calculada, detectando qualquer inversão ou perda de bits.

---

## 3. Decisões de Projeto e Estruturas de Dados (TADs)

O código foi rigorosamente modularizado em arquivos de cabeçalho (`include/`) e implementação (`src/`):

### 3.1 TAD Min-Heap (`include/heap.h`, `src/heap.c`)
Implementado sobre um vetor de ponteiros com capacidade fixa de 256 elementos (quantidade máxima de símbolos de 1 byte). O critério de ordenação e desempate no heap garante estabilidade:

```c
static int compara(const HuffNode *a, const HuffNode *b) {
    if (a->freq < b->freq) return -1;
    if (a->freq > b->freq) return 1;
    return a->ordem - b->ordem; /* Desempate pela ordem de inserção */
}
```

### 3.2 Manipulação de Bits (`include/bitio.h`, `src/bitio.c`)
Como os códigos de Huffman possuem comprimentos fracionários de byte, implementou-se o `BitWriter` e o `BitReader`, acumulando bits em buffers de 1 byte enviados ao disco quando cheios (MSB primeiro):

```c
int bw_put_bit(BitWriter *bw, int bit) {
    bw->buf = (unsigned char)((bw->buf << 1) | (bit ? 1 : 0));
    bw->n_bits++;
    if (bw->n_bits == 8) {
        if (fputc(bw->buf, bw->f) == EOF) return -1;
        bw->buf = 0; bw->n_bits = 0;
    }
    return 0;
}
```

### 3.3 Formato do Cabeçalho e Serialização Compacta (`src/cabecalho.c`)
O formato `.abcz` grava:
* Assinatura `ABCZ` (4 bytes)
* Versão do formato (1 byte)
* Tamanho do arquivo original (8 bytes, little-endian)
* Checksum CRC-32 (4 bytes)
* Quantidade de bytes distintos e tabela de frequências compactada com codificação **varint** (7 bits de dado por byte com o bit 7 indicando continuação), evitando inflar arquivos pequenos.

### 3.4 Compressão e Descompressão em 2 Passadas (`src/compressao.c`, `src/descompressao.c`)
Para processar arquivos de qualquer porte sem estourar a memória RAM, a leitura é feita em blocos de 64 KB (`TAM_BLOCO`). Na 1ª passada, o arquivo é lido para contabilizar frequências e calcular o CRC-32. Após a geração dos códigos, a 2ª passada grava os bits codificados. Na descompressão, a árvore é percorrida bit a bit até alcançar nós folhas, emitindo o byte original e finalizando exatamente quando o tamanho original é atingido.

---

## 4. Casos de Teste e Mensuração de Desempenho

O programa foi testado sob três cenários de redundância com arquivos de **500 KB (512.000 Bytes)**, comparando o desempenho com a ferramenta padrão comercial **gzip**:

* **Cenário 1 — Alta Redundância:** Padrão homogêneo com repetição de blocos e poucos símbolos distintos.
* **Cenário 2 — Média Redundância:** Texto contínuo em linguagem natural com vocabulário empírico representativo.
* **Cenário 3 — Baixa Redundância:** Bytes pseudo-aleatórios com distribuição uniforme (alta entropia, ~8 bits/símbolo).

### Tabela 1: Comparativo de Eficiência Espacial e Integridade

| Cenário de Teste | Tamanho Original | Huffman (GarchY) | Razão ($R = \frac{S_c}{S_o}$) | Economia (%) | Gzip Comercial | Razão Gzip | Integridade CRC-32 |
|---|---|---|---|---|---|---|---|
| **Alta Redundância** | 512.000 B | 231.263 B | **0,4517** | **54,83%** | 1.578 B | 0,0031 | **[OK] 100% Íntegro** |
| **Média Redundância** | 512.000 B | 283.361 B | **0,5534** | **44,66%** | 5.604 B | 0,0109 | **[OK] 100% Íntegro** |
| **Baixa Redundância** | 512.000 B | 512.787 B | **1,0015** | **-0,15%** | 512.196 B | 1,0004 | **[OK] 100% Íntegro** |

### Tabela 2: Eficiência Temporal de Compressão e Descompressão

| Cenário de Teste | Tempo Comp. Huffman | Tempo Comp. Gzip | Tempo Decomp. Huffman | Tempo Decomp. Gzip |
|---|---|---|---|---|
| **Alta Redundância** | 51,10 ms | 12,06 ms | 48,65 ms | 23,47 ms |
| **Média Redundância** | 59,73 ms | 17,25 ms | 49,37 ms | 121,85 ms |
| **Baixa Redundância** | 36,83 ms | 144,73 ms | 56,70 ms | 161,16 ms |

### Análise Crítica dos Resultados:
1. **Desempenho do Huffman:** O GarchY obteve reduções substanciais de 54,83% e 44,66% em dados textuais/estruturados com tempos inferiores a 60 ms, atingindo o limite de entropia esperado para modelos estáticos por símbolo.
2. **Diferença para o Gzip:** O gzip emprega o algoritmo Deflate (LZ77 + Huffman). O LZ77 busca repetições de cadeias de caracteres em uma janela deslizante e substitui frases inteiras por referências `(distância, comprimento)`. Por isso, em textos com repetições longas, o gzip atinge taxas de compressão muito superiores. Já o algoritmo de Huffman isolado atua sobre a frequência individual de bytes, sendo imbatível em simplicidade e baixo uso de memória.
3. **Baixa Redundância:** Em ruído aleatório, a entropia máxima impede compressão sem perdas. Ambos sofreram leve expansão pelos cabeçalhos (+0,15% no Huffman e +0,04% no gzip).
4. **Corretude:** Em 100% dos testes, a integridade foi confirmada byte a byte e pelo CRC-32.

---

## 5. Participação e Entregas Individuais da Equipe

* **Integrante 1 (jrfeltran):** TAD Min-Heap, Árvore de Huffman e geração de códigos prefixados (`heap.c`, `huffman.c`).
* **Integrante 2 (RAFAELMX30):** TAD BitStream, formato de cabeçalho compacto varint e rotina de compressão em 2 passadas (`bitio.c`, `cabecalho.c`, `compressao.c`).
* **Integrante 3 (CWAF-Renan):** Rotina de descompressão bit a bit e algoritmo de integridade por checksum CRC-32 (`descompressao.c`, `checksum.c`).
* **Integrante 4 (Rodrigo Araújo — Martter):** Menu interativo no terminal (`main.c`), normalização de caminhos de arquivos, cronometragem com `clock()`, suíte de testes comparativos contra o gzip (`benchmark_gzip.py`), instruções de compilação no `leia-me.txt` e redação deste Relatório Técnico.
