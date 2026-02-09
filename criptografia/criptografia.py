#!/usr/bin/python3

# Importando pyaes
import pyaes
# Importando sys
import sys

# Funcao de alinhamento
def alinhamento(entrada):
    # Retornando entrada alinhada (32 caracteres = 16 bytes)
    return entrada.ljust(((len(entrada) >> 5) + 1) << 5, "0")

# Funcao principal
def main(argv):
    # Abrindo arquivos de entrada e de saida
    with open(argv[1], "r") as entrada, open(argv[2], "w") as saida:
        # Carregando conteudo do arquivo
        linha = entrada.readlines()
        # Declarando chave compartilhada
        s = ""
        # Realizando comandos da entrada
        for i in range(1, int(linha[0]) + 1):
            # Separando palavras da linha
            comando = linha[i].split()
            # Selecionando comando
            match comando[0]:
                # Diffie-Hellman
                case "dh":
                    # Obtendo parametros
                    a = int(comando[1], 16)
                    b = int(comando[2], 16)
                    g = int(comando[3], 16)
                    p = int(comando[4], 16)
                    # Calculando chave compartilhada
                    s = "%X" % pow(g, a * b, p)
                    s = s[len(s) - len(comando[1]):]
                    # Gerando saida s
                    saida.write("s=" + s + "\n")
                # Decriptacao AES
                case "d":
                    # Convertendo para vetor de bytes com alinhamento de 16 bytes
                    c = bytes.fromhex(alinhamento(comando[1]))
                    # Gerando saida m
                    saida.write("m=" + "".join(["".join("%02X" % j for j in pyaes.AES(bytes.fromhex(s)).decrypt(c[i:i + 16])) for i in range(0, len(c), 16)]) + "\n")
                # Encriptacao AES
                case "e":
                    # Convertendo para vetor de bytes com alinhamento de 16 bytes
                    m = bytes.fromhex(alinhamento(comando[1]))
                    # Gerando saida c
                    saida.write("c=" + "".join(["".join("%02X" % j for j in pyaes.AES(bytes.fromhex(s)).encrypt(m[i:i + 16])) for i in range(0, len(m), 16)]) + "\n")

# Execucao principal
if __name__ == "__main__":
    # Chamando a funcao main
    main(sys.argv)
