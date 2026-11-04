# Compressão — `src/compressao.c`

## O que faz

Lê um arquivo qualquer, comprime com Huffman e grava o resultado em um `.abcz`.

---

## Visão geral do fluxo

```
arquivo de entrada
      │
      ▼
1ª passada: contar frequências de cada byte + calcular CRC-32
      │
      ▼
construir árvore de Huffman (heap.c + huffman.c)
      │
      ▼
gerar tabela de códigos (1 código binário por byte)
      │
      ▼
gravar cabeçalho no .abcz (cabecalho.c)
      │
      ▼
2ª passada: substituir cada byte pelo seu código de bits
      │
      ▼
arquivo .abcz
```

O arquivo é lido **duas vezes** porque os arquivos grandes não cabem inteiros na memória. Na primeira passada só contamos; na segunda, já sabemos os códigos e gravamos os bits.

---

## Função principal

```c
int comprimir_arquivo(const char *caminho_entrada,
                      const char *caminho_saida,
                      ResultadoCompressao *resultado);
```

Retorna `COMP_OK` (0) ou um código negativo de erro (`COMP_ERRO_*`).  
Se `resultado != NULL`, preenche com estatísticas para o menu exibir.

---

## 1ª passada — frequências e checksum

```c
while ((n = fread(bloco, 1, TAM_BLOCO, ent)) > 0) {
    huff_contar_freq(bloco, n, cab.freq);
    cab.checksum = checksum_crc32(cab.checksum, bloco, n);
    cab.tam_original += n;
}
```

- Lê em blocos de **64 KB** para não estourar a memória em arquivos grandes.
- `cab.freq[i]` acumula quantas vezes o byte `i` apareceu (0–255).
- `cab.checksum` é o CRC-32 acumulado de todo o arquivo.
- `cab.tam_original` é o tamanho em bytes, usado na descompressão para saber quando parar.

---

## Construção da árvore e geração dos códigos

```c
raiz = huff_construir_arvore(cab.freq);
huff_gerar_codigos(raiz, codigos);
```

Delegado para `huffman.c`. Cada `codigos[byte]` tem:
- `bits[]` — sequência de 0s e 1s do código.
- `tam` — comprimento do código (0 se o byte não aparece).

Bytes mais frequentes recebem códigos mais curtos → arquivo final menor.

---

## Escrita do cabeçalho

```c
tam_cab = cabecalho_escrever(sai, &cab);
```

Grava no `.abcz` a assinatura, versão, tamanho original, checksum e a tabela de frequências. Veja `docs/cabecalho.md` para o formato completo.

---

## 2ª passada — gravação dos dados comprimidos

```c
bw_init(&bw, sai);
while ((n = fread(bloco, 1, TAM_BLOCO, ent)) > 0) {
    for (i = 0; i < n; i++) {
        const HuffCodigo *c = &codigos[bloco[i]];
        for (k = 0; k < c->tam; k++)
            bw_put_bit(&bw, c->bits[k]);
    }
}
bw_flush(&bw);
```

- O `BitWriter` acumula bits e só grava no arquivo quando completa 1 byte (8 bits).
- `bw_flush` grava o último byte incompleto, preenchendo com zeros à direita.
- A ordem é **MSB primeiro** (bit mais significativo do código vai primeiro no arquivo).

---

## Tratamento de erros

- Se der qualquer erro após criar o `.abcz`, o arquivo parcial é **apagado**:
  ```c
  if (ret != COMP_OK && saida_criada)
      remove(caminho_saida);
  ```
- Entrada e saída com o mesmo caminho são rejeitadas antes de abrir qualquer arquivo.
- `fclose(sai)` é verificado — pode revelar erros de escrita pendentes em buffers do SO.

---

## Detalhes de implementação

| Detalhe | Motivo |
|---|---|
| `static unsigned char bloco[TAM_BLOCO]` | Evita alocar 64 KB na pilha a cada chamada |
| `static HuffCodigo codigos[HUFF_SIMBOLOS]` | Idem — 256 × 257 bytes seria grande na pilha |
| Verificação `lidos2 != cab.tam_original` | Detecta se o arquivo mudou de tamanho entre as duas passadas |

---

## Códigos de retorno

| Código | Significado |
|---|---|
| `COMP_OK` | Sucesso |
| `COMP_ERRO_ABRIR_ENTRADA` | Arquivo de entrada não encontrado ou sem permissão |
| `COMP_ERRO_ABRIR_SAIDA` | Não foi possível criar o `.abcz` |
| `COMP_ERRO_LEITURA` | Erro durante a leitura |
| `COMP_ERRO_ESCRITA` | Erro durante a gravação |
| `COMP_ERRO_MEMORIA` | `malloc` falhou ao construir a árvore |
| `COMP_ERRO_MESMO_ARQUIVO` | Entrada e saída apontam para o mesmo arquivo |
