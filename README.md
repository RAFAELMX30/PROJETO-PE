# PROJETO-PE

Compactador e descompactador de arquivos em C usando o algoritmo de Huffman.
Trabalho de Programação Estruturada (MCTA028-15), UFABC 2026.3.

## Como compilar e rodar

```
make
./projetoPE
```

No Windows: `projetoPE.exe`. O comando completo do gcc, sem make, está no `leia-me.txt`.

## O que o programa faz

- Comprime qualquer arquivo para `.abcz` e descomprime de volta
- Mostra tamanhos, razão de compressão e tempo
- Confere a integridade com CRC-32 (funcionalidade adicional)
- Mostra a tabela de códigos de Huffman de um arquivo
- `tests/benchmark_gzip.py` compara com o gzip em 3 cenários de redundância

## Integrantes

- José Ricardo Feltran Júnior: min-heap, árvore de Huffman e códigos
- Rafael Maximiano Luiz: escrita/leitura de bits, cabeçalho e compressão
- Renan Henrique Ferreira Monteiro: descompressão e checksum
- Rodrigo Martel Poggi de Araújo: menu, métricas, testes com gzip e relatório

O relatório técnico está em `docs/`.
