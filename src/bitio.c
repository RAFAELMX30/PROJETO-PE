#include "bitio.h"

void bw_init(BitWriter *bw, FILE *f) {
    bw->f = f;
    bw->buffer = 0;
    bw->nbits = 0;
}

void br_init(BitReader *br, FILE *f) {
    br->f = f;
    br->buffer = 0;
    br->nbits = 8; /* forca a leitura do primeiro byte */
}

int bw_put_bit(BitWriter *bw, int bit) {
    bw->buffer = (uint8_t)((bw->buffer << 1) | (bit & 1));
    bw->nbits++;

    if (bw->nbits == 8) {
        if (fputc(bw->buffer, bw->f) == EOF)
            return -1;
        bw->buffer = 0;
        bw->nbits = 0;
    }
    return 0;
}

int bw_flush(BitWriter *bw) {
    if (bw->nbits != 0) {
        bw->buffer = (uint8_t)(bw->buffer << (8 - bw->nbits));
        if (fputc(bw->buffer, bw->f) == EOF)
            return -1;
        bw->buffer = 0;
        bw->nbits = 0;
    }
    return 0;
}

int br_get_bit(BitReader *br) {
    int bit;

    if (br->nbits == 8) {
        int byte = fgetc(br->f);
        if (byte == EOF)
            return EOF;
        br->buffer = (uint8_t)byte;
        br->nbits = 0;
    }

    bit = (br->buffer >> (7 - br->nbits)) & 1;
    br->nbits++;
    return bit;
}
