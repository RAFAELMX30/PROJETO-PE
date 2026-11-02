#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>
#include <stdint.h>

#define HUFF_SIMBOLOS 256

/* no da arvore de Huffman
   folha = esq e dir NULL
   bit 0 vai pra esquerda, bit 1 pra direita */
struct HuffNode {
    unsigned char byte; /* so vale nas folhas */
    uint64_t freq;
    int ordem; /* desempate no heap */
    struct HuffNode *esq;
    struct HuffNode *dir;
};
typedef struct HuffNode HuffNode;

/* codigo de um byte (tam = 0 se o byte nao aparece) */
typedef struct {
    unsigned char bits[HUFF_SIMBOLOS];
    int tam;
} HuffCodigo;

/* conta quantas vezes cada byte aparece (freq tem que comecar zerado) */
void huff_contar_freq(const unsigned char *buf, size_t n, uint64_t freq[HUFF_SIMBOLOS]);

/* monta a arvore, retorna NULL se o arquivo for vazio
   se so tiver 1 byte diferente, o codigo dele vira "0" */
HuffNode *huff_construir_arvore(const uint64_t freq[HUFF_SIMBOLOS]);

/* preenche o codigo de cada byte percorrendo a arvore */
void huff_gerar_codigos(const HuffNode *raiz, HuffCodigo codigos[HUFF_SIMBOLOS]);

/* retorna 1 se o no for folha */
int huff_eh_folha(const HuffNode *no);

/* libera a arvore inteira */
void huff_liberar_arvore(HuffNode *raiz);

#endif
