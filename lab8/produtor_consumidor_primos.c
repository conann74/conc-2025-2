/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Nome: Daniel Li Vam Man*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

// Variaveis globais
long long int N; // total de inteiros a serem produzidos
int M; // tamanho do nthreadsanal de inteiros
int nthreads; // numero de threads consumidoras

long long int *buffer; // buffer compartilhado
int in = 0; // indice de insercao
int out = 0; // indice de retirada

long long int total_primos = 0; // total de primos encontrados
long long int *contagem_primos_consumidor; // array para contagem de primos por consumidor
long long int numeros_consumidos = 0; // total de numeros consumidos
int consumidosNoLote = 0; // contador de numeros consumidos no lote atual

// Semaforos
sem_t podeProduzir; // semaforo para controlar a producao
sem_t bufferCheio; // semaforo para controlar a quantidade de itens no buffer para os consumidores
sem_t mutexBuffer; // semaforo para exclusao mutua no buffer
sem_t mutexResultados; // semaforo para exclusao mutua nos resultados

int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long long int i = 3; i <= sqrt(n); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void *produtor(void *arg) {
    long long int item_atual = 1;
    long long int produzidos = 0;

    while (produzidos < N) {
        sem_wait(&podeProduzir);

        sem_wait(&mutexBuffer);
        int lote_atual = 0;
        for (int i = 0; i < M && produzidos < N; i++) {
            buffer[in] = item_atual;
            in = (in + 1) % M;
            item_atual++;
            produzidos++;
            lote_atual++;
        }
        sem_post(&mutexBuffer);

        for (int i = 0; i < lote_atual; i++) {
            sem_post(&bufferCheio);
        }
    }

    printf("FIM DO PRODUTOR. Sinalizando consumidores.\n");
    for (int i = 0; i < nthreads; i++) {
        sem_post(&bufferCheio);
    }

    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    int id = *(int *)arg;
    long long int meus_primos = 0;

    while (1) {
        sem_wait(&bufferCheio);

        sem_wait(&mutexResultados);
        if (numeros_consumidos >= N) {
            sem_post(&mutexResultados);
            sem_post(&bufferCheio); //Libera para a prox thread consumidora tambem poder sair
            break;
        }
        numeros_consumidos++;
        sem_post(&mutexResultados);

        sem_wait(&mutexBuffer);
        long long int item = buffer[out];
        out = (out + 1) % M;
        sem_post(&mutexBuffer);

        if (ehPrimo(item)) {
            meus_primos++;
        }

        sem_wait(&mutexResultados);
        consumidosNoLote++;
        if (consumidosNoLote == M) {
            printf("CONSUMIDOR %d: Ultimo item do lote consumido. Sinalizando produtor.\n", id);
            sem_post(&podeProduzir);
            consumidosNoLote = 0;
        }
        sem_post(&mutexResultados);// Libera o mutexResultados
    }

    sem_wait(&mutexResultados);
    total_primos += meus_primos;
    contagem_primos_consumidor[id] = meus_primos; // Armazena a contagem individual
    sem_post(&mutexResultados);

    free(arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t produtor_tid; // thread produtora
    pthread_t *consumidores_tid;// array de threads consumidoras

    if (argc < 4) {
        printf("Uso: %s <N: total de inteiros> <M: tamanho do canal de inteiros> <nthreads>: numero de threads consumidoras>\n", argv[0]);
        return 1;
    }
    N = atoll(argv[1]);
    M = atoi(argv[2]);
    nthreads = atoi(argv[3]);

    if (nthreads <= 0) {
      printf("O numero de threads deve ser maior que zero.\n");
      return 1;
    }

    consumidores_tid = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
    buffer = (long long int *)malloc(sizeof(long long int) * M);
    contagem_primos_consumidor = (long long *)malloc(sizeof(long long) * nthreads);

    sem_init(&podeProduzir, 0, 1);
    sem_init(&bufferCheio, 0, 0);
    sem_init(&mutexBuffer, 0, 1);
    sem_init(&mutexResultados, 0, 1);

    printf("Iniciando a thread produtora...\n");
    if(pthread_create(&produtor_tid, NULL, produtor, NULL)) {
        printf("--ERRO: pthread_create() produtora\n"); return 2;
    }

    printf("Iniciando %d threads consumidoras...\n", nthreads);
    for (int i = 0; i < nthreads; i++) {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        if(pthread_create(&consumidores_tid[i], NULL, consumidor, (void *)id)){
            printf("--ERRO: pthread_create() consumidoras\n"); return 3;
        }
    }

    if(pthread_join(produtor_tid, NULL)){
        printf("--ERRO: pthread_join() produtora\n"); return 4;
    }
    for (int i = 0; i < nthreads; i++) {
        if(pthread_join(consumidores_tid[i], NULL)){
            printf("--ERRO: pthread_join() consumidoras\n"); return 5;
        }
    }
    
    printf("\n--- Resultados ---\n");
    int vencedor_id = 0;
    long long int max_primos = 0;
    for (int i = 0; i < nthreads; i++) {
        printf("Consumidor %d encontrou %lld primos.\n", i, contagem_primos_consumidor[i]);
        if (contagem_primos_consumidor[i] > max_primos) {
            max_primos = contagem_primos_consumidor[i];
            vencedor_id = i;
        }
    }
    printf("Total de primos encontrados: %lld\n", total_primos);
    printf("Consumidor vencedor: ID %d com %lld primos.\n", vencedor_id, max_primos);

    sem_destroy(&podeProduzir);
    sem_destroy(&bufferCheio);
    sem_destroy(&mutexBuffer);
    sem_destroy(&mutexResultados);
    free(buffer);
    free(contagem_primos_consumidor);
    free(consumidores_tid);
    
    return 0;
}
