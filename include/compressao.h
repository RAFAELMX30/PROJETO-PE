#ifndef COMPRESSAO_H
#define COMPRESSAO_H

#include <stdint.h>
#include "huffman.h"

/* codigos de retorno de comprimir_arquivo */
#define COMP_OK                    0
#define COMP_ERRO_ABRIR_ENTRADA   -1
#define COMP_ERRO_ABRIR_SAIDA     -2
#define COMP_ERRO_LEITURA         -3
#define COMP_ERRO_ESCRITA         -4
#define COMP_ERRO_MEMORIA         -5
#define COMP_ERRO_MESMO_ARQUIVO   -6  /* entrada e saida com o mesmo caminho */

/* dados que o menu pode usar pra mostrar as estatisticas */
typedef struct {
    uint64_t tam_original;    /* bytes do arquivo original */
    uint64_t tam_comprimido;  /* bytes do .abcz (cabecalho + dados) */
    uint32_t checksum;        /* CRC-32 do arquivo original */
    uint64_t freq[HUFF_SIMBOLOS]; /* frequencia de cada byte (pra mostrar os codigos:
                                     huff_construir_arvore + huff_gerar_codigos) */
} ResultadoCompressao;

/* comprime caminho_entrada e grava o resultado em caminho_saida (.abcz).
   resultado pode ser NULL se nao precisar das estatisticas.
   se der erro, o arquivo de saida parcial e apagado.
   retorna COMP_OK ou um dos COMP_ERRO_* */
int comprimir_arquivo(const char *caminho_entrada, const char *caminho_saida,
                      ResultadoCompressao *resultado);

/* texto curto explicando o codigo de erro (pra o menu mostrar) */
const char *comp_mensagem_erro(int codigo);

#endif
