#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <ctype.h>

#include "compressao.h"
#include "descompressao.h"
#include "cabecalho.h"
#include "huffman.h"
#include "checksum.h"

#define MAX_CAMINHO 1024
#define TAM_BLOCO   65536

/* Limpa caracteres de quebra de linha (\r e \n) e remove aspas ao redor */
static void normalizar_caminho(char *destino, const char *origem, size_t max_tam) {
    size_t len;
    const char *inicio = origem;
    const char *fim;

    while (*inicio == ' ' || *inicio == '\t') inicio++;

    len = strlen(inicio);
    if (len == 0) {
        destino[0] = '\0';
        return;
    }

    fim = inicio + len - 1;
    while (fim > inicio && (*fim == '\n' || *fim == '\r' || *fim == ' ' || *fim == '\t')) {
        fim--;
    }

    /* Se estiver envolto em aspas duplas ou simples, remove */
    if (*inicio == '\"' && *fim == '\"' && fim > inicio) {
        inicio++;
        fim--;
    } else if (*inicio == '\'' && *fim == '\'' && fim > inicio) {
        inicio++;
        fim--;
    }

    len = (size_t)(fim - inicio + 1);
    if (len >= max_tam)
        len = max_tam - 1;

    memcpy(destino, inicio, len);
    destino[len] = '\0';
}

