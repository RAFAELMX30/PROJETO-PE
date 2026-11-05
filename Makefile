CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude

CORE = src/huffman.c src/heap.c src/bitio.c \
       src/cabecalho.c src/checksum.c src/compressao.c src/descompressao.c

all: garchy comprimir_cli descomprimir_cli

garchy: $(CORE) src/main.c
	$(CC) $(CFLAGS) $^ -o $@

comprimir_cli: $(CORE) tests/comprimir_cli.c
	$(CC) $(CFLAGS) $^ -o $@

descomprimir_cli: $(CORE) tests/descomprimir_cli.c
	$(CC) $(CFLAGS) $^ -o $@

test: garchy comprimir_cli descomprimir_cli
	./comprimir_cli README.md README.abcz
	./descomprimir_cli README.abcz README_recuperado.md
	@diff README.md README_recuperado.md && echo "OK: arquivos identicos" || echo "FALHOU: arquivos diferentes"

clean:
	rm -f garchy garchy.exe comprimir_cli comprimir_cli.exe descomprimir_cli descomprimir_cli.exe README.abcz README_recuperado.md

.PHONY: all test clean
