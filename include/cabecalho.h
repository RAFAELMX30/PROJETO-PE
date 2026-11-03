#ifndef CABECALHO_H
#define CABECALHO_H

#include <stdio.h>
#include <stdint.h>
#include "huffman.h"

/*
 * Formato do arquivo .abcz (todos os numeros em little-endian):
 *
 *   offset  tam   campo
 *   0       4     assinatura "ABCZ"
 *   4       1     versao do formato (1)
 *   5       8     tamanho original, em bytes
 *   13      4     checksum CRC-32 do arquivo original
 *   17      2     quantidade de bytes diferentes (0 a 256)
 *   19      ...   para cada byte diferente, em ordem crescente do byte:
 *                   1 byte  = valor do byte
 *                   varint  = frequencia (7 bits por byte, bit 7 = "tem mais")
 *   ...     ...   dados comprimidos (bits dos codigos de Huffman, MSB primeiro,
 *                 ultimo byte completado com zeros)
 *
 * A tabela de frequencias so guarda os bytes que aparecem (os outros tem
 * frequencia 0), entao o cabecalho de um arquivo pequeno fica pequeno.
 * Na descompressao a arvore e reconstruida com huff_construir_arvore(freq),
 * que e deterministica, entao sai identica a da compressao.
 */

#define ABCZ_VERSAO 1

typedef struct {
    uint64_t tam_original;
    uint32_t checksum;
    uint64_t freq[HUFF_SIMBOLOS];
} CabecalhoAbcz;

/* codigos de retorno de cabecalho_ler (negativos = erro) */
#define CAB_OK              0
#define CAB_ERRO_IO        -1  /* erro de leitura ou arquivo cortado */
#define CAB_ERRO_ASSINATURA -2 /* nao e um arquivo .abcz */
#define CAB_ERRO_VERSAO    -3  /* versao desconhecida */
#define CAB_ERRO_INVALIDO  -4  /* campos inconsistentes (arquivo corrompido) */

/* grava o cabecalho na posicao atual de f.
   retorna a quantidade de bytes gravados, ou -1 se der erro */
long cabecalho_escrever(FILE *f, const CabecalhoAbcz *cab);

/* le o cabecalho da posicao atual de f e deixa f logo depois dele
   (pronto pra criar o BitReader). cab->freq e totalmente preenchido. */
int cabecalho_ler(FILE *f, CabecalhoAbcz *cab);

#endif