/* Leitura segura de string do teclado */
static int ler_linha(char *buffer, size_t tam) {
    if (fgets(buffer, (int)tam, stdin) == NULL)
        return 0;

    /* Remove quebra de linha final */
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

/* Formata tamanho em Bytes para exibição legível (B, KB, MB) */
static void formatar_bytes(uint64_t bytes, char *saida, size_t max_tam) {
    if (bytes < 1024) {
        snprintf(saida, max_tam, "%" PRIu64 " B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(saida, max_tam, "%.2f KB (%" PRIu64 " bytes)", (double)bytes / 1024.0, bytes);
    } else {
        snprintf(saida, max_tam, "%.2f MB (%" PRIu64 " bytes)", (double)bytes / (1024.0 * 1024.0), bytes);
    }
}

/* Pausa amigável */
static void pausar(void) {
    printf("\nPressione [ENTER] para continuar...");
    while (getchar() != '\n');
}

/* =========================================================================
 * 1) COMPRESSÃO DE ARQUIVO
 * ========================================================================= */
static void executar_compressao(void) {
    char entrada_raw[MAX_CAMINHO], entrada[MAX_CAMINHO];
    char saida_raw[MAX_CAMINHO], saida[MAX_CAMINHO];
    ResultadoCompressao res;
    clock_t inicio, fim;
    double tempo_s;
    FILE *teste;
    int ret;

    printf("\n=======================================================\n");
    printf("              COMPRESSAO DE ARQUIVO (.abcz)           \n");
    printf("=======================================================\n");

    printf("Informe o caminho do arquivo original para comprimir:\n> ");
    if (!ler_linha(entrada_raw, sizeof(entrada_raw))) return;
    normalizar_caminho(entrada, entrada_raw, sizeof(entrada));

    if (strlen(entrada) == 0) {
        printf("Erro: Caminho de entrada nao pode ser vazio.\n");
        pausar();
        return;
    }

    teste = fopen(entrada, "rb");
    if (!teste) {
        printf("Erro: Nao foi possivel abrir o arquivo \"%s\". Verifique o caminho.\n", entrada);
        pausar();
        return;
    }
    fclose(teste);

    /* Sugere nome padrão adicionando .abcz */
    printf("\nInforme o caminho do arquivo de saida (ou pressione ENTER para \"%s.abcz\"):\n> ", entrada);
    if (!ler_linha(saida_raw, sizeof(saida_raw))) return;
    normalizar_caminho(saida, saida_raw, sizeof(saida));

    if (strlen(saida) == 0) {
        snprintf(saida, sizeof(saida), "%.*s.abcz", (int)(sizeof(saida) - 10), entrada);
    }

    printf("\nComprimindo \"%s\" -> \"%s\"...\n", entrada, saida);

    inicio = clock();
    ret = comprimir_arquivo(entrada, saida, &res);
    fim = clock();

    tempo_s = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (ret != COMP_OK) {
        printf("\n[FALHA] Erro na compressao: %s (codigo %d)\n", comp_mensagem_erro(ret), ret);
    } else {
        char str_orig[64], str_comp[64];
        double razao = (res.tam_original > 0) ? (double)res.tam_comprimido / (double)res.tam_original : 1.0;
        double reducao = (1.0 - razao) * 100.0;

        formatar_bytes(res.tam_original, str_orig, sizeof(str_orig));
        formatar_bytes(res.tam_comprimido, str_comp, sizeof(str_comp));

        printf("\n=======================================================\n");
        printf("           RESULTADO DA COMPRESSAO COM SUCESSO!        \n");
        printf("=======================================================\n");
        printf("  Arquivo original   : %s\n", entrada);
        printf("  Tamanho original   : %s\n", str_orig);
        printf("  Arquivo comprimido : %s\n", saida);
        printf("  Tamanho comprimido : %s\n", str_comp);
        printf("  Razao de compressao: %.4f\n", razao);
        if (reducao >= 0.0) {
            printf("  Economia de espaco : %.2f%% de reducao\n", reducao);
        } else {
            printf("  Variacao de espaco : +%.2f%% (arquivo expandiu - baixa redundancia)\n", -reducao);
        }
        printf("  Checksum CRC-32    : 0x%08" PRIX32 "\n", res.checksum);
        printf("  Tempo de execucao  : %.4f segundos (%.2f ms)\n", tempo_s, tempo_s * 1000.0);
        if (tempo_s > 0.00001 && res.tam_original > 0) {
            double mb_s = ((double)res.tam_original / (1024.0 * 1024.0)) / tempo_s;
            printf("  Velocidade media   : %.2f MB/s\n", mb_s);
        }
        printf("=======================================================\n");
    }

    pausar();
}

/* =========================================================================
 * 2) DESCOMPRESSÃO DE ARQUIVO
 * ========================================================================= */
static void executar_descompressao(void) {
    char entrada_raw[MAX_CAMINHO], entrada[MAX_CAMINHO];
    char saida_raw[MAX_CAMINHO], saida[MAX_CAMINHO];
    ResultadoDescompressao res;
    clock_t inicio, fim;
    double tempo_s;
    FILE *teste;
    int ret;

    printf("\n=======================================================\n");
    printf("             DESCOMPRESSAO DE ARQUIVO (.abcz)          \n");
    printf("=======================================================\n");

    printf("Informe o caminho do arquivo comprimido (.abcz):\n> ");
    if (!ler_linha(entrada_raw, sizeof(entrada_raw))) return;
    normalizar_caminho(entrada, entrada_raw, sizeof(entrada));

    if (strlen(entrada) == 0) {
        printf("Erro: Caminho de entrada nao pode ser vazio.\n");
        pausar();
        return;
    }

    teste = fopen(entrada, "rb");
    if (!teste) {
        printf("Erro: Nao foi possivel abrir \"%s\". Verifique o caminho.\n", entrada);
        pausar();
        return;
    }
    fclose(teste);

    /* Sugere nome removendo .abcz se existir */
    {
        char sugestao[MAX_CAMINHO];
        size_t len = strlen(entrada);
        if (len > 5 && strcmp(entrada + len - 5, ".abcz") == 0) {
            memcpy(sugestao, entrada, len - 5);
            sugestao[len - 5] = '\0';
        } else {
            snprintf(sugestao, sizeof(sugestao), "%.*s.recuperado", (int)(sizeof(sugestao) - 20), entrada);
        }

        printf("\nInforme o caminho do arquivo restaurado (ou ENTER para \"%s\"):\n> ", sugestao);
        if (!ler_linha(saida_raw, sizeof(saida_raw))) return;
        normalizar_caminho(saida, saida_raw, sizeof(saida));

        if (strlen(saida) == 0) {
            strncpy(saida, sugestao, sizeof(saida) - 1);
            saida[sizeof(saida) - 1] = '\0';
        }
    }

    printf("\nDescomprimindo \"%s\" -> \"%s\"...\n", entrada, saida);

    inicio = clock();
    ret = descomprimir_arquivo(entrada, saida, &res);
    fim = clock();

    tempo_s = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (ret != DECOMP_OK && ret != DECOMP_ERRO_CHECKSUM) {
        printf("\n[FALHA] Erro na descompressao: %s (codigo %d)\n", decomp_mensagem_erro(ret), ret);
    } else {
        char str_rec[64];
        formatar_bytes(res.tam_original, str_rec, sizeof(str_rec));

        printf("\n=======================================================\n");
        printf("              DESCOMPRESSAO FINALIZADA!                \n");
        printf("=======================================================\n");
        printf("  Arquivo restaurado   : %s\n", saida);
        printf("  Tamanho restaurado   : %s\n", str_rec);
        printf("  CRC-32 armazenado    : 0x%08" PRIX32 "\n", res.checksum_armazenado);
        printf("  CRC-32 calculado     : 0x%08" PRIX32 "\n", res.checksum_calculado);
        printf("  Tempo de execucao    : %.4f segundos (%.2f ms)\n", tempo_s, tempo_s * 1000.0);

        if (ret == DECOMP_OK) {
            printf("  Status de Integridade: [OK] ARQUIVO 100%% INTEGRO E IDENTICO!\n");
        } else {
            printf("  Status de Integridade: [ALERTA] FALHA DE CHECKSUM!\n");
            printf("  O arquivo foi corrompido ou modificado.\n");
        }
        printf("=======================================================\n");
    }

    pausar();
}

/* =========================================================================
 * 3) VISUALIZAR TABELA DE CÓDIGOS DE HUFFMAN
 * ========================================================================= */
static void visualizar_codigos_huffman(void) {
    char entrada_raw[MAX_CAMINHO], entrada[MAX_CAMINHO];
    uint64_t freq[HUFF_SIMBOLOS];
    HuffCodigo codigos[HUFF_SIMBOLOS];
    HuffNode *raiz;
    FILE *f;
    int i, simbolos_distintos = 0;
    int eh_abcz = 0;

    printf("\n=======================================================\n");
    printf("        VISUALIZAR TABELA DE CODIGOS DE HUFFMAN        \n");
    printf("=======================================================\n");
    printf("Informe o caminho de um arquivo (original ou .abcz):\n> ");
    if (!ler_linha(entrada_raw, sizeof(entrada_raw))) return;
    normalizar_caminho(entrada, entrada_raw, sizeof(entrada));

    if (strlen(entrada) == 0) {
        printf("Erro: Caminho nao pode ser vazio.\n");
        pausar();
        return;
    }

    f = fopen(entrada, "rb");
    if (!f) {
        printf("Erro: Nao foi possivel abrir \"%s\".\n", entrada);
        pausar();
        return;
    }

    memset(freq, 0, sizeof(freq));

    /* Tenta verificar se é um .abcz lendo o cabeçalho */
    {
        CabecalhoAbcz cab;
        if (cabecalho_ler(f, &cab) == CAB_OK) {
            eh_abcz = 1;
            memcpy(freq, cab.freq, sizeof(freq));
        } else {
            /* Se não for .abcz, lê o arquivo original e conta frequências */
            static unsigned char buf[TAM_BLOCO];
            size_t n;
            rewind(f);
            while ((n = fread(buf, 1, TAM_BLOCO, f)) > 0) {
                huff_contar_freq(buf, n, freq);
            }
        }
    }
    fclose(f);

    raiz = huff_construir_arvore(freq);
    if (!raiz) {
        printf("\nO arquivo esta vazio ou nao possui simbolos validos.\n");
        pausar();
        return;
    }

    huff_gerar_codigos(raiz, codigos);

    printf("\nOrigem dos dados: %s (%s)\n", entrada, eh_abcz ? "Cabecalho .abcz" : "Arquivo original");
    printf("-------------------------------------------------------------------\n");
    printf(" Byte (Hex)  Caractere   Frequencia     Tam (bits)  Codigo Huffman \n");
    printf("-------------------------------------------------------------------\n");

    for (i = 0; i < HUFF_SIMBOLOS; i++) {
        if (freq[i] > 0) {
            int k;
            char repr[16];
            unsigned char b = (unsigned char)i;
            simbolos_distintos++;

            if (b >= 32 && b <= 126) {
                snprintf(repr, sizeof(repr), "'%c'", b);
            } else if (b == '\n') {
                snprintf(repr, sizeof(repr), "'\\n'");
            } else if (b == '\t') {
                snprintf(repr, sizeof(repr), "'\\t'");
            } else if (b == '\r') {
                snprintf(repr, sizeof(repr), "'\\r'");
            } else {
                snprintf(repr, sizeof(repr), "ctrl/bin");
            }

            printf(" 0x%02X (%3d)   %-8s   %-12" PRIu64 "   %-10d  ", i, i, repr, freq[i], codigos[i].tam);
            for (k = 0; k < codigos[i].tam; k++) {
                putchar(codigos[i].bits[k] ? '1' : '0');
            }
            putchar('\n');
        }
    }

    printf("-------------------------------------------------------------------\n");
    printf("Total de simbolos distintos presentes: %d de 256\n", simbolos_distintos);

    huff_liberar_arvore(raiz);
    pausar();
}

/* =========================================================================
 * 4) TESTE DE INTEGRIDADE RÁPIDO (ROUNDTRIP)
 * ========================================================================= */
static void executar_teste_roundtrip(void) {
    char entrada_raw[MAX_CAMINHO], entrada[MAX_CAMINHO];
    char temp_comp[MAX_CAMINHO], temp_rec[MAX_CAMINHO];
    ResultadoCompressao r_comp;
    ResultadoDescompressao r_decomp;
    FILE *f_orig, *f_rec;
    clock_t t0, t1, t2;
    int ret, identicos = 1;
    size_t n1, n2;
    unsigned char b1[4096], b2[4096];

    printf("\n=======================================================\n");
    printf("     TESTE DE INTEGRIDADE AUTOMATICO (ROUNDTRIP)       \n");
    printf("=======================================================\n");
    printf("Informe um arquivo para testar compressao + descompressao:\n> ");
    if (!ler_linha(entrada_raw, sizeof(entrada_raw))) return;
    normalizar_caminho(entrada, entrada_raw, sizeof(entrada));

    if (strlen(entrada) == 0) {
        printf("Erro: Caminho nao pode ser vazio.\n");
        pausar();
        return;
    }

    snprintf(temp_comp, sizeof(temp_comp), "%.*s.__temp_teste.abcz", (int)(sizeof(temp_comp) - 30), entrada);
    snprintf(temp_rec,  sizeof(temp_rec),  "%.*s.__temp_teste.rec",  (int)(sizeof(temp_rec) - 30), entrada);

    printf("\n[1/3] Comprimindo arquivo original...\n");
    t0 = clock();
    ret = comprimir_arquivo(entrada, temp_comp, &r_comp);
    t1 = clock();
    if (ret != COMP_OK) {
        printf("Falha na compressao: %s\n", comp_mensagem_erro(ret));
        pausar();
        return;
    }

    printf("[2/3] Descomprimindo arquivo gerado...\n");
    ret = descomprimir_arquivo(temp_comp, temp_rec, &r_decomp);
    t2 = clock();
    if (ret != DECOMP_OK) {
        printf("Falha na descompressao: %s\n", decomp_mensagem_erro(ret));
        remove(temp_comp);
        remove(temp_rec);
        pausar();
        return;
    }

    printf("[3/3] Comparando arquivo original com arquivo recuperado byte a byte...\n");
    f_orig = fopen(entrada, "rb");
    f_rec  = fopen(temp_rec, "rb");

    if (!f_orig || !f_rec) {
        printf("Erro ao reabrir arquivos para checagem byte a byte.\n");
        identicos = 0;
    } else {
        while ((n1 = fread(b1, 1, sizeof(b1), f_orig)) > 0) {
            n2 = fread(b2, 1, sizeof(b2), f_rec);
            if (n1 != n2 || memcmp(b1, b2, n1) != 0) {
                identicos = 0;
                break;
            }
        }
        if (fread(b2, 1, 1, f_rec) > 0) identicos = 0;
    }
    if (f_orig) fclose(f_orig);
    if (f_rec)  fclose(f_rec);

    /* Limpeza dos arquivos temporários */
    remove(temp_comp);
    remove(temp_rec);

    printf("\n-------------------------------------------------------\n");
    printf("                RELATORIO DO TESTE                    \n");
    printf("-------------------------------------------------------\n");
    printf("  Tamanho original     : %" PRIu64 " bytes\n", r_comp.tam_original);
    printf("  Tamanho comprimido   : %" PRIu64 " bytes (%.2f%% do original)\n",
           r_comp.tam_comprimido,
           r_comp.tam_original ? (100.0 * (double)r_comp.tam_comprimido / (double)r_comp.tam_original) : 100.0);
    printf("  Tempo compressao     : %.4f s\n", (double)(t1 - t0) / CLOCKS_PER_SEC);
    printf("  Tempo descompressao  : %.4f s\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
    printf("  CRC-32 original      : 0x%08" PRIX32 "\n", r_comp.checksum);
    printf("  CRC-32 recuperado    : 0x%08" PRIX32 "\n", r_decomp.checksum_calculado);
    printf("  Comparacao binaria   : %s\n", identicos ? "[PASSOU] 100% Identicos byte a byte" : "[FALHOU] Diferenca encontrada!");
    printf("-------------------------------------------------------\n");

    pausar();
}

/* =========================================================================
 * 5) SOBRE O PROJETO / INTEGRANTES
 * ========================================================================= */
static void exibir_sobre(void) {
    printf("\n=======================================================\n");
    printf("      GarchY / PROJETO-PE — COMPACTADOR HUFFMAN        \n");
    printf("=======================================================\n");
    printf(" Disciplina: Programacao Estruturada (MCTA028-15)\n");
    printf(" Instituicao: Universidade Federal do ABC (UFABC)\n");
    printf(" Quadrimestre: 2026.3\n");
    printf(" Professores: Roberto Sadao Yokoyama\n");
    printf("              Luneque Del Rio de Souza e Silva Junior\n\n");
    printf(" Equipe (Discentes):\n");
    printf("  * Jose Ricardo Feltran Junior (RA: 11202421133) - Integrante 1\n");
    printf("  * Rafael Maximiano Luiz (RA: 11202432420) - Integrante 2\n");
    printf("  * Renan Henrique Ferreira Monteiro (RA: 11202020303) - Integrante 3\n");
    printf("  * Rodrigo Martel Poggi de Araujo (RA: 11202522393) - Integrante 4\n");
    printf("=======================================================\n");
    pausar();
}

/* =========================================================================
 * MENU PRINCIPAL
 * ========================================================================= */
int main(void) {
    char opcao_str[32];
    int opcao;

    for (;;) {
        printf("\n=======================================================\n");
        printf("           GarchY — Compactador de Arquivos           \n");
        printf("               UFABC - P.E. - 2026.3                  \n");
        printf("=======================================================\n");
        printf("  1) Comprimir arquivo (.abcz)\n");
        printf("  2) Descomprimir arquivo (.abcz)\n");
        printf("  3) Visualizar tabela de codigos de Huffman\n");
        printf("  4) Executar teste de integridade rapido (Roundtrip)\n");
        printf("  5) Sobre o projeto e equipe\n");
        printf("  0) Sair\n");
        printf("=======================================================\n");
        printf("Escolha uma opcao: ");

        if (!ler_linha(opcao_str, sizeof(opcao_str))) {
            break;
        }

        opcao = atoi(opcao_str);

        switch (opcao) {
        case 1:
            executar_compressao();
            break;
        case 2:
            executar_descompressao();
            break;
        case 3:
            visualizar_codigos_huffman();
            break;
        case 4:
            executar_teste_roundtrip();
            break;
        case 5:
            exibir_sobre();
            break;
        case 0:
            printf("\nEncerrando o programa. Ate mais!\n");
            return 0;
        default:
            printf("\nOpcao invalida! Digite um numero de 0 a 5.\n");
            pausar();
            break;
        }
    }

    return 0;
}
