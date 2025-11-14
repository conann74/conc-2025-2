/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de um pool de threads oferecido por Java */
/* -------------------------------------------------------------------*/
/* Nome: Daniel Li Vam Man*/

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

//classe do contador compartilhado
class ContadorCompartilhado {
    private long valor = 0;

    public synchronized void incrementar() {
        this.valor++;
    }

    public synchronized long getValor() {
        return this.valor;
    }
}

//classe runnable
class Worker implements Runnable {
  private final long steps;

  private final ContadorCompartilhado contador;

  //construtor
  Worker(long numSteps, ContadorCompartilhado contador) {
    this.steps = numSteps;
    this.contador = contador;
  }
 
  //método para execução
  public void run() {
    long s = 0;
    for (long i=1; i<this.steps; i++) {
      s += i;
    }
    System.out.println(s);
    //chamando o metodo sincronizado do monitor
    this.contador.incrementar();
  }
}

//classe do método main
public class AnotherHelloPool {
  private static final int NTHREADS = 10;
  private static final int WORKERS = 50;

  public static void main(String[] args) {
    //cria um pool de threads (NTHREADS)
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);

    //cria uma unica instância do nosso monitor
    ContadorCompartilhado contadorCompartilhado = new ContadorCompartilhado();

    //dispara a execução dos workers
    for (int i = 1; i < WORKERS; i++) {
      Runnable worker = new Worker(i, contadorCompartilhado);
      executor.execute(worker);
    }
    //termina a execução das threads no pool (não permite que o executor aceite novos objetos)
    executor.shutdown();
    //espera todas as threads terminarem
    while (!executor.isTerminated()) {}
    System.out.println("Total de tarefas executadas (valor final do contador): " + contadorCompartilhado.getValor());
    System.out.println("Terminou");
   }
}
