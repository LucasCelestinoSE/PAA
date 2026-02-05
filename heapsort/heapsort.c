#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // Essencial para o malloc

typedef struct no {
    int prioridade;
    int TAM;
    uint8_t *bytes;
} No;

// Funções auxiliares permanecem iguais
uint32_t esquerdo(uint32_t i) { return 2 * i + 1; }
uint32_t direito(uint32_t i) { return 2 * i + 2; }

// AGORA: Troca structs inteiras, não apenas ints
void trocar(No* V, uint32_t i, uint32_t j) {
    No aux = V[i];
    V[i] = V[j];
    V[j] = aux;
}

// AGORA: Recebe um vetor de No e compara PRIORIDADES
void heapify(No* V, uint32_t T, uint32_t i) {
    uint32_t P = i;
    uint32_t E = esquerdo(i);
    uint32_t D = direito(i);

    // Comparação focada no campo 'prioridade'
    if (E < T && V[E].prioridade > V[P].prioridade) P = E;
    if (D < T && V[D].prioridade > V[P].prioridade) P = D;

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

int main() {
    uint32_t n = 6;
    // Criando um vetor de pacotes (No)
    No *pacotes = (No *) malloc(n * sizeof(No));

    // Exemplo de preenchimento baseado no seu exercício
    pacotes[0].prioridade = 0;
    pacotes[1].prioridade = 2;
    pacotes[2].prioridade = 63;
    pacotes[3].prioridade = 15;
    pacotes[4].prioridade = 32;
    pacotes[5].prioridade = 11;
    for (int i = 0; i < n; i++) {
        printf("Prioridade original: %d, \n", pacotes[i].prioridade);

    }

    heapsort(pacotes, n);
    for (int i = n; i >= 0; i--) {
        printf("Prioridade original: %d, \n", pacotes[i].prioridade);
    }

    // Lembre-se sempre de liberar a memória
    free(pacotes);

    return 0;

    // Ler n pacotes pacotes e quantidade de bytes
    // Criar vetor com N
}