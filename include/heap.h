#ifndef HEAP_H
#define HEAP_H

struct HuffNode;

typedef struct {
    struct HuffNode **dados;
    int tamanho;
    int capacidade;
} MinHeap;

/* Cria um heap vazio com a capacidade dada. Retorna NULL se faltar memória. */
MinHeap *heap_criar(int capacidade);

/* Libera o heap (não libera os nós que estiverem dentro dele). */
void heap_destruir(MinHeap *h);

/* Insere um nó. Retorna 1 em sucesso, 0 se o heap estiver cheio. */
int heap_inserir(MinHeap *h, struct HuffNode *no);

/* Remove e retorna o menor nó. Retorna NULL se o heap estiver vazio. */
struct HuffNode *heap_remover_min(MinHeap *h);

/* Quantidade de nós no heap. */
int heap_tamanho(const MinHeap *h);

#endif