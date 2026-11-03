#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

/* CRC-32 (mesmo polinomio do gzip/zip).
   comece com crc = 0 e va chamando bloco por bloco:
       crc = checksum_crc32(crc, bloco1, n1);
       crc = checksum_crc32(crc, bloco2, n2);
   o resultado final e o checksum do arquivo inteiro.
   a descompressao usa a mesma funcao pra conferir o arquivo recuperado. */
uint32_t checksum_crc32(uint32_t crc, const unsigned char *buf, size_t n);

#endif
