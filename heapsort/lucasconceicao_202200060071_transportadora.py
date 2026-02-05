import sys
import time

class Pacote:
    __slots__ = ['prioridade', 'tamanho', 'conteudo_bytes']
    
    def __init__(self, prioridade, tamanho, conteudo_bytes):
        self.prioridade = prioridade
        self.tamanho = tamanho
        self.conteudo_bytes = conteudo_bytes
    def __repr__(self):
        return f"P(p={self.prioridade}, t={self.tamanho}, conteudo_bytes={self.conteudo_bytes})"

def string_to_int(s):
    result = 0
    for char in s:
        result = result * 10 + (ord(char) - 48)
    return result


def heapify_iterativo(arr, n, i):
    """Versão iterativa do heapify - mais eficiente"""
    while True:
        maior = i
        esquerda = 2 * i + 1
        direita = 2 * i + 2

        if esquerda < n and arr[esquerda].prioridade > arr[maior].prioridade:
            maior = esquerda

        if direita < n and arr[direita].prioridade > arr[maior].prioridade:
            maior = direita

        if maior == i:
            break
            
        arr[i], arr[maior] = arr[maior], arr[i]
        i = maior

def heapsort(arr):
    n = len(arr)
    
    # Constrói max-heap
    for i in range(n // 2 - 1, -1, -1):
        heapify_iterativo(arr, n, i)
    
    # Extrai elementos
    for i in range(n - 1, 0, -1):
        arr[0], arr[i] = arr[i], arr[0]
        heapify_iterativo(arr, i, 0)
    
    arr.reverse()
def formatar_e_escrever_saida(lista_pacotes, nome_arquivo):
    """
    Formata o conteúdo dos pacotes, unindo os bytes com vírgulas,
    separando os pacotes com '|' e envolvendo-os em '|'. Escreve o resultado em um arquivo.
    """
    # 1. Para cada pacote, junta sua lista de bytes em uma única string com vírgulas.
    # Ex: ['06', '07', '08'] se torna "06,07,08"
    conteudos_individuais = [','.join(p.conteudo_bytes) for p in lista_pacotes]

    # 2. Junta as strings de conteúdo, separando-as por "|" e envolvendo o resultado em "|".
    # Ex: ["06,07,08", "01,02"] se torna "|06,07,08|01,02|"
    linha_final = '|' + '|'.join(conteudos_individuais) + '|'

    # 3. Abre o arquivo de saída em modo 'append' e escreve a linha final.
    with open(nome_arquivo, "a") as f_saida:
        f_saida.write(linha_final + '\n')

def main(args):
    start_time = time.time()
    
    with open(args[1], "r") as f:
        linhas = f.readlines()

    # Define o nome do arquivo de saída e o limpa antes de começar
    arquivo_saida = args[2]
    with open(arquivo_saida, "w") as f:
        pass # Garante que o arquivo esteja vazio no início da execução

    # Parse do cabeçalho
    valores = linhas[0].split()
    TAMANHO_DO_BUFFER = string_to_int(valores[1])
    tam_buffer_atual = 0
    lista_de_pacotes = []
    
    # Ler arquivo e ir adicionando os bytes no buffer
    for linha in linhas[1:]:
        partes = linha.split()
        prioridade = string_to_int(partes[0])
        tamanho = string_to_int(partes[1])
        bytes_conteudo = partes[2:]
        pacote = Pacote(prioridade, tamanho, bytes_conteudo)

        if tam_buffer_atual + pacote.tamanho <= TAMANHO_DO_BUFFER:
            lista_de_pacotes.append(pacote)
            tam_buffer_atual += pacote.tamanho
        else:
            heapsort(lista_de_pacotes)
            formatar_e_escrever_saida(lista_de_pacotes, arquivo_saida)
            
            # Reinicia o buffer com o pacote atual
            tam_buffer_atual = pacote.tamanho
            lista_de_pacotes = [pacote]

    # Processa os pacotes restantes no buffer
    if lista_de_pacotes:
        heapsort(lista_de_pacotes)
        formatar_e_escrever_saida(lista_de_pacotes, arquivo_saida)
        
   
if __name__ == "__main__":
    main(sys.argv)