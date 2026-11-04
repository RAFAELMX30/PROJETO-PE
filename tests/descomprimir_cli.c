/* uso: ./descomprimir_cli entrada.abcz saida */
#include <stdio.h>
#include <inttypes.h>
#include "descompressao.h"

int main(int argc, char *argv[]) {
    ResultadoDescompressao r;
    int ret;

    if (argc != 3) {
        printf("uso: %s entrada.abcz saida\n", argv[0]);
        return 1;
    }

    ret = descomprimir_arquivo(argv[1], argv[2], &r);
    if (ret != DECOMP_OK && ret != DECOMP_ERRO_CHECKSUM) {
        printf("erro: %s\n", decomp_mensagem_erro(ret));
        return 1;
    }

    printf("recuperado: %" PRIu64 " bytes\n", r.tam_original);
    printf("checksum armazenado:  %08" PRIx32 "\n", r.checksum_armazenado);
    printf("checksum calculado:   %08" PRIx32 "\n", r.checksum_calculado);

    if (ret == DECOMP_ERRO_CHECKSUM) {
        printf("AVISO: %s\n", decomp_mensagem_erro(ret));
        return 2;
    }
    printf("integridade: OK\n");
    return 0;
}
