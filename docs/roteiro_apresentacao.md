# GarchY — Roteiro de Apresentação

**Disciplina:** MCTA028-15 — Programação Estruturada · UFABC · 2026.3  
**Docentes:** Prof. Roberto Sadao Yokoyama · Prof. Luneque Del Rio de Souza e Silva Junior  
**Duração estimada:** 22–25 minutos

---

> **Como usar este roteiro**  
> Cada seção pertence a um integrante. O texto em bloco de código é a fala sugerida — leia ou adapte em voz alta. As notas em itálico são orientações de palco. Os blocos `bash` são os comandos a digitar no terminal durante a demo.

---

## [ 01 ] RODRIGO — Abertura e Demo ao Vivo

**Responsabilidade declarada:** Menu interativo, métricas, tratamento de caminhos, benchmark, relatório  
**Tempo:** ~5 min

### Objetivo

Apresentar o problema, o time e fazer a compressão acontecer na frente dos professores antes de qualquer explicação teórica. O resultado concreto ancora tudo que vem depois.

### Fala

```text
Bom dia. Nosso projeto se chama GarchY — um compactador e
descompactador de arquivos escrito inteiramente em ANSI C,
sem nenhuma dependência externa além da biblioteca padrão.

O problema que resolvemos: todo arquivo usa 8 bits por byte,
independente do conteúdo. Se um texto tem a letra 'a' aparecendo
40% do tempo, ela ainda ocupa os mesmos 8 bits que a letra 'z',
que aparece menos de 1%. O algoritmo de Huffman resolve isso:
bytes frequentes ganham códigos curtos, raros ganham códigos longos.
No total, o arquivo encolhe.

Antes de entrar nos detalhes, vou mostrar isso acontecendo.
```

*Abrir o terminal. Navegar até o diretório do projeto.*

```bash
./garchy
```

*Escolher opção 1 (Comprimir). Usar o `README.md` como entrada. Aceitar o nome de saída sugerido.*

```text
Veja o resultado: o README.md tem 6,3 KB. O arquivo .abcz saiu
com 3,8 KB — uma redução de 40%. O CRC-32 calculado está
registrado no cabeçalho para verificação posterior.
Tempo de execução abaixo de 1 ms.

A equipe foi dividida em quatro frentes que se encaixam em sequência:
José Ricardo construiu as estruturas de dados centrais.
Rafael implementou a escrita dos bits e o formato do arquivo.
Eu fechei a interface e os testes.
E o Renan implementou a descompressão e a verificação de integridade —
o que garante que o arquivo que acabou de ser comprimido volta
exatamente igual.
```

### Transição

```text
Para chegar a esses números, precisamos de duas estruturas
construídas do zero: um Min-Heap e uma Árvore de Huffman.
O José Ricardo vai explicar como.
```

---

## [ 02 ] JOSÉ RICARDO — Min-Heap e Árvore de Huffman

**Responsabilidade declarada:** `heap.c`, `huffman.c`  
**Tempo:** ~6 min

### Objetivo

Explicar as duas estruturas no núcleo do algoritmo. O Min-Heap não é detalhe de implementação — ele determina a qualidade e a reprodutibilidade da árvore. O argumento do desempate determinístico é o ponto que diferencia um Huffman correto de um que falha na descompressão.

### Fala

```text
Minha parte foi construir o coração do GarchY: a fila de
prioridade e a árvore que gera os códigos.

O Min-Heap é um vetor de até 256 ponteiros — exatamente o número
máximo de bytes distintos em um arquivo de qualquer formato.
A propriedade que mantemos: o nó pai sempre tem frequência menor
ou igual à dos filhos.

Inserir um nó: colocamos no final do vetor, subimos comparando
com o pai — índice (i-1)/2 — até restaurar a propriedade.
Remover o mínimo: extraímos a raiz, colocamos o último elemento
no lugar, descemos comparando com os dois filhos até restaurar.
Ambas as operações são O(log N), com N máximo de 256.
```

*Abrir `src/heap.c`. Mostrar `heap_inserir` e `heap_remover_min`.*

```text
Um detalhe crítico: quando dois nós têm a mesma frequência,
o critério de desempate é o campo 'ordem' — um contador que
incrementa a cada nó criado. Isso garante que a mesma árvore
seja reconstruída identicamente durante a descompressão,
a partir apenas das frequências salvas no cabeçalho.
Sem isso, árvores com empates seriam ambíguas.

A construção da árvore segue o algoritmo clássico de Huffman:
cada byte com frequência maior que zero vira uma folha.
Enquanto o heap tem mais de um nó, extraímos os dois menores,
criamos um nó pai cuja frequência é a soma das duas,
e reinserimos. O único nó que sobra é a raiz.

Um caso especial: arquivo com um único valor de byte distinto.
Nesse caso criamos uma raiz artificial para a folha única —
sem isso, a DFS não teria para onde ir.
```

