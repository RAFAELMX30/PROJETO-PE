#include <stdlib.h>
#include "heap.h"
#include "huffman.h"

static int menor(const HuffNode *a, const HuffNode *b) {
    if (a->freq != b->freq)
        return a->freq < b->freq;
    return a->ordem < b->ordem;
}

static void trocar(HuffNode **x, HuffNode **y) {
    HuffNode *t = *x;
    *x = *y;
    *y = t;
}

MinHeap *heap_criar(int capacidade) {
    MinHeap *h;
    if (capacidade <= 0)
        return NULL;
    h = malloc(sizeof(MinHeap));
    if (h == NULL)
        return NULL;
    h->dados = malloc((size_t)capacidade * sizeof(HuffNode *));
    if (h->dados == NULL) {
        free(h);
        return NULL;
    }
    h->tamanho = 0;
    h->capacidade = capacidade;
    return h;
}

void heap_destruir(MinHeap *h) {
    if (h == NULL)
        return;
    free(h->dados);
    free(h);
}

int heap_inserir(MinHeap *h, HuffNode *no) {
    int i, pai;
    if (h == NULL || no == NULL || h->tamanho == h->capacidade)
        return 0;

    i = h->tamanho++;
    h->dados[i] = no;
    while (i > 0) {
        pai = (i - 1) / 2;
        if (!menor(h->dados[i], h->dados[pai]))
            break;
        trocar(&h->dados[i], &h->dados[pai]);
        i = pai;
    }
    return 1;
}

HuffNode *heap_remover_min(MinHeap *h) {
    HuffNode *min;
    int i, esq, dir, menor_i;
    if (h == NULL || h->tamanho == 0)
        return NULL;

    min = h->dados[0];
    h->tamanho--;
    h->dados[0] = h->dados[h->tamanho];

    i = 0;
    for (;;) {
        esq = 2 * i + 1;
        dir = 2 * i + 2;
        menor_i = i;
        if (esq < h->tamanho && menor(h->dados[esq], h->dados[menor_i]))
            menor_i = esq;
        if (dir < h->tamanho && menor(h->dados[dir], h->dados[menor_i]))
            menor_i = dir;
        if (menor_i == i)
            break;
        trocar(&h->dados[i], &h->dados[menor_i]);
        i = menor_i;
    }
    return min;
}

int heap_tamanho(const MinHeap *h) {
    return h == NULL ? 0 : h->tamanho;
}
