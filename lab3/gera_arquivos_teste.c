#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

int main(int argc, char*argv[]) {
    long int n;
    float *vetorA; // vetor que será gerada
    float *vetorB; // vetor que será gerada
    float produtoInterno; // resultado do produto interno
    FILE * descritorArquivo; //ponteiro para o arquivo

    //recebe os argumentos de entrada
    if (argc < 3) {
        fprintf(stderr, "Digite: %s <dimensao> <nome arquivo saida>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]); // Converte o argumento de string para long int

    //aloca memoria para os vetores
    printf("Alocando memoria para os vetores de dimensao N=%ld\n", n);
    vetorA = (float*) malloc(sizeof(float) * n);
    vetorB = (float*) malloc(sizeof(float) * n);

    if (vetorA == NULL || vetorB == NULL) {
        fprintf(stderr, "Erro de alocao da memoria dos vetores\n");
        return 2;
    }

    //preenche os vetores com valores float aleatorios entre 0 e 1
    printf("Geracao de valores aleatorios para os vetores\n");
    srand(time(NULL)); 
    for (long int i = 0; i < n; i++) {
        vetorA[i] = ((float)rand() / (float)RAND_MAX);
        vetorB[i] = ((float)rand() / (float)RAND_MAX);
    }

    //calcula o produto interno dos vetores
    printf("Calculo do produto interno dos vetores\n");
    for (long int i = 0; i < n; i++) {
        produtoInterno += vetorA[i] * vetorB[i];
    }
    printf("Produto interno calculado: %f\n", produtoInterno);

    //escreve os vetores e o resultado no arquivo binario
    printf("Salvando os vetores e resultado no arquivo\n");
    descritorArquivo = fopen(argv[2], "wb");
    if (!descritorArquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        free(vetorA);
        free(vetorB);
        return 3;
    }

    fwrite(&n, sizeof(long int), 1, descritorArquivo); // escreve a dimensao
    fwrite(vetorA, sizeof(float), n, descritorArquivo); // escreve os elementos do vetor A
    fwrite(vetorB, sizeof(float), n, descritorArquivo); // escreve os elementos do vetor B
    fwrite(&produtoInterno, sizeof(float), 1, descritorArquivo); // escreve o resultado

    fclose(descritorArquivo);
    free(vetorA);   
    free(vetorB);
    printf("Arquivo salvo com sucesso\n");
    return 0;

}
