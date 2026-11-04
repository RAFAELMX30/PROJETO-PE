CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude

CORE = src/huffman.c src/heap.c src/bitio.c \
       src/cabecalho.c src/checksum.c src/compressao.c src/descompressao.c

all: comprimir_cli descomprimir_cli

comprimir_cli: $(CORE) tests/comprimir_cli.c
	$(CC) $(CFLAGS) $^ -o $@

descomprimir_cli: $(CORE) tests/descomprimir_cli.c
	$(CC) $(CFLAGS) $^ -o $@

test: comprimir_cli descomprimir_cli
	./comprimir_cli README.md README.abcz
	./descomprimir_cli README.abcz README_recuperado.md
	@diff README.md README_recuperado.md && echo "OK: arquivos identicos" || echo "FALHOU: arquivos diferentes"

clean:
	rm -f comprimir_cli descomprimir_cli README.abcz README_recuperado.md

.PHONY: all test clean
