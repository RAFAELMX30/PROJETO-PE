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

## Por que fizemos assim

### Reconstruir a árvore das frequências, não salvar a árvore pronta

A alternativa seria salvar a estrutura da árvore diretamente no arquivo — os nós e sua topologia — para não precisar reconstruí-la. O problema: ponteiros são endereços de memória. Salvar um ponteiro em disco é inútil porque na próxima execução o endereço simplesmente não existe mais. Serializar a topologia da árvore exigiria um formato recursivo com marcadores para nós internos e folhas — mais código, mais complexidade, e mais espaço no cabeçalho.

As frequências funcionam porque a árvore de Huffman é **determinística**: dada a mesma tabela de frequências e o mesmo critério de desempate (campo `ordem`), o algoritmo de construção sempre produz a mesma árvore. As frequências já precisam estar no cabeçalho de qualquer forma e são compactas (no máximo 256 inteiros). Nenhuma informação extra é necessária.

---

### `tam_original` como critério de parada, não um marcador especial

A alternativa seria inserir uma sequência especial de bits no final do bitstream para sinalizar o fim dos dados — como um byte de EOF.

O problema: para o marcador funcionar, ele precisaria de um valor que nunca aparece naturalmente nos dados comprimidos. Como processamos qualquer arquivo binário, todos os padrões de bits são possíveis. A solução seria escaping — preceder o marcador real com um caractere de escape quando ele aparecer nos dados — o que complica o código e aumenta o tamanho do arquivo para certos tipos de entrada.

`tam_original` resolve sem custo extra: já precisamos armazenar o tamanho original no cabeçalho para calcular a razão de compressão e o CRC. Usá-lo como critério de parada é zero custo de implementação. Os bits de padding do último byte são simplesmente ignorados — a contagem chega ao fim antes de consumi-los.

---

### Manter o arquivo em erro de checksum, apagar nos outros erros

Em erro de checksum, **todos** os bytes foram decodificados e gravados — a árvore foi percorrida até o fim e o arquivo de saída está completo em tamanho. O usuário pode querer inspecionar os dados, comparar byte a byte com o original, ou decidir se a diferença é relevante para o caso específico (talvez o arquivo tenha sido editado intencionalmente). Apagar seria perda de informação sem benefício.

Nos outros erros — I/O corrompido, formato inválido, memória insuficiente — o arquivo de saída está **incompleto**. Um arquivo truncado que parece completo é mais perigoso do que nenhum arquivo. A decisão foi apagar e avisar com o código de erro adequado, sem deixar rastro que possa ser confundido com um arquivo válido.

---

### Percorrer a árvore bit a bit, não usar tabela de decodificação direta

A alternativa eficiente seria construir uma tabela inversa onde, dado um prefixo de bits, se obtém o byte e o comprimento do código diretamente — sem percorrer a árvore nó a nó. É o que compiladores e compressores de produção fazem (Huffman canônico com tabela de lookup de comprimento fixo).

O problema: a tabela exige conhecer o comprimento máximo dos códigos para definir seu tamanho. No pior caso, Huffman gera códigos de até 255 bits — uma tabela de 2^255 entradas é impossível. Implementações reais contornam isso com tabelas de múltiplos estágios e reordenação dos códigos (Huffman canônico), o que exige dezenas de linhas adicionais e lógica de reorganização da árvore.

Percorrer a árvore é O(comprimento do código) por byte — na prática O(log N) médio para N símbolos distintos. Para os arquivos de centenas de KB usados nos testes, a diferença de velocidade em relação a uma tabela direta é imperceptível. A clareza e correção do código compensam.

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
