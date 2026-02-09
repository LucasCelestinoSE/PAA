#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 10000
#define MAX_CONTEUDO 1000

typedef struct {
    int prioridade;
    int tamanho;
    char **bytes;
} No;

typedef struct {
    No *dados;
    int tamanho;
    int capacidade;
} VetorPacotes;

void criar_pacote(Pacote *p, int prioridade, int tamanho, char **conteudo, int num_conteudo) {
    p->prioridade = prioridade;
    p->tamanho = tamanho;
    p->num_conteudo = num_conteudo;
    
    p->conteudo_bytes = (char **)malloc(num_conteudo * sizeof(char *));
    for (int i = 0; i < num_conteudo; i++) {
        p->conteudo_bytes[i] = strdup(conteudo[i]);
    }
}

void liberar_pacote(Pacote *p) {
    for (int i = 0; i < p->num_conteudo; i++) {
        free(p->conteudo_bytes[i]);
    }
    free(p->conteudo_bytes);
}

void inicializar_vetor(VetorPacotes *v) {
    v->capacidade = 10;
    v->tamanho = 0;
    v->dados = (Pacote *)malloc(v->capacidade * sizeof(Pacote));
}

void adicionar_pacote(VetorPacotes *v, Pacote p) {
    if (v->tamanho >= v->capacidade) {
        v->capacidade *= 2;
        v->dados = (Pacote *)realloc(v->dados, v->capacidade * sizeof(Pacote));
    }
    v->dados[v->tamanho++] = p;
}

void liberar_vetor(VetorPacotes *v) {
    for (int i = 0; i < v->tamanho; i++) {
        liberar_pacote(&v->dados[i]);
    }
    free(v->dados);
}

void swap_manual(Pacote *a, Pacote *b) {
    Pacote temp = *a;
    *a = *b;
    *b = temp;
}

void reverse_manual(Pacote *arr, int tamanho) {
    int start = 0;
    int end = tamanho - 1;
    while (start < end) {
        swap_manual(&arr[start], &arr[end]);
        start++;
        end--;
    }
}

int string_to_int(const char *s) {
    int resultado = 0;
    int sinal = 1;
    int i = 0;
    
    // Remove espaços em branco no início
    while (s[i] == ' ' || s[i] == '\t') {
        i++;
    }
    
    // Verifica se há sinal
    if (s[i] == '-') {
        sinal = -1;
        i++;
    } else if (s[i] == '+') {
        i++;
    }
    
    // Converte dígitos para número
    while (s[i] >= '0' && s[i] <= '9') {
        resultado = resultado * 10 + (s[i] - '0');
        i++;
    }
    
    return resultado * sinal;
}

void heapify(Pacote *arr, int n, int i) {
    int maior = i;
    int esquerda = 2 * i + 1;
    int direita = 2 * i + 2;

    if (esquerda < n && arr[esquerda].prioridade > arr[maior].prioridade) {
        maior = esquerda;
    }

    if (direita < n && arr[direita].prioridade > arr[maior].prioridade) {
        maior = direita;
    }

    if (maior != i) {
        swap_manual(&arr[i], &arr[maior]);
        heapify(arr, n, maior);
    }
}

