import difflib
import itertools
import os

def comparar_arquivos_texto(caminho_arquivo1, caminho_arquivo2):
    """
    Compara dois arquivos de texto linha por linha e imprime as diferenças.

    Args:
        caminho_arquivo1 (str): O caminho para o primeiro arquivo.
        caminho_arquivo2 (str): O caminho para o segundo arquivo.
    """
    try:
        # 1. Ler o conteúdo dos arquivos
        with open(caminho_arquivo1, 'r', encoding='utf-8') as arq1:
            linhas1 = arq1.readlines()
        with open(caminho_arquivo2, 'r', encoding='utf-8') as arq2:
            linhas2 = arq2.readlines()

    except FileNotFoundError:
        print("Erro: Um ou ambos os arquivos não foram encontrados.")
        return

    # 2. Inicializar o objeto Differ
    d = difflib.Differ()
    
    # 3. Gerar as diferenças
    # O método compare() retorna um iterador de strings de diferença.
    diff = d.compare(linhas1, linhas2)

    print(f"--- Comparando {os.path.basename(caminho_arquivo1)} e {os.path.basename(caminho_arquivo2)} ---")
    
    # Variáveis para rastrear os números das linhas nos arquivos originais
    num_linha1 = 0
    num_linha2 = 0
    
    # 4. Iterar sobre as diferenças e exibir os resultados
    for i, linha_diff in enumerate(diff):
        # A linha de diferença começa com um código de um caractere:
        # ' ' : linhas idênticas
        # '-' : linha presente apenas no primeiro arquivo (excluída)
        # '+' : linha presente apenas no segundo arquivo (adicionada)
        # '?' : linha que indica diferenças internas (modificada)
        
        codigo = linha_diff[0]
        texto = linha_diff[2:].rstrip('\n') # Remover o código e a quebra de linha

        if codigo == ' ':
            # Linha idêntica - avança o contador de ambos
            num_linha1 += 1
            num_linha2 += 1
        
        elif codigo == '-':
            # Linha presente apenas no primeiro arquivo (excluída)
            print(f"\n❌ Linha {num_linha1 + 1} (Arquivo 1) EXCLUÍDA:\n< {texto}")
            num_linha1 += 1
            
        elif codigo == '+':
            # Linha presente apenas no segundo arquivo (adicionada)
            print(f"\n➕ Linha {num_linha2 + 1} (Arquivo 2) ADICIONADA:\n> {texto}")
            num_linha2 += 1
            
        elif codigo == '?':
            # Linha de pista para diferenças intra-linha (começa sempre após '-' ou '+')
            # Não precisamos da linha '?' em si, pois o difflib a usa internamente
            continue 
            
        else: # Linha de contexto (que precede '?' - que é sempre '-' ou '+')
            if linhas_diff[i-1][0] == '-':
                 # A linha anterior foi uma exclusão, então a atual é a linha de contexto
                 pass
            elif linhas_diff[i-1][0] == '+':
                # A linha anterior foi uma adição, então a atual é a linha de contexto
                pass
            
            # Se for uma linha modificada, o difflib mostra:
            # - Linha N do arquivo 1 (com '-')
            # - Linha N do arquivo 2 (com '+')
            # - Linha de dica (com '?')

            # O bloco abaixo tenta identificar onde a MODIFICAÇÃO acontece
            if i > 0 and linha_diff[0] in ('-', '+'):
                linha_anterior = list(itertools.islice(d.compare(linhas1, linhas2), i - 1, i))[0]
                if linha_anterior[0] in ('-', '+'):
                    # Estamos em um bloco de modificação:
                    
                    # Vamos encontrar o ponto exato da diferença (coluna) se for uma modificação
                    if codigo == '-': # Linha do Arquivo 1 (a ser subtraída)
                        # A linha '?' virá em seguida com a posição do caractere
                        pass 
                    elif codigo == '+': # Linha do Arquivo 2 (a ser adicionada)
                        # A linha '?' virá imediatamente após (se já não tiver vindo)
                        if (i+1 < len(list(itertools.islice(d.compare(linhas1, linhas2), 0, None)))) and list(itertools.islice(d.compare(linhas1, linhas2), i + 1, i+2))[0][0] == '?':
                            # Se a próxima linha for a pista
                            pista = list(itertools.islice(d.compare(linhas1, linhas2), i + 1, i+2))[0][2:].rstrip('\n')
                            coluna_diff = pista.find('^') # Encontra o marcador de posição
                            
                            # Ajustar contadores
                            if codigo == '-':
                                num_linha1 += 1
                            elif codigo == '+':
                                num_linha2 += 1
                                
                            print(f"\n⚠️ Linha {num_linha1 if codigo == '-' else num_linha2} MODIFICADA (Dif. na coluna {coluna_diff + 1}):")
                            print(f"- {texto}")
                            
                            # A linha com '+' ou '-' anterior e a linha de dica '?' já foram processadas
                            # ou serão processadas na próxima iteração, o que pode ser confuso.
                            # Para simplificar, vou confiar na saída padrão do difflib:
                            
                            # Vamos resetar os contadores para que a saída seja mais limpa
                            # E apenas usar o print(linha_diff, end='')
                            break # Sai da iteração complexa para usar a simples

    # Re-executando de forma mais simples para garantir que a saída seja padrão difflib:
    print("\n--- Saída Padrão do difflib ---")
    d_simples = difflib.Differ()
    diff_simples = d_simples.compare(linhas1, linhas2)
    
    # 5. Imprimir a saída do difflib com indicação de linha e caractere de diferença
    linhas_originais = []
    
    # 6. Combinar as linhas com seus códigos e números (para fins de impressão)
    # A saída de d.compare() já indica as diferenças, a parte mais difícil é mapear de volta
    # o número *correto* da linha original.
    
    print("\nLegenda:")
    print("  ' ' Linhas idênticas")
    print("  '-' Linhas no 1º arquivo (excluídas)")
    print("  '+' Linhas no 2º arquivo (adicionadas)")
    print("  '?' Linhas que indicam diferenças internas (modificadas)")

    num1, num2 = 1, 1
    for linha in d.compare(linhas1, linhas2):
        codigo = linha[0]
        texto = linha[2:].rstrip('\n')
        
        prefixo = ""
        if codigo == ' ':
            prefixo = f"{num1:4} {num2:4}  "
            num1 += 1
            num2 += 1
        elif codigo == '-':
            prefixo = f"{num1:4}      - "
            num1 += 1
        elif codigo == '+':
            prefixo = f"     {num2:4} + "
            num2 += 1
        elif codigo == '?':
            prefixo = f"          ? "
            # A linha '?' não avança os contadores de linha
            
        print(prefixo + texto)


# --- Dados de Teste ---
# 1. Crie dois arquivos de teste para ver o programa em ação
arquivo_a = "roteador.output"
arquivo_b = "saida_correcao"

texto_a = """Esta é a linha um.
Esta é a linha dois.
Esta é a linha tres, um pouco diferente.
Esta é a linha quatro.
Linha que só existe no arquivo A.
"""

texto_b = """Esta é a linha um.
Esta é a linha 2.
Esta é a linha tres, um tanto diferente.
Esta é a linha quatro.
Esta é a linha cinco (nova).
Linha que só existe no arquivo B.
"""

# Salvar arquivos
with open(arquivo_a, 'w', encoding='utf-8') as f:
    f.write(texto_a)
with open(arquivo_b, 'w', encoding='utf-8') as f:
    f.write(texto_b)

# --- Executar a Comparação ---
comparar_arquivos_texto(arquivo_a, arquivo_b)

# Opcional: Remover os arquivos de teste
os.remove(arquivo_a)
os.remove(arquivo_b)