import sys
import time
from math import fabs, floor 

TAMANHO_TABELA_HASH = 1024

class Container:
    def __init__(self, codigo, cnpj, peso, indice = -1, proximo=None):
        self.codigo = codigo
        self.cnpj = cnpj
        self.peso = peso 
        self.indice = indice
        self.proximo = proximo

hash_table = {}

def hash_function(codigo):
    hash_value = 0
    limit = min(len(codigo), 11) 
    for i in range(limit):
        char_val = ord(codigo[i])
        hash_value += char_val
        hash_value = (hash_value * char_val) % TAMANHO_TABELA_HASH
    return hash_value

def insert(container):
    if container is None:
        return False
    hash_table[container.codigo] = container
    return True

def search(codigo):
    return hash_table.get(codigo)

def get_diferenca_percentual(peso_c1, peso_c2):
    if peso_c1 == 0:
        return 100
    
    diff = (fabs(peso_c1 - peso_c2) / peso_c1) * 100
    return round(diff) 

def excede_limite_peso(peso_c1, peso_c2):
    return get_diferenca_percentual(peso_c1, peso_c2) > 10

def insertion_sort_por_indice(containers):
    n = len(containers)
    for i in range(1, n):
        chave = containers[i]
        j = i - 1
        while j >= 0 and containers[j].indice > chave.indice:
            containers[j + 1] = containers[j]
            j -= 1
        containers[j + 1] = chave

def intercalar_containers_por_peso(containers, auxiliar, inicio, meio, fim):
    indice_esquerdo = inicio
    indice_direito = meio + 1
    indice_auxiliar = inicio

    while indice_esquerdo <= meio and indice_direito <= fim:
        container_esquerdo = containers[indice_esquerdo]
        container_direito = containers[indice_direito]
        
        container_original_esquerdo = search(container_esquerdo.codigo)
        container_original_direito = search(container_direito.codigo)

        peso_original_esq = container_original_esquerdo.peso if container_original_esquerdo else 0.0
        peso_original_dir = container_original_direito.peso if container_original_direito else 0.0
        c_orig_esq_idx = container_original_esquerdo.indice if container_original_esquerdo else float('inf')
        c_orig_dir_idx = container_original_direito.indice if container_original_direito else float('inf')
        
        diferenca_esquerda = get_diferenca_percentual(peso_original_esq, container_esquerdo.peso)
        diferenca_direita = get_diferenca_percentual(peso_original_dir, container_direito.peso)
        
        if diferenca_esquerda > diferenca_direita or \
           (diferenca_esquerda == diferenca_direita and c_orig_esq_idx < c_orig_dir_idx):
            auxiliar[indice_auxiliar] = containers[indice_esquerdo]
            indice_esquerdo += 1
        else:
            auxiliar[indice_auxiliar] = containers[indice_direito]
            indice_direito += 1
        indice_auxiliar += 1

    while indice_esquerdo <= meio:
        auxiliar[indice_auxiliar] = containers[indice_esquerdo]
        indice_esquerdo += 1
        indice_auxiliar += 1

    while indice_direito <= fim:
        auxiliar[indice_auxiliar] = containers[indice_direito]
        indice_direito += 1
        indice_auxiliar += 1

    for i in range(inicio, fim + 1):
        containers[i] = auxiliar[i]

def mergesort(containers, auxiliar, inicio, fim):
    if inicio < fim:
        meio = inicio + (fim - inicio) // 2
        
        mergesort(containers, auxiliar, inicio, meio)
        mergesort(containers, auxiliar, meio + 1, fim)
        intercalar_containers_por_peso(containers, auxiliar, inicio, meio, fim)

def main():
    if len(sys.argv) != 3:
        print("Uso: python3 nome_do_programa.py <arquivo_entrada> <arquivo_saida>")
        return

    input_filename = sys.argv[1]
    output_filename = sys.argv[2]
    
    start_time = time.time()

    try:
        with open(input_filename, 'r') as f_input:
            lines = [line.strip() for line in f_input if line.strip()]
    except FileNotFoundError:
        return
    
    line_iterator = iter(lines)
    
    try:
        n_containers_cadastrados_float = float(next(line_iterator))
        n_containers_cadastrados = floor(n_containers_cadastrados_float)
    except (StopIteration, ValueError):
        return
        
    for i in range(n_containers_cadastrados):
        try:
            parts = next(line_iterator).split()
            codigo = parts[0]
            cnpj = parts[1]
            peso = float(parts[2]) 
            
            container_cadastrado = Container(codigo, cnpj, peso, i)
            insert(container_cadastrado)
        except (StopIteration, IndexError, ValueError):
            return
            
    try:
        n_containers_selecionados_float = float(next(line_iterator))
        n_containers_selecionados = floor(n_containers_selecionados_float)
    except (StopIteration, ValueError):
        return
        
    containers_cnpj_errado = []
    containers_peso_errado = []

    for i in range(n_containers_selecionados):
        try:
            parts = next(line_iterator).split()
            container_selecionado = Container(parts[0], parts[1], float(parts[2]))
        except (StopIteration, IndexError, ValueError):
            continue

        container_relacionado = search(container_selecionado.codigo)
        
        if container_relacionado is not None:
            if container_selecionado.cnpj != container_relacionado.cnpj:
                container_selecionado.indice = container_relacionado.indice 
                containers_cnpj_errado.append(container_selecionado)
                continue 
            
            if excede_limite_peso(container_relacionado.peso, container_selecionado.peso):
                containers_peso_errado.append(container_selecionado)

    output_lines = []

    insertion_sort_por_indice(containers_cnpj_errado)

    for c_errado in containers_cnpj_errado:
        c_original = search(c_errado.codigo)
        if c_original:
            output_lines.append(f"{c_errado.codigo}:{c_original.cnpj}<->{c_errado.cnpj}")
            
    if containers_peso_errado:
        n_peso_errado = len(containers_peso_errado)
        auxiliar_ordenacao = [Container("", "", 0.0) for _ in range(n_peso_errado)]
        
        mergesort(containers_peso_errado, auxiliar_ordenacao, 0, n_peso_errado - 1)

    for c_errado in containers_peso_errado:
        c_original = search(c_errado.codigo)
        if c_original:
            diferenca_absoluta = round(abs(c_original.peso - c_errado.peso))
            diferenca_percentual = get_diferenca_percentual(c_original.peso, c_errado.peso)

            output_lines.append(f"{c_original.codigo}:{diferenca_absoluta}kg({diferenca_percentual}%)")

    try:
        with open(output_filename, 'w') as f_output:
            f_output.write('\n'.join(output_lines) + '\n')
    except IOError:
        return

    end_time = time.time()
    
    print(f"Tempo de execucao total em segundos: {end_time - start_time:.6f}")


if __name__ == "__main__":
    main()