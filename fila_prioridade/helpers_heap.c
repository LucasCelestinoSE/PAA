#include <stdio.h>
#include <stdint.h>

/* 
O objetivo é entender como funciona 
 construção
 Seleção de item no heap ( busca )
 Inserção de um item no heap
 remoção
 alteração
*/ 

// Filho esquerdo, 2i + 1
// Filho direito 2i + 2
uint8_t simbolos[256];
int esquerda(int i) {return 2 * i + 1;}
int direita(int i) {return 2*i+2;}
void trocar(int32_t* V, uint32_t i, uint32_t j) {
    int32_t aux = V[i];
    V[i] = V[j];
    V[j] = aux;
}

void heapify(int32_t* V, uint32_t T, uint32_t i){
    uint32_t P = i, E = esquerda(i), D = direita(i);
    // Filho da esquerda é maior
    if(E < T && V[E] > V[P]) P = E;
    // Filho da direita é maior
    if(D < T && V[D] > V[P]) P = D;
    // Troca e chamada recursiva
    if(P != i) {
        trocar(V, P, i);
        heapify(V,T,P);
    }

}
void construirHeap(int arr[], int T) {
    // Começa do último pai (n/2 - 1) e vai até a raiz (0)
    for (int i = T / 2 - 1; i >= 0; i--) {
        heapify(arr, T, i);
    }
}
void imprimirHeap(int V[], int T) {
    printf("Estrutura do Heap (Max-Heap):\n");
    for (int i = 0; i < (T / 2); i++) { // Percorre apenas os nós que têm filhos
        printf("Pai: [%d] ", V[i]);
        
        if (esquerda(i) < T) 
            printf("| Filho Esq: %d ", V[esquerda(i)]);
        
        if (direita(i) < T) 
            printf("| Filho Dir: %d", V[direita(i)]);
            
        printf("\n");
    }
}

int main (){
    // para ser um heap Si <= S_mod(i / 2)]
    uint8_t vector[5] = {0xAA,0xAA,0xAA,0xAA,0xAA};
    for(int i = 0; i < 5; i++){
        simbolos[vector[i]] += 1;
    }
    printf("%d", simbolos[0xAA]);
    

    return 0;
}