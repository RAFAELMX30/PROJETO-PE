# Checksum — `src/checksum.c`

## O que faz

Calcula o **CRC-32** de um bloco de bytes. Usado para verificar se o arquivo recuperado pela descompressão é idêntico ao original.

---

## O que é CRC-32

CRC (Cyclic Redundancy Check) é um algoritmo de detecção de erros. Ele processa os bytes de um arquivo e produz um número de 32 bits (o "checksum"). Se qualquer byte do arquivo for alterado, o CRC calculado será diferente.

É o mesmo algoritmo usado pelo `gzip`, `zlib` e Ethernet. Não é criptográfico — serve apenas para detectar corrupção acidental, não adulteração intencional.

---

## Interface

```c
uint32_t checksum_crc32(uint32_t crc, const unsigned char *buf, size_t n);
```

- `crc` — valor acumulado de chamadas anteriores. Na **primeira** chamada, passe `0`.
- `buf` — bloco de bytes a processar.
- `n` — tamanho do bloco.
- Retorna o CRC acumulado até agora.

### Uso incremental (como no projeto)

```c
uint32_t crc = 0;
while ((n = fread(bloco, 1, TAM_BLOCO, f)) > 0)
    crc = checksum_crc32(crc, bloco, n);
/* crc agora é o CRC-32 do arquivo inteiro */
```

Processar em blocos produz o **mesmo resultado** que processar tudo de uma vez.

---

## Como funciona

### Tabela de lookup

O algoritmo usa uma tabela de 256 entradas pré-calculada (`tabela[256]`). Cada entrada corresponde a um valor possível de byte (0–255) e contém o resto da divisão polinomial para aquele byte.

```c
static void montar_tabela(void) {
    for (i = 0; i < 256; i++) {
        c = i;
        for (k = 0; k < 8; k++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        tabela[i] = c;
    }
}
```

- `0xEDB88320` é o polinômio CRC-32 padrão (IEEE 802.3) em representação invertida.
- A tabela é montada **uma única vez** na primeira chamada (`tabela_pronta`).

### Processamento byte a byte

```c
crc = ~crc;
for (i = 0; i < n; i++)
    crc = tabela[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
return ~crc;
```

| Passo | O que faz |
|---|---|
| `crc = ~crc` | Inversão inicial (padrão CRC-32) |
| `crc ^ buf[i]` | Mistura o byte atual com o CRC acumulado |
| `& 0xFF` | Pega apenas o byte menos significativo como índice na tabela |
| `^ (crc >> 8)` | Aplica o resultado da tabela e desloca o CRC |
| `return ~crc` | Inversão final (padrão CRC-32) |

As inversões no início e no final são parte do padrão CRC-32 e garantem que sequências de zeros no começo ou no fim do arquivo não sejam ignoradas.

---

## Como o projeto usa o checksum

### Durante a compressão (`compressao.c`)

```c
cab.checksum = checksum_crc32(cab.checksum, bloco, n);
```

O CRC é calculado enquanto o arquivo original é lido na 1ª passada. O valor final é gravado no cabeçalho do `.abcz`.

### Durante a descompressão (`descompressao.c`)

```c
crc = checksum_crc32(crc, bloco, pos);
```

O CRC é recalculado sobre os bytes recuperados. Ao final:

```c
if (crc != cab.checksum)
    ret = DECOMP_ERRO_CHECKSUM;
```

Se os valores forem iguais, o arquivo foi recuperado intacto.

---

## Por que fizemos assim

### Por que CRC-32 e não MD5 ou SHA-256?

MD5 e SHA-256 são algoritmos criptográficos — projetados para resistir a adulteração **intencional**, onde um atacante tenta forjar um hash sem conhecer os dados originais. Para o nosso propósito (detectar corrupção **acidental** — bit flip por falha de disco ou truncamento de arquivo), CRC-32 é suficiente e muito mais rápido.

CRC-32 processa ~1 GB/s em hardware moderno. SHA-256 sem aceleração de hardware processa ~200 MB/s. Para arquivos de centenas de MB, a diferença é relevante. Além disso, implementar CRC-32 do zero cabe em 25 linhas de C; SHA-256 exigiria centenas de linhas ou uma dependência externa — ambas opções piores para um projeto acadêmico sem dependências.

---

### Por que tabela de lookup e não calcular direto?

Sem tabela, calcular o CRC de cada byte exigiria 8 iterações de divisão polinomial — 8 operações por byte. Com a tabela de 256 entradas pré-calculada, o custo é 1 XOR + 1 lookup por byte — aproximadamente 8× mais rápido.

A tabela ocupa 256 × 4 bytes = 1 KB na memória, valor desprezível. O custo de pré-calculá-la na primeira chamada é amortizado em qualquer arquivo com mais de ~100 bytes.

---

### Por que o polinômio `0xEDB88320` (IEEE 802.3)?

É o padrão CRC-32 usado por `zlib`, `gzip`, PNG, Ethernet, ZIP e USB. Suas propriedades matemáticas são bem estudadas: detecta todos os erros de rajada de até 32 bits, todos os erros com número ímpar de bits, e todas as combinações de 1 e 2 erros em mensagens de qualquer tamanho prático. Usar um polinômio diferente traria riscos de propriedades inferiores sem nenhuma vantagem para o projeto.

---

### Por que a representação invertida (reflected)?

O hardware CRC original processava bits do LSB para o MSB. A representação invertida — `0xEDB88320` em vez de `0x04C11DB7` — é a convenção em software que replica esse comportamento e é o padrão em todas as implementações de referência. Garante que o CRC calculado pelo nosso programa seja idêntico ao de qualquer outra ferramenta que use CRC-32 IEEE 802.3.

---

### Por que as inversões `~crc` no início e no fim?

Sem a inversão inicial (`crc = ~crc`), um arquivo que começa com vários bytes zero teria o mesmo CRC que se esses zeros não existissem — o estado inicial zero XOR com zero continua zero, e o CRC seria cego ao preenchimento inicial. Com a inversão, o estado inicial é `0xFFFFFFFF`, tornando o algoritmo sensível a zeros no começo do arquivo. A inversão final (`return ~crc`) é parte do padrão IEEE e garante compatibilidade com outras implementações.

---

### Por que não usar soma simples ou XOR de bytes?

**Soma de bytes:** não detecta bytes trocados de posição (A + B = B + A). Não detecta a substituição de um byte por dois bytes que somam o mesmo valor.

**XOR byte a byte:** não detecta a adição ou remoção de pares de bytes idênticos (A XOR A = 0). Uma transposição de dois bytes idênticos é invisível.

**CRC-32:** trata os dados como um número binário e calcula o resto da divisão por um polinômio — operação sensível à posição de cada bit. Uma mudança de 1 bit em qualquer posição sempre produz um CRC diferente.

---

## Propriedades

| Propriedade | Valor |
|---|---|
| Tamanho do checksum | 32 bits (4 bytes) |
| Detecta | Qualquer alteração de 1 bit, inversões, truncamentos |
| Não detecta | Adulterações intencionais (não é criptográfico) |
| Colisões | Raras mas possíveis (1 em ~4 bilhões por acaso) |
| Custo | O(n) no tamanho do arquivo, muito rápido na prática |
