#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define BUF_IN_SZ (1 << 20)
#define HEX2BUF(c1, c2) ((hex_byte[(unsigned char)(c1)] << 4) | hex_byte[(unsigned char)(c2)])

/* Tabela estática hex -> nibble (evita branches no hot path) */
static unsigned char hex_byte[256];

static void init_hex_table(void) {
    static int inited;
    if (inited) return;
    inited = 1;
    for (int i = '0'; i <= '9'; i++) hex_byte[i] = i - '0';
    for (int i = 'A'; i <= 'F'; i++) hex_byte[i] = i - 'A' + 10;
    for (int i = 'a'; i <= 'f'; i++) hex_byte[i] = i - 'a' + 10;
}

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
    fpm->cap = 512;
    fpm->V = (no**)malloc((fpm->cap + 1) * sizeof(no*));
    return fpm;
}

/* Heapify (descer): garante que a subárvore em i satisfaz a propriedade de heap mínimo. O(N) no build-heap. */
static void heapify(fila_p_min *fpm, int i) {
    while (2 * i <= fpm->tam) {
        int m = 2 * i;
        if (2 * i + 1 <= fpm->tam && fpm->V[2 * i + 1]->freq < fpm->V[2 * i]->freq)
            m = 2 * i + 1;
        if (fpm->V[i]->freq <= fpm->V[m]->freq) break;
        no *aux = fpm->V[i];
        fpm->V[i] = fpm->V[m];
        fpm->V[m] = aux;
        i = m;
    }
}

/* Build-heap (heapify reverso): constrói o heap em tempo O(N) de baixo para cima. */
static void build_heap(fila_p_min *fpm) {
    for (int i = fpm->tam / 2; i >= 1; i--)
        heapify(fpm, i);
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
    heapify(fpm, 1);
    return min;
}

no *construir_arvore(int H[], int n) {
    fila_p_min *fpm = criar_fila_p_min();
    /* Inserir todos os nós no vetor de uma vez (símbolos com frequência > 0). */
    for (int i = 0; i < n; i++) {
        if (H[i] > 0) {
            if (fpm->tam + 1 >= fpm->cap) {
                fpm->cap *= 2;
                fpm->V = (no**)realloc(fpm->V, (fpm->cap + 1) * sizeof(no*));
            }
            no *novo = (no*)malloc(sizeof(no));
            novo->freq = H[i];
            novo->S = (char)i;
            novo->E = NULL;
            novo->D = NULL;
            fpm->V[++fpm->tam] = novo;
        }
    }
    if (fpm->tam == 0) {
        free(fpm->V);
        free(fpm);
        return NULL;
    }
    /* Construção do heap em tempo linear (heapify reverso / build-heap). */
    build_heap(fpm);
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

void tabela_codigos_iter(no *raiz, char *T[], int *T_len) {
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
            int idx = (unsigned char)node->S;
            if (current.depth == 0) {
                T[idx][0] = '0';
                T[idx][1] = '\0';
                T_len[idx] = 1;
            } else {
                memcpy(T[idx], current.path, current.depth);
                T[idx][current.depth] = '\0';
                T_len[idx] = current.depth;
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


char* HUF(char **strings, int t) {
    init_hex_table();
    uint8_t *bytes = (uint8_t*)malloc((size_t)t * sizeof(uint8_t));
    for (int i = 0; i < t; i++)
        bytes[i] = HEX2BUF(strings[i][0], strings[i][1]);

    int histograma[256] = {0};
    for (int i = 0; i < t; i++) histograma[bytes[i]]++;

    no *arvore = construir_arvore(histograma, 256);
    if (!arvore) {
        free(bytes);
        return NULL;
    }

    char *codigos = (char*)calloc(256 * 256, sizeof(char));
    int T_len[256];
    memset(T_len, 0, sizeof(T_len));
    char *T[256];
    for (int i = 0; i < 256; i++) T[i] = &codigos[i * 256];

    tabela_codigos_iter(arvore, T, T_len);

    size_t total_len = 0;
    for (int i = 0; i < t; i++) total_len += (size_t)T_len[bytes[i]];

    char *saida = (char*)malloc(total_len + 1);
    char *ptr = saida;
    for (int i = 0; i < t; i++) {
        int len = T_len[bytes[i]];
        memcpy(ptr, T[bytes[i]], (size_t)len);
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

    char *saida = (char*)malloc((size_t)t * 4 + 1);
    int index = 0;
    int count = 1;
    char c0 = strings[0][0], c1 = strings[0][1];

    for (int i = 1; i <= t; i++) {
        if (i == t || (strings[i][0] != c0 || strings[i][1] != c1) || count == 255) {
            saida[index++] = hex_table[(count >> 4) & 0xF];
            saida[index++] = hex_table[count & 0xF];
            saida[index++] = c0;
            saida[index++] = c1;
            if (i < t) {
                c0 = strings[i][0];
                c1 = strings[i][1];
            }
            count = 1;
        } else {
            count++;
        }
    }
    saida[index] = '\0';
    return saida;
}

/* Parser de entrada: lê inteiro e avança p */
static int parse_int(char **p) {
    char *s = *p;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    int v = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    *p = s;
    return v;
}

/* Avança p até passar de 2 chars hex e copia esses 2 chars para dest. */
static void parse_hex2_preserve(char **p, char *dest) {
    char *s = *p;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    dest[0] = s[0];
    dest[1] = s[1];
    dest[2] = '\0';
    *p = s + 2;
}

int main(int argc, char* argv[]) {
    clock_t inicio, fim;
    double tempo_gasto;
    inicio = clock();
    if (argc < 3) return 1;

    FILE* input = fopen(argv[1], "r");
    if (!input) return 1;

    fseek(input, 0, SEEK_END);
    long fsize = ftell(input);
    fseek(input, 0, SEEK_SET);
    if (fsize <= 0 || fsize > (long)(50 << 20)) { fclose(input); return 1; }
    size_t size = (size_t)fsize;
    char *buf = (char*)malloc(size + 1);
    if (!buf) { fclose(input); return 1; }
    size_t nread = fread(buf, 1, size, input);
    fclose(input);
    buf[nread] = '\0';

    init_hex_table();

    FILE* output = fopen(argv[2], "w");
    if (!output) { free(buf); return 1; }
    setvbuf(output, NULL, _IOFBF, 256 * 1024);

    char *p = buf;
    int quantidade_sequencias = parse_int(&p);

    for (int i = 0; i < quantidade_sequencias; i++) {
        sequencia Sequencia;
        Sequencia.tamanho = parse_int(&p);

        /* Buffer único: todos os tokens em um bloco contíguo */
        char *block = (char*)malloc((size_t)Sequencia.tamanho * 3);
        if (!block) { free(buf); fclose(output); return 1; }
        Sequencia.conteudo = (char**)malloc((size_t)Sequencia.tamanho * sizeof(char*));
        if (!Sequencia.conteudo) { free(block); free(buf); fclose(output); return 1; }
        for (int j = 0; j < Sequencia.tamanho; j++) {
            Sequencia.conteudo[j] = block + j * 3;
            parse_hex2_preserve(&p, Sequencia.conteudo[j]);
        }

        Sequencia.conteudo_comprimido_HUFF = HUF(Sequencia.conteudo, Sequencia.tamanho);
        corrige_HUF(&Sequencia.conteudo_comprimido_HUFF);
        Sequencia.conteudo_comprimido_RLE = runLengthEncongind(Sequencia.conteudo, Sequencia.tamanho);

        size_t len_h = strlen(Sequencia.conteudo_comprimido_HUFF);
        size_t len_r = strlen(Sequencia.conteudo_comprimido_RLE);
        int den = 2 * Sequencia.tamanho;
        Sequencia.percentual_HUFF = (float)len_h * 100.f / (float)den;
        Sequencia.percentual_RLE  = (float)len_r * 100.f / (float)den;

        /* Gabarito não tem newline após a última linha; imprimir \n antes de cada linha exceto a primeira. */
        int first_line = (i == 0);
        if (Sequencia.percentual_HUFF < Sequencia.percentual_RLE) {
            if (!first_line) fputc('\n', output);
            fprintf(output, "%d->HUF(%.2f%%)=%s", i, Sequencia.percentual_HUFF, Sequencia.conteudo_comprimido_HUFF);
            first_line = 0;
        } else if (Sequencia.percentual_HUFF > Sequencia.percentual_RLE) {
            if (!first_line) fputc('\n', output);
            fprintf(output, "%d->RLE(%.2f%%)=%s", i, Sequencia.percentual_RLE, Sequencia.conteudo_comprimido_RLE);
            first_line = 0;
        } else {
            if (!first_line) fputc('\n', output);
            fprintf(output, "%d->HUF(%.2f%%)=%s\n%d->RLE(%.2f%%)=%s", i, Sequencia.percentual_HUFF, Sequencia.conteudo_comprimido_HUFF, i, Sequencia.percentual_RLE, Sequencia.conteudo_comprimido_RLE);
            first_line = 0;
        }

        free(block);
        free(Sequencia.conteudo);
        free(Sequencia.conteudo_comprimido_HUFF);
        free(Sequencia.conteudo_comprimido_RLE);
    }

    free(buf);
    fclose(output);
    fim = clock();
    tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %f segundos\n", tempo_gasto);
    printf("Saída escrita em: %s\n", argv[2]);
    return 0;
}