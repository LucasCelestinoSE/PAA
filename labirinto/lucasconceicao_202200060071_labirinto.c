// Compilar com: gcc -Wall -O3 Main.c -o Main
// Uso: ./Main input output

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define TAMANHO 100

typedef struct {
    int x, y;
} Pos;

typedef struct No {
    char cmd[10];
    Pos from, to;
    struct No *next;
} No;

int vis[TAMANHO][TAMANHO];
char grid[TAMANHO][TAMANHO];
int w, h;
Pos start;
Pos end;
No* path = NULL;
int found = 0;

int mx[] = {0, -1, 0, 1};
int my[] = {1, 0, -1, 0};
char* moves[] = {"D", "F", "E", "T"};

void print_path(FILE* out, int id) {
    fprintf(out, "L%d:INI@%d,%d", id, start.x, start.y);
    
    reverse_path();
    
    No* curr = path;
    while (curr != NULL) {
        if (strcmp(curr->cmd, "BT") == 0) {
            fprintf(out, "|BT@%d,%d->%d,%d", curr->from.x, curr->from.y, curr->to.x, curr->to.y);
        } else {
            fprintf(out, "|%s->%d,%d", curr->cmd, curr->to.x, curr->to.y);
        }
        curr = curr->next;
    }
    
    if (found) {
        fprintf(out, "|FIM@%d,%d\n", end.x, end.y);
    } else {
        fprintf(out, "|FIM@-,-\n");
    }
}

void reverse_path() {
    No* prev = NULL;
    No* curr = path;
    No* next = NULL;
    while (curr != NULL) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    path = prev;
}

void add_step(char* cmd, Pos from, Pos to) {
    No* new_node = (No*)malloc(sizeof(No));
    strcpy(new_node->cmd, cmd);
    new_node->from = from;
    new_node->to = to;
    new_node->next = path;
    path = new_node;
}

int is_exit(Pos p) {
    if ((p.x == 0 || p.x == h-1 || p.y == 0 || p.y == w-1) && 
        !(p.x == start.x && p.y == start.y)) {
        return 1;
    }
    return 0;
}

int in_bounds(Pos p) {
    return p.x >= 0 && p.x < h && p.y >= 0 && p.y < w;
}

int search(Pos curr) {
    vis[curr.x][curr.y] = 1;
    
    if (is_exit(curr)) {
        end = curr;
        found = 1;
        return 1;
    }
    
    for (int i = 0; i < 4; i++) {
        Pos next = {curr.x + mx[i], curr.y + my[i]};
        
        if (in_bounds(next) && !vis[next.x][next.y] && grid[next.x][next.y] == 0) {
            add_step(moves[i], curr, next);
            if (search(next)) {
                return 1;
            }
            add_step("BT", next, curr);
        }
    } return 0;
}

void free_path() {
    while (path != NULL) {
        No* tmp = path;
        path = path->next;
        free(tmp);
    }
}

void solve(FILE* in, FILE* out, int id) {
    fscanf(in, "%d %d", &w, &h);
    
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            char c;
            fscanf(in, " %c", &c);
            if (c == '1') {
                grid[i][j] = 1;
            } else if (c == '0') {
                grid[i][j] = 0;
            } else if (c == 'X') {
                grid[i][j] = 0;
                start.x = i;
                start.y = j;
            }
            vis[i][j] = 0;
        }
    }
    
    search(start);
    print_path(out, id);
    free_path();
    found = 0;
}

int main(int argc, char* argv[]) {
    clock_t tempo_total = clock();
    
    FILE* entrada = fopen(argv[1], "r");
    FILE* saida = fopen(argv[2], "w");
    
    int n;
    fscanf(entrada, "%d", &n);
    
    for (int j = 0;j < n; j++) {
        solve(entrada, saida, j);
    }
    
    fclose(entrada);
    fclose(saida);
    
    tempo_total = clock() - tempo_total;
    printf("Tempo de execução total em segundos: %lf\n", ((double)tempo_total) / CLOCKS_PER_SEC);
    return 0;
}