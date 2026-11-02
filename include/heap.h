#ifndef HEAP_H
#define HEAP_H

struct HuffNode;

/* fila de prioridade (min-heap) com os nos da arvore */
typedef struct {
    struct HuffNode **dados;
    int tamanho;
    int capacidade;
} MinHeap;

/* cria um heap vazio */
MinHeap *heap_criar(int capacidade);

/* libera o heap, mas nao os nos */
void heap_destruir(MinHeap *h);

/* retorna 0 se estiver cheio */
int heap_inserir(MinHeap *h, struct HuffNode *no);

/* remove o menor no (NULL se estiver vazio) */
struct HuffNode *heap_remover_min(MinHeap *h);

int heap_tamanho(const MinHeap *h);

#endif
