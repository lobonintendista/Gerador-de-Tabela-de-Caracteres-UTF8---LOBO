#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

// Função para exibir o cabeçalho do programa
void exibirCabecalho() {
    printf("\n     =================================================\n");
    printf("        Gerador de Tabela de Caracteres UTF8 - LOBO\n");
    printf("     =================================================\n\n");
}

typedef struct {
    char entrada[12]; // Armazena os bytes em hexadecimal (máximo 4 bytes * 2 caracteres + '\0')
    char caractere[9]; // Armazena o caractere UTF-8 ou a representação [XX]
} TabelaUTF8;

// Função de comparação para qsort
int compararEntradas(const void *a, const void *b) {
    return strcmp(((TabelaUTF8 *)a)->entrada, ((TabelaUTF8 *)b)->entrada);
}

int processarArquivo(const char *binFilename) {
    FILE *binFile;
    FILE *txtFile;
    unsigned char byteLido[4];  // Buffer para ler até 4 bytes (para UTF-8)
    unsigned long offsetEntrada, offsetSaida;
    TabelaUTF8 tabela[65536];  // Tabela para armazenar caracteres e seus bytes
    int totalEntradas = 0;

    // Abrindo o arquivo binário
    binFile = fopen(binFilename, "rb");
    if (binFile == NULL) {
        printf("Erro: Não foi possível abrir o arquivo '%s'.\n", binFilename);
        return 1;
    }

    // Pedindo os offsets de entrada e saída
    printf("  -  Digite o offset inicial de entrada (em hexadecimal): ");
    scanf("%lx", &offsetEntrada);
    printf("  -  Digite o offset de saída (em hexadecimal): ");
    scanf("%lx", &offsetSaida);

    if (offsetEntrada < 0 || offsetSaida < offsetEntrada) {
        printf("Erro: Offsets inválidos. Verifique os valores fornecidos.\n");
        fclose(binFile);
        return 1;
    }

    fseek(binFile, offsetEntrada, SEEK_SET);

    for (unsigned long offset = offsetEntrada; offset <= offsetSaida;) {
        if (fread(byteLido, 1, 1, binFile) != 1) break;

        if (byteLido[0] < 0x80) {
            // Caractere ASCII (1 byte)
            sprintf(tabela[totalEntradas].entrada, "%02X", byteLido[0]);

            // Se é um caractere imprimível, usamos ele; caso contrário, formatamos como [XX]
            if (byteLido[0] >= 0x20 && byteLido[0] <= 0x7E) {
                sprintf(tabela[totalEntradas].caractere, "%c", byteLido[0]);
            } else {
                sprintf(tabela[totalEntradas].caractere, "[%02X]", byteLido[0]);
            }

        } else if ((byteLido[0] & 0xE0) == 0xC0) {
            // Caractere UTF-8 de 2 bytes
            if (fread(&byteLido[1], 1, 1, binFile) == 1) {
                sprintf(tabela[totalEntradas].entrada, "%02X%02X", byteLido[0], byteLido[1]);
                sprintf(tabela[totalEntradas].caractere, "%c%c", byteLido[0], byteLido[1]);
                offset += 1;
            }
        } else if ((byteLido[0] & 0xF0) == 0xE0) {
            // Caractere UTF-8 de 3 bytes
            if (fread(&byteLido[1], 1, 2, binFile) == 2) {
                sprintf(tabela[totalEntradas].entrada, "%02X%02X%02X", byteLido[0], byteLido[1], byteLido[2]);
                sprintf(tabela[totalEntradas].caractere, "%c%c%c", byteLido[0], byteLido[1], byteLido[2]);
                offset += 2;
            }
        } else if ((byteLido[0] & 0xF8) == 0xF0) {
            // Caractere UTF-8 de 4 bytes
            if (fread(&byteLido[1], 1, 3, binFile) == 3) {
                sprintf(tabela[totalEntradas].entrada, "%02X%02X%02X%02X", byteLido[0], byteLido[1], byteLido[2], byteLido[3]);
                sprintf(tabela[totalEntradas].caractere, "%c%c%c%c", byteLido[0], byteLido[1], byteLido[2], byteLido[3]);
                offset += 3;
            }
        }

        // Checar se o caractere já foi adicionado
        int duplicado = 0;
        for (int i = 0; i < totalEntradas; i++) {
            if (strcmp(tabela[i].entrada, tabela[totalEntradas].entrada) == 0) {
                duplicado = 1;
                break;
            }
        }

        // Adiciona à tabela apenas se não for duplicado
        if (!duplicado) {
            totalEntradas++;
        }

        offset++;
    }

    fclose(binFile);

    // Ordena a tabela em ordem crescente
    qsort(tabela, totalEntradas, sizeof(TabelaUTF8), compararEntradas);

    // Criando o arquivo de saída
    char txtFilename[256];
    snprintf(txtFilename, sizeof(txtFilename), "%s_tabela.tbl", binFilename);
    txtFile = fopen(txtFilename, "w");

    if (txtFile == NULL) {
        printf("Erro: Não foi possível criar o arquivo '%s'.\n", txtFilename);
        return 1;
    }

    // Escreve os resultados ordenados no arquivo de texto
    for (int i = 0; i < totalEntradas; i++) {
        fprintf(txtFile, "%s=%s\n", tabela[i].entrada, tabela[i].caractere);
    }

    fclose(txtFile);
    printf("\nTabela de caracteres gerada e salva em '%s'.\n", txtFilename);
    return 0;
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    char binFilename[256];
    char opcao;

    exibirCabecalho();  // Chamando a função exibirCabecalho()

    do {
        printf("  -  Digite o nome do arquivo binário (com extensão): ");
        scanf("%s", binFilename);
        if (processarArquivo(binFilename) != 0) {
            printf("Erro ao processar o arquivo '%s'. Tente novamente.\n", binFilename);
        }
        printf("\nDeseja processar outro arquivo? (s/n): ");
        scanf(" %c", &opcao);
    } while (opcao == 's' || opcao == 'S');

    printf("\n=================================\n");
    printf("         Fim do Programa         \n");
    printf("=================================\n\n");
    printf("             by LOBO             \n\n");

    return 0;
}
