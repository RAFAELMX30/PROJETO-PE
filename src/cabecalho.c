#include <string.h>
#include "cabecalho.h"

static const unsigned char ASSINATURA[4] = { 'A', 'B', 'C', 'Z' };

/* ---------- escrita ---------- */

static int put_u(FILE *f, uint64_t valor, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        if (fputc((int)((valor >> (8 * i)) & 0xFF), f) == EOF)
            return -1;
    }
    return 0;
}

/* grava valor em varint e devolve quantos bytes usou (ou -1) */
static int put_varint(FILE *f, uint64_t valor) {
    int n = 0;
    while (valor >= 0x80) {
        if (fputc((int)((valor & 0x7F) | 0x80), f) == EOF)
            return -1;
        valor >>= 7;
        n++;
    }
    if (fputc((int)valor, f) == EOF)
        return -1;
    return n + 1;
}

long cabecalho_escrever(FILE *f, const CabecalhoAbcz *cab) {
    long total = 0;
    int i, usados = 0, r;

    for (i = 0; i < HUFF_SIMBOLOS; i++)
        if (cab->freq[i] > 0)
            usados++;

    if (fwrite(ASSINATURA, 1, 4, f) != 4) return -1;
    if (fputc(ABCZ_VERSAO, f) == EOF) return -1;
    if (put_u(f, cab->tam_original, 8) < 0) return -1;
    if (put_u(f, cab->checksum, 4) < 0) return -1;
    if (put_u(f, (uint64_t)usados, 2) < 0) return -1;
    total = 4 + 1 + 8 + 4 + 2;

    for (i = 0; i < HUFF_SIMBOLOS; i++) {
        if (cab->freq[i] == 0)
            continue;
        if (fputc(i, f) == EOF) return -1;
        r = put_varint(f, cab->freq[i]);
        if (r < 0) return -1;
        total += 1 + r;
    }
    return total;
}

/* ---------- leitura ---------- */

static int get_u(FILE *f, int nbytes, uint64_t *out) {
    int i, c;
    uint64_t v = 0;
    for (i = 0; i < nbytes; i++) {
        c = fgetc(f);
        if (c == EOF)
            return -1;
        v |= (uint64_t)c << (8 * i);
    }
    *out = v;
    return 0;
}

static int get_varint(FILE *f, uint64_t *out) {
    uint64_t v = 0;
    int shift = 0, c;
    for (;;) {
        c = fgetc(f);
        if (c == EOF)
            return -1;
        if (shift >= 64)
            return -2; /* varint grande demais */
        v |= (uint64_t)(c & 0x7F) << shift;
        if (!(c & 0x80))
            break;
        shift += 7;
    }
    *out = v;
    return 0;
}

int cabecalho_ler(FILE *f, CabecalhoAbcz *cab) {
    unsigned char ass[4];
    uint64_t tmp, usados, soma = 0;
    int i, anterior = -1, c, r;

    memset(cab, 0, sizeof(*cab));

    if (fread(ass, 1, 4, f) != 4)
        return CAB_ERRO_IO;
    if (memcmp(ass, ASSINATURA, 4) != 0)
        return CAB_ERRO_ASSINATURA;

    c = fgetc(f);
    if (c == EOF)
        return CAB_ERRO_IO;
    if (c != ABCZ_VERSAO)
        return CAB_ERRO_VERSAO;

    if (get_u(f, 8, &tmp) < 0) return CAB_ERRO_IO;
    cab->tam_original = tmp;
    if (get_u(f, 4, &tmp) < 0) return CAB_ERRO_IO;
    cab->checksum = (uint32_t)tmp;
    if (get_u(f, 2, &usados) < 0) return CAB_ERRO_IO;
    if (usados > HUFF_SIMBOLOS)
        return CAB_ERRO_INVALIDO;

    for (i = 0; i < (int)usados; i++) {
        c = fgetc(f);
        if (c == EOF)
            return CAB_ERRO_IO;
        if (c <= anterior) /* tem que vir em ordem crescente, sem repetir */
            return CAB_ERRO_INVALIDO;
        anterior = c;

        r = get_varint(f, &tmp);
        if (r == -1) return CAB_ERRO_IO;
        if (r < 0 || tmp == 0) return CAB_ERRO_INVALIDO;
        cab->freq[c] = tmp;
        soma += tmp;
    }

    /* a soma das frequencias tem que bater com o tamanho original */
    if (soma != cab->tam_original)
        return CAB_ERRO_INVALIDO;

    return CAB_OK;
}
