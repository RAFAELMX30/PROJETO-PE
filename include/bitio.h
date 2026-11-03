#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>
#include <stdint.h>

/* escrita de bits: junta 8 bits e grava 1 byte (bit mais significativo primeiro) */
typedef struct {
    FILE *f;
    uint8_t buffer;
    int nbits; /* quantos bits ja estao no buffer (0 a 7) */
} BitWriter;

/* leitura de bits na mesma ordem da escrita */
typedef struct {
    FILE *f;
    uint8_t buffer;
    int nbits; /* quantos bits do buffer ja foram usados (8 = precisa ler outro byte) */
} BitReader;

void bw_init(BitWriter *bw, FILE *f);

/* grava 1 bit (0 ou 1). retorna 0 se deu certo, -1 se deu erro de escrita */
int bw_put_bit(BitWriter *bw, int bit);

/* grava o byte incompleto, completando com zeros. retorna 0 ou -1 */
int bw_flush(BitWriter *bw);

void br_init(BitReader *br, FILE *f);

/* le 1 bit. retorna 0 ou 1, ou EOF quando acabar o arquivo */
int br_get_bit(BitReader *br);

#endif
