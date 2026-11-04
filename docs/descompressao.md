# Descompressão — `src/descompressao.c`

## O que faz

Lê um arquivo `.abcz`, reconstrói a árvore de Huffman, decodifica os bits e grava o arquivo original. Ao final, verifica a integridade comparando o CRC-32 calculado com o armazenado no cabeçalho.

---

## Visão geral do fluxo

```
arquivo .abcz
      │
      ▼
ler cabeçalho: tam_original, checksum, freq[] (cabecalho.c)
      │
      ▼
reconstruir árvore de Huffman com freq[] (huffman.c)
      │                        ↑ idêntica à da compressão
      ▼                          porque é determinística
ler bits um a um, percorrer a árvore
      │
      ├─ bit 0 → vai para o filho esquerdo
      ├─ bit 1 → vai para o filho direito
      └─ chegou em folha → grava o byte, volta para a raiz
      │
      ▼
ao recuperar tam_original bytes → parar
      │
      ▼
comparar CRC-32 calculado com o do cabeçalho
      │
      ▼
arquivo original recuperado
```

---

## Função principal

```c
int descomprimir_arquivo(const char *caminho_entrada,
                         const char *caminho_saida,
                         ResultadoDescompressao *resultado);
```

Retorna `DECOMP_OK` (0) ou um código negativo de erro (`DECOMP_ERRO_*`).  
Se `resultado != NULL`, preenche com `tam_original`, `checksum_armazenado` e `checksum_calculado`.

---

## Leitura do cabeçalho

```c
cab_ret = cabecalho_ler(ent, &cab);
```

Depois dessa chamada, `cab` contém:
- `cab.tam_original` — quantos bytes o arquivo original tinha.
- `cab.checksum` — CRC-32 do arquivo original (calculado durante a compressão).
- `cab.freq[0..255]` — frequência de cada byte, usada para reconstruir a árvore.

O arquivo fica posicionado logo após o cabeçalho, pronto para o `BitReader`.

---

## Reconstrução da árvore

```c
raiz = huff_construir_arvore(cab.freq);
```

A árvore é **reconstruída a partir das frequências**, não precisa ser armazenada diretamente no arquivo. Como o algoritmo de construção é determinístico (mesmo desempate por `ordem`), a árvore gerada aqui é idêntica à usada durante a compressão.

---

## Decodificação bit a bit

```c
br_init(&br, ent);

for (i = 0; i < cab.tam_original; i++) {
    no = raiz;
    while (!huff_eh_folha(no)) {
        bit = br_get_bit(&br);
        no = bit ? no->dir : no->esq;
    }
    bloco[pos++] = no->byte;
    /* flush do bloco quando enche */
}
```

- A cada iteração do `for`, decodificamos **exatamente 1 byte** do arquivo original.
- O `while` percorre a árvore seguindo os bits: 0 → esquerda, 1 → direita.
- Ao chegar em uma **folha**, `no->byte` é o byte original.
- A contagem `cab.tam_original` é o critério de parada — isso descarta os bits de padding do último byte gravado pelo `bw_flush`.

---

## Escrita em blocos e cálculo do checksum

```c
bloco[pos++] = no->byte;
if (pos == TAM_BLOCO) {
    crc = checksum_crc32(crc, bloco, pos);
    fwrite(bloco, 1, pos, sai);
    pos = 0;
}
```

- Os bytes recuperados são acumulados em `bloco` (64 KB) antes de gravar no disco.
- O CRC-32 é calculado **incrementalmente** sobre os mesmos blocos gravados.
- Ao final do loop, o restante do bloco é descarregado e o CRC finalizado.

---

## Verificação de integridade

```c
verificar:
    if (crc != cab.checksum)
        ret = DECOMP_ERRO_CHECKSUM;
```

- Se os CRCs batem: arquivo recuperado com sucesso.
- Se não batem: o arquivo de saída é **mantido** (os dados estão lá, o usuário decide o que fazer), mas a função retorna `DECOMP_ERRO_CHECKSUM`.
- Causas possíveis de falha: arquivo `.abcz` corrompido ou modificado.

---

## Caso especial: arquivo vazio

Quando `cab.tam_original == 0`, o loop de decodificação não executa e `crc` permanece `0`. Durante a compressão, o CRC de um arquivo vazio também é `0`, então a verificação passa normalmente.

---

## Tratamento de erros

| Situação | Comportamento |
|---|---|
| Erro de leitura, formato, memória, escrita | Arquivo de saída parcial é **apagado** |
| Checksum diferente | Arquivo de saída é **mantido**, retorna `DECOMP_ERRO_CHECKSUM` |
| Entrada == saída | Rejeitado antes de abrir qualquer arquivo |

---

## Códigos de retorno

| Código | Significado |
|---|---|
| `DECOMP_OK` | Sucesso, integridade confirmada |
| `DECOMP_ERRO_ABRIR_ENTRADA` | Arquivo `.abcz` não encontrado |
| `DECOMP_ERRO_ABRIR_SAIDA` | Não foi possível criar o arquivo de saída |
| `DECOMP_ERRO_LEITURA` | Arquivo cortado ou erro de I/O |
| `DECOMP_ERRO_ESCRITA` | Erro ao gravar o arquivo recuperado |
| `DECOMP_ERRO_MEMORIA` | `malloc` falhou ao construir a árvore |
| `DECOMP_ERRO_MESMO_ARQUIVO` | Entrada e saída com o mesmo caminho |
| `DECOMP_ERRO_FORMATO` | Assinatura, versão ou campos inválidos |
| `DECOMP_ERRO_CHECKSUM` | Dados recuperados, mas integridade não confirmada |
