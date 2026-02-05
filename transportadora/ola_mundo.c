#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TAM_MAX_PLACA 8       // Limite de caracteres para placa
#define TAM_MAX_COD 14        // Limite de caracteres para código

// Representa um veículo de transporte
typedef struct {
    char placa[TAM_MAX_PLACA];  // Identificação do veículo
    int32_t capacidade_peso;     // Carga máxima em peso
    int32_t capacidade_volume;   // Carga máxima em volume
} Veiculo;

// Representa um item a ser transportado
typedef struct {
    char codigo[TAM_MAX_COD];   // Identificador do item
    float valor;                 // Preço do item
    int32_t peso;                // Massa do item
    int32_t volume;              // Espaço ocupado pelo item
    int alocado;                 // Indica se já foi alocado
    int indice_original;         // Índice no array inicial
} Item;

// Retorna o maior valor entre dois números
float obterMaximo(float x, float y) {
    return (x > y) ? x : y;  // Comparação e retorno
}

// Constrói a matriz de programação dinâmica
void construirMatrizOtimizacao(float ***matriz, Veiculo veiculo, Item *itens, int32_t total) {
    for (int i = 0; i <= total; i++) {  // Itera sobre itens
        for (int j = 0; j <= veiculo.capacidade_volume; j++) {  // Itera sobre capacidade de volume
            for (int k = 0; k <= veiculo.capacidade_peso; k++) {  // Itera sobre capacidade de peso
                if (i == 0 || j == 0 || k == 0) {
                    matriz[i][j][k] = 0;  // Inicialização da base
                } else if (itens[i - 1].volume <= j && itens[i - 1].peso <= k && !itens[i - 1].alocado) {
                    matriz[i][j][k] = obterMaximo(
                        itens[i - 1].valor + matriz[i - 1][j - itens[i - 1].volume][k - itens[i - 1].peso],
                        matriz[i - 1][j][k]
                    );  // Decide entre incluir ou excluir o item
                } else {
                    matriz[i][j][k] = matriz[i - 1][j][k];  // Mantém valor anterior
                }
            }
        }
    }
}

// Identifica e registra os itens escolhidos pela matriz
void identificarItensEscolhidos(float ***matriz, Veiculo veiculo, Item *itensTemp, int32_t total, FILE *saida, int32_t *contador, Item *itensBase) {
    int massaAcumulada = 0, espacoAcumulado = 0;
    char *codigosEscolhidos[total];  // Armazena códigos dos itens selecionados
    int qtd = 0;

    // Reconstrói a solução ótima
    for (int i = total, j = veiculo.capacidade_volume, k = veiculo.capacidade_peso; i > 0 && j > 0 && k > 0; i--) {
        if (matriz[i][j][k] != matriz[i - 1][j][k]) {  // Verifica se item foi escolhido
            codigosEscolhidos[qtd++] = itensTemp[i - 1].codigo;  // Armazena código
            massaAcumulada += itensTemp[i - 1].peso;  // Soma peso
            espacoAcumulado += itensTemp[i - 1].volume;  // Soma volume
            itensBase[itensTemp[i - 1].indice_original].alocado = 1;  // Marca como alocado
            j -= itensTemp[i - 1].volume;  // Reduz capacidade de volume
            k -= itensTemp[i - 1].peso;    // Reduz capacidade de peso
            (*contador)++;  // Incrementa contador
        }
    }

    // Gera saída formatada
    fprintf(saida, "[%s]R$%.2f,%dKG(%d%%),%dL(%d%%)->", veiculo.placa, matriz[total][veiculo.capacidade_volume][veiculo.capacidade_peso],
            massaAcumulada, (int)((float) massaAcumulada / veiculo.capacidade_peso * 100 + 0.5),
            espacoAcumulado, (int)((float) espacoAcumulado / veiculo.capacidade_volume * 100 + 0.5));
    
    // Lista itens na ordem reversa
    for (int x = qtd - 1; x >= 0; x--) {
        fprintf(saida, "%s%s", codigosEscolhidos[x], (x > 0) ? "," : "\n");
    }
}

// Executa otimização para todos os veículos
void executarOtimizacaoVeiculos(Veiculo *veiculos, Item *itens, int32_t totalVeiculos, int32_t totalItens, FILE *saida) {
    int pesoMaximo = 0, volumeMaximo = 0;

    // Determina dimensões máximas necessárias
    for (int i = 0; i < totalVeiculos; i++) {
        if (veiculos[i].capacidade_peso > pesoMaximo) pesoMaximo = veiculos[i].capacidade_peso;
        if (veiculos[i].capacidade_volume > volumeMaximo) volumeMaximo = veiculos[i].capacidade_volume;
    }

    // Cria estrutura tridimensional para programação dinâmica
    float ***matriz = malloc((totalItens + 1) * sizeof(float **));
    for (int i = 0; i <= totalItens; i++) {
        matriz[i] = malloc((volumeMaximo + 1) * sizeof(float *));
        for (int j = 0; j <= volumeMaximo; j++) {
            matriz[i][j] = malloc((pesoMaximo + 1) * sizeof(float));
        }
    }

    int contadorItens = 0;
    for (int i = 0; i < totalVeiculos; i++) {
        // Filtra itens disponíveis
        int qtdDisponivel = 0;
        for (int j = 0; j < totalItens; j++) {
            if (!itens[j].alocado) qtdDisponivel++;
        }
        Item *itensDisponiveis = malloc(qtdDisponivel * sizeof(Item));
        int posicao = 0;
        for (int j = 0; j < totalItens; j++) {
            if (!itens[j].alocado) {
                itensDisponiveis[posicao++] = itens[j];
            }
        }

        construirMatrizOtimizacao(matriz, veiculos[i], itensDisponiveis, qtdDisponivel);
        identificarItensEscolhidos(matriz, veiculos[i], itensDisponiveis, qtdDisponivel, saida, &contadorItens, itens);

        free(itensDisponiveis);
    }

    // Libera memória alocada
    for (int i = 0; i <= totalItens; i++) {
        for (int j = 0; j <= volumeMaximo; j++) {
            free(matriz[i][j]);
        }
        free(matriz[i]);
    }
    free(matriz);
}

// Processa e exibe itens não alocados
void gerarRelatorioRestante(Item *itens, int32_t totalItens, FILE *saida) {
    float valorRestante = 0;
    int pesoRestante = 0, volumeRestante = 0;
    
    fprintf(saida, "PENDENTE:");
    
    for (int i = 0; i < totalItens; i++) {
        if (!itens[i].alocado) {
            valorRestante += itens[i].valor;
            pesoRestante += itens[i].peso;
            volumeRestante += itens[i].volume;
        }
    }
    
    fprintf(saida, "R$%.2f,%dKG,%dL->", valorRestante, pesoRestante, volumeRestante);
    int primeiro = 1;
    for (int i = 0; i < totalItens; i++) {
        if (!itens[i].alocado) {
            if (!primeiro) fprintf(saida, ",");
            fprintf(saida, "%s", itens[i].codigo);
            primeiro = 0;
        }
    }
    fprintf(saida, "\n");
}

// Ponto de entrada do programa
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    FILE *entrada = fopen(argv[1], "r");
    if (!entrada) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    FILE *saida = fopen(argv[2], "w");
    if (!saida) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(entrada);
        return 1;
    }

    int qtdVeiculos, qtdItens;
    fscanf(entrada, "%d", &qtdVeiculos);
    Veiculo *veiculos = malloc(qtdVeiculos * sizeof(Veiculo));
    for (int i = 0; i < qtdVeiculos; i++) {
        fscanf(entrada, "%s %d %d", veiculos[i].placa, &veiculos[i].capacidade_peso, &veiculos[i].capacidade_volume);
    }

    fscanf(entrada, "%d", &qtdItens);
    Item *itens = malloc(qtdItens * sizeof(Item));
    for (int i = 0; i < qtdItens; i++) {
        fscanf(entrada, "%s %f %d %d", itens[i].codigo, &itens[i].valor, &itens[i].peso, &itens[i].volume);
        itens[i].alocado = 0;
        itens[i].indice_original = i;
    }

    executarOtimizacaoVeiculos(veiculos, itens, qtdVeiculos, qtdItens, saida);
    gerarRelatorioRestante(itens, qtdItens, saida);
    
    free(veiculos);
    free(itens);
    fclose(entrada);
    fclose(saida);
    return 0;
}