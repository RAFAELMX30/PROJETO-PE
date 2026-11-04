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

## Propriedades

| Propriedade | Valor |
|---|---|
| Tamanho do checksum | 32 bits (4 bytes) |
| Detecta | Qualquer alteração de 1 bit, inversões, truncamentos |
| Não detecta | Adulterações intencionais (não é criptográfico) |
| Colisões | Raras mas possíveis (1 em ~4 bilhões por acaso) |
| Custo | O(n) no tamanho do arquivo, muito rápido na prática |
