#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define ALPHABET 4

static inline int char_to_index(char c) {
    switch(c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
    }
    return -1;
}

typedef struct {
    int len, link;
    int next[ALPHABET];
} State;

typedef struct {
    char nome[30];
    int n_genes;
    char **genes;
} Doenca;

typedef struct {
    char nome[30];
    int percentual;
    int indice_original;
} Resultado;

void swap(Resultado *a, Resultado *b) {
    Resultado temp = *a;
    *a = *b;
    *b = temp;
}

int partition(Resultado arr[], int low, int high) {
    Resultado pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (arr[j].percentual > pivot.percentual || (arr[j].percentual == pivot.percentual && arr[j].indice_original < pivot.indice_original)) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void meu_fds(Resultado arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        meu_fds(arr, low, pi - 1);
        meu_fds(arr, pi + 1, high);
    }
}

int compute_match(const char *gene, int subcadeia, State *sa) {
    int m = strlen(gene);
    int total = 0;
    int i = 0;
    while(i < m) {
        int cur = 0;
        int j = i;
        int len = 0;
        while(j < m) {
            int c = char_to_index(gene[j]);
            if(sa[cur].next[c] != -1) {
                cur = sa[cur].next[c];
                len++;
                j++;
            } else {
                break;
            }
        }
        if(len >= subcadeia) {
            total += len;
            i = j;
        } else {
            i++;
        }
    }
    return total;
}

int main(int argc, char *argv[]) {
    clock_t tempo_total = clock();

    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");

    int subcadeia;
    fscanf(input, "%d", &subcadeia);

    char *dna = malloc(1000000 * sizeof(char));
    fscanf(input, "%s", dna);
    int n_dna = strlen(dna);

    int saSize = 2 * n_dna;
    State *sa = malloc(saSize * sizeof(State));
    int size = 1;
    int last = 0;
    
    sa[0].len = 0;
    sa[0].link = -1;
    for (int i = 0; i < ALPHABET; i++)
        sa[0].next[i] = -1;
    
    for (int i = 0; i < n_dna; i++) {
        int c = char_to_index(dna[i]);
        int cur = size++;
        sa[cur].len = sa[last].len + 1;
        for (int j = 0; j < ALPHABET; j++)
            sa[cur].next[j] = -1;
        
        int p = last;
        while(p != -1 && sa[p].next[c] == -1) {
            sa[p].next[c] = cur;
            p = sa[p].link;
        }
        if(p == -1) {
            sa[cur].link = 0;
        } else {
            int q = sa[p].next[c];
            if(sa[p].len + 1 == sa[q].len) {
                sa[cur].link = q;
            } else {
                int clone = size++;
                sa[clone].len = sa[p].len + 1;
                for (int j = 0; j < ALPHABET; j++)
                    sa[clone].next[j] = sa[q].next[j];
                sa[clone].link = sa[q].link;
                while(p != -1 && sa[p].next[c] == q) {
                    sa[p].next[c] = clone;
                    p = sa[p].link;
                }
                sa[q].link = sa[cur].link = clone;
            }
        }
        last = cur;
    }

    int n_doencas;
    fscanf(input, "%d", &n_doencas);
    Doenca *doencas = malloc(n_doencas * sizeof(Doenca));
    
    for (int i = 0; i < n_doencas; i++) {
        fscanf(input, "%s %d", doencas[i].nome, &doencas[i].n_genes);
        doencas[i].genes = malloc(doencas[i].n_genes * sizeof(char *));
        char *bloco_genes = malloc(doencas[i].n_genes * 1001 * sizeof(char));
        for (int j = 0; j < doencas[i].n_genes; j++) {
            doencas[i].genes[j] = bloco_genes + (j * 1001);
            fscanf(input, "%1000s", doencas[i].genes[j]);
        }
    }

    Resultado *resultados = malloc(n_doencas * sizeof(Resultado));

    for (int i = 0; i < n_doencas; i++) {
        Doenca d = doencas[i];
        int detectados = 0;
        for (int j = 0; j < d.n_genes; j++) {
            char *gene = d.genes[j];
            int gene_len = strlen(gene);
            int total_matched = compute_match(gene, subcadeia, sa);
            if ((total_matched * 100) / gene_len >= 90)
                detectados++;
        }
        resultados[i].percentual = (detectados * 100 + d.n_genes / 2) / d.n_genes;
        strcpy(resultados[i].nome, d.nome);
        resultados[i].indice_original = i;
    }
    
    meu_fds(resultados, 0, n_doencas - 1);
    
    for (int i = 0; i < n_doencas; i++) {
        fprintf(output, "%s->%d%%\n", resultados[i].nome, resultados[i].percentual);
    }
    
    for (int i = 0; i < n_doencas; i++) {
        free(doencas[i].genes[0]);
        free(doencas[i].genes);
    }
    free(doencas);
    free(resultados);
    free(dna);
    free(sa);
    fclose(input);
    fclose(output);
    
    tempo_total = clock() - tempo_total;
    printf("Tempo de execução total em segundos: %lf\n", ((double)tempo_total) / CLOCKS_PER_SEC);
    return 0;
}