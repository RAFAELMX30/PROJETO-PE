#include <stdio.h>
#include <string.h>
#include "descompressao.h"
#include "cabecalho.h"
#include "huffman.h"
#include "checksum.h"
#include "bitio.h"

#define TAM_BLOCO 65536

const char *decomp_mensagem_erro(int codigo) {
    switch (codigo) {
    case DECOMP_OK:                  return "ok";
    case DECOMP_ERRO_ABRIR_ENTRADA:  return "nao foi possivel abrir o arquivo de entrada";
    case DECOMP_ERRO_ABRIR_SAIDA:    return "nao foi possivel criar o arquivo de saida";
    case DECOMP_ERRO_LEITURA:        return "erro ao ler o arquivo comprimido";
    case DECOMP_ERRO_ESCRITA:        return "erro ao gravar o arquivo de saida";
    case DECOMP_ERRO_MEMORIA:        return "memoria insuficiente";
    case DECOMP_ERRO_MESMO_ARQUIVO:  return "entrada e saida nao podem ser o mesmo arquivo";
    case DECOMP_ERRO_FORMATO:        return "arquivo invalido ou corrompido";
    case DECOMP_ERRO_CHECKSUM:       return "integridade comprometida: checksum diferente";
    default:                         return "erro desconhecido";
    }
}

int descomprimir_arquivo(const char *caminho_entrada, const char *caminho_saida,
                         ResultadoDescompressao *resultado) {
    static unsigned char bloco[TAM_BLOCO];
    CabecalhoAbcz cab;
    BitReader br;
    HuffNode *raiz = NULL, *no;
    FILE *ent = NULL, *sai = NULL;
    uint64_t i;
    size_t pos = 0;
    uint32_t crc = 0;
    int bit, ret = DECOMP_OK, saida_criada = 0, cab_ret;

    if (strcmp(caminho_entrada, caminho_saida) == 0)
        return DECOMP_ERRO_MESMO_ARQUIVO;

    ent = fopen(caminho_entrada, "rb");
    if (ent == NULL)
        return DECOMP_ERRO_ABRIR_ENTRADA;

    cab_ret = cabecalho_ler(ent, &cab);
    if (cab_ret != CAB_OK) {
        ret = (cab_ret == CAB_ERRO_IO) ? DECOMP_ERRO_LEITURA : DECOMP_ERRO_FORMATO;
        goto fim;
    }

    sai = fopen(caminho_saida, "wb");
    if (sai == NULL) {
        ret = DECOMP_ERRO_ABRIR_SAIDA;
        goto fim;
    }
    saida_criada = 1;

    if (cab.tam_original == 0)
        goto verificar;

    raiz = huff_construir_arvore(cab.freq);
    if (raiz == NULL) {
        ret = DECOMP_ERRO_MEMORIA;
        goto fim;
    }

    br_init(&br, ent);

    for (i = 0; i < cab.tam_original; i++) {
        no = raiz;
        while (!huff_eh_folha(no)) {
            bit = br_get_bit(&br);
            if (bit == EOF) { ret = DECOMP_ERRO_LEITURA; goto fim; }
            no = bit ? no->dir : no->esq;
            if (no == NULL) { ret = DECOMP_ERRO_FORMATO; goto fim; }
        }
        bloco[pos++] = no->byte;
        if (pos == TAM_BLOCO) {
            crc = checksum_crc32(crc, bloco, pos);
            if (fwrite(bloco, 1, pos, sai) != pos) { ret = DECOMP_ERRO_ESCRITA; goto fim; }
            pos = 0;
        }
    }

    if (pos > 0) {
        crc = checksum_crc32(crc, bloco, pos);
        if (fwrite(bloco, 1, pos, sai) != pos) { ret = DECOMP_ERRO_ESCRITA; goto fim; }
    }

verificar:
    if (crc != cab.checksum)
        ret = DECOMP_ERRO_CHECKSUM;

    if (fclose(sai) != 0 && ret == DECOMP_OK)
        ret = DECOMP_ERRO_ESCRITA;
    sai = NULL;

    if (resultado != NULL) {
        resultado->tam_original        = cab.tam_original;
        resultado->checksum_armazenado = cab.checksum;
        resultado->checksum_calculado  = crc;
    }

fim:
    huff_liberar_arvore(raiz);
    if (ent != NULL) fclose(ent);
    if (sai != NULL) fclose(sai);
    /* em erro de checksum o arquivo e mantido; nos outros erros e apagado */
    if (ret != DECOMP_OK && ret != DECOMP_ERRO_CHECKSUM && saida_criada)
        remove(caminho_saida);
    return ret;
}
