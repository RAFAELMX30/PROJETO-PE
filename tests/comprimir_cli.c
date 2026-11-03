/* programa so pra testar a compressao sozinha (o menu de verdade e do integrante 4)
   uso: ./comprimir entrada saida.abcz */
#include <stdio.h>
#include <inttypes.h>
#include "compressao.h"

int main(int argc, char *argv[]) {
    ResultadoCompressao r;
    int ret;

    if (argc != 3) {
        printf("uso: %s entrada saida.abcz\n", argv[0]);
        return 1;
    }

    ret = comprimir_arquivo(argv[1], argv[2], &r);
    if (ret != COMP_OK) {
        printf("erro: %s\n", comp_mensagem_erro(ret));
        return 1;
    }

    printf("original:   %" PRIu64 " bytes\n", r.tam_original);
    printf("comprimido: %" PRIu64 " bytes\n", r.tam_comprimido);
    if (r.tam_original > 0)
        printf("taxa:       %.2f%%\n", 100.0 * (double)r.tam_comprimido / (double)r.tam_original);
    printf("checksum:   %08" PRIx32 "\n", r.checksum);
    return 0;
}
