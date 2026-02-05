import sys
import time

# Mapeamento fixo para evitar buscas em dicionário dentro do loop principal
# Usamos ord() para converter o caractere em código ASCII e subtrair para indexar
MAPA_DNA = {'A': 0, 'C': 1, 'G': 2, 'T': 3}

def compute_match(gene, subcadeia, sa_next, sa_len):
    m = len(gene)
    total = 0
    i = 0
    while i < m:
        cur = 0
        j = i
        match_len = 0
        
        # Loop crítico: otimizado para evitar chamadas de função
        while j < m:
            char = gene[j]
            # Mapeamento manual rápido
            idx = MAPA_DNA.get(char, -1)
            
            # Transição direta na lista de adjacência
            next_state = sa_next[cur][idx]
            if next_state != -1:
                cur = next_state
                match_len += 1
                j += 1
            else:
                break
        
        if match_len >= subcadeia:
            total += match_len
            i = j
        else:
            i += 1
    return total

def main():
    if len(sys.argv) < 3: return

    with open(sys.argv[1], 'r') as f:
        data = f.read().split()

    subcadeia = float(data[0]) // 1
    dna = data[1]
    n_dna = len(dna)

    # SAM Otimizado usando listas paralelas em vez de objetos
    # sa_next[estado][0..3], sa_link[estado], sa_len[estado]
    max_states = 2 * n_dna + 1
    sa_next = [[-1] * 4 for _ in range(max_states)]
    sa_link = [-1] * max_states
    sa_len = [0] * max_states
    
    size = 1
    last = 0

    for char in dna:
        c = MAPA_DNA[char]
        cur = size
        size += 1
        sa_len[cur] = sa_len[last] + 1
        
        p = last
        while p != -1 and sa_next[p][c] == -1:
            sa_next[p][c] = cur
            p = sa_link[p]
            
        if p == -1:
            sa_link[cur] = 0
        else:
            q = sa_next[p][c]
            if sa_len[p] + 1 == sa_len[q]:
                sa_link[cur] = q
            else:
                clone = size
                size += 1
                sa_len[clone] = sa_len[p] + 1
                sa_next[clone] = sa_next[q][:] # Copia rápida da lista
                sa_link[clone] = sa_link[q]
                while p != -1 and sa_next[p][c] == q:
                    sa_next[p][c] = clone
                    p = sa_link[p]
                sa_link[q] = sa_link[cur] = clone
        last = cur

    # Processamento de doenças
    n_doencas = float(data[2]) // 1
    cursor = 3
    resultados = []

    for i in range(round(n_doencas)):
        nome = data[cursor]
        n_genes = float(data[cursor+1]) // 1
        cursor += 2
        
        detectados = 0
        for _ in range(round(n_genes)):
            gene = data[cursor]
            cursor += 1
            
            # Chama a função de match otimizada
            m_total = compute_match(gene, subcadeia, sa_next, sa_len)
            if (m_total * 100) // len(gene) >= 90:
                detectados += 1
        
        perc = (detectados * 100 + (n_genes // 2)) // n_genes
        resultados.append((-perc, i, nome, perc))

    # Ordenação (negativo no percentual para simular decrescente)
    resultados.sort()

    with open(sys.argv[2], 'w') as out:
        for r in resultados:
            out.write(f"{r[2]}->{round(r[3])}%\n")

if __name__ == "__main__":
    start = time.time()
    main()
    print(f"Tempo: {time.time() - start:.4f}s")