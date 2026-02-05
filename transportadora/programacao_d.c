#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

int FULL_PLC = 8
int FULL_CODE = 14

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

typedef struct {
    char placa[FULL_PLC];
    int32_t peso;
    int32_t volume;
} Carro;

typedef struct {
    char codigo[FULL_CODE];
    float valor;
    int32_t peso;
    int32_t volume;
    int usado;
    int posicao_original;
} Produto;

// ============================================================================
// PADRÃO STRATEGY - Interface para algoritmos de otimização
// ============================================================================

// Definição da função strategy
typedef void (*EstrategiaOtimizacao)(Carro, Produto*, int32_t, FILE*, int32_t*, Produto*);

// Estrutura que encapsula a estratégia
typedef struct {
    const char *nome;
    EstrategiaOtimizacao executar;
} Strategy;

// ============================================================================
// ESTRATÉGIA 1: Programação Dinâmica (3D Knapsack)
// ============================================================================

float maximo(float a, float b) {
    return (a > b) ? a : b;
}

void preencherTabelaDP(float ***dp, Carro carro, Produto *produtos, int32_t n) {
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= carro.volume; j++) {
            for (int k = 0; k <= carro.peso; k++) {
                if (i == 0 || j == 0 || k == 0) {
                    dp[i][j][k] = 0;
                } else if (produtos[i - 1].volume <= j && produtos[i - 1].peso <= k && !produtos[i - 1].usado) {
                    dp[i][j][k] = maximo(
                        produtos[i - 1].valor + dp[i - 1][j - produtos[i - 1].volume][k - produtos[i - 1].peso],
                        dp[i - 1][j][k]
                    );
                } else {
                    dp[i][j][k] = dp[i - 1][j][k];
                }
            }
        }
    }
}

void selecionarProdutosDP(float ***dp, Carro carro, Produto *produtosAux, int32_t n, FILE *output, int32_t *quantidade, Produto *produtosOriginal) {
    int pesoTotal = 0, volumeTotal = 0;
    char *selecionados[n];
    int count = 0;

    for (int i = n, j = carro.volume, k = carro.peso; i > 0 && j > 0 && k > 0; i--) {
        if (dp[i][j][k] != dp[i - 1][j][k]) {
            selecionados[count++] = produtosAux[i - 1].codigo;
            pesoTotal += produtosAux[i - 1].peso;
            volumeTotal += produtosAux[i - 1].volume;
            produtosOriginal[produtosAux[i - 1].posicao_original].usado = 1;
            j -= produtosAux[i - 1].volume;
            k -= produtosAux[i - 1].peso;
            (*quantidade)++;
        }
    }

    fprintf(output, "[%s]R$%.2f,%dKG(%d%%),%dL(%d%%)->", carro.placa, dp[n][carro.volume][carro.peso],
            pesoTotal, (int)((float) pesoTotal / carro.peso * 100 + 0.5),
            volumeTotal, (int)((float) volumeTotal / carro.volume * 100 + 0.5));
    
    for (int x = count - 1; x >= 0; x--) {
        fprintf(output, "%s%s", selecionados[x], (x > 0) ? "," : "\n");
    }
}

void estrategiaDP(Carro carro, Produto *produtos, int32_t p, FILE *output, int32_t *quantidade, Produto *produtosOriginal) {
    // Filtra produtos não usados
    int n_aux = 0;
    for (int j = 0; j < p; j++) {
        if (!produtos[j].usado) n_aux++;
    }
    
    Produto *produtosAux = malloc(n_aux * sizeof(Produto));
    int idx = 0;
    for (int j = 0; j < p; j++) {
        if (!produtos[j].usado) {
            produtosAux[idx++] = produtos[j];
        }
    }

    // Aloca tabela DP
    float ***dp = malloc((n_aux + 1) * sizeof(float **));
    for (int i = 0; i <= n_aux; i++) {
        dp[i] = malloc((carro.volume + 1) * sizeof(float *));
        for (int j = 0; j <= carro.volume; j++) {
            dp[i][j] = malloc((carro.peso + 1) * sizeof(float));
            memset(dp[i][j], 0, (carro.peso + 1) * sizeof(float));
        }
    }

    preencherTabelaDP(dp, carro, produtosAux, n_aux);
    selecionarProdutosDP(dp, carro, produtosAux, n_aux, output, quantidade, produtosOriginal);

    // Libera memória
    for (int i = 0; i <= n_aux; i++) {
        for (int j = 0; j <= carro.volume; j++) {
            free(dp[i][j]);
        }
        free(dp[i]);
    }
    free(dp);
    free(produtosAux);
}

// ============================================================================
// ESTRATÉGIA 2: Greedy (Ganancioso) - por valor/peso
// ============================================================================

int compararPorValorPeso(const void *a, const void *b) {
    Produto *p1 = (Produto *)a;
    Produto *p2 = (Produto *)b;
    float razao1 = p1->valor / (p1->peso + p1->volume);
    float razao2 = p2->valor / (p2->peso + p2->volume);
    return (razao2 > razao1) ? 1 : -1;
}

void estrategiaGreedy(Carro carro, Produto *produtos, int32_t p, FILE *output, int32_t *quantidade, Produto *produtosOriginal) {
    int n_aux = 0;
    for (int j = 0; j < p; j++) {
        if (!produtos[j].usado) n_aux++;
    }
    
    Produto *produtosAux = malloc(n_aux * sizeof(Produto));
    int idx = 0;
    for (int j = 0; j < p; j++) {
        if (!produtos[j].usado) {
            produtosAux[idx++] = produtos[j];
        }
    }

    qsort(produtosAux, n_aux, sizeof(Produto), compararPorValorPeso);

    float valorTotal = 0;
    int pesoTotal = 0, volumeTotal = 0;
    char *selecionados[n_aux];
    int count = 0;

    for (int i = 0; i < n_aux; i++) {
        if (pesoTotal + produtosAux[i].peso <= carro.peso && 
            volumeTotal + produtosAux[i].volume <= carro.volume) {
            selecionados[count++] = produtosAux[i].codigo;
            valorTotal += produtosAux[i].valor;
            pesoTotal += produtosAux[i].peso;
            volumeTotal += produtosAux[i].volume;
            produtosOriginal[produtosAux[i].posicao_original].usado = 1;
            (*quantidade)++;
        }
    }

    fprintf(output, "[%s]R$%.2f,%dKG(%d%%),%dL(%d%%)->", carro.placa, valorTotal,
            pesoTotal, (int)((float) pesoTotal / carro.peso * 100 + 0.5),
            volumeTotal, (int)((float) volumeTotal / carro.volume * 100 + 0.5));
    
    for (int x = 0; x < count; x++) {
        fprintf(output, "%s%s", selecionados[x], (x < count - 1) ? "," : "\n");
    }

    free(produtosAux);
}

// ============================================================================
// PROCESSAMENTO COM STRATEGY PATTERN
// ============================================================================

void processarCarrosComStrategy(Carro *carros, Produto *produtos, int32_t c, int32_t p, FILE *output, Strategy estrategia) {
    int quantidade = 0;
    
    printf("📊 Executando estratégia: %s\n", estrategia.nome);
    
    for (int i = 0; i < c; i++) {
        estrategia.executar(carros[i], produtos, p, output, &quantidade, produtos);
    }
}

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

void calcularPendentes(Produto *produtos, int32_t p, FILE *output) {
    float valorPendente = 0;
    int pesoPendente = 0, volumePendente = 0;
    
    fprintf(output, "PENDENTE:");
    
    for (int i = 0; i < p; i++) {
        if (!produtos[i].usado) {
            valorPendente += produtos[i].valor;
            pesoPendente += produtos[i].peso;
            volumePendente += produtos[i].volume;
        }
    }
    
    fprintf(output, "R$%.2f,%dKG,%dL->", valorPendente, pesoPendente, volumePendente);
    int primeiro = 1;
    for (int i = 0; i < p; i++) {
        if (!produtos[i].usado) {
            if (!primeiro) fprintf(output, ",");
            fprintf(output, "%s", produtos[i].codigo);
            primeiro = 0;
        }
    }
    fprintf(output, "\n");
}

double medirTempoExecucao(clock_t inicio, clock_t fim) {
    return ((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000.0;
}

// ============================================================================
// FUNÇÃO PRINCIPAL
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida> [estrategia]\n", argv[0]);
        printf("Estratégias disponíveis: dp, greedy\n");
        return 1;
    }

    // Define estratégias disponíveis
    Strategy estrategias[] = {
        {"Programação Dinâmica (3D Knapsack)", estrategiaDP},
        {"Ganancioso (Greedy)", estrategiaGreedy}
    };

    // Padrão: DP
    Strategy estrategiaEscolhida = estrategias[0];

    // Se houver argumento, valida
    if (argc >= 4) {
        if (strcmp(argv[3], "greedy") == 0) {
            estrategiaEscolhida = estrategias[1];
        } else if (strcmp(argv[3], "dp") != 0) {
            printf("❌ Estratégia desconhecida: %s\n", argv[3]);
            return 1;
        }
    }

    // Abre arquivos
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir arquivo de saída");
        fclose(input);
        return 1;
    }

    // Lê dados
    int c, p;
    fscanf(input, "%d", &c);
    Carro *carros = malloc(c * sizeof(Carro));
    for (int i = 0; i < c; i++) {
        fscanf(input, "%s %d %d", carros[i].placa, &carros[i].peso, &carros[i].volume);
    }

    fscanf(input, "%d", &p);
    Produto *produtos = malloc(p * sizeof(Produto));
    for (int i = 0; i < p; i++) {
        fscanf(input, "%s %f %d %d", produtos[i].codigo, &produtos[i].valor, &produtos[i].peso, &produtos[i].volume);
        produtos[i].usado = 0;
        produtos[i].posicao_original = i;
    }

    // Processa com medição de tempo
    clock_t inicio = clock();
    
    processarCarrosComStrategy(carros, produtos, c, p, output, estrategiaEscolhida);
    calcularPendentes(produtos, p, output);
    
    clock_t fim = clock();
    double tempo = medirTempoExecucao(inicio, fim);

    // Exibe resultado
    printf("✅ Processamento concluído\n");
    printf("⏱️  Tempo: %.2f ms\n", tempo);

    // Libera memória
    free(carros);
    free(produtos);
    fclose(input);
    fclose(output);

    return 0;
}