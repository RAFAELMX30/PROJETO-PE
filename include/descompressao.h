#ifndef DESCOMPRESSAO_H
#define DESCOMPRESSAO_H

#include <stdint.h>

#define DECOMP_OK                   0
#define DECOMP_ERRO_ABRIR_ENTRADA  -1
#define DECOMP_ERRO_ABRIR_SAIDA    -2
#define DECOMP_ERRO_LEITURA        -3
#define DECOMP_ERRO_ESCRITA        -4
#define DECOMP_ERRO_MEMORIA        -5
#define DECOMP_ERRO_MESMO_ARQUIVO  -6
#define DECOMP_ERRO_FORMATO        -7
#define DECOMP_ERRO_CHECKSUM       -8

typedef struct {
    uint64_t tam_original;
    uint32_t checksum_armazenado;
    uint32_t checksum_calculado;
} ResultadoDescompressao;

/* descomprime caminho_entrada (.abcz) e grava o original em caminho_saida.
   resultado pode ser NULL. em erro de leitura/memoria/formato o arquivo
   parcial e apagado; em erro de checksum o arquivo e mantido.
   retorna DECOMP_OK ou um dos DECOMP_ERRO_* */
int descomprimir_arquivo(const char *caminho_entrada, const char *caminho_saida,
                         ResultadoDescompressao *resultado);

const char *decomp_mensagem_erro(int codigo);

#endif
