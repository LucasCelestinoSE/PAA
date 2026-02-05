// E/S padrao
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// TODO
// # Pegar dados da entrada.
// # Criar banco de dados em memoria.

typedef struct container {
    int codigo;
    char cnpj[15];
    float peso;
} container_t;
typedef struct initialize_args {
    int num_containers;
    // Ponteiro para as strings (cada string tem no máximo 40 caracteres)
    char (*entrys)[40]; 
} args_t;
args_t split_function(FILE* input)
{
    char linha[256];
    container_t temp;
    size_t linha_no = 0;
    args_t args
    while (fgets(linha, sizeof linha, input)) {
        linha_no++;
        

        if (linha[0] == '\0') continue; /* pular linhas vazias */
        /* imprime a linha atual com o número */
        printf("linha %zu: %s\n", linha_no, linha);
    }
}
// Funcao principal
int main(int argc, char* argv[]) {
    container_t cadastros;
    // Ilustrando uso de argumentos de programa
    printf("#ARGS = %i\n", argc);
    printf("PROGRAMA = %s\n", argv[0]);
    printf("ARG1 = %s, ARG2 = %s\n", argv[1], argv[2]);
    // Abrindo arquivos

    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    // TODO
    // ...
    //
    split_function(input);
    // Fechando arquivos
    fclose(input);
    fclose(output);
    // Finalizando programa
    return 0;
}
