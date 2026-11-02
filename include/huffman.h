#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>

#define HUFF_SIMBOLOS 256

/* Nó da árvore de Huffman.
 * Folha: esq == NULL e dir == NULL; o campo "byte" guarda o símbolo.
 * Convenção dos bits: 0 = esquerda, 1 = direita. */
struct HuffNode {
    unsigned char byte;     /* símbolo (válido só nas folhas)             */
    uint64_t freq;          /* frequência do símbolo ou soma dos filhos   */
    int ordem;              /* desempate no heap (árvore determinística)  */
    struct HuffNode *esq;
    struct HuffNode *dir;
};
typedef struct HuffNode HuffNode;

/* Código de um byte: bits[i] vale 0 ou 1; tam = quantidade de bits.
 * tam == 0 significa que o byte não aparece no arquivo. */
typedef struct {
    unsigned char bits[HUFF_SIMBOLOS];
    int tam;
} HuffCodigo;

/* Soma ao vetor freq a contagem dos n bytes de buf.
 * Zere freq antes da primeira chamada; pode ser chamada por blocos. */
void huff_contar_freq(const unsigned char *buf, size_t n, uint64_t freq[HUFF_SIMBOLOS]);

/* Monta a árvore a partir das frequências.
 * - Arquivo vazio (todas as frequências 0): retorna NULL.
 * - Um único byte distinto: a raiz é um nó interno com a folha à
 *   esquerda, então o código desse byte é "0".
 * Mesmas frequências geram sempre a mesma árvore.
 * Retorna NULL também se faltar memória (confira se freq não era tudo 0). */
HuffNode *huff_construir_arvore(const uint64_t freq[HUFF_SIMBOLOS]);

/* Preenche codigos[0..255] percorrendo a árvore (raiz pode ser NULL). */
void huff_gerar_codigos(const HuffNode *raiz, HuffCodigo codigos[HUFF_SIMBOLOS]);

/* Retorna 1 se o nó for folha, 0 caso contrário. */
int huff_eh_folha(const HuffNode *no);

/* Libera toda a árvore (aceita NULL). */
void huff_liberar_arvore(HuffNode *raiz);

#endif
