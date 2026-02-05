// linha de comando para compilar: gcc -Wall -O3 Main.c -o Main -lm -lpthread 2>&1
// linha de comando para executar: ./Main prova.porto.input prova.porto.output

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TAMANHO_CODIGO 12
#define TAMANHO_CNPJ 19
#define TAMANHO_TABELA_HASH 1024

typedef struct Container {
    char codigo[TAMANHO_CODIGO];
    char cnpj[TAMANHO_CNPJ];
    int peso;
    int indice;
    struct Container* proximo;  
} Container;

Container* hashTable[TAMANHO_TABELA_HASH];

unsigned int hashFunction(char* codigo){
    unsigned int hash_value = 0;
    for (int i = 0; i < TAMANHO_CODIGO && codigo[i] != '\0'; i++){
        hash_value += codigo[i];
        hash_value = (hash_value * codigo[i]) % TAMANHO_TABELA_HASH;
    }
    return hash_value;
}


bool insert(Container *c){
    if (c == NULL) return false;
    int idx = hashFunction(c->codigo);
    c->proximo = hashTable[idx];
    hashTable[idx] = c;
    return true;
}

Container* search(char *codigo){
    int idx = hashFunction(codigo);
    Container* tmp = hashTable[idx];
    while (tmp != NULL) {
        if (strcmp(tmp->codigo, codigo) == 0) {
            return tmp;  
        }
        tmp = tmp->proximo;
    }
    return NULL;
}

int getDiferencaPercentual(int peso_c1, int peso_c2) {
    float diff = (fabs(peso_c1 - peso_c2) / (float)peso_c1) * 100;
    return (roundf(diff));
}

bool excedeLimitePeso(int peso_c1, int peso_c2){
    return getDiferencaPercentual(peso_c1, peso_c2) > 10;
}

int compararPorIndice(const void* a, const void* b) {
    Container* c1 = (Container*)a;
    Container* c2 = (Container*)b;
    return c1->indice - c2->indice;
}

void intercalarContainersPorPeso(Container* containers, Container* auxiliar, int inicio, int meio, int fim) {
int indiceEsquerdo = inicio, indiceDireito = meio + 1, indiceAuxiliar = inicio;
    
    while (indiceEsquerdo <= meio && indiceDireito <= fim) {
        Container* containerEsquerdo = &containers[indiceEsquerdo];
        Container* containerDireito = &containers[indiceDireito];
        
        Container* containerOriginalEsquerdo = search(containerEsquerdo->codigo);
        Container* containerOriginalDireito = search(containerDireito->codigo);

        int diferencaEsquerda = getDiferencaPercentual(containerOriginalEsquerdo->peso, containerEsquerdo->peso);
        int diferencaDireita = getDiferencaPercentual(containerOriginalDireito->peso, containerDireito->peso);
        
        if (diferencaEsquerda > diferencaDireita || 
            (diferencaEsquerda == diferencaDireita && containerOriginalEsquerdo->indice < containerOriginalDireito->indice)) {
            auxiliar[indiceAuxiliar++] = containers[indiceEsquerdo++];
        } else {
            auxiliar[indiceAuxiliar++] = containers[indiceDireito++];
        }
    }

    while (indiceEsquerdo <= meio) {
        auxiliar[indiceAuxiliar++] = containers[indiceEsquerdo++];
    }

    while (indiceDireito <= fim) {
        auxiliar[indiceAuxiliar++] = containers[indiceDireito++];
    }

    for (int i = inicio; i <= fim; i++) {
        containers[i] = auxiliar[i];
    }
}

void mergesort(Container* containers, Container* auxiliar, int inicio, int fim) {
    if (inicio < fim) {
        int meio = inicio + (fim - inicio) / 2;
        
        mergesort(containers, auxiliar, inicio, meio);
        mergesort(containers, auxiliar, meio + 1, fim);
        intercalarContainersPorPeso(containers, auxiliar, inicio, meio, fim);
    }
}





int main(int argc, char* argv[]) {
    clock_t tempo_total;
    tempo_total = clock();

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");

    int n_containers_cadastrados;
    fscanf(input, "%d", &n_containers_cadastrados);

for (int i = 0; i < n_containers_cadastrados; i++) {
    Container* container_cadastrado = (Container*) malloc(sizeof(Container));
    fscanf(input, "%11s %18s %d", container_cadastrado->codigo, container_cadastrado->cnpj,  &container_cadastrado->peso);
    container_cadastrado->indice = i;
    insert(container_cadastrado);
}

    int n_containers_selecionados;
    fscanf(input, "%d", &n_containers_selecionados);

    Container* containers_cnpj_errado = malloc(n_containers_selecionados * sizeof(Container));
    Container* containers_peso_errado = malloc(n_containers_selecionados * sizeof(Container));
    int counter_cnpj_errado = 0;
    int counter_peso_errado = 0;

    for (int i = 0; i < n_containers_selecionados; i++) {
        Container container_selecionado;
        fscanf(input, "%11s %18s %d", container_selecionado.codigo, container_selecionado.cnpj,  &container_selecionado.peso);

        Container* container_relacionado = search(container_selecionado.codigo);
        if (container_relacionado != NULL){
            if (strcmp(container_selecionado.cnpj, container_relacionado->cnpj) != 0) {
                container_selecionado.indice = container_relacionado->indice; 
                containers_cnpj_errado[counter_cnpj_errado] = container_selecionado;
                counter_cnpj_errado++;
                continue;
            }

            if (excedeLimitePeso(container_relacionado->peso, container_selecionado.peso)){
                containers_peso_errado[counter_peso_errado] = container_selecionado;
                counter_peso_errado++;
                continue;
            }
        } 
    }

    qsort(containers_cnpj_errado, counter_cnpj_errado, sizeof(Container), compararPorIndice);

    for (int i = 0; i < counter_cnpj_errado; i++){
        fprintf(output, "%s:%s<->%s\n",
        containers_cnpj_errado[i].codigo, search(containers_cnpj_errado[i].codigo)->cnpj, containers_cnpj_errado[i].cnpj);
    }

    Container* auxiliarOrdenacao = malloc(n_containers_selecionados * sizeof(Container));
    mergesort(containers_peso_errado, auxiliarOrdenacao, 0, counter_peso_errado - 1);

    for (int i = 0; i < counter_peso_errado; i++){
        Container c1 = *search(containers_peso_errado[i].codigo);
        Container c2 = containers_peso_errado[i];

        int diferenca_absoluta = abs(c1.peso - c2.peso);
        int diferenca_percentual = getDiferencaPercentual(c1.peso, c2.peso);

        fprintf(output, "%s:%dkg(%d%%)\n", c1.codigo, diferenca_absoluta, diferenca_percentual);
    }

    free(containers_cnpj_errado);
    free(containers_peso_errado);
    free(auxiliarOrdenacao);
    fclose(input);
    fclose(output);

    tempo_total = clock() - tempo_total;
    printf("Tempo de execucao total em segundos: %lf\n", ((double)tempo_total)/((CLOCKS_PER_SEC)));

    return 0;
}
