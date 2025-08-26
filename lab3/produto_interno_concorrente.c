#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"   // Para a medição de tempo (clock_gettime)

long int n; // Dimensão dos vetores
float *vetorA; // Vetor A
float *vetorB; // Vetor B
float *somas_parciais; // Vetor para armazenar somas parciais

typedef struct {
    short int id;
    short int nthreads;
    long int dim;
}t_args;

void * tarefa (void * args) {
    t_args *arg = (t_args*) args;
    long int fatia, ini, fim;
    float soma = 0.0;

    fatia = arg->dim / arg->nthreads;
    ini = arg->id * fatia;
    fim = ini + fatia;

    if (arg->id == (arg->nthreads) - 1) fim = arg->dim;
    for (long int i = ini; i < fim; i++) {
        soma += vetorA[i] * vetorB[i];
    }

    somas_parciais[arg->id] = soma;

    free(args);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    short int nthreads;
    double start, end, delta;
    FILE * descritorArquivo;
    float resultado_concorrente = 0.0;
    float resultado_sequencial = 0.0;

    if (argc < 3) {
        fprintf(stderr, "ERRO de entrada, digite: <numero_threads> <arquivo_de_teste.bin>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);
    char *nomeArquivo = argv[2];

    descritorArquivo = fopen(nomeArquivo, "rb");
    if (!descritorArquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        return 2;
    }
    fread(&n, sizeof(long int), 1, descritorArquivo);
    vetorA = (float*) malloc(sizeof(float) * n);
    vetorB = (float*) malloc(sizeof(float) * n);
    if (!vetorA  || !vetorB ) {
        fprintf(stderr, "Erro de alocao da memoria dos vetores\n");
        fclose(descritorArquivo);
        return 3;
    }
    fread(vetorA, sizeof(float), n, descritorArquivo);
    fread(vetorB, sizeof(float), n, descritorArquivo);
    fread(&resultado_sequencial, sizeof(float), 1, descritorArquivo);
    fclose(descritorArquivo);
    printf("Arquivo lido com sucesso. Dimensao N=%ld, Numero de Threads =%d\n" , n, nthreads);

    somas_parciais = (float*) malloc(sizeof(float) * nthreads);
    if (!somas_parciais) {
        fprintf(stderr, "Erro de alocao da memoria do vetor de somas parciais\n");
        free(vetorA);
        free(vetorB);
        return 4;
    }

    pthread_t tid[nthreads];

    printf("Iniciando o calculo concorrente do produto interno\n");
    GET_TIME(start);//começa a contagem de tempo

    for (short int i = 0; i < nthreads; i++) {
        t_args *args = (t_args*) malloc(sizeof(t_args));
        if (args == NULL) {
            fprintf(stderr, "Erro de alocacao de argumentos da thread %hd.\n", i);
            free(vetorA);
            free(vetorB);
            free(somas_parciais);
            return 5;
        }
        args->id = i;
        args->nthreads = nthreads;
        args->dim = n;

        if (pthread_create(&tid[i], NULL, tarefa, (void*) args)) {
            fprintf(stderr, "Erro na criacao da thread %hd.\n", i);
            free(vetorA);
            free(vetorB);
            free(somas_parciais);
            return 6;
        }
    }

    for(short int i = 0; i < nthreads; i++) {
        if (pthread_join(tid[i], NULL)) {
            fprintf(stderr, "Erro no join da thread %hd.\n", i);
            free(vetorA);
            free(vetorB);
            free(somas_parciais);
            return 7;
        }
    }

    GET_TIME(end); //termina a contagem de tempo
    delta = end - start; //calcula o intervalo de tempo para processamento do vetor

    for (short int i = 0; i < nthreads; i++) {
        resultado_concorrente += somas_parciais[i];
    }
    printf("Produto interno concorrente: %f\n", resultado_concorrente);
    printf("Produto interno sequencial: %f\n", resultado_sequencial);

    float erro = (resultado_sequencial - resultado_concorrente) / resultado_sequencial;

    printf("Tempo de execucao concorrente: %.6f segundos\n", delta);
    printf("Variacao Relativa: %f\n", erro);
    free(vetorA);
    free(vetorB);
    free(somas_parciais);
    return 0;
}
