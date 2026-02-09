#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TAM_PLACA   8
#define TAM_CODIGO 14

typedef struct {
    char id[TAM_PLACA];
    int32_t peso_max;
    int32_t vol_max;
} Veiculo;

typedef struct {
    char cod[TAM_CODIGO];
    float val;
    int32_t peso;
    int32_t vol;
    int ja_utilizado;
    int indice_original;
} Item;

static inline float maior_entre(float x, float y) {
    return x >= y ? x : y;
}

static void montar_matriz(float ***m, Veiculo v, Item *itens, int32_t num_itens) {
    int32_t vol_lim = v.vol_max;
    int32_t peso_lim = v.peso_max;

    for (int idx = 0; idx <= num_itens; idx++) {
        for (int j = 0; j <= vol_lim; j++) {
            for (int k = 0; k <= peso_lim; k++) {
                if (idx == 0 || j == 0 || k == 0) {
                    m[idx][j][k] = 0.0f;
                    continue;
                }
                Item *cur = &itens[idx - 1];
                if (cur->vol > j || cur->peso > k || cur->ja_utilizado) {
                    m[idx][j][k] = m[idx - 1][j][k];
                    continue;
                }
                float com_item = cur->val + m[idx - 1][j - cur->vol][k - cur->peso];
                float sem_item = m[idx - 1][j][k];
                m[idx][j][k] = maior_entre(com_item, sem_item);
            }
        }
    }
}

static void recuperar_escolhas(float ***m, Veiculo v, Item *itens_aux, int32_t n,
                              FILE *out, int32_t *qtd, Item *itens_orig) {
    int peso_acum = 0, vol_acum = 0;
    char *codigos_escolhidos[n];
    int num_escolhidos = 0;

    int idx = n;
    int j = v.vol_max;
    int k = v.peso_max;

    while (idx > 0 && j > 0 && k > 0) {
        if (m[idx][j][k] != m[idx - 1][j][k]) {
            Item *cur = &itens_aux[idx - 1];
            codigos_escolhidos[num_escolhidos++] = cur->cod;
            peso_acum += cur->peso;
            vol_acum += cur->vol;
            itens_orig[cur->indice_original].ja_utilizado = 1;
            j -= cur->vol;
            k -= cur->peso;
            (*qtd)++;
        }
        idx--;
    }

    float valor_final = m[n][v.vol_max][v.peso_max];
    float pct_peso = (float)peso_acum / (float)v.peso_max * 100.0f;
    float pct_vol = (float)vol_acum / (float)v.vol_max * 100.0f;

    fprintf(out, "[%s]R$%.2f,%dKG(%.0f%%),%dL(%.0f%%)->",
            v.id, valor_final, peso_acum, pct_peso, vol_acum, pct_vol);

    for (int x = num_escolhidos - 1; x >= 0; x--) {
        fprintf(out, "%s%s", codigos_escolhidos[x], x > 0 ? "," : "\n");
    }
}

static int32_t achar_maior_peso(Veiculo *veiculos, int num_veiculos) {
    int32_t m = 0;
    for (int i = 0; i < num_veiculos; i++)
        if (veiculos[i].peso_max > m) m = veiculos[i].peso_max;
    return m;
}

static int32_t achar_maior_volume(Veiculo *veiculos, int num_veiculos) {
    int32_t m = 0;
    for (int i = 0; i < num_veiculos; i++)
        if (veiculos[i].vol_max > m) m = veiculos[i].vol_max;
    return m;
}

static float ***alocar_matriz(int linhas, int cols_vol, int cols_peso) {
    float ***m = (float ***)malloc((size_t)(linhas + 1) * sizeof(float **));
    for (int i = 0; i <= linhas; i++) {
        m[i] = (float **)malloc((size_t)(cols_vol + 1) * sizeof(float *));
        for (int j = 0; j <= cols_vol; j++)
            m[i][j] = (float *)malloc((size_t)(cols_peso + 1) * sizeof(float));
    }
    return m;
}

static void liberar_matriz(float ***m, int linhas, int cols_vol) {
    for (int i = 0; i <= linhas; i++) {
        for (int j = 0; j <= cols_vol; j++)
            free(m[i][j]);
        free(m[i]);
    }
    free(m);
}

static void otimizar_carregamento(Veiculo *veiculos, Item *itens, int num_veiculos,
                                  int num_itens, FILE *out) {
    int32_t peso_max = achar_maior_peso(veiculos, num_veiculos);
    int32_t vol_max = achar_maior_volume(veiculos, num_veiculos);

    float ***mat = alocar_matriz(num_itens, vol_max, peso_max);
    int32_t qtd_total = 0;

    for (int v = 0; v < num_veiculos; v++) {
        int n_aux = 0;
        for (int i = 0; i < num_itens; i++)
            if (!itens[i].ja_utilizado) n_aux++;

        Item *aux = (Item *)malloc((size_t)n_aux * sizeof(Item));
        int pos = 0;
        for (int i = 0; i < num_itens; i++) {
            if (!itens[i].ja_utilizado)
                aux[pos++] = itens[i];
        }

        montar_matriz(mat, veiculos[v], aux, n_aux);
        recuperar_escolhas(mat, veiculos[v], aux, n_aux, out, &qtd_total, itens);
        free(aux);
    }

    liberar_matriz(mat, num_itens, vol_max);
}

static void escrever_pendentes(Item *itens, int num_itens, FILE *out) {
    float val_total = 0.0f;
    int32_t peso_total = 0, vol_total = 0;

    for (int i = 0; i < num_itens; i++) {
        if (itens[i].ja_utilizado) continue;
        val_total += itens[i].val;
        peso_total += itens[i].peso;
        vol_total += itens[i].vol;
    }

    fprintf(out, "PENDENTE:R$%.2f,%dKG,%dL->", val_total, peso_total, vol_total);

    int primeiro = 1;
    for (int i = 0; i < num_itens; i++) {
        if (!itens[i].ja_utilizado) {
            if (!primeiro) fputc(',', out);
            fprintf(out, "%s", itens[i].cod);
            primeiro = 0;
        }
    }
    fputc('\n', out);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    FILE *entrada = fopen(argv[1], "r");
    if (!entrada) {
        perror("Erro ao abrir o arquivo de entrada");
        return 1;
    }

    FILE *saida = fopen(argv[2], "w");
    if (!saida) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(entrada);
        return 1;
    }

    int c, p;
    fscanf(entrada, "%d", &c);
    Veiculo *veiculos = (Veiculo *)malloc((size_t)c * sizeof(Veiculo));
    for (int i = 0; i < c; i++)
        fscanf(entrada, "%s %d %d", veiculos[i].id, &veiculos[i].peso_max, &veiculos[i].vol_max);

    fscanf(entrada, "%d", &p);
    Item *itens = (Item *)malloc((size_t)p * sizeof(Item));
    for (int i = 0; i < p; i++) {
        fscanf(entrada, "%s %f %d %d", itens[i].cod, &itens[i].val, &itens[i].peso, &itens[i].vol);
        itens[i].ja_utilizado = 0;
        itens[i].indice_original = i;
    }

    otimizar_carregamento(veiculos, itens, c, p, saida);
    escrever_pendentes(itens, p, saida);

    free(veiculos);
    free(itens);
    fclose(entrada);
    fclose(saida);
    return 0;
}
