#include <stdio.h>
#include <string.h>
#include "compressao.h"
#include "huffman.h"
#include "cabecalho.h"
#include "checksum.h"
#include "bitio.h"

#define TAM_BLOCO 65536

/* arquivos grandes sao lidos em blocos, nao ficam inteiros na memoria.
   por isso o arquivo de entrada e lido duas vezes:
     1a passada: conta frequencias e calcula o checksum
     2a passada: troca cada byte pelo seu codigo de Huffman */

const char *comp_mensagem_erro(int codigo) {
    switch (codigo) {
    case COMP_OK:                  return "ok";
    case COMP_ERRO_ABRIR_ENTRADA:  return "nao foi possivel abrir o arquivo de entrada";
    case COMP_ERRO_ABRIR_SAIDA:    return "nao foi possivel criar o arquivo de saida";
    case COMP_ERRO_LEITURA:        return "erro ao ler o arquivo de entrada";
    case COMP_ERRO_ESCRITA:        return "erro ao gravar o arquivo de saida";
    case COMP_ERRO_MEMORIA:        return "memoria insuficiente";
    case COMP_ERRO_MESMO_ARQUIVO:  return "entrada e saida nao podem ser o mesmo arquivo";
    default:                       return "erro desconhecido";
    }
}

int comprimir_arquivo(const char *caminho_entrada, const char *caminho_saida,
                      ResultadoCompressao *resultado) {
    static unsigned char bloco[TAM_BLOCO];
    static HuffCodigo codigos[HUFF_SIMBOLOS];
    CabecalhoAbcz cab;
    BitWriter bw;
    HuffNode *raiz = NULL;
    FILE *ent = NULL, *sai = NULL;
    uint64_t bits_total = 0, lidos2 = 0;
    long tam_cab;
    size_t n, i;
    int j, ret = COMP_OK, saida_criada = 0;

    if (strcmp(caminho_entrada, caminho_saida) == 0)
        return COMP_ERRO_MESMO_ARQUIVO;

    ent = fopen(caminho_entrada, "rb");
    if (ent == NULL)
        return COMP_ERRO_ABRIR_ENTRADA;

    /* ---- 1a passada: frequencias + checksum ---- */
    memset(&cab, 0, sizeof(cab));
    while ((n = fread(bloco, 1, TAM_BLOCO, ent)) > 0) {
        huff_contar_freq(bloco, n, cab.freq);
        cab.checksum = checksum_crc32(cab.checksum, bloco, n);
        cab.tam_original += n;
    }
    if (ferror(ent)) {
        ret = COMP_ERRO_LEITURA;
        goto fim;
    }

    /* ---- arvore e codigos (integrante 1) ---- */
    raiz = huff_construir_arvore(cab.freq);
    if (raiz == NULL && cab.tam_original > 0) {
        ret = COMP_ERRO_MEMORIA;
        goto fim;
    }
    huff_gerar_codigos(raiz, codigos); /* aceita raiz NULL (arquivo vazio) */

    for (j = 0; j < HUFF_SIMBOLOS; j++)
        bits_total += cab.freq[j] * (uint64_t)codigos[j].tam;

    /* ---- cabecalho ---- */
    sai = fopen(caminho_saida, "wb");
    if (sai == NULL) {
        ret = COMP_ERRO_ABRIR_SAIDA;
        goto fim;
    }
    saida_criada = 1;

    tam_cab = cabecalho_escrever(sai, &cab);
    if (tam_cab < 0) {
        ret = COMP_ERRO_ESCRITA;
        goto fim;
    }

    /* ---- 2a passada: dados comprimidos ---- */
    if (cab.tam_original > 0) {
        if (fseek(ent, 0, SEEK_SET) != 0) {
            ret = COMP_ERRO_LEITURA;
            goto fim;
        }
        bw_init(&bw, sai);
        while ((n = fread(bloco, 1, TAM_BLOCO, ent)) > 0) {
            lidos2 += n;
            for (i = 0; i < n; i++) {
                const HuffCodigo *c = &codigos[bloco[i]];
                int k;
                for (k = 0; k < c->tam; k++) {
                    if (bw_put_bit(&bw, c->bits[k]) < 0) {
                        ret = COMP_ERRO_ESCRITA;
                        goto fim;
                    }
                }
            }
        }
        if (ferror(ent) || lidos2 != cab.tam_original) { /* arquivo mudou no meio? */
            ret = COMP_ERRO_LEITURA;
            goto fim;
        }
        if (bw_flush(&bw) < 0) {
            ret = COMP_ERRO_ESCRITA;
            goto fim;
        }
    }

    /* fclose pode revelar erro de escrita pendente */
    if (fclose(sai) != 0) {
        sai = NULL;
        ret = COMP_ERRO_ESCRITA;
        goto fim;
    }
    sai = NULL;

    if (resultado != NULL) {
        resultado->tam_original = cab.tam_original;
        resultado->tam_comprimido = (uint64_t)tam_cab + (bits_total + 7) / 8;
        resultado->checksum = cab.checksum;
        memcpy(resultado->freq, cab.freq, sizeof(cab.freq));
    }

fim:
    huff_liberar_arvore(raiz);
    if (ent != NULL)
        fclose(ent);
    if (sai != NULL)
        fclose(sai);
    if (ret != COMP_OK && saida_criada)
        remove(caminho_saida); /* nao deixa .abcz pela metade */
    return ret;
}
