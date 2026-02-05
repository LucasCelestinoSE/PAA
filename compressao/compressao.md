# Compressao de dados
 A compressão de dados consiste em realizar o uso de uma ou várias técnicas de reduzir a quantidade de bits, perdendo um pouco ou mantendo toda a informação.
 A primeira tecnica e o primeiro exemplo de compressão de dados é utilizando bases nitrogenadas.
 Digamos que você precise ler um arquivo com milhões de bases nitrogenadas em padrão ASCII
 Se olharmos bem, temos o padrão ASCII com no mínimo 8 bits por caractere, entretanto, o alfabeto utilizado das bases nitrogenada é de 4 simbolos. 
 Se fizemos um calculo simples, podemos deduzir que 8 bits para representar uma base nitrogenada é muito, e o mínimo para se representar tal alfabeto é de 2 bits.
 Formato otimizado x Formato ASCII
 00 -> A           | 01000001 -> A
 10 -> G           | 01000111 -> G
 11 -> T           | 01010100 -> T
 01 -> C           | 01000011 -> C

 # RLE (Run-length Encoding)
 > Esta técnica de repetição consiste em contabilizar a repetição de símbolos em uma sequência.
 