void heapsort(Pacote *arr, int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        swap_manual(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
    
    reverse_manual(arr, n);
}

void main_logic(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return;
    }

    const char *nome_arquivo_entrada = argv[1];
    const char *nome_arquivo_saida = argv[2];

    FILE *arquivo_saida = fopen(nome_arquivo_saida, "w");
    if (!arquivo_saida) {
        fprintf(stderr, "Erro ao abrir o arquivo de saída: %s\n", nome_arquivo_saida);
        return;
    }

    FILE *golden_input = fopen(nome_arquivo_entrada, "r");
    if (!golden_input) {
        fprintf(arquivo_saida, "Erro ao abrir o arquivo de entrada: %s\n", nome_arquivo_entrada);
        fclose(arquivo_saida);
        return;
    }

    char primeira_linha[MAX_LINE];
    if (!fgets(primeira_linha, MAX_LINE, golden_input)) {
        fprintf(arquivo_saida, "Erro ao ler a primeira linha do arquivo de entrada.\n");
        fclose(golden_input);
        fclose(arquivo_saida);
        return;
    }

    char *token = strtok(primeira_linha, " \t\n");
    int count = 0;
    int TAMANHO_DO_BUFFER = 0;
    
    while (token != NULL && count < 2) {
        if (count == 1) {
            TAMANHO_DO_BUFFER = string_to_int(token);
        }
        count++;
        token = strtok(NULL, " \t\n");
    }

    if (count < 2) {
        fprintf(arquivo_saida, "Erro no formato da linha de cabeçalho.\n");
        fclose(golden_input);
        fclose(arquivo_saida);
        return;
    }

    VetorPacotes lista_de_pacotes;
    inicializar_vetor(&lista_de_pacotes);

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, golden_input)) {
        char *partes[MAX_CONTEUDO];
        int num_partes = 0;
        
        token = strtok(line, " \t\n");
        while (token != NULL && num_partes < MAX_CONTEUDO) {
            partes[num_partes++] = token;
            token = strtok(NULL, " \t\n");
        }

        if (num_partes < 3) {
            continue;
        }

        int prioridade = string_to_int(partes[0]);
        int tamanho = string_to_int(partes[1]);
        
        Pacote p;
        criar_pacote(&p, prioridade, tamanho, &partes[2], num_partes - 2);
        adicionar_pacote(&lista_de_pacotes, p);
    }

    fclose(golden_input);

    int indice_pacote_atual = 0;
    while (indice_pacote_atual < lista_de_pacotes.tamanho) {
        Pacote *chunk_atual = (Pacote *)malloc(lista_de_pacotes.tamanho * sizeof(Pacote));
        int tamanho_chunk_atual = 0;
        int tamanho_chunk = 0;
        int inicio_chunk = indice_pacote_atual;
        
        while (indice_pacote_atual < lista_de_pacotes.tamanho) {
            Pacote *proximo_pacote = &lista_de_pacotes.dados[indice_pacote_atual];
            if (tamanho_chunk + proximo_pacote->tamanho <= TAMANHO_DO_BUFFER) {
                chunk_atual[tamanho_chunk_atual++] = *proximo_pacote;
                tamanho_chunk += proximo_pacote->tamanho;
                indice_pacote_atual++;
            } else {
                break;
            }
        }
        
        if (tamanho_chunk_atual == 0 && indice_pacote_atual < lista_de_pacotes.tamanho && inicio_chunk == indice_pacote_atual) {
            Pacote *proximo_pacote = &lista_de_pacotes.dados[indice_pacote_atual];
            chunk_atual[tamanho_chunk_atual++] = *proximo_pacote;
            indice_pacote_atual++;
        }

        if (tamanho_chunk_atual == 0) {
            free(chunk_atual);
            break;
        }
        
        heapsort(chunk_atual, tamanho_chunk_atual);
        
        fprintf(arquivo_saida, "|");
        for (int i = 0; i < tamanho_chunk_atual; i++) {
            Pacote *pacote = &chunk_atual[i];
            
            for (int j = 0; j < pacote->num_conteudo; j++) {
                fprintf(arquivo_saida, "%s", pacote->conteudo_bytes[j]);
                if (j < pacote->num_conteudo - 1) {
                    fprintf(arquivo_saida, ",");
                }
            }
            
            if (i < tamanho_chunk_atual - 1) {
                fprintf(arquivo_saida, "|");
            }
        }
        fprintf(arquivo_saida, "|\n");
        
        free(chunk_atual);
    }

    liberar_vetor(&lista_de_pacotes);
    fclose(arquivo_saida);
}

void imprimir_nos(No *vetor, int n) {
    for (int i = 0; i < n; i++) {
        printf("|");
        for (int j = 0; j < vetor[i].tamanho; j++) {
            printf("%s%s", vetor[i].bytes[j], (j < vetor[i].tamanho - 1) ? "," : "");
        }
        printf("|\n");
    }
}

int main(int argc, char *argv[]) {
    main_logic(argc, argv);
    return 0;
}