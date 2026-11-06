import os
import shutil
import time
import gzip
import random
import subprocess
import hashlib

EXT = ".exe" if os.name == "nt" else ""
EXE_COMP = os.path.join(".", "comprimir_cli" + EXT)
EXE_DESC = os.path.join(".", "descomprimir_cli" + EXT)
# usa o gzip do sistema se existir (Linux); senao o modulo gzip do Python
GZIP_CLI = shutil.which("gzip")

DIRETORIO_TESTES = "tests/cenarios"
os.makedirs(DIRETORIO_TESTES, exist_ok=True)

# 1. Gerar os 3 cenários de redundância (500 KB cada)
TAMANHO_ARQUIVO = 500 * 1024 # 500 KB

def gerar_cenario_alta(caminho):
    # trecho curto repetido ate completar o tamanho
    padrao = b"ATGCATGCATGCATGCATGC_UFABC_2026_REDUNDANCIA_ALTA_"
    conteudo = (padrao * ((TAMANHO_ARQUIVO // len(padrao)) + 1))[:TAMANHO_ARQUIVO]
    with open(caminho, "wb") as f:
        f.write(conteudo)

def gerar_cenario_media(caminho):
    # frases sorteadas de uma lista pequena
    frases = [
        b"A disciplina de Programacao Estruturada na UFABC aborda Tipos Abstratos de Dados.\n",
        b"O algoritmo de Huffman constroi uma arvore binaria utilizando uma fila de prioridade min-heap.\n",
        b"Bytes com maior frequencia recebem codigos menores reduzindo o tamanho total do arquivo.\n",
        b"A integridade dos dados e garantida pelo calculo do checksum CRC-32 durante a compressao.\n",
        b"Estruturas eficientes como arvores e tabelas de espalhamento otimizam o processamento em C.\n"
    ]
    dados = bytearray()
    random.seed(42)
    while len(dados) < TAMANHO_ARQUIVO:
        dados.extend(random.choice(frases))
    with open(caminho, "wb") as f:
        f.write(dados[:TAMANHO_ARQUIVO])

def gerar_cenario_baixa(caminho):
    # Bytes pseudo-aleatórios uniformes (baixa redundância / alta entropia)
    random.seed(12345)
    dados = bytes([random.randint(0, 255) for _ in range(TAMANHO_ARQUIVO)])
    with open(caminho, "wb") as f:
        f.write(dados)

def calcular_sha256(caminho):
    h = hashlib.sha256()
    with open(caminho, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()

def executar_benchmark():
    cenarios = [
        ("Alta Redundancia", os.path.join(DIRETORIO_TESTES, "cenario_alta.txt"), gerar_cenario_alta),
        ("Media Redundancia", os.path.join(DIRETORIO_TESTES, "cenario_media.txt"), gerar_cenario_media),
        ("Baixa Redundancia", os.path.join(DIRETORIO_TESTES, "cenario_baixa.bin"), gerar_cenario_baixa),
    ]

    print("Comparacao projetoPE (Huffman) x gzip")

    resultados = []

    for nome, caminho, func_geradora in cenarios:
        func_geradora(caminho)
        tam_orig = os.path.getsize(caminho)
        hash_orig = calcular_sha256(caminho)

        caminho_abcz = caminho + ".abcz"
        caminho_rec_huff = caminho + ".rec_huff"
        caminho_gz = caminho + ".gz"
        caminho_rec_gz = caminho + ".rec_gz"

        # --- HUFFMAN (projetoPE) ---
        t0 = time.perf_counter()
        subprocess.run([EXE_COMP, caminho, caminho_abcz], check=True, stdout=subprocess.DEVNULL)
        t_comp_huff = time.perf_counter() - t0
        tam_abcz = os.path.getsize(caminho_abcz)

        t0 = time.perf_counter()
        subprocess.run([EXE_DESC, caminho_abcz, caminho_rec_huff], check=True, stdout=subprocess.DEVNULL)
        t_decomp_huff = time.perf_counter() - t0

        hash_rec_huff = calcular_sha256(caminho_rec_huff)
        integro_huff = (hash_orig == hash_rec_huff)

        # --- GZIP (Deflate: LZ77 + Huffman) ---
        t0 = time.perf_counter()
        if GZIP_CLI:
            with open(caminho_gz, "wb") as f_out:
                subprocess.run([GZIP_CLI, "-c", caminho], check=True, stdout=f_out)
        else:
            with open(caminho, "rb") as f_in, gzip.open(caminho_gz, "wb") as f_out:
                f_out.writelines(f_in)
        t_comp_gz = time.perf_counter() - t0
        tam_gz = os.path.getsize(caminho_gz)

        t0 = time.perf_counter()
        if GZIP_CLI:
            with open(caminho_rec_gz, "wb") as f_out:
                subprocess.run([GZIP_CLI, "-dc", caminho_gz], check=True, stdout=f_out)
        else:
            with gzip.open(caminho_gz, "rb") as f_in, open(caminho_rec_gz, "wb") as f_out:
                f_out.writelines(f_in)
        t_decomp_gz = time.perf_counter() - t0

        hash_rec_gz = calcular_sha256(caminho_rec_gz)
        integro_gz = (hash_orig == hash_rec_gz)

        # Métricas
        razao_huff = tam_abcz / tam_orig
        razao_gz = tam_gz / tam_orig
        red_huff = (1.0 - razao_huff) * 100.0
        red_gz = (1.0 - razao_gz) * 100.0

        resultados.append({
            "nome": nome,
            "tam_orig": tam_orig,
            "tam_abcz": tam_abcz,
            "razao_huff": razao_huff,
            "red_huff": red_huff,
            "t_comp_huff": t_comp_huff,
            "t_decomp_huff": t_decomp_huff,
            "integro_huff": integro_huff,
            "tam_gz": tam_gz,
            "razao_gz": razao_gz,
            "red_gz": red_gz,
            "t_comp_gz": t_comp_gz,
            "t_decomp_gz": t_decomp_gz,
            "integro_gz": integro_gz,
        })

    # Imprimir tabela comparativa
    print()
    print("%-18s %10s %10s %7s %10s %7s  %s" % ("cenario", "original", "huffman", "R", "gzip", "R", "integridade"))
    for r in resultados:
        print("%-18s %10d %10d %7.4f %10d %7.4f  %s" % (r['nome'], r['tam_orig'], r['tam_abcz'], r['razao_huff'],
              r['tam_gz'], r['razao_gz'], "ok" if r['integro_huff'] else "FALHOU"))

    print()
    print("tempos em ms:")
    print("%-18s %12s %12s %12s %12s" % ("cenario", "comp huff", "comp gzip", "desc huff", "desc gzip"))
    for r in resultados:
        print("%-18s %12.2f %12.2f %12.2f %12.2f" % (r['nome'], r['t_comp_huff'] * 1000, r['t_comp_gz'] * 1000,
              r['t_decomp_huff'] * 1000, r['t_decomp_gz'] * 1000))

if __name__ == "__main__":
    executar_benchmark()
