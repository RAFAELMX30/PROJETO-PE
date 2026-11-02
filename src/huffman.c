#include <stdlib.h>
#include "huffman.h"
#include "heap.h"

void huff_contar_freq(const unsigned char *buf, size_t n, uint64_t freq[HUFF_SIMBOLOS]) {
    size_t i;
    for (i = 0; i < n; i++)
        freq[buf[i]]++;
}

static HuffNode *novo_no(unsigned char byte, uint64_t freq, int ordem,
                         HuffNode *esq, HuffNode *dir) {
    HuffNode *no = malloc(sizeof(HuffNode));
    if (no == NULL)
        return NULL;
    no->byte = byte;
    no->freq = freq;
    no->ordem = ordem;
    no->esq = esq;
    no->dir = dir;
    return no;
}

int huff_eh_folha(const HuffNode *no) {
    return no != NULL && no->esq == NULL && no->dir == NULL;
}

void huff_liberar_arvore(HuffNode *raiz) {
    if (raiz == NULL)
        return;
    huff_liberar_arvore(raiz->esq);
    huff_liberar_arvore(raiz->dir);
    free(raiz);
}

/* Esvazia o heap liberando as subárvores que sobraram (caso de erro). */
static void liberar_heap_com_nos(MinHeap *h) {
    HuffNode *no;
    while ((no = heap_remover_min(h)) != NULL)
        huff_liberar_arvore(no);
    heap_destruir(h);
}

HuffNode *huff_construir_arvore(const uint64_t freq[HUFF_SIMBOLOS]) {
    MinHeap *h;
    HuffNode *a, *b, *pai, *raiz;
    int i;
    int prox_ordem = HUFF_SIMBOLOS; /* folhas usam ordem = byte; internos 256, 257, ... */

    h = heap_criar(HUFF_SIMBOLOS);
    if (h == NULL)
        return NULL;

    /* uma folha para cada byte que aparece */
    for (i = 0; i < HUFF_SIMBOLOS; i++) {
        if (freq[i] > 0) {
            HuffNode *folha = novo_no((unsigned char)i, freq[i], i, NULL, NULL);
            if (folha == NULL) {
                liberar_heap_com_nos(h);
                return NULL;
            }
            heap_inserir(h, folha);
        }
    }

    /* arquivo vazio */
    if (heap_tamanho(h) == 0) {
        heap_destruir(h);
        return NULL;
    }

    /* um único byte distinto: raiz interna com a folha à esquerda (código "0") */
    if (heap_tamanho(h) == 1) {
        a = heap_remover_min(h);
        heap_destruir(h);
        raiz = novo_no(0, a->freq, prox_ordem, a, NULL);
        if (raiz == NULL)
            free(a);
        return raiz;
    }

    /* junta sempre os dois menores até sobrar só a raiz */
    while (heap_tamanho(h) > 1) {
        a = heap_remover_min(h);
        b = heap_remover_min(h);
        pai = novo_no(0, a->freq + b->freq, prox_ordem++, a, b);
        if (pai == NULL) {
            huff_liberar_arvore(a);
            huff_liberar_arvore(b);
            liberar_heap_com_nos(h);
            return NULL;
        }
        heap_inserir(h, pai);
    }

    raiz = heap_remover_min(h);
    heap_destruir(h);
    return raiz;
}

static void gerar_rec(const HuffNode *no, unsigned char *caminho, int prof,
                      HuffCodigo codigos[HUFF_SIMBOLOS]) {
    int i;
    if (no == NULL)
        return;
    if (huff_eh_folha(no)) {
        for (i = 0; i < prof; i++)
            codigos[no->byte].bits[i] = caminho[i];
        codigos[no->byte].tam = prof;
        return;
    }
    caminho[prof] = 0;
    gerar_rec(no->esq, caminho, prof + 1, codigos);
    caminho[prof] = 1;
    gerar_rec(no->dir, caminho, prof + 1, codigos);
}

void huff_gerar_codigos(const HuffNode *raiz, HuffCodigo codigos[HUFF_SIMBOLOS]) {
    unsigned char caminho[HUFF_SIMBOLOS];
    int i;
    for (i = 0; i < HUFF_SIMBOLOS; i++)
        codigos[i].tam = 0;
    if (raiz == NULL)
        return;
    gerar_rec(raiz, caminho, 0, codigos);
}