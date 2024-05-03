// Isis Ardisson Logullo 7577410

#include <stdio.h>
#include <stdlib.h>
#include "ep2.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <bits/pthreadtypes.h>

int d, n;
int debug = 0;
int *pista[10];
int *podeContinuar;
int countQuebrados = 0;
int maxVelocidade = 0;
struct timeval comeco;
pthread_mutex_t *sem[10], *ciclistasDisponiveis, *voltasSem, randMutex;
pthread_barrier_t barrier;
pthread_t *threads;
StackNode **stacks;
Ciclista *ciclistas;

int stackEmpty(StackNode *stack) {
  return stack == NULL;
}

int stackSize(StackNode *stack) {
  if (stackEmpty(stack))
    return 0;
  return stack->size;
}

StackNode *push(StackNode *stack, int id) {
  StackNode *QueueNode = malloc(sizeof(StackNode));
  QueueNode->size = stackSize(stack) + 1;
  QueueNode->id = id;
  QueueNode->next = stack;
  return QueueNode;
}

StackNode *pop(StackNode *stack) {
  StackNode *next = NULL;

  if (!stackEmpty(stack)) {
    next = stack->next;
    free(stack);
  }
  return next;
}

int top(StackNode *stack) {
  if (!stackEmpty(stack))
    return stack->id;
  return -1;
}

void printStack(StackNode *stack) {
  StackNode *current = stack;
  int size = stackSize(stack);
  int *ranking = malloc(size * sizeof(int));

  for (int i = 0; i < size; i++) {
    ranking[i] = current->id;
    current = current->next;
  }

  for (int i = size - 1, j = 1; i >= 0; i--, j++) {
    printf("Ciclista %d está %dº lugar\n", ranking[i] + 1, j);
  }

  free(ranking);
}

void merge(int *vetor, int a, int q, int b, Ciclista *ciclistas) {
  int i, j, k;
  int * aux = malloc((b - a + 1)*sizeof(int));
  
  for (i = a; i <= q; i++)
    aux[i - a] = vetor[i];

  for (i = b, j = q - a + 1; i >= q+1 ; i--, j++)
    aux[j] = vetor[i];

  i = 0;
  j = b - a;

  for (k = a; k <= b; k++) {
    if (ciclistas[aux[i]].volta < ciclistas[aux[j]].volta) {
      vetor[k] = aux[i];
      i++;
    }
    else if (ciclistas[aux[i]].volta == ciclistas[aux[j]].volta) {
      if (ciclistas[aux[j]].tempoExecucao > ciclistas[aux[i]].tempoExecucao) {
        vetor[k] = aux[j];
        j--;
      }
      else {
        vetor[k] = aux[i];
        i++;
      }
    }
    else {
      vetor[k] = aux[j];
      j--;
    }
  }

  free(aux);
}

void mergeSort(int *vetor, int a, int b, Ciclista *ciclistas) {
  if (a < b) {
    int q = (a + b)/2;
    mergeSort(vetor, a, q, ciclistas);
    mergeSort(vetor, q + 1, b, ciclistas);

    merge(vetor, a, q, b, ciclistas);
  }
}

void printRank(int *vetor, int b) {
  int rank = 1, i;

  printf("Ciclista %d está em %dº lugar ", ciclistas[vetor[b]].id, rank);
  printf("com tempo %lf segundos\n", ciclistas[vetor[b]].tempoExecucao);

  for (i = b - 1; i >= 0; i--) {
    if (!ciclistas[vetor[i]].quebrado) {
      rank++;
      printf("Ciclista %d está em %dº lugar ", ciclistas[vetor[i]].id, rank);
      printf("com tempo %lf segundos\n", ciclistas[vetor[i]].tempoExecucao);
    }
  }

  for (i = b - 1; i >= 0; i--) {
    if (ciclistas[vetor[i]].quebrado) {
      printf("Ciclista %d quebrou na volta %d ", ciclistas[vetor[i]].id, ciclistas[vetor[i]].volta);
      printf("com tempo %lf segundos\n", ciclistas[vetor[i]].tempoExecucao);
    }
  }
}

int randomVelocidade(int id) {
  int probabilidade;
  int valor;

  pthread_mutex_lock(&randMutex);
  valor = (rand() % 100) + 1;
  pthread_mutex_unlock(&randMutex);

  if (ciclistas[id].velocidade == 3)
    return 3;

  if (ciclistas[id].volta >= 2 * (n - countQuebrados - 1)) {
    if (valor <= 10)
      return 3;
  }

  switch (ciclistas[id].velocidade) {
    case 1:
      probabilidade = 30;
      break;
    case 2:
      probabilidade = 50;
      break;
    default:
      break;
  }

  if (valor > probabilidade)
    return 2;
  else
    return 1;
}

void mudaPosicao(int ciclista) {
  int linha, coluna, mudou = 0, voltas;
  struct timeval tempoAtual;

  linha = ciclistas[ciclista].posicaoLinha;
  coluna = ciclistas[ciclista].posicaoColuna;

  pthread_mutex_lock(&sem[linha][coluna]);

  if (ciclistas[ciclista].volta >= 2 * n) {
    ciclistas[ciclista].terminou = 1;
    pthread_mutex_unlock(&sem[linha][coluna]);
    return;
  }

  pthread_mutex_lock(&sem[linha][(coluna + 1) % d]);
  if (pista[linha][(coluna + 1) % d] == 0) {
    ciclistas[ciclista].posicaoColuna = (coluna + 1) % d;
    pista[linha][(coluna + 1)% d] = ciclista + 1;
    mudou = 1;
    pthread_mutex_unlock(&sem[linha][(coluna + 1) % d]);
  }
  else {
    pthread_mutex_unlock(&sem[linha][(coluna + 1) % d]);

    for (int i = linha + 1; i < 10 && !mudou; i++) {
      pthread_mutex_lock(&sem[i][coluna]);
      pthread_mutex_lock(&sem[i][(coluna + 1) % d]);

      if (pista[i][coluna] == 0 && pista[i][(coluna + 1) % d] == 0) {
        ciclistas[ciclista].posicaoLinha = i;
        ciclistas[ciclista].posicaoColuna = (coluna + 1) % d;
        pista[i][(coluna + 1) % d] = ciclista + 1;
        mudou = 1;
      }

      pthread_mutex_unlock(&sem[i][(coluna + 1) % d]);
      pthread_mutex_unlock(&sem[i][coluna]);
    }
  }

  if (mudou) {
    if ((coluna + 1) % d == 0) {
      ciclistas[ciclista].volta++;
      voltas = ciclistas[ciclista].volta;
      ciclistas[ciclista].velocidade = randomVelocidade(ciclista);

      gettimeofday(&tempoAtual, NULL);

      ciclistas[ciclista].tempoExecucao = tempoAtual.tv_sec - comeco.tv_sec;
      ciclistas[ciclista].tempoExecucao += (tempoAtual.tv_usec - tempoAtual.tv_usec) / 1e6;
      
      pthread_mutex_lock(&voltasSem[voltas]);
      stacks[voltas] = push(stacks[voltas], ciclista);
      pthread_mutex_unlock(&voltasSem[voltas]);
    }
    pista[linha][coluna] = 0;
  }

  pthread_mutex_unlock(&sem[linha][coluna]);
}

void * thread(void * id) {
  int ciclista = *((int *) id);
  int count = 10;
  int tempoFaltante = 0;
  int tempoExecucao = 0;

  while (1) {
    switch (ciclistas[ciclista].velocidade) {
      case 1:
        tempoFaltante = (maxVelocidade ? 6 : 2);
        break;
      case 2:
        tempoFaltante = (maxVelocidade ? 3 : 1);
        break;
      default:
        tempoFaltante = 1;
        break;
    }

    while (tempoFaltante != 0) {
      
      tempoFaltante--;
      pthread_barrier_wait(&barrier);
      if (tempoFaltante == 0)
        mudaPosicao(ciclista);
      podeContinuar[ciclista] = 0;

      pthread_barrier_wait(&barrier);

      while (!podeContinuar[ciclista])
        usleep(100);

      if (ciclistas[ciclista].quebrado || ciclistas[ciclista].eliminado)
        pthread_exit(NULL);

      if (!tempoExecucao && maxVelocidade) {
        tempoExecucao = 1;
        tempoFaltante *= 3;
      }
    }

    
    count--;
  }

  return NULL;
}

void printPista() {
  printf("\n\n");

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < d; j++) {
      printf("%3d|  ", pista[i][j]);
    }
    printf("\n");
  }

  printf("\n\n");
}

void juiz(int ciclistasRestantes) {
  int volta = 1, linha, coluna;
  int ciclistaAtual;
  int probabilidadeQuebra;
  int alguemSobrou;
  int voltaCompletada;

  while (ciclistasRestantes > 1) {
    maxVelocidade = 0;
    for (int i = 0; i < n; i++) {
      if (ciclistas[i].velocidade == 3)
        maxVelocidade = 1;
    }

    if (maxVelocidade)
      usleep(20000);
    else
      usleep(60000);

    pthread_barrier_wait(&barrier);
    pthread_barrier_wait(&barrier);

    if (debug)
      printPista();

    alguemSobrou = 0;

    if (ciclistasRestantes > 5) {
      for (int i = 0; i < n; i++) {
        probabilidadeQuebra = (rand() % 100) + 1;

        if (ciclistas[i].volta == 0 || ciclistas[i].quebrado || ciclistas[i].eliminado || ciclistas[i].terminou)
          continue;
        
        if (probabilidadeQuebra <= 15 && ciclistas[i].volta % 6 == 0 && ciclistas[i].posicaoColuna == 0) {
          ciclistas[i].quebrado = 1;

          printf("Ciclista %d quebrou!! ", ciclistas[i].id);
          printf("Ele estava na volta %d\n", ciclistas[i].volta);

          countQuebrados += 1;
          ciclistasRestantes--;

          pista[ciclistas[i].posicaoLinha][ciclistas[i].posicaoColuna] = 0;

          alguemSobrou = 1;

          if (ciclistasRestantes == 5)
            break;
        }
      }
    }

    voltaCompletada = 1;

    for (int i = 0; i < n; i++)
      if (!ciclistas[i].quebrado && !ciclistas[i].eliminado && ciclistas[i].volta < volta) {
        voltaCompletada = 0;
        break;
      }
    

    if (voltaCompletada) {
      printf("\n\nVOLTA %d\n", volta);
      printStack(stacks[volta]);

      if (volta % 2 == 0){
        while (ciclistas[top(stacks[volta])].quebrado || ciclistas[top(stacks[volta])].eliminado)
          stacks[volta] = pop(stacks[volta]);

        if (ciclistas[top(stacks[volta])].posicaoColuna == 0) {
          ciclistaAtual = top(stacks[volta]);
          stacks[volta] = pop(stacks[volta]);

          ciclistas[ciclistaAtual].eliminado = 1;
          linha = ciclistas[ciclistaAtual].posicaoLinha;
          coluna = ciclistas[ciclistaAtual].posicaoColuna;

          printf("Ciclista %d foi eliminado!! ", ciclistaAtual + 1);
          printf("Tempo final ciclista %d: %lf\n", ciclistaAtual + 1, ciclistas[ciclistaAtual].tempoExecucao);         
          ciclistasRestantes--;

          pista[linha][coluna] = 0;
    
          alguemSobrou = 1;
        }
      }
      volta += 1;
    }

    if (alguemSobrou) {
      if (ciclistasRestantes > 1) {
        pthread_barrier_destroy(&barrier);
        pthread_barrier_init(&barrier, NULL, ciclistasRestantes + 1);
      }
      else {
        volta -= 1;
        printf("CICLISTA VENCEDOR: %d\n", top(stacks[volta]) + 1);
        ciclistas[top(stacks[volta])].eliminado = 1;
      }
    }

    for (int i = 0; i < n; i++)
      podeContinuar[i] = 1;
  }

  pthread_barrier_destroy(&barrier);
  printf("CICLISTAS QUEBRADOS: %d\n", countQuebrados);
}

void insereCiclistaPista(int numeroCiclistas, int linha, int coluna) {
  int id = numeroCiclistas - 1;

  pista[linha][coluna] = numeroCiclistas;
  ciclistas[id].posicaoColuna = coluna;
  ciclistas[id].posicaoLinha = linha;
  ciclistas[id].velocidade = 1;
  ciclistas[id].volta = 0;
  ciclistas[id].quebrado = 0;
  ciclistas[id].eliminado = 0;
  ciclistas[id].id = numeroCiclistas;
  ciclistas[id].terminou = 0;
}

void inicioPista() {
  int ciclistasRestantes;
  int coluna = 0;
  int i;
  int ciclistaAtual = n;

  for (int i = 0; i < 10; i++) {
    pista[i] = malloc(d*sizeof(int));
    
    for (int j = 0; j < d; j++)
      pista[i][j] = 0;
  }

  if (n % 5 != 0) {
    ciclistasRestantes = (n % 5);
    i = 0;
    while (ciclistasRestantes != 0) {
      insereCiclistaPista(ciclistaAtual, i, coluna);
      i += 1;
      ciclistasRestantes--;
      ciclistaAtual--;
    }
    coluna++;
  }

  ciclistasRestantes = n - (n % 5);
  i = 0;
  while (ciclistasRestantes != 0) {
    if (i == 5) {
      coluna++;
      i = 0;
    }
    else {
      insereCiclistaPista(ciclistaAtual, i, coluna);
      i += 1;
      ciclistasRestantes--;
      ciclistaAtual--;
    }
  }
}

void inicioMutex() {
  ciclistasDisponiveis = malloc(n * sizeof(pthread_mutex_t));
  voltasSem = malloc(2 * (n + 1) * sizeof(pthread_mutex_t));

  for (int i = 0; i < n; i++) {
    pthread_mutex_init(&ciclistasDisponiveis[i], NULL);
    pthread_mutex_lock(&ciclistasDisponiveis[i]);
  }

  for (int i = 0; i < 2 * (n + 1); i++)
    pthread_mutex_init(&voltasSem[i], NULL);
  
  pthread_mutex_init(&randMutex, NULL);

  for (int i = 0; i < 10; i++) {
    sem[i] = malloc(d * sizeof(pthread_mutex_t));

    for (int j = 0; j < d; j++) {
      pthread_mutex_init(&sem[i][j], NULL);
    }
  }
}

void freeMemoria(int *ids) {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < d; j++) {
      pthread_mutex_destroy(&sem[i][j]);
    }
    
    free(sem[i]);
    free(pista[i]);
  }

  for (int i = 0; i < n; i++) {
    pthread_join(threads[i], NULL);
    pthread_mutex_destroy(&ciclistasDisponiveis[i]);
  }

  for (int i = 0; i < 2 * (n + 1); i++) {
    pthread_mutex_destroy(&voltasSem[i]);

    while (!stackEmpty(stacks[i])){
      stacks[i] = pop(stacks[i]);
    }
  }

  pthread_mutex_destroy(&randMutex);

  free(ciclistasDisponiveis);
  free(voltasSem);
  free(ciclistas);
  free(threads);
  free(ids);
  free(podeContinuar);
  free(stacks);
}

int main(int argc, char ** argv) {
  int *ids;

  d = atoi(argv[1]);
  n = atoi(argv[2]);

  for (int i = 3; i < argc; i++) {
    if (argv[i][0] == '-') 
      debug = 1;
  }

  srand(time(NULL));
  gettimeofday(&comeco, NULL);

  ciclistas = malloc(n * sizeof(Ciclista));
  stacks = malloc(2 * (n + 1) * sizeof(StackNode *));
  threads = malloc(n * sizeof(pthread_t));
  ids = malloc(n * sizeof(int));
  podeContinuar = malloc(n * sizeof(int));

  pthread_barrier_init(&barrier, NULL, n + 1);

  for (int i = 0; i < 2 * (n + 1); i++)
    stacks[i] = NULL;

  for (int i = 0; i < n; i++)
    podeContinuar[i] = 0;

  inicioMutex();
  inicioPista();

  if (debug)
    printPista();

  for (int i = 0; i < n; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, thread, &ids[i]);
  }

  juiz(n);

  printf("\nFIM DA CORRIDA!!!\n");

  if (debug)
    printPista();

  printf("\nCLASSIFICAÇÃO FINAL:\n");
  mergeSort(ids, 0, n - 1, ciclistas);
  printRank(ids, n - 1);

  freeMemoria(ids);

  return 0;
}
