#include <stdio.h>
#include <stdlib.h>

#define NUM_LINHAS 1800

int main() {
    FILE *arquivo = fopen("dados.txt", "r");
    vetor => ponteiro => int vetor
    // 1. Aloca as linhas (esqueleto)
    int **vetor = (int **) malloc(NUM_LINHAS * sizeof(int *));
    
    // Vetor auxiliar para guardar o tamanho de cada linha (opcional, mas útil para ordenação depois)
    int *tamanhos = (int *) malloc(NUM_LINHAS * sizeof(int));

    for (int i = 0; i < NUM_LINHAS; i++) {
        int qtd_colunas;
        
        // Lê quantos números tem nesta linha
        if (fscanf(arquivo, "%d", &qtd_colunas) == 1) {
            tamanhos[i] = qtd_colunas;

            // 2. Aloca a linha com o tamanho exato lido
            vetor[i] = (int *) malloc(qtd_colunas * sizeof(int));

            // 3. Preenche a linha lendo os números
            for (int j = 0; j < qtd_colunas; j++) {
                fscanf(arquivo, "%d", &vetor[i][j]);
            }
        }
    }

    // Agora você tem 'vetor[i][j]' acessível perfeitamente
    // e sem desperdício de memória.
    
    // ... Implemente seu QuickSort/MergeSort aqui ...

    fclose(arquivo);
    return 0;
}