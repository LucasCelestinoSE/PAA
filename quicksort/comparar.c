#include <stdio.h>

int main(int argc, char* argv[]) {
    // Verifica se o arquivo foi passado
    if (argc < 2) {
        printf("Uso: %s <nome_do_arquivo>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("Erro ao abrir arquivo.\n");
        return 1;
    }

    int quantidade_sequencia;
    // Lê o número inteiro da primeira linha
    fscanf(input, "%d", &quantidade_sequencia);
    printf("Quantidade de sequencias: %d \n", quantidade_sequencia);

    // Cria um buffer para armazenar a linha (ajuste o tamanho 100 conforme necessidade)
    char conteudo_linha[100];

    for(int i = 0; i < quantidade_sequencia; i++) {
        // Lê a string da próxima linha e salva em 'conteudo_linha'
        fscanf(input, "%s", conteudo_linha); 
        
        printf("Linha %d: %s \n", i, conteudo_linha);
    }

    fclose(input); // Sempre feche o arquivo!
    return 0;
}   