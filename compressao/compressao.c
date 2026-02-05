#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct sequencia {
    int tamanho;
    char** conteudo;
    float percentual_HUFF;
    float percentual_RLE;
    char* conteudo_comprimido_RLE;
    char* conteudo_comprimido_HUFF;
} sequencia;

typedef struct node {
    int freq;
    char S;
    struct node *D;
    struct node *E;
} no;

typedef struct fila_p_min {
    int tam;
    int cap;
    no **V;
} fila_p_min;

fila_p_min *criar_fila_p_min() {
    fila_p_min *fpm = (fila_p_min*)malloc(sizeof(fila_p_min));
    fpm->tam = 0;
    fpm->cap = 256;  // Capacidade inicial aumentada
    fpm->V = (no**)malloc((fpm->cap + 1) * sizeof(no*));
    return fpm;
}

void inserir(fila_p_min *fpm, int freq, char S, no *E, no *D) {
    if (fpm->tam + 1 >= fpm->cap) {
        fpm->cap *= 2;
        fpm->V = (no**)realloc(fpm->V, (fpm->cap + 1) * sizeof(no*));
    }
    fpm->tam++;
    no* novo = (no*)malloc(sizeof(no));
    novo->freq = freq;
    novo->S = S;
    novo->E = E;
    novo->D = D;
    int i = fpm->tam;
    fpm->V[i] = novo;
    while (i > 1 && fpm->V[i]->freq < fpm->V[i / 2]->freq) {
        no *aux = fpm->V[i];
        fpm->V[i] = fpm->V[i / 2];
        fpm->V[i / 2] = aux;
        i = i / 2;
    }
}

no *extrair_min(fila_p_min *fpm) {
    no *min = fpm->V[1];
    fpm->V[1] = fpm->V[fpm->tam];
    fpm->tam--;
    int i = 1;
    while (2 * i <= fpm->tam) {
        int m = 2 * i;
        if (2 * i + 1 <= fpm->tam && fpm->V[2 * i + 1]->freq < fpm->V[2 * i]->freq) m = 2 * i + 1;
        if (fpm->V[i]->freq <= fpm->V[m]->freq) break;
        no *aux = fpm->V[i];
        fpm->V[i] = fpm->V[m];
        fpm->V[m] = aux;
        i = m;
    }
    return min;
}

no *construir_arvore(int H[], int n) {
    fila_p_min *fpm = criar_fila_p_min();
    for (int i = 0; i < n; i++) {
        if (H[i] > 0) {
            inserir(fpm, H[i], (char)i, NULL, NULL);
        }
    }
    if (fpm->tam == 0) {
        free(fpm->V);
        free(fpm);
        return NULL;
    }
    while (fpm->tam > 1) {
        no *x = extrair_min(fpm);
        no *y = extrair_min(fpm);
        inserir(fpm, x->freq + y->freq, '\0', x, y);
    }
    no *raiz = extrair_min(fpm);
    free(fpm->V);
    free(fpm);
    return raiz;
}

void tabela_codigos_iter(no *raiz, char *T[]) {
    if (raiz == NULL) return;

    typedef struct {
        no *node;
        char path[256];
        int depth;
    } StackNode;

    StackNode stack[512];
    int top = 0;

    stack[0].node = raiz;
    stack[0].depth = 0;
    stack[0].path[0] = '\0';
    top++;

    while (top > 0) {
        StackNode current = stack[--top];
        no *node = current.node;

        if (node->E == NULL && node->D == NULL) {
            if (current.depth == 0) {
                strcpy(T[(unsigned char)node->S], "0");
            } else {
                strncpy(T[(unsigned char)node->S], current.path, current.depth);
                T[(unsigned char)node->S][current.depth] = '\0';
            }
        } else {
            if (node->D) {
                StackNode next;
                next.node = node->D;
                next.depth = current.depth + 1;
                memcpy(next.path, current.path, current.depth);
                next.path[current.depth] = '1';
                stack[top++] = next;
            }
            if (node->E) {
                StackNode next;
                next.node = node->E;
                next.depth = current.depth + 1;
                memcpy(next.path, current.path, current.depth);
                next.path[current.depth] = '0';
                stack[top++] = next;
            }
        }
    }
}

void liberar_arvore(no *raiz) {
    if (raiz == NULL) return;
    liberar_arvore(raiz->E);
    liberar_arvore(raiz->D);
    free(raiz);
}


// 2. Buffer único para códigos
char* HUF(char **strings, int t) {
    uint8_t *bytes = (uint8_t*)malloc(t * sizeof(uint8_t));
    // Pré-processamento otimizado
    for (int i = 0; i < t; i++) {
        char c1 = strings[i][0];
        char c2 = strings[i][1];
        int val1, val2;

        // Converter primeiro caractere
        if (c1 >= '0' && c1 <= '9') {
            val1 = c1 - '0';
        } else if (c1 >= 'A' && c1 <= 'F') {
            val1 = c1 - 'A' + 10;
        } else if (c1 >= 'a' && c1 <= 'f') {
            val1 = c1 - 'a' + 10;
        } else {
            // Tratar erro, se necessário
            val1 = 0;
        }

        // Converter segundo caractere
        if (c2 >= '0' && c2 <= '9') {
            val2 = c2 - '0';
        } else if (c2 >= 'A' && c2 <= 'F') {
            val2 = c2 - 'A' + 10;
        } else if (c2 >= 'a' && c2 <= 'f') {
            val2 = c2 - 'a' + 10;
        } else {
            // Tratar erro, se necessário
            val2 = 0;
        }

        bytes[i] = (uint8_t)((val1 << 4) | val2);
    }

    int histograma[256] = {0};
    for (int i = 0; i < t; i++) histograma[bytes[i]]++;

    no *arvore = construir_arvore(histograma, 256);
    if (!arvore) {
        free(bytes);
        return NULL;
    }

    // Buffer único para todos os códigos
    char *codigos = (char*)calloc(256 * 256, sizeof(char));
    char *T[256];
    for (int i = 0; i < 256; i++) T[i] = &codigos[i * 256];

    tabela_codigos_iter(arvore, T);

    // Cálculo de tamanho
    size_t total_len = 0;
    for (int i = 0; i < t; i++) total_len += strlen(T[bytes[i]]);

    char *saida = (char*)malloc(total_len + 1);
    char *ptr = saida;

    for (int i = 0; i < t; i++) {
        char *code = T[bytes[i]];
        size_t len = strlen(code);
        memcpy(ptr, code, len);
        ptr += len;
    }
    *ptr = '\0';

    free(bytes);
    liberar_arvore(arvore);
    free(codigos);
    return saida;
}


const char hex_table[] = "0123456789ABCDEF";

void bin_to_hex(char **bin) {
    char *binary = *bin;
    size_t len = strlen(binary);
    if (len == 0) {
        free(*bin);
        *bin = strdup("0");
        return;
    }

    // Alinhamento para múltiplo de 8 bits (1 byte)
    size_t new_len = (len + 7) / 8 * 8;
    size_t padding = new_len - len;
    
    char *padded = (char*)malloc(new_len + 1);
    // Padding à DIREITA
    memcpy(padded, binary, len);
    memset(padded + len, '0', padding);
    padded[new_len] = '\0';

    // Cada byte (8 bits) gera 2 caracteres hexadecimais
    char *hex = (char*)malloc(new_len / 4 + 1);
    for (size_t i = 0; i < new_len; i += 8) {
        int val1 = ((padded[i] - '0') << 3) |
                   ((padded[i+1] - '0') << 2) |
                   ((padded[i+2] - '0') << 1) |
                   (padded[i+3] - '0');
        int val2 = ((padded[i+4] - '0') << 3) |
                   ((padded[i+5] - '0') << 2) |
                   ((padded[i+6] - '0') << 1) |
                   (padded[i+7] - '0');
        hex[i/4] = hex_table[val1];
        hex[i/4 + 1] = hex_table[val2];
    }
    hex[new_len/4] = '\0';

    free(padded);
    free(*bin);
    *bin = hex;
}

void corrige_HUF(char **HUF) {
    if (strlen(*HUF) == 0) {
        free(*HUF);
        *HUF = strdup("0");
    } else {
        bin_to_hex(HUF);
    }
}

char* runLengthEncongind(char **strings, int t) {
    if (t == 0) return strdup("");

    char *saida = (char*)malloc(t * 4 + 1);
    int index = 0;
    int count = 1;
    char *current = strings[0];

    for (int i = 1; i <= t; i++) {
        if (i == t || strcmp(strings[i], current) != 0 || count == 255) {
            // Conversão direta do count para hex
            saida[index++] = hex_table[(count >> 4) & 0xF];
            saida[index++] = hex_table[count & 0xF];
            saida[index++] = current[0];
            saida[index++] = current[1];
            
            if (i < t) current = strings[i];
            count = 1;
        } else {
            count++;
        }
    }
    saida[index] = '\0';
    return saida;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(input);
        return 1;
    }

    int quantidade_sequencias;
    if (fscanf(input, "%d", &quantidade_sequencias) != 1) {
        fprintf(stderr, "Erro ao ler a quantidade de sequências\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    for (int i = 0; i < quantidade_sequencias; i++) {
        sequencia Sequencia;
        if (fscanf(input, "%d", &Sequencia.tamanho) != 1) {
            fprintf(stderr, "Erro ao ler o tamanho da sequência %d\n", i);
            fclose(input);
            fclose(output);
            return 1;
        }

        Sequencia.conteudo = (char**)malloc(Sequencia.tamanho * sizeof(char*));
        if (!Sequencia.conteudo) {
            perror("Erro ao alocar memória para o conteúdo da sequência");
            fclose(input);
            fclose(output);
            return 1;
        }

        for (int j = 0; j < Sequencia.tamanho; j++) {
            Sequencia.conteudo[j] = (char*)malloc(3 * sizeof(char));
            if (!Sequencia.conteudo[j]) {
                perror("Erro ao alocar memória para o conteúdo da sequência");
                for (int k = 0; k < j; k++) {
                    free(Sequencia.conteudo[k]);
                }
                free(Sequencia.conteudo);
                fclose(input);
                fclose(output);
                return 1;
            }
            if (fscanf(input, "%2s", Sequencia.conteudo[j]) != 1) {
                fprintf(stderr, "Erro ao ler o conteúdo da sequência %d\n", i);
                for (int k = 0; k <= j; k++) {
                    free(Sequencia.conteudo[k]);
                }
                free(Sequencia.conteudo);
                fclose(input);
                fclose(output);
                return 1;
            }
        }

        Sequencia.conteudo_comprimido_HUFF = HUF(Sequencia.conteudo, Sequencia.tamanho);
        if (!Sequencia.conteudo_comprimido_HUFF) {
            fprintf(stderr, "Erro ao comprimir usando HUF\n");
            for (int j = 0; j < Sequencia.tamanho; j++) {
                free(Sequencia.conteudo[j]);
            }
            free(Sequencia.conteudo);
            fclose(input);
            fclose(output);
            return 1;
        }
        corrige_HUF(&Sequencia.conteudo_comprimido_HUFF);

        Sequencia.conteudo_comprimido_RLE = runLengthEncongind(Sequencia.conteudo, Sequencia.tamanho);
        if (!Sequencia.conteudo_comprimido_RLE) {
            fprintf(stderr, "Erro ao comprimir usando RLE\n");
            for (int j = 0; j < Sequencia.tamanho; j++) {
                free(Sequencia.conteudo[j]);
            }
            free(Sequencia.conteudo);
            free(Sequencia.conteudo_comprimido_HUFF);
            fclose(input);
            fclose(output);
            return 1;
        }

        Sequencia.percentual_HUFF = ((float)strlen(Sequencia.conteudo_comprimido_HUFF) / (float) (2 * Sequencia.tamanho)) * 100;
        Sequencia.percentual_RLE = ((float)strlen(Sequencia.conteudo_comprimido_RLE) / (float) (2 * Sequencia.tamanho)) * 100;

        if (Sequencia.percentual_HUFF < Sequencia.percentual_RLE) {
            fprintf(output, "%d->HUF(%.2f%%)=%s\n", i, Sequencia.percentual_HUFF, Sequencia.conteudo_comprimido_HUFF);
        } else if (Sequencia.percentual_HUFF > Sequencia.percentual_RLE) {
            fprintf(output, "%d->RLE(%.2f%%)=%s\n", i, Sequencia.percentual_RLE, Sequencia.conteudo_comprimido_RLE);
        } else {
            fprintf(output, "%d->HUF(%.2f%%)=%s\n", i, Sequencia.percentual_HUFF, Sequencia.conteudo_comprimido_HUFF);
            fprintf(output, "%d->RLE(%.2f%%)=%s\n", i, Sequencia.percentual_RLE, Sequencia.conteudo_comprimido_RLE);
        }

        for (int j = 0; j < Sequencia.tamanho; j++) {
            free(Sequencia.conteudo[j]);
        }
        free(Sequencia.conteudo);
        free(Sequencia.conteudo_comprimido_HUFF);
        free(Sequencia.conteudo_comprimido_RLE);
    }

    fclose(input);
    fclose(output);
    return 0;
}