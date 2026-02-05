// ...existing code...
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t VETOR[] = {45,13,17,51,3,21};

// protótipo para a função de intercalação usada em merge_sort
void intercalar(int32_t* S, int32_t* E, int32_t i, int32_t m, int32_t j){
    int32_t i1 = i, i2 = m + 1, k = i;
    // Enquanto houver elementos em ambos os lados
    while (i1 <= m && i2 <= j){
        if (E[i1] <= E[i2]){
            S[k++] = E[i1++];
        } else {
            S[k++] = E[i2++];
        }
    }
    // copia o que restou de um dos lados
    if (i1 > m){
        while (i2 <= j) S[k++] = E[i2++];
    } else {
        while (i1 <= m) S[k++] = E[i1++];
    }
}

void merge_sort(int32_t* S, int32_t* E, int32_t i, int32_t j) {
    if (i >= j) return;
    int32_t m = (i + j) / 2;
    // ordena as duas metades (usando S como vetor principal)
    merge_sort(S, E, i, m);
    merge_sort(S, E, m + 1, j);
    // copiar o intervalo atual para o buffer E antes de intercalar
    for (int32_t k = i; k <= j; ++k) E[k] = S[k];
    intercalar(S, E, i, m, j);
}

int main(void) {
    size_t n = sizeof(VETOR) / sizeof(VETOR[0]);
    int32_t *E = malloc(n * sizeof *E);
    if (!E) return 1;

    merge_sort(VETOR, E, 0, (int32_t)(n - 1));

    for (size_t k = 0; k < n; ++k) {
        printf("%d%c", VETOR[k], (k + 1 < n) ? ' ' : '\n');
    }

    free(E);
    return 0;
}
