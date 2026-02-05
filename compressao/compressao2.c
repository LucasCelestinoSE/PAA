#include <stdint.h>
typedef struct no
{  
    // Frequência
    uint32_t F; 
    // Código do Símbolo
    char S;
    // Nó direito
    no* D;
    no* E;
} no;
