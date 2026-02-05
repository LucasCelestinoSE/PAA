#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Estrutura para armazenar a saída formatada
typedef struct {
    int tam_vetor;
    char** vetor; 
} saida_formatada;

// Definição do Enum de tipos de partição
typedef enum {
    LOMUTO_VANILLA,
    LOMUTO_MEDIAN,
    LOMUTO_RANDOM,
    HOARE_VANILLA,
    HOARE_MEDIAN,
    HOARE_RANDOM
} PartitionType;

// Estrutura para armazenar resultados da execução
typedef struct {
    PartitionType tipo;
    long long chamadas;
    long long trocas;
    long long total;
} ResultadoExecucao;

void swap(int* a, int* b);
int hoare(int arr[], int low, int high, long long* trocas);
int lomuto(int arr[], int low, int high, long long* trocas);
int lomuto_median_of_3(int arr[], int low, int high, long long* trocas);
int hoare_median_of_3(int arr[], int low, int high, long long* trocas);
int lomuto_random(int arr[], int low, int high, long long* trocas);
int hoare_random(int arr[], int low, int high, long long* trocas);

// Função de partição Lomuto com pivô aleatório
int lomuto_random(int arr[], int low, int high, long long* trocas) {
    int random_index = low + abs(arr[low]) % (high - low + 1);
    swap(&arr[high], &arr[random_index]);
    (*trocas)++; 
    return lomuto(arr, low, high, trocas);
}

// Função de partição Hoare com pivô aleatório
int hoare_random(int arr[], int low, int high, long long* trocas) {
    int random_index = low + abs(arr[low]) % (high - low + 1);
    swap(&arr[low], &arr[random_index]);
    (*trocas)++;
    return hoare(arr, low, high, trocas);
}

// Função para encontrar o índice da mediana de três
int median_of_three_index(int arr[], int low, int high) {
    int n = high - low + 1;
    int idx1 = low + n / 4;
    int idx2 = low + n / 2;
    int idx3 = low + (3 * n) / 4;

    int val1 = arr[idx1];
    int val2 = arr[idx2];
    int val3 = arr[idx3];

    if ((val1 >= val2 && val1 <= val3) || (val1 <= val2 && val1 >= val3)) {
        return idx1;
    }
    if ((val2 >= val1 && val2 <= val3) || (val2 <= val1 && val2 >= val3)) {
        return idx2;
    }
    return idx3;
}

// Função de partição Lomuto com pivô de mediana de 3
int lomuto_median_of_3(int arr[], int low, int high, long long* trocas) {
    int pivot_index = median_of_three_index(arr, low, high);
    swap(&arr[pivot_index], &arr[high]);
    (*trocas)++;
    return lomuto(arr, low, high, trocas);
}

// Função de partição Hoare com pivô de mediana de 3
int hoare_median_of_3(int arr[], int low, int high, long long* trocas) {
    int pivot_index = median_of_three_index(arr, low, high);
    swap(&arr[pivot_index], &arr[low]);
    (*trocas)++;
    return hoare(arr, low, high, trocas);
}

// Função de partição Lomuto
int lomuto(int arr[], int low, int high, long long* trocas) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
            (*trocas)++;
        }
    }
    
    swap(&arr[i + 1], &arr[high]);
    (*trocas)++;
    return i + 1;
}

// Função de partição Hoare
int hoare(int arr[], int low, int high, long long* trocas) {
    int pivot = arr[low];
    int i = low - 1;
    int j = high + 1;

    while (1) {
        do {
            i++;
        } while (arr[i] < pivot);

        do {
            j--;
        } while (arr[j] > pivot);

        if (i >= j)
            return j;

        swap(&arr[i], &arr[j]);
        (*trocas)++;
    }
}

// Função QuickSort
void quickSort(int arr[], int low, int high, PartitionType partitionType, long long* chamadas, long long* trocas) {
    (*chamadas)++;
    if (low >= high) return;

    int pi;
    
    switch(partitionType) {
        case LOMUTO_VANILLA:
            pi = lomuto(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
            
        case HOARE_VANILLA:
            pi = hoare(arr, low, high, trocas);
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
            
        case LOMUTO_RANDOM:
            pi = lomuto_random(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
            
        case HOARE_RANDOM:
            pi = hoare_random(arr, low, high, trocas);
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
            
        case LOMUTO_MEDIAN:
            pi = lomuto_median_of_3(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
            
        case HOARE_MEDIAN:
            pi = hoare_median_of_3(arr, low, high, trocas);
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
            break;
    }
}

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// Função para copiar vetor
int* copiar_vetor(int* original, int tamanho) {
    int* copia = (int*) malloc(tamanho * sizeof(int));
    for(int i = 0; i < tamanho; i++) {
        copia[i] = original[i];
    }
    return copia;
}

// Função para executar Sort com uma estratégia específica
ResultadoExecucao executarSort(int* vetor_original, int tamanho, PartitionType estrategia) {
    ResultadoExecucao resultado;
    resultado.tipo = estrategia;
    resultado.chamadas = 0;
    resultado.trocas = 0;
    // Copia o vetor para não modificar o original
    int* vetor_copia = copiar_vetor(vetor_original, tamanho);
    
    // Executa o QuickSort
    quickSort(vetor_copia, 0, tamanho - 1, estrategia, &resultado.chamadas, &resultado.trocas);
    resultado.total = resultado.chamadas + resultado.trocas;
    // Libera a cópia
    free(vetor_copia);
    
    return resultado;
}

// Função para obter o nome da estratégia
const char* obter_nome_estrategia(PartitionType tipo) {
    switch(tipo) {
        case LOMUTO_VANILLA: return "LP";
        case LOMUTO_MEDIAN: return "LM";
        case LOMUTO_RANDOM: return "LA";
        case HOARE_VANILLA: return "HP";
        case HOARE_MEDIAN: return "HM";
        case HOARE_RANDOM: return "HA";
        default: return "Desconhecido";
    }
}
void executarOrdenacao(ResultadoExecucao resultados[], FILE* output) {
    // Selection Sort com desempate pelo PartitionType
    for(int i = 0; i < 5; i++) {
        int min_idx = i;
        
        for(int j = i + 1; j < 6; j++) {
            if(resultados[j].total < resultados[min_idx].total) {
                min_idx = j;
            }
            else if(resultados[j].total == resultados[min_idx].total && 
                    resultados[j].tipo < resultados[min_idx].tipo) {
                min_idx = j;
            }
        }
        
        if(min_idx != i) {
            ResultadoExecucao temp = resultados[i];
            resultados[i] = resultados[min_idx];
            resultados[min_idx] = temp;
        }
    }
    
    // Escreve os resultados ordenados no arquivo
    for(int i = 0; i < 6; i++) {
        fprintf(output, "%s(%lld)", 
               obter_nome_estrategia(resultados[i].tipo),
               resultados[i].total);
        if(i < 5) fprintf(output, ",");
    }
    // ❌ REMOVER esta linha - ela adiciona quebra após cada sequência
    // fprintf(output, "\n");
}

int main(int argc, char* argv[]) {
    clock_t inicio, fim;
    double tempo_gasto;
    inicio = clock(); // Marca o início
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        printf("Erro ao abrir arquivo de entrada\n");
        return 1;
    }

    FILE* output = fopen(argv[2], "w");
    if (!output) {
        printf("Erro ao criar arquivo de saída\n");
        fclose(input);
        return 1;
    }

    int num_sequencias;
    fscanf(input, "%d", &num_sequencias);
    
    for(int i = 0; i < num_sequencias; i++) {
        int tamanho_sequencia;
        fscanf(input, "%d", &tamanho_sequencia);
        
        int* sequencia = (int*) malloc(tamanho_sequencia * sizeof(int));
        for(int j = 0; j < tamanho_sequencia; j++) {
            fscanf(input, "%d", &sequencia[j]);
        }
        
        fprintf(output, "[%d]:", tamanho_sequencia);
        
        ResultadoExecucao resultados[6];
        
        for(int estrategia = 0; estrategia < 6; estrategia++) {
            resultados[estrategia] = executarSort(sequencia, tamanho_sequencia, (PartitionType)estrategia);
        }
        
        executarOrdenacao(resultados, output);
        
        // ✅ Adiciona quebra de linha APENAS se não for a última sequência
        if(i < num_sequencias - 1) {
            fprintf(output, "\n");
        }
        
        free(sequencia);
    }
    
    fclose(input);
    fclose(output);
    fim = clock(); // Marca o fim
    tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %f segundos\n", tempo_gasto);
    printf("Saída escrita em: %s\n", argv[2]);
    return 0;
}