*Abrir `src/huffman.c`. Mostrar `huff_construir_arvore`.*

```text
Geração dos códigos: DFS recursiva na árvore. Ao ir para a
esquerda, acrescentamos 0 ao caminho; à direita, 1. Quando
chegamos a uma folha, o caminho percorrido é o código daquele byte.
O comprimento médio resultante é ótimo para a distribuição
estatística deste arquivo específico.
```

*Mostrar `huff_gerar_codigos`. Opcionalmente: na opção 3 do menu, visualizar os códigos gerados para o README.md.*

### Transição

```text
Com os códigos prontos — sequências de bits de comprimento variável —
o Rafael implementou como esses bits chegam fisicamente ao disco.
```

---

## [ 03 ] RAFAEL — BitStream, Cabeçalho e Rotina de Compressão

**Responsabilidade declarada:** `bitio.c`, `cabecalho.c`, `compressao.c`  
**Tempo:** ~6 min

### Objetivo

Explicar três camadas que o algoritmo exige mas que são invisíveis no resultado final: a escrita bit a bit, o formato binário compacto, e o pipeline de duas passadas que processa arquivos grandes sem estourar memória.

### Fala

```text
Minha responsabilidade foi implementar a camada que escreve e
lê os bits reais — o BitStream — e o formato do arquivo .abcz.

O problema prático: um código Huffman não tem comprimento
múltiplo de 8 bits. O byte 'a' pode ser representado por 3 bits,
o byte 'z' por 12. Não dá para simplesmente escrever byte a byte.

O BitWriter mantém um buffer de 1 byte e um contador de bits usados.
Cada bit entra no buffer pelo MSB. Quando os 8 estão cheios,
o byte é escrito no arquivo e o buffer é zerado.
No final da compressão, bw_flush completa o byte parcial com
zeros de padding — esses bits extras serão ignorados na
descompressão, que para pelo tamanho original.
O BitReader faz o inverso: lê um byte por vez e entrega os bits
um a um da esquerda para a direita.
```

*Abrir `src/bitio.c`. Mostrar `bw_put_bit` e `br_get_bit`.*

```text
O formato .abcz tem cabeçalho estruturado da seguinte forma:
quatro bytes de assinatura mágica "ABCZ" — para validar o arquivo
antes de tentar decodificar qualquer coisa —, um byte de versão,
oito bytes com o tamanho original em little-endian,
quatro bytes de CRC-32, e a tabela de frequências.

A tabela de frequências usa codificação varint: só gravamos os
bytes que realmente aparecem no arquivo. Cada frequência é
serializada com 7 bits de dado por byte, usando o bit 7 como
flag de continuação. Isso evita gravar sempre 256×8=2048 bytes
fixos quando o arquivo usa poucos símbolos distintos.
```

*Abrir `src/cabecalho.c`. Mostrar `put_varint` e a estrutura de `cabecalho_escrever`.*

```text
A compressão acontece em duas passadas sobre o arquivo de entrada.
Por que duas? O cabeçalho precisa do CRC-32 e do tamanho original,
que só conhecemos depois de ler o arquivo inteiro.
E arquivos grandes não cabem na memória.

Primeira passada: lemos em blocos de 64 KB, contamos frequências,
acumulamos o CRC-32 incrementalmente e somamos o tamanho.
Com isso, construímos a árvore e escrevemos o cabeçalho.
Segunda passada: voltamos ao início do arquivo, substituímos
cada byte pelo seu código, escrevemos bit a bit.

Se qualquer erro de I/O ocorre, o arquivo .abcz parcial é
deletado automaticamente — nunca deixamos um comprimido quebrado.
```

*Abrir `src/compressao.c`. Mostrar as duas passadas de leitura e o `goto fim` com `remove`.*

### Transição

```text
O arquivo .abcz está no disco. O Renan vai mostrar como lemos
esses bits de volta — e como garantimos que o resultado é
byte a byte idêntico ao original.
```

---

## [ 04 ] RENAN — Descompressão, CRC-32 e Resultados

**Responsabilidade declarada:** `descompressao.c`, `checksum.c`  
**Tempo:** ~5 min + encerramento

### Objetivo

Fechar o ciclo técnico com a descompressão, mostrar como o CRC-32 detecta qualquer corrupção, e apresentar os resultados comparativos. O encerramento deve ser limpo — uma frase de síntese e abertura para perguntas.

### Fala

```text
Minha parte fecha o ciclo: dado um arquivo .abcz,
como recuperamos exatamente o original?

A descompressão começa lendo o cabeçalho e reconstruindo
a mesma árvore de Huffman — usando as frequências salvas e
o mesmo critério de desempate determinístico que o José Ricardo
implementou. A partir daí, o BitReader entrega um bit por vez:
0 vai para o filho esquerdo, 1 para o direito.
Quando chegamos a uma folha, emitimos aquele byte e voltamos à raiz.

Repetimos exatamente 'tam_original' vezes — esse número no cabeçalho
é o único critério de parada. Não há separadores no bitstream.
Os zeros de padding do último byte são automaticamente ignorados.
```

*Abrir `src/descompressao.c`. Mostrar o loop `for (i = 0; i < cab.tam_original; i++)`.*

```text
A verificação de integridade usa CRC-32 padrão IEEE 802.3,
com o polinômio 0xEDB88320 em representação invertida.
Implementei uma tabela de 256 entradas pré-calculada.
A cada byte emitido durante a descompressão, atualizamos
o CRC com uma única operação:

    crc = tabela[(crc ^ byte) & 0xFF] ^ (crc >> 8)

Ao final, comparamos com o valor armazenado no cabeçalho.
Qualquer bit invertido, byte truncado ou arquivo modificado
produz um CRC diferente — detectamos e avisamos.

Uma decisão deliberada: em erro de checksum, o arquivo
recuperado é mantido em disco. Pode ser útil para investigação.
Nos outros erros — formato inválido, falha de I/O — o arquivo
parcial é deletado automaticamente.
```

*Abrir `src/checksum.c`. Mostrar `montar_tabela` e a operação incremental em `checksum_crc32`.*

*Mostrar a opção 4 do menu (Roundtrip) para o README.md. O programa comprimir, descomprimir e comparar byte a byte ao vivo.*

### Resultados

```text
Testamos em três cenários com arquivos de 500 KB cada,
comparando com o gzip.
```

| Cenário              | Original  | GarchY    | Razão  | Economia | Gzip      | Integridade       |
|----------------------|-----------|-----------|--------|----------|-----------|-------------------|
| Alta Redundância     | 512.000 B | 231.263 B | 0,4517 | 54,83%   | 1.578 B   | CRC-32 ✓          |
| Média Redundância    | 512.000 B | 283.361 B | 0,5534 | 44,66%   | 5.604 B   | CRC-32 ✓          |
| Baixa Redundância    | 512.000 B | 512.787 B | 1,0015 | -0,15%   | 512.196 B | CRC-32 ✓          |

```text
Alta e média redundância: o GarchY comprime de forma substantiva.
Baixa redundância — bytes pseudo-aleatórios com entropia máxima —
não existe compressão sem perda possível. Ambos os compressores
sofrem leve expansão pelo overhead do cabeçalho. Comportamento esperado.

A diferença de razão para o gzip existe porque o gzip usa Deflate:
LZ77 combinado com Huffman. O LZ77 detecta sequências repetidas
e as substitui por referências. O nosso Huffman opera só sobre
a frequência individual de bytes — mais simples, mais transparente,
e ainda assim eficaz para distribuições não-uniformes.

Em 100% dos testes, a integridade foi confirmada byte a byte
e pelo CRC-32.
```

### Encerramento

```text
Implementamos do zero, em ANSI C, sem bibliotecas externas:
Min-Heap, Árvore de Huffman, BitStream bit a bit,
formato .abcz com varint, CRC-32 IEEE 802.3,
e um menu interativo com métricas em tempo real.

Compila com -Wall -Wextra sem nenhum warning.

Obrigado. Ficamos à disposição para perguntas.
```

---

## Checklist pré-apresentação

- [ ] Projeto compilado (`make`) e executável `garchy` disponível no PATH ou no diretório
- [ ] Terminal com fonte legível no projetor (mínimo 14pt)
- [ ] README.md disponível no diretório para usar na demo ao vivo
- [ ] Relatório técnico aberto em segundo plano para mostrar as tabelas
- [ ] Cada integrante sabe qual arquivo de código vai mostrar na sua fala

---

*Arquivo gerado em 2026-11-05 · GarchY / MCTA028-15 · UFABC 2026.3*
