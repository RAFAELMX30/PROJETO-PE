# PROJETO-PE — Compactador e Descompactador de Arquivos

**Disciplina:** Programação Estruturada  
**Instituição:** Universidade Federal do ABC (UFABC)  
**Quadrimestre:** 2026.3

## Sobre o projeto

O projeto consiste no desenvolvimento de um programa em linguagem C capaz de comprimir e descomprimir arquivos utilizando o **algoritmo de Huffman**, baseado na frequência de ocorrência dos bytes presentes no arquivo original.

O objetivo é implementar os algoritmos e as estruturas de dados necessárias para a compactação e descompactação, sem depender de bibliotecas prontas que realizem essas operações.

O programa também contará com uma funcionalidade extra de verificação de integridade, permitindo identificar possíveis alterações ou erros nos dados durante o processo de descompressão.

## Funcionalidades
 
- Compressão de arquivos utilizando o algoritmo de Huffman.
- Descompressão de arquivos previamente comprimidos pelo programa.
- Suporte a arquivos de texto e binários.
- Armazenamento das informações necessárias para reconstruir a árvore de Huffman.
- Verificação de integridade do arquivo descomprimido por meio de checksum.
- exibição de estatísticas de compressão, incluindo tamanho original, tamanho comprimido e taxa de compressão.
- Exibição dos códigos de Huffman gerados.
- Comparação dos resultados com o compactador gzip.

## Algoritmo de compressão

Será utilizado o algoritmo de Huffman, um método de compressão sem perdas (*lossless*), que atribui códigos binários de tamanhos variáveis aos bytes do arquivo de acordo com suas frequências.

Bytes mais frequentes tendem a receber códigos menores, enquanto bytes menos frequentes recebem códigos maiores.

O processo de compressão seguirá estas etapas:

1. Ler o arquivo original em modo binário e contabilizar a frequência de cada byte.
2. Construir uma árvore de Huffman utilizando uma fila de prioridade.
3. Gerar os códigos binários correspondentes a cada byte.
4. Gravar o cabeçalho e os dados comprimidos no arquivo de saída.
5. Registrar o checksum do arquivo original para posterior verificação.

### Descompressão

O processo de descompressão seguirá estas etapas:

1. Ler o cabeçalho do arquivo comprimido.
2. Reconstruir a árvore de Huffman a partir das frequências armazenadas.
3. Decodificar os bits e recuperar os bytes originais.
4. Utilizar o tamanho original registrado no cabeçalho para determinar quando a descompressão deve terminar.
5. Calcular o checksum do arquivo recuperado e compará-lo com o valor armazenado.

## Estruturas de dados

O projeto utilizará as seguintes estruturas de dados, implementadas pela equipe em C:

- **Árvore de Huffman:** representa os bytes e suas frequências, permitindo gerar códigos de tamanho variável.
- **Min-heap (fila de prioridade):** organiza os nós pela frequência, permitindo selecionar eficientemente os dois nós de menor frequência durante a construção da árvore.
- **Tabela de códigos:** armazena os códigos binários associados a cada um dos 256 valores possíveis de byte.

As estruturas serão desenvolvidas pela equipe, sem utilizar funções prontas de ordenação para substituir a implementação da fila de prioridade.

## Formato do arquivo comprimido

Os arquivos comprimidos utilizarão a extensão `.abcz`.

O cabeçalho armazenará as informações necessárias para reconstruir a árvore e recuperar o arquivo original:

- **Tabela de frequências:** 256 valores, um para cada byte possível.
- **Tamanho original:** quantidade de bytes do arquivo antes da compressão.
- **Checksum:** valor utilizado para verificar a integridade dos dados recuperados.

Após o cabeçalho, serão gravados os dados comprimidos em formato de bits.

O formato definitivo do cabeçalho, incluindo a representação binária dos campos, será definido durante a implementação.

## Funcionalidade extra: verificação de integridade

O programa calculará um checksum do arquivo original durante a compressão e armazenará o resultado no cabeçalho.

Durante a descompressão, o checksum será recalculado e comparado com o valor registrado. Caso os valores sejam diferentes, o programa informará que a integridade do arquivo não pôde ser confirmada.

Essa funcionalidade será utilizada para detectar possíveis corrupções ou alterações nos dados recuperados.

## Divisão de tarefas

A equipe será organizada em quatro frentes de desenvolvimento:

| Integrante | Responsabilidade |
|---|---|
| Integrante 1 | Implementação do min-heap, construção da árvore de Huffman e geração dos códigos. |
| Integrante 2 | Implementação da escrita e leitura de bits, do cabeçalho e da rotina de compressão. |
| Integrante 3 | Implementação da descompressão e da verificação de integridade por checksum. |
| Integrante 4 | Implementação do menu, tratamento de caminhos, medição de tempo e tamanho, testes comparativos com gzip e relatório. |

Apesar da divisão, todos os integrantes deverão compreender o funcionamento completo do programa para contribuir com a integração, os testes e a apresentação.

## Testes e avaliação

O programa será testado com diferentes tipos e tamanhos de arquivos, incluindo:

- Arquivos de texto.
- Arquivos binários.
- Arquivos vazios.
- Arquivos com apenas um byte ou um único valor de byte repetido.
- Arquivos com diferentes distribuições de frequência.
- Arquivos comprimidos e posteriormente descomprimidos para comparação com os originais.
- Arquivos utilizados para verificar o comportamento do checksum.

Também serão realizadas comparações com o gzip, observando o tamanho final dos arquivos e o tempo de execução.


## Tecnologias utilizadas

- **Linguagem:** C
- **Controle de versão:** Git
- **Hospedagem do código:** GitHub
- **Compactador de referência para testes:** gzip

## Integridade e colaboração

O desenvolvimento será realizado de forma colaborativa, com o código-fonte e a documentação versionados neste repositório.

As alterações serão integradas e testadas pela equipe para garantir que as rotinas de compressão e descompressão funcionem corretamente em conjunto.

---

**Repositório acadêmico — Programação Estruturada, UFABC, 2026.3.**
