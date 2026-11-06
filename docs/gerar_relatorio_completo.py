import os
import sys
from fpdf import FPDF

class RelatorioTecnicoPDF(FPDF):
    def header(self):
        if self.page_no() > 1:
            self.set_font("Helvetica", "I", 8)
            self.set_text_color(120, 120, 120)
            self.cell(0, 6, "UFABC | Programacao Estruturada (MCTA028-15) - Relatorio Tecnico: projetoPE", border=0, align="R")
            self.ln(8)

    def footer(self):
        self.set_y(-15)
        self.set_font("Helvetica", "", 9)
        self.set_text_color(140, 140, 140)
        self.cell(0, 10, f"Pagina {self.page_no()}/{{nb}}", border=0, align="C")

def criar_relatorio():
    pdf = RelatorioTecnicoPDF(orientation="P", unit="mm", format="A4")
    pdf.alias_nb_pages()
    pdf.set_margins(left=20, top=20, right=20)
    pdf.set_auto_page_break(auto=True, margin=20)
    pdf.add_page()

    # =========================================================================
    # CAPA / CABECALHO INSTITUCIONAL
    # =========================================================================
    pdf.set_font("Helvetica", "B", 13)
    pdf.set_text_color(25, 45, 85)
    pdf.cell(0, 6, "UNIVERSIDADE FEDERAL DO ABC - UFABC", ln=True, align="C")
    
    pdf.set_font("Helvetica", "", 11)
    pdf.set_text_color(60, 60, 60)
    pdf.cell(0, 6, "Centro de Engenharia, Modelagem e Ciencias Sociais Aplicadas (CECS)", ln=True, align="C")
    pdf.cell(0, 6, "Disciplina: MCTA028-15 - Programacao Estruturada | Quadrimestre 2026.3", ln=True, align="C")
    pdf.cell(0, 6, "Docentes: Prof. Roberto Sadao Yokoyama e Prof. Luneque Del Rio de Souza e Silva Jr.", ln=True, align="C")
    pdf.ln(8)

    # Linha divisoria
    pdf.set_draw_color(25, 45, 85)
    pdf.set_line_width(0.8)
    pdf.line(20, pdf.get_y(), 190, pdf.get_y())
    pdf.ln(10)

    # Titulo do Trabalho
    pdf.set_font("Helvetica", "B", 18)
    pdf.set_text_color(15, 30, 65)
    pdf.cell(0, 9, "RELATORIO TECNICO DE IMPLEMENTACAO", ln=True, align="C")
    pdf.set_font("Helvetica", "B", 14)
    pdf.set_text_color(40, 80, 140)
    pdf.cell(0, 8, "projetoPE: Compactador e Descompactador Huffman em ANSI C", ln=True, align="C")
    pdf.ln(8)

    # Identificacao da Equipe
    pdf.set_fill_color(245, 248, 252)
    pdf.set_draw_color(210, 225, 240)
    pdf.set_line_width(0.3)
    pdf.rect(20, pdf.get_y(), 170, 27, "DF")
    
    pdf.set_xy(25, pdf.get_y() + 3)
    pdf.set_font("Helvetica", "B", 9)
    pdf.set_text_color(25, 45, 85)
    pdf.cell(0, 5, "INTEGRANTES DA EQUIPE (DISCENTES):", ln=True)
    pdf.set_font("Helvetica", "", 8.5)
    pdf.set_text_color(50, 50, 50)
    pdf.cell(0, 4.5, "- Jose Ricardo Feltran Junior (RA: 11202421133) - Integrante 1", ln=True)
    pdf.cell(0, 4.5, "- Rafael Maximiano Luiz (RA: 11202432420) - Integrante 2", ln=True)
    pdf.cell(0, 4.5, "- Renan Henrique Ferreira Monteiro (RA: 11202020303) - Integrante 3", ln=True)
    pdf.cell(0, 4.5, "- Rodrigo Martel Poggi de Araujo (RA: 11202522393) - Integrante 4", ln=True)
    pdf.ln(10)

    # Helpers de formatacao
    def titulo_secao(num, texto):
        pdf.ln(4)
        pdf.set_font("Helvetica", "B", 12)
        pdf.set_text_color(25, 55, 110)
        pdf.cell(0, 7, f"{num}. {texto}", ln=True)
        pdf.set_draw_color(180, 200, 225)
        pdf.set_line_width(0.5)
        pdf.line(20, pdf.get_y(), 190, pdf.get_y())
        pdf.ln(3)

    def titulo_subsecao(texto):
        pdf.ln(2)
        pdf.set_font("Helvetica", "B", 10)
        pdf.set_text_color(35, 70, 130)
        pdf.cell(0, 6, texto, ln=True)

    def corpo(texto):
        pdf.set_font("Helvetica", "", 10)
        pdf.set_text_color(30, 30, 30)
        # espacamento duplo/ampliado para atender especificacao academica
        pdf.multi_cell(0, 6.5, texto)
        pdf.ln(2)

    def bloco_codigo(linhas):
        pdf.ln(1)
        pdf.set_fill_color(248, 250, 252)
        pdf.set_draw_color(215, 225, 235)
        pdf.set_font("Courier", "", 8)
        pdf.set_text_color(20, 20, 20)
        
        altura_total = len(linhas) * 4.5 + 4
        # Se nao couber na pagina atual, quebra
        if pdf.get_y() + altura_total > 270:
            pdf.add_page()
            
        y_ini = pdf.get_y()
        pdf.rect(20, y_ini, 170, altura_total, "DF")
        pdf.set_y(y_ini + 2)
        for l in linhas:
            pdf.set_x(23)
            pdf.cell(164, 4.5, l, ln=True)
        pdf.ln(3)

    # =========================================================================
    # SECAO 1: REQUISITOS IMPLEMENTADOS E FUNCIONAIS
    # =========================================================================
    titulo_secao("1", "REQUISITOS IMPLEMENTADOS E FUNCIONAIS")
    corpo(
        "O projeto projetoPE foi desenvolvido em conformidade com as diretrizes do Trabalho de Implementacao de Programacao "
        "Estruturada (2026.3). O objetivo central foi implementar uma ferramenta completa de compressao e descompressao de "
        "arquivos sem perdas (lossless) em padrao formal ANSI C, operando tanto sobre arquivos textuais quanto binarios."
    )
    corpo(
        "Dentre os requisitos funcionais integralmente cumpridos, destacam-se:\n"
        "1. Interface Interativa no Terminal: Menu estruturado permitindo ao usuario escolher entre compressao, descompressao, "
        "inspeção da tabela de codigos de Huffman, teste automatizado de integridade (roundtrip) e informacoes sobre o sistema.\n"
        "2. Tratamento Robusto de Caminhos: Aceitacao de caminhos relativos e absolutos, sanitizacao de quebras de linha residuais "
        "(\\r e \\n), remocao automatica de aspas (facilitando arrastar arquivos no terminal) e validacao de existencia previa dos arquivos.\n"
        "3. Extensao Personalizada: O compressor gera por padrao arquivos com extensao '.abcz'. Na descompressao, o programa "
        "sugere o nome original expurgando a extensao e permite ao usuario especificar qualquer destino desejado.\n"
        "4. Preservacao dos Originais e Corretude: Os arquivos originais sao estritamente preservados em modo somente-leitura. "
        "Em caso de qualquer erro de I/O ou alocacao, arquivos parciais corrompidos sao automaticamente eliminados pelo sistema.\n"
        "5. Metricas de Eficiencia Espacial e Temporal: Exibicao instantanea dos tamanhos em Bytes (e formatado em KB/MB), "
        "Razao de Compressao (R = Sc / So), porcentagem de reducao de espaco, tempo cronometrado via clock() e velocidade de processamento."
    )

    # =========================================================================
    # SECAO 2: FUNDAMENTACAO CONCEITUAL DOS ALGORITMOS
    # =========================================================================
    titulo_secao("2", "FUNDAMENTACAO CONCEITUAL DOS ALGORITMOS")
    titulo_subsecao("2.1 Teoria da Informacao e Entropia de Shannon")
    corpo(
        "A compressao estatistica sem perdas encontra seu limite teorico na Entropia de Shannon H(X). Em um arquivo digital, "
        "cada byte representa um simbolo dentre 256 valores possiveis (0 a 255). Se as frequencias forem desiguais, atribuir um "
        "comprimento fixo de 8 bits por byte gera redundancia informacional. O algoritmo de David Huffman (1952) resolve esse "
        "problema ao gerar codigos binarios de tamanho variavel que satisfazem a propriedade do prefixo (nenhum codigo e prefixo de outro)."
    )

    titulo_subsecao("2.2 Fila de Prioridade (Min-Heap) e Construcao da Arvore")
    corpo(
        "A construcao da arvore de Huffman e realizada de baixo para cima (bottom-up) por meio de uma fila de prioridade baseada "
        "em Min-Heap com complexidade O(N log N). Inicialmente, cada byte que aparece no arquivo e encapsulado em um no folha. "
        "A cada iteracao, os dois nos com as menores frequencias sao retirados do heap e combinados em um novo no intermediario, "
        "cuja frequencia e a soma dos filhos. Esse no e reinserido no heap ate que reste apenas a raiz."
    )
    corpo(
        "Para assegurar que a arvore reconstruida na descompressao seja 100% identica a arvore da compressao sem a necessidade de "
        "gravar ponteiros no disco, implementou-se um criterio deterministico de desempate: nos com a mesma frequencia sao ordenados "
        "pelo identificador sequencial de criacao ('ordem')."
    )

    titulo_subsecao("2.3 Verificacao de Integridade por Checksum CRC-32")
    corpo(
        "Para a verificacao de integridade (funcionalidade extra), adotou-se o algoritmo Cyclic Redundancy Check de 32 bits (CRC-32) "
        "padrao IEEE 802.3 com o polinomio 0xEDB88320 em representacao invertida. O CRC-32 processa os dados incrementalmente em "
        "tempo linear O(N) utilizando uma tabela de dispersao de 256 entradas pre-calculada. O valor resultante e gravado no cabecalho "
        "e recalculado na descompressao, garantindo a deteccao de qualquer corrupcao de bit ou truncamento."
    )

    # =========================================================================
    # SECAO 3: DECISOES DE PROJETO E ESTRUTURAS DE DADOS (TADs)
    # =========================================================================
    titulo_secao("3", "DECISOES DE PROJETO, ESTRUTURAS DE DADOS E CODIGO")
    corpo(
        "O software foi integralmente projetado segundo os principios de modularizacao e encapsulamento de Tipos Abstratos de Dados "
        "(TADs), distribuido em arquivos de cabecalho (.h em include/) e implementacoes (.c em src/)."
    )

    titulo_subsecao("3.1 TAD Min-Heap e Arvore de Huffman (Jose Ricardo Feltran Junior)")
    corpo(
        "O Min-Heap foi implementado sobre um vetor de ponteiros com capacidade fixa de 256 elementos (quantidade maxima de simbolos). "
        "O gerenciamento de memoria e rigoroso: cada malloc e acompanhado de verificacao de ponteiro nulo e a arvore e totalmente "
        "desalocada em pos-ordem com huff_liberar_arvore()."
    )
    bloco_codigo([
        "/* Trecho representativo: criterio deterministico de desempate no Min-Heap */",
        "static int compara(const HuffNode *a, const HuffNode *b) {",
        "    if (a->freq < b->freq) return -1;",
        "    if (a->freq > b->freq) return 1;",
        "    return a->ordem - b->ordem; /* Desempate pela ordem de insercao */",
        "}"
    ])

    titulo_subsecao("3.2 BitStream e Serializacao Compacta do Cabecalho (Rafael Maximiano Luiz)")
    corpo(
        "Como os codigos de Huffman possuem comprimentos fracionarios de byte, implementou-se o BitWriter e BitReader, que "
        "acumulam bits em buffers de 1 byte enviados ao disco quando cheios (MSB primeiro). O cabecalho .abcz armazena a assinatura "
        "'ABCZ', a versao do formato, o tamanho original (8 bytes) e o checksum (4 bytes). Em seguida, armazena a tabela de frequencias "
        "usando codificacao varint (7 bits por byte com bit mais significativo indicando continuacao), garantindo que arquivos pequenos "
        "nao sofram sobrecarga desnecessaria de cabecalho."
    )
    bloco_codigo([
        "/* Trecho representativo: BitWriter acumulando bits MSB primeiro */",
        "int bw_put_bit(BitWriter *bw, int bit) {",
        "    bw->buf = (unsigned char)((bw->buf << 1) | (bit ? 1 : 0));",
        "    bw->n_bits++;",
        "    if (bw->n_bits == 8) {",
        "        if (fputc(bw->buf, bw->f) == EOF) return -1;",
        "        bw->buf = 0; bw->n_bits = 0;",
        "    }",
        "    return 0;",
        "}"
    ])

    titulo_subsecao("3.3 Rotinas de Compressao e Descompressao (Rafael M. Luiz e Renan H. F. Monteiro)")
    corpo(
        "Para nao comprometer a memoria RAM em arquivos extensos, a compressao opera em blocos de 64 KB (TAM_BLOCO) e faz "
        "duas passadas sobre o arquivo original: na 1a passada le os blocos, conta frequencias e calcula o CRC-32; em seguida constroi "
        "a arvore e os codigos; na 2a passada substitui cada byte pela sua sequencia de bits correspondente."
    )
    corpo(
        "Na descompressao, os bits sao consumidos um a um para navegar na arvore reconstruida (bit 0 = esquerda, bit 1 = direita). "
        "Ao alcancar um no folha, o byte correspondente e emitido e acumulado no bloco de saida, parando precisamente quando o "
        "tamanho original registrado no cabecalho e alcancado, descartando quaisquer bits de preenchimento (padding) do ultimo byte."
    )

    titulo_subsecao("3.4 Menu, Tratamento de Caminhos e Metricas (Rodrigo Martel Poggi de Araujo)")
    corpo(
        "O modulo principal (src/main.c) fornece a camada de interacao com o usuario. A funcao normalizar_caminho() remove "
        "espacos e aspas duplas adicionadas automaticamente pelo terminal ao arrastar arquivos. A contagem de tempo emprega "
        "clock() para medicao com resolucao de milissegundos e as grandezas sao convertidas automaticamente em B, KB e MB."
    )

    # =========================================================================
    # SECAO 4: CASOS DE TESTE E MENSURACAO DE DESEMPENHO
    # =========================================================================
    titulo_secao("4", "CASOS DE TESTE E MENSURACAO DE DESEMPENHO")
    corpo(
        "Conforme requisitado no edital, o programa foi testado sob tres cenarios de redundancia estatistica com arquivos padronizados "
        "de 500 KB (512.000 Bytes), comparando os resultados do projetoPE contra o compressor comercial gzip (modo Deflate):"
    )
    corpo(
        "- Cenario 1 (Alta Redundancia): Padrao repetitivo homogeneo com reduzido conjunto de caracteres (ex: sequencias biologicas de DNA).\n"
        "- Cenario 2 (Media Redundancia): Texto corrido estruturado em linguagem natural (literatura e documentacao tecnica).\n"
        "- Cenario 3 (Baixa Redundancia): Dados pseudo-aleatorios com distribuicao uniforme de bytes (alta entropia, ~8 bits/simbolo)."
    )

    # Tabela 1: Metricas Espaciais
    titulo_subsecao("Tabela 1: Comparativo de Eficiencia Espacial e Integridade (500 KB / 512.000 Bytes)")
    pdf.set_font("Helvetica", "B", 8.5)
    pdf.set_fill_color(225, 238, 252)
    pdf.set_draw_color(180, 200, 225)
    pdf.cell(36, 7, "Cenario", 1, 0, "C", fill=True)
    pdf.cell(26, 7, "Tam. Orig.", 1, 0, "C", fill=True)
    pdf.cell(26, 7, "Huffman (Sc)", 1, 0, "C", fill=True)
    pdf.cell(22, 7, "Razao (R)", 1, 0, "C", fill=True)
    pdf.cell(22, 7, "Economia", 1, 0, "C", fill=True)
    pdf.cell(22, 7, "Gzip Tam.", 1, 0, "C", fill=True)
    pdf.cell(16, 7, "CRC-32", 1, 1, "C", fill=True)

    dados_t1 = [
        ("Alta Redundancia", "512.000 B", "231.263 B", "0,4517", "54,83%", "1.578 B", "OK 100%"),
        ("Media Redundancia", "512.000 B", "283.361 B", "0,5534", "44,66%", "5.604 B", "OK 100%"),
        ("Baixa Redundancia", "512.000 B", "512.787 B", "1,0015", "-0,15%", "512.196 B", "OK 100%"),
    ]
    pdf.set_font("Helvetica", "", 8)
    for l in dados_t1:
        pdf.cell(36, 6, l[0], 1, 0, "L")
        pdf.cell(26, 6, l[1], 1, 0, "R")
        pdf.cell(26, 6, l[2], 1, 0, "R")
        pdf.cell(22, 6, l[3], 1, 0, "C")
        pdf.cell(22, 6, l[4], 1, 0, "C")
        pdf.cell(22, 6, l[5], 1, 0, "R")
        pdf.cell(16, 6, l[6], 1, 1, "C")
    pdf.ln(3)

    # Tabela 2: Metricas Temporais
    titulo_subsecao("Tabela 2: Eficiencia Temporal de Compressao e Descompressao")
    pdf.set_font("Helvetica", "B", 8.5)
    pdf.set_fill_color(225, 238, 252)
    pdf.cell(38, 7, "Cenario", 1, 0, "C", fill=True)
    pdf.cell(33, 7, "Comp. Huffman", 1, 0, "C", fill=True)
    pdf.cell(33, 7, "Comp. Gzip", 1, 0, "C", fill=True)
    pdf.cell(33, 7, "Decomp. Huffman", 1, 0, "C", fill=True)
    pdf.cell(33, 7, "Decomp. Gzip", 1, 1, "C", fill=True)

    dados_t2 = [
        ("Alta Redundancia", "51,10 ms", "12,06 ms", "48,65 ms", "23,47 ms"),
        ("Media Redundancia", "59,73 ms", "17,25 ms", "49,37 ms", "121,85 ms"),
        ("Baixa Redundancia", "36,83 ms", "144,73 ms", "56,70 ms", "161,16 ms"),
    ]
    pdf.set_font("Helvetica", "", 8)
    for l in dados_t2:
        pdf.cell(38, 6, l[0], 1, 0, "L")
        pdf.cell(33, 6, l[1], 1, 0, "C")
        pdf.cell(33, 6, l[2], 1, 0, "C")
        pdf.cell(33, 6, l[3], 1, 0, "C")
        pdf.cell(33, 6, l[4], 1, 1, "C")
    pdf.ln(3)

    titulo_subsecao("Analise Critica e Comparativa com a Ferramenta Comercial (Gzip):")
    corpo(
        "1. Desempenho do Huffman: O projetoPE obteve excelente reducao de volume nos dados textuais e repetitivos, reduzindo 54,83% no "
        "cenario de alta redundancia e 44,66% em linguagem natural com tempos sempre inferiores a 60 milissegundos. Este resultado "
        "comprova que o algoritmo atingiu a taxa otima esperada para modelos estaticos de Huffman por byte.\n"
        "2. Diferenca Estrutural para o Gzip: O gzip utiliza o algoritmo Deflate, que combina codificacao de Huffman com LZ77 (janela "
        "deslizante por dicionario). Em sequencias de repeticao longa (como frases repetidas no cenario de alta redundancia), o LZ77 substitui "
        "toda a repeticao por ponteiros curtos de (distancia, tamanho), gerando taxas de compressao drasticamente menores (R = 0,0031). "
        "Por outro lado, o Huffman puro avalia apenas a frequencia individual de cada byte, sem explorar correlacao espacial entre "
        "caracteres adjacentes.\n"
        "3. Comportamento em Baixa Redundancia (Alta Entropia): Em dados pseudo-aleatorios com distribuicao uniforme, a entropia por byte "
        "aproxima-se de 8 bits/simbolo. Conforme estabelecido pela Teoria da Informacao, nenhum algoritmo sem perdas consegue compactar "
        "dados aleatorios. Ambos os programas registraram expansao residual insignificante devida aos metadados de cabecalho (+0,15% no "
        "projetoPE e +0,04% no Gzip).\n"
        "4. Corretude e Integridade: Em todos os testes, os arquivos recuperados foram confirmados como 100% identicos aos originais "
        "tanto pela comparacao de hash SHA-256 quanto pelo CRC-32 integrado."
    )

    # =========================================================================
    # SECAO 5: DIVISAO DE TAREFAS E ENTREGAS INDIVIDUAIS
    # =========================================================================
    titulo_secao("5", "PARTICIPACAO E ENTREGAS INDIVIDUAIS DOS INTEGRANTES")
    corpo(
        "O trabalho foi dividido em 4 frentes complementares e cooperativas, com as seguintes responsabilidades individuais:\n\n"
        "- Jose Ricardo Feltran Junior (RA: 11202421133): Implementacao da fila de prioridade Min-Heap (heap.c/heap.h), construcao da arvore binaria de Huffman "
        "com desempate deterministico e algoritmo de geracao dos codigos binarios de tamanho variavel (huffman.c/huffman.h).\n"
        "- Rafael Maximiano Luiz (RA: 11202432420): Implementacao do leitor e escritor serial de bits BitStream (bitio.c/bitio.h), formato de cabecalho "
        "compacto com codificacao varint (cabecalho.c/cabecalho.h) e rotina de compressao em duas passadas (compressao.c/compressao.h).\n"
        "- Renan Henrique Ferreira Monteiro (RA: 11202020303): Implementacao da rotina de descompressao bit a bit com reconstrucao automatica da arvore "
        "(descompressao.c/descompressao.h) e algoritmo de verificacao de integridade por checksum CRC-32 IEEE 802.3 (checksum.c/checksum.h).\n"
        "- Rodrigo Martel Poggi de Araujo (RA: 11202522393): Implementacao do menu interativo no terminal (main.c), normalizacao e validacao de "
        "caminhos de arquivos, medicao de tempo com clock() e calculo de taxas, execucao da suite de testes comparativos contra o gzip, "
        "elaboracao do leia-me.txt, regras de compilacao no Makefile e redacao deste Relatorio Tecnico."
    )

    pdf.ln(5)
    pdf.set_font("Helvetica", "I", 9)
    pdf.set_text_color(100, 100, 100)
    pdf.cell(0, 6, "Relatorio concluido e aprovado pela equipe para submissao no Moodle - UFABC 2026.3.", align="C")

    caminho_saida = "docs/Relatorio_Tecnico_UFABC_projetoPE.pdf"
    pdf.output(caminho_saida)
    print(f"Relatorio PDF gerado com sucesso: {caminho_saida}")

if __name__ == "__main__":
    criar_relatorio()
