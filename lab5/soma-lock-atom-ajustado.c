/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Nome: Daniel Li Vam Man*/

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
int nthreads; //qtde de threads (passada linha de comando)
int estado = 0; // variavel de estado

pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond_multiplo; // Condicao para a extra esperar
pthread_cond_t cond_impresso; // Condicao para as ExecutaTarefa esperarem

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);
  long int valor_final = nthreads * 100000;

  for (int i=0; i<100000; i++) {
    pthread_mutex_lock(&mutex);
    //--entrada na SC
    // enquanto o estado for 1 (impressao em andamento), espere
    while (estado) {
        pthread_cond_wait(&cond_impresso, &mutex);
    }

    soma++;

    // Se a thread gerou um multiplo DENTRO do intervalo de trabalho da 'extra'...
    if (soma % 1000 == 0 && soma > 0 && soma <= valor_final) {
      // muda o estado
      estado = 1;
      
      // avisa para a thread de log que eh hora de imprimir
      pthread_cond_signal(&cond_multiplo);
      
      // espera ate que a thread de log faca a impressao
      pthread_cond_wait(&cond_impresso, &mutex);
    }
    //--saida da SC
    pthread_mutex_unlock(&mutex);
  }

  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");
  
  long int valor_final = nthreads * 100000;
  long int total_de_multiplos = valor_final / 1000;

  for(int i=0; i<total_de_multiplos; i++) {
    pthread_mutex_lock(&mutex);
    
    // espera ate que uma thread de trabalho gere um multiplo
    while (!estado) {
        pthread_cond_wait(&cond_multiplo, &mutex);
    }

    printf("soma = %ld\n", soma);

    // troca o estado
    estado = 0;
    
    // libera as threads de trabalho que estao esperando
    pthread_cond_broadcast(&cond_impresso);
    
    pthread_mutex_unlock(&mutex);
  }

  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema

    //--le e avalia os parametros de entrada
    if(argc<2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
    }
    nthreads = atoi(argv[1]);
    if (nthreads <= 0) {
      printf("O numero de threads deve ser maior que zero.\n");
      return 1;
    }

    //--aloca as estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}

    // inicializa os recursos de sincronizacao
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_multiplo, NULL);
    pthread_cond_init(&cond_impresso, NULL);

    //--cria as threads
    for(long int t=0; t<nthreads; t++) {
      if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
      }
    }

    //--cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    //--espera todas as threads terminarem
    for (int t=0; t<nthreads+1; t++) {
      if (pthread_join(tid[t], NULL)) {
          printf("--ERRO: pthread_join() \n"); exit(-1); 
      } 
    } 

    printf("Valor de 'soma' = %ld\n", soma);

    // Libera os recursos de sincronizacao
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_multiplo);
    pthread_cond_destroy(&cond_impresso);
    
    free(tid);
    return 0;
}
