def quicksort(arr):
    # Caso base: arrays com 0 ou 1 elemento já estão ordenados
    if len(arr) <= 1:
        return arr
    
    # Escolha do pivô (aqui usamos o elemento do meio)
    pivo = arr[len(arr) // 2]
    
    # Criação das sub-listas
    esquerda = [x for x in arr if x < pivo]
    meio = [x for x in arr if x == pivo]
    direita = [x for x in arr if x > pivo]
    
    # Chamada recursiva
    return quicksort(esquerda) + meio + quicksort(direita)

# Testando
lista = [10, 7, 8, 9, 1, 5]
ordenada = quicksort(lista)
print(f"Lista Original: {lista}")
print(f"Lista Ordenada: {ordenada}")
