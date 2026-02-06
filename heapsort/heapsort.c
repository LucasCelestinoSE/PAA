#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct no {
    int prioridade;
    int TAM;
    uint8_t *bytes;
} No;

// --- Funções do Heap (Mantidas iguais) ---
uint32_t esquerdo(uint32_t i) { return 2 * i + 1; }
uint32_t direito(uint32_t i) { return 2 * i + 2; }

void trocar(No* V, uint32_t i, uint32_t j) {
    No aux = V[i];
    V[i] = V[j];
    V[j] = aux;
}

void heapify(No* V, uint32_t T, uint32_t i) {
    uint32_t P = i;
    uint32_t E = esquerdo(i);
    uint32_t D = direito(i);

    // Alterado: usa < para encontrar o menor e subir no heap
    if (E < T && V[E].prioridade < V[P].prioridade) P = E;
    if (D < T && V[D].prioridade < V[P].prioridade) P = D;

    if (P != i) {
        trocar(V, i, P);
        heapify(V, T, P);
    }
}

void construir_heap(No V[], uint32_t n) {
    for (int32_t i = (n / 2) - 1; i >= 0; i--) {
        heapify(V, n, i);
    }
}

void heapsort(No V[], uint32_t n) {
    construir_heap(V, n);
    for (int32_t i = n - 1; i > 0; i--) {
        trocar(V, 0, i);
        heapify(V, i, 0);
    }
}

// --- Main ---
int main(int argc, char* argv[]) {
    if (argc < 3) return 1;

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    int num_total_pacotes, limite_bytes;
    fscanf(input, "%d %d", &num_total_pacotes, &limite_bytes);

    // Buffer para guardar os pacotes que serão ordenados juntos
    No *buffer = (No *)malloc(num_total_pacotes * sizeof(No));
    int count_buffer = 0;
    int bytes_no_buffer = 0;

    for (int i = 0; i < num_total_pacotes; i++) {
        // Lendo os dados do próximo pacote do arquivo
        int p, tam;
        fscanf(input, "%d %d", &p, &tam);

        // Se adicionar este pacote estourar o limite, processamos o buffer atual
        if (bytes_no_buffer + tam > limite_bytes && count_buffer > 0) {
           heapsort(buffer, count_buffer);
            for (int j = 0; j < count_buffer; j++) { // Ordem direta: 0, 1, 2...
                fprintf(output, "|");
                for (int k = 0; k < buffer[j].TAM; k++) {
                    fprintf(output, "%02X%s", buffer[j].bytes[k], (k < buffer[j].TAM - 1) ? "," : "");
                }
                free(buffer[j].bytes);
            }
            // Verifique se o gabarito realmente pede o | e o \n na última linha
            fprintf(output, "|\n");

            // Reseta o buffer
            count_buffer = 0;
            bytes_no_buffer = 0;
        }

        // Adiciona o pacote atual ao buffer
        buffer[count_buffer].prioridade = p;
        buffer[count_buffer].TAM = tam;
        buffer[count_buffer].bytes = (uint8_t *)malloc(tam * sizeof(uint8_t));
        for (int j = 0; j < tam; j++) {
            fscanf(input, "%hhx", &buffer[count_buffer].bytes[j]);
        }
        
        bytes_no_buffer += tam;
        count_buffer++;
    }

    // Processa o que restou no buffer após o fim do arquivo
    if (count_buffer > 0) {
        heapsort(buffer, count_buffer);
        for (int j = count_buffer - 1; j >= 0; j--) {
            fprintf(output, "|");
            for (int k = 0; k < buffer[j].TAM; k++) {
                fprintf(output, "%02X%s", buffer[j].bytes[k], (k < buffer[j].TAM - 1) ? "," : "");
            }
            free(buffer[j].bytes);
        }
        fprintf(output, "|\n");
    }

    free(buffer);
    fclose(input);
    fclose(output);
    return 0;
}