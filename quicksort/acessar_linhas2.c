#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    STANDARD = 1,
    MEDIAN = 2,
    RANDOM = 3
} PivotStrategy;

typedef struct {
    int swaps;
    int calls;
    char code[3];
} QuickSortAlgorithm;

QuickSortAlgorithm LP = {0, 0, "LP"};
QuickSortAlgorithm HP = {0, 0, "HP"};
QuickSortAlgorithm LM = {0, 0, "LM"};
QuickSortAlgorithm HM = {0, 0, "HM"};
QuickSortAlgorithm LA = {0, 0, "LA"};
QuickSortAlgorithm HA = {0, 0, "HA"};

// Função abs customizada para evitar biblioteca math
int my_abs(int x) {
    return x < 0 ? -x : x;
}

void merge(QuickSortAlgorithm *arr, unsigned int left, unsigned int middle, unsigned int right) {
    unsigned int len = right - left + 1;
    QuickSortAlgorithm *array = (QuickSortAlgorithm*)malloc(len * sizeof(QuickSortAlgorithm));

    for (unsigned int i = 0; i < len; i++) {
        array[i] = arr[left + i];
    }

    unsigned int n1 = 0;
    unsigned int n2 = middle - left + 1;
    unsigned int k = left;

    unsigned int end1 = middle - left;
    unsigned int end2 = len - 1;

    while (k <= right) {
        if (n1 > end1) {
            arr[k++] = array[n2++];
            continue;
        }

        if (n2 > end2) {
            arr[k++] = array[n1++];
            continue;
        }

        int metric_n1 = array[n1].calls + array[n1].swaps;
        int metric_n2 = array[n2].calls + array[n2].swaps;

        if (metric_n1 <= metric_n2) {
            arr[k++] = array[n1++];
        } else {
            arr[k++] = array[n2++];
        }
    }

    free(array);
}

void merge_sort(QuickSortAlgorithm *arr, unsigned int left, unsigned int right) {
    if (left < right) {
        unsigned int middle = (left + right) / 2;
        merge_sort(arr, left, middle);
        merge_sort(arr, middle + 1, right);
        merge(arr, left, middle, right);
    }
}

int median(const int arr[], int i, int j) {
    int n = j - i + 1;

    int i1 = i + (n / 4);
    int i2 = i + (n / 2);
    int i3 = i + (3 * n / 4);
    int v1 = arr[i1];
    int v2 = arr[i2];
    int v3 = arr[i3];

    if ((v2 <= v1 && v1 <= v3) || (v3 <= v1 && v1 <= v2)) return i1;
    if ((v1 <= v2 && v2 <= v3) || (v3 <= v2 && v2 <= v1)) return i2;
    return i3;
}

int random_pivot(const int arr[], int i, int j) {
    int n = j - i + 1;
    return i + my_abs(arr[i]) % n;
}

void swap(int arr[], int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

int lomuto(int arr[], int i, int j, int strategy) {
    QuickSortAlgorithm *metric;
    int p, left = i - 1, right = i;

    switch (strategy) {
        case STANDARD:
            metric = &LP;
            break;
        case MEDIAN:
            metric = &LM;
            p = median(arr, i, j);
            swap(arr, j, p);
            metric->swaps++;
            break;
        case RANDOM:
            metric = &LA;
            p = random_pivot(arr, i, j);
            swap(arr, j, p);
            metric->swaps++;
            break;
        default:
            metric = &LP;
            break;
    }

    p = arr[j];

    for (right = i; right < j; right++) {
        if (arr[right] <= p) {
            swap(arr, ++left, right);
            metric->swaps++;
        }
    }

    swap(arr, ++left, right);
    metric->swaps++;

    return left;
}

int hoare(int arr[], int i, int j, int strategy) {
    QuickSortAlgorithm *metric;
    int p, left = i - 1, right = j + 1;

    switch (strategy) {
        case STANDARD:
            metric = &HP;
            break;
        case MEDIAN:
            metric = &HM;
            p = median(arr, i, j);
            swap(arr, i, p);
            metric->swaps++;
            break;
        case RANDOM:
            metric = &HA;
            p = random_pivot(arr, i, j);
            swap(arr, i, p);
            metric->swaps++;
            break;
        default:
            metric = &HP;
            break;
    }

    p = arr[i];

    while (1) {
        while (arr[--right] > p);
        while (arr[++left] < p);
        if (left < right) {
            swap(arr, left, right);
            metric->swaps++;
        } else {
            return right;
        }
    }
}

void quicksort_lomuto(int arr[], int i, int j, int strategy, QuickSortAlgorithm *metric) {
    metric->calls++;

    if (i < j) {
        int p = lomuto(arr, i, j, strategy);
        quicksort_lomuto(arr, i, p - 1, strategy, metric);
        quicksort_lomuto(arr, p + 1, j, strategy, metric);
    }
}

void quicksort_hoare(int arr[], int i, int j, int strategy, QuickSortAlgorithm *metric) {
    metric->calls++;

    if (i < j) {
        int p = hoare(arr, i, j, strategy);
        quicksort_hoare(arr, i, p, strategy, metric);
        quicksort_hoare(arr, p + 1, j, strategy, metric);
    }
}

void reset_metrics() {
    LP.swaps = 0;
    LP.calls = 0;

    LM.swaps = 0;
    LM.calls = 0;

    LA.swaps = 0;
    LA.calls = 0;

    HP.swaps = 0;
    HP.calls = 0;

    HM.swaps = 0;
    HM.calls = 0;

    HA.swaps = 0;
    HA.calls = 0;
}

void copy_array(int arr1[], int arr2[], int n) {
    memcpy(arr2, arr1, n * sizeof(int));
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    FILE *input_file = fopen(input_filename, "r");
    FILE *output_file = fopen(output_filename, "w");

    if (!input_file) {
        printf("Erro ao abrir arquivo de entrada\n");
        return 1;
    }

    if (!output_file) {
        printf("Erro ao criar arquivo de saída\n");
        fclose(input_file);
        return 1;
    }

    int v, n;
    fscanf(input_file, "%d", &v);

    while (v > 0) {
        fscanf(input_file, "%d", &n);
        reset_metrics();
        
        int *original = (int*)malloc(n * sizeof(int));
        int *sorted = (int*)malloc(n * sizeof(int));

        for (int i = 0; i < n; i++) {
            fscanf(input_file, "%d", &original[i]);
            sorted[i] = original[i];
        }

        quicksort_hoare(sorted, 0, n - 1, STANDARD, &HP);
        copy_array(original, sorted, n);

        quicksort_hoare(sorted, 0, n - 1, MEDIAN, &HM);
        copy_array(original, sorted, n);

        quicksort_hoare(sorted, 0, n - 1, RANDOM, &HA);
        copy_array(original, sorted, n);

        quicksort_lomuto(sorted, 0, n - 1, STANDARD, &LP);
        copy_array(original, sorted, n);

        quicksort_lomuto(sorted, 0, n - 1, MEDIAN, &LM);
        copy_array(original, sorted, n);

        quicksort_lomuto(sorted, 0, n - 1, RANDOM, &LA);

        QuickSortAlgorithm s_algorithms[6];

        s_algorithms[0] = LP;
        s_algorithms[1] = LM;
        s_algorithms[2] = LA;
        s_algorithms[3] = HP;
        s_algorithms[4] = HM;
        s_algorithms[5] = HA;

        merge_sort(s_algorithms, 0, 5);

        fprintf(output_file, "[%d]:", n);

        for (int i = 0; i < 5; i++) {
            fprintf(output_file, "%s(%d),", s_algorithms[i].code, 
                    s_algorithms[i].swaps + s_algorithms[i].calls);
        }
        fprintf(output_file, "%s(%d)\n", s_algorithms[5].code, 
                s_algorithms[5].swaps + s_algorithms[5].calls);

        free(original);
        free(sorted);
        v--;
    }

    fclose(input_file);
    fclose(output_file);
    
    return 0;
}