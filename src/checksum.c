#include "checksum.h"

static uint32_t tabela[256];
static int tabela_pronta = 0;

static void montar_tabela(void) {
    uint32_t i, k, c;
    for (i = 0; i < 256; i++) {
        c = i;
        for (k = 0; k < 8; k++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        tabela[i] = c;
    }
    tabela_pronta = 1;
}

uint32_t checksum_crc32(uint32_t crc, const unsigned char *buf, size_t n) {
    size_t i;
    if (!tabela_pronta)
        montar_tabela();
    crc = ~crc;
    for (i = 0; i < n; i++)
        crc = tabela[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}
