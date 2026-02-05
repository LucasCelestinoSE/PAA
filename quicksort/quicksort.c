#include <stdio.h>
#include <stdlib.h> 
#include <time.h>   

// Estrutura para armazenar a saída formatada
typedef struct {
    int tam_vetor;
    char** vetor; 
} saida_formatada;

void swap(int* a, int* b);
int hoare(int arr[], int low, int high, long long* trocas);
int lomuto(int arr[], int low, int high, long long* trocas);
int lomuto_median_of_3(int arr[], int low, int high, long long* trocas);
int hoare_median_of_3(int arr[], int low, int high, long long* trocas);

// Função de partição Lomuto com pivô aleatório
int lomuto_random(int arr[], int low, int high, long long* trocas) {
    int random_index = low + abs(arr[low]) % (high - low + 1);
    swap(&arr[high], &arr[random_index]);
    (*trocas)++; 

    
    return lomuto(arr, low, high, trocas);
}

// Função de partição Hoare com pivô aleatório
int hoare_random(int arr[], int low, int high, long long* trocas) {
    // Seleciona o pivô de forma pseudo-aleatória
    int random_index = low + abs(arr[low]) % (high - low + 1);
    swap(&arr[low], &arr[random_index]);
    (*trocas)++; // Conta a troca do pivô

    // Chama a partição Hoare padrão
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


// partition function
int lomuto(int arr[], int low, int high, long long* trocas) {
    
    // Choose the pivot
    int pivot = arr[high];
    
    // Index of smaller element and indicates 
    // the right position of pivot found so far
    int i = low - 1;

    // Traverse arr[low..high] and move all smaller
    // elements to the left side. Elements from low to 
    // i are smaller after every iteration
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
            (*trocas)++;
        }
    }
    
    // Move pivot after smaller elements and
    // return its position
    swap(&arr[i + 1], &arr[high]);
    (*trocas)++;
    return i + 1;
}

int hoare(int arr[], int low, int high, long long* trocas) {
    int pivot = arr[low];
    int i = low - 1;
    int j = high + 1;

    while (1) {
        // Find leftmost element greater than or equal to pivot
        do {
            i++;
        } while (arr[i] < pivot);

        // Find rightmost element smaller than or equal to pivot
        do {
            j--;
        } while (arr[j] > pivot);

        // If two pointers met
        if (i >= j)
            return j;

        swap(&arr[i], &arr[j]);
        (*trocas)++;
    }
}

// The QuickSort function implementation
void quickSort(int arr[], int low, int high, int partitionType, long long* chamadas, long long* trocas) {
    (*chamadas)++;
    if (low < high) {
        int pi;
        // pi is the partition return index of pivot
        if(partitionType == 0 ) { // Lomuto
            pi = lomuto(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
        if(partitionType == 1 ) { // Hoare
            pi = hoare(arr, low, high, trocas);
            // recursion calls for smaller elements
            // and greater or equals elements
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
        if(partitionType == 2 ) { // Lomuto Random
            pi = lomuto_random(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
        if(partitionType == 3 ) { // Hoare Random
            pi = hoare_random(arr, low, high, trocas);
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
        if(partitionType == 4 ) { // Lomuto Mediana de 3
            pi = lomuto_median_of_3(arr, low, high, trocas);
            quickSort(arr, low, pi - 1, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
        if(partitionType == 5 ) { // Hoare Mediana de 3
            pi = hoare_median_of_3(arr, low, high, trocas);
            quickSort(arr, low, pi, partitionType, chamadas, trocas);
            quickSort(arr, pi + 1, high, partitionType, chamadas, trocas);
        }
    }
}

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}
int main(int argc, char* argv[]) {
    // Inicializa o gerador de números aleatórios
    printf("ARGS = %i\n", argc);
    printf("PROGRAMA = %s %s , %s \n " , argv[0], argv[1], argv[2]);
    srand(time(NULL));
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    int quantidade_sequencia;
    fscanf(input, "%d", &quantidade_sequencia);
    printf("quantidade de sequencia %d \n", quantidade_sequencia);
    int original_arr[] = {-23, 10, 7, -34, 432, 3};
    int n = sizeof(original_arr) / sizeof(original_arr[0]);
    int arr[n];
    for (int j = 0; j < quantidade_sequencia; j++ ){
        printf("Testando entrada %d \n", quantidade_sequencia);
        
    for (int k = 0; k <= 5; k++) {
        // Resetar o array para o estado original
        for (int i = 0; i < n; i++) {
            arr[i] = original_arr[i];
        }

        // Resetar contadores
        long long chamadas = 0;
        long long trocas = 0;

        if (k == 0) {
            printf("Iniciando Lomuto \n");
            quickSort(arr, 0, n - 1, 0, &chamadas, &trocas);
        }
        if (k == 1) {
            printf("Iniciando Hoare \n");
            quickSort(arr, 0, n - 1, 1, &chamadas, &trocas);
        }
        if (k == 2) {
            printf("Iniciando Lomuto Randomico\n");
            quickSort(arr, 0, n - 1, 2, &chamadas, &trocas);
        }
        if (k == 3) {
            printf("Iniciando Hoare Randomico\n");
            quickSort(arr, 0, n - 1, 3, &chamadas, &trocas);
        }
        if (k == 4) {
            printf("Iniciando Lomuto Mediana de 3\n");
            quickSort(arr, 0, n - 1, 4, &chamadas, &trocas);
        }
        if (k == 5) {
            printf("Iniciando Hoare Mediana de 3\n");
            quickSort(arr, 0, n - 1, 5, &chamadas, &trocas);
        }

        printf("Vetor ordenado: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");

        printf("Numero de chamadas recursivas e trocas: %lld\n", chamadas + trocas);
    }
    }
    fclose(input);
    fclose(output);
    return 0;
}