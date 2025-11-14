/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/
/* Nome: Daniel Li Vam Man*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.ArrayList;
import java.util.List;


//classe runnable
class MyCallable implements Callable<Long> {
  private long numero;
  //construtor
  MyCallable(long n) {
    this.numero = n;
  }
  
  private boolean ehPrimo(long n) {
      if (n <= 1) return false;
      if (n == 2) return true;
      if (n % 2 == 0) return false;
      for (long i = 3; i < Math.sqrt(n)+1 ; i += 2) {
         if (n % i == 0) return false;
      }
      return true;
  }

  //método para execução
  public Long call() throws Exception {
    if (ehPrimo(this.numero)) {
        return 1L;
    } else {
        return 0L;
    }
  }
}

//classe do método main
public class FutureHello  {
  private static final int N = 100000;
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    //cria um pool de threads (NTHREADS)
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    //cria uma lista para armazenar referencias de chamadas assincronas
    List<Future<Long>> list = new ArrayList<Future<Long>>();

    for (int i = 1; i <= N; i++) {
      Callable<Long> worker = new MyCallable(i); // Cria uma tarefa para o numero 'i'
      /*submit() permite enviar tarefas Callable ou Runnable e obter um objeto Future para acompanhar o progresso e recuperar o resultado da tarefa
       */
      Future<Long> submit = executor.submit(worker);
      list.add(submit);
    }

    System.out.println("Total de tarefas: " + list.size());
    //pode fazer outras tarefas...

    //recupera os resultados e faz o somatório final
    long sum = 0;
    for (Future<Long> future : list) {
      try {
        sum += future.get(); //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }
    System.out.println("Total de primos encontrados de 1 ate " + N + ": " + sum);
    executor.shutdown();
  }
}
