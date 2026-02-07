#include <stdint.h>
typedef struct no
{  
    // Frequência
    uint32_t F; 
    // Código do Símbolo
    char S;
    // Nó direito
    no* D;
    // Nó esquerdo
    no* E;
} no;
uint8_t simbolos[256];
typedef struct {
    no** dados;    // Array de ponteiros para nós
    uint32_t cap;  // Capacidade máxima
    uint32_t tam;  // Tamanho atual
} fila_p_min;

fila_p_min* criar_fila_p_min() {
   
    return fpm;
}

// Construção da árvore de prefixos (Trie)
no* construir_arvore(uint32_t H[], uint32_t n) {
    // Criação de FIla de prioridade mínima
    fila_p_min* fpm = criar_fila_p_min();
    // Inserindo símbolos não nulos na fila;
    for (uint32_t i = 0; i < n; i++){
        if (H[i] inserir(fpm, H[i], i, NULL, NULL ));
    // Combinação do nós com menor frequência 
    while(tamanho(fpm) > 1 ) {
        no* x = extrair_min(fpm);
        no* y = extrair_min(fpm);
        inserir(fpm, x.freq + y.freq, "\0", x,y);

    }
    // Retornando a raiz da árvore
    return extrair_min(fpm);
    }
}