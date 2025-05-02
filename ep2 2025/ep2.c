/* Isis Ardisson Logullo 7577410 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "ep2.h"


int d, k, controle;
int *continua;
int debug = 0;
int quebrados = 0;
int maxV = 0;
int *pista[10];
struct timeval inicio;
pthread_mutex_t *sem[10], *ciclistasRestantes, *semVolta, randMutex;
pthread_barreira_t barreira;
pthread_t *threads;
No **pilhas;
Ciclista *Ciclistas;

//Pilha
int pilhaVazia(No *pilha) {
  return pilha == NULL;
}

int tamanhoPilha(No *pilha) {
  if (empty(pilha))
    return 0;

  return pilha->tam;
}

No *push(No *pilha, int id) {
  No *node = malloc(sizeof(No));

  node->tam = pilhaSize(pilha) + 1;
  node->id = id;
  node->prox = pilha;
  
  return node;
}

No *pop(No *pilha) {
  No *prox = NULL;

  if (!pilhaVazia(pilha)) {
    prox = pilha->prox;
    free(pilha);
  }

  return prox;
}

int top(No *pilha) {
  if (!empty(pilha))
    return pilha->id;

  return -1;
}

void imprimePilha(No *pilha) {
  No *atual = pilha;
  int tam = pilhaSize(pilha);
  int *posicao = malloc(tam * sizeof(int));

  for (int i = 0; i < tam; i++) {
    posicao[i] = atual->id;
    atual = atual->prox;
  }

  for (int j = tam - 1, p = 1; j >= 0; j--, p++) {
    printf("%dº lugar\n -> %d ciclista", p, posicao[j] + 1);
  }

  free(posicao);
}

//MergeSort
void merge(int *v, int p, int q, int r, Ciclista *ciclistas) {
  int i, j, k;
  int * aux = malloc((r - p + 1)*sizeof(int));
  
  for (i = p; i <= q; i++)
    aux[i - p] = v[i];

  for (i = r, j = q - p + 1; i >= q+1 ; i--, j++)
    aux[j] = v[i];

  i = 0;
  j = r - p;

  for (k = p; k <= r; k++) {
    if (ciclistas[aux[i]].volta < ciclistas[aux[j]].volta) {
      v[k] = aux[i];
      i++;
    }
    else if (ciclistas[aux[i]].volta == ciclistas[aux[j]].volta) {
      if (ciclistas[aux[j]].tempo > ciclistas[aux[i]].tempo) {
        v[k] = aux[j];
        j--;
      }
      else {
        v[k] = aux[i];
        i++;
      }
    }
    else {
      v[k] = aux[j];
      j--;
    }
  }

  free(aux);
}

void mergeSort(int *v, int p, int r, Ciclista *ciclistas) {
  if (p < r) {
    int q = (p + r)/2;

    mergeSort(v, p, q, ciclistas);
    mergeSort(v, q + 1, r, ciclistas);
    merge(v, p, q, r, ciclistas);
  }
}

//Inicializacoes
void insereCiclista(int numero, int m, int n) {
  int id = numero - 1;

  pista[m][n] = numero;
  Ciclistas[id].id = numero;
  Ciclistas[id].m = m;
  Ciclistas[id].n = n;
  Ciclistas[id].v = 1;
  Ciclistas[id].terminou = 0;
  Ciclistas[id].volta = 0;
  Ciclistas[id].quebrado = 0;
  Ciclistas[id].desclassificado = 0;

}

void gerapista() {
  int i, restantes;
  int n = 0;
  int atual = k;

  for (int i = 0; i < 10; i++) {
    pista[i] = malloc(d*sizeof(int));
    
    for (int j = 0; j < d; j++)
      pista[i][j] = 0;
  }

  if (k % 5 != 0) {
    restantes = (k % 5);
    i = 0;

    while (restantes != 0) {

      insereCiclista(atual, i, n);
      i += 1;
      restantes--;
      atual--;

    }

    n++;
  }

  restantes = k - (k % 5);
  i = 0;

  while (restantes != 0) {

    if (i == 5) {

      n++;
      i = 0;

    }
    else {

      insereCiclista(atual, i, n);
      i += 1;
      restantes--;
      atual--;

    }
  }
}

void mutex() {
  ciclistasRestantes = malloc(k * sizeof(pthread_mutex_t));
  semVolta = malloc(2 * (k + 1) * sizeof(pthread_mutex_t));

  for (int i = 0; i < k; i++) {

    pthread_mutex_init(&ciclistasRestantes[i], NULL);
    pthread_mutex_lock(&ciclistasRestantes[i]);

  }

  for (int i = 0; i < 2 * (k + 1); i++)
    pthread_mutex_init(&semVolta[i], NULL);
  

  pthread_mutex_init(&randMutex, NULL);


  for (int i = 0; i < 10; i++) {

    sem[i] = malloc(d * sizeof(pthread_mutex_t));

    for (int j = 0; j < d; j++) {

      pthread_mutex_init(&sem[i][j], NULL);
    }

  }

}



//Impressoes
void imprimePosicao(int *v, int b) {
  int posicao = 1, i;

  printf("%dº lugar -> ciclista %d : %lf segundos\n", posicao, Ciclistas[v[b]].id, Ciclistas[v[b]].tempo);

  for (i = b - 1; i >= 0; i--) {
    if (!Ciclistas[v[i]].quebrado) {
      posicao++;
      printf("%dº lugar -> ciclista %d : %lf segundos\n", posicao, Ciclistas[v[i]].id, Ciclistas[v[i]].tempo);
    }
  }

  for (i = b - 1; i >= 0; i--) {
    if (Ciclistas[v[i]].quebrado) {
      printf("QUEBROU!! Ciclista %d, volta %d : %lf segundos\n", Ciclistas[v[i]].id, Ciclistas[v[i]].volta, Ciclistas[v[i]].tempo);
    }
  }
}

void imprimePista() {
  fprintf(stderr, "\n\n");

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < d; j++) {

      fprintf(stderr, "%3d|  ", pista[i][j]);
    }

    fprintf(stderr, "\n");

  }

  fprintf(stderr, "\n\n");
}







int geraProbabilidades(int id) {
  int prob;
  int valor;

  pthread_mutex_lock(&randMutex);
  valor = (rand() % 100) + 1;
  pthread_mutex_unlock(&randMutex);

  if (Ciclistas[id].v == 3)
    return 3;

  if (Ciclistas[id].volta >= 2 * (k - quebrados - 1)) {
    if (valor <= 10)
      return 3;
  }

  switch (Ciclistas[id].v) {
    case 1:
      prob = 20;
      break;
    case 2:
      prob = 40;
      break;
    default:
      break;
  }

  if (valor > prob)
    return 2;
  else
    return 1;
}

void changePosition(int Ciclista) {
  int m, n, changed = 0, volta;
  struct timeval atualTime;

  m = Ciclistas[Ciclista].m;
  n = Ciclistas[Ciclista].n;

  pthread_mutex_lock(&sem[m][n]);

  if (Ciclistas[Ciclista].volta >= 2 * k) {
    Ciclistas[Ciclista].terminou = 1;
    pthread_mutex_unlock(&sem[m][n]);
    return;
  }

  pthread_mutex_lock(&sem[m][(n + 1) % d]);
  if (pista[m][(n + 1) % d] == 0) {
    Ciclistas[Ciclista].n = (n + 1) % d;
    pista[m][(n + 1)% d] = Ciclista + 1;
    changed = 1;
    pthread_mutex_unlock(&sem[m][(n + 1) % d]);
  }
  else {
    pthread_mutex_unlock(&sem[m][(n + 1) % d]);

    for (int i = m + 1; i < 10 && !changed; i++) {
      pthread_mutex_lock(&sem[i][n]);
      pthread_mutex_lock(&sem[i][(n + 1) % d]);

      if (pista[i][n] == 0 && pista[i][(n + 1) % d] == 0) {
        Ciclistas[Ciclista].m = i;
        Ciclistas[Ciclista].n = (n + 1) % d;
        pista[i][(n + 1) % d] = Ciclista + 1;
        changed = 1;
      }

      pthread_mutex_unlock(&sem[i][(n + 1) % d]);
      pthread_mutex_unlock(&sem[i][n]);
    }
  }

  if (changed) {
    if ((n + 1) % d == 0) {
      Ciclistas[Ciclista].volta++;
      volta = Ciclistas[Ciclista].volta;
      Ciclistas[Ciclista].v = randomv(Ciclista);

      gettimeofday(&atualTime, NULL);

      Ciclistas[Ciclista].tempo = atualTime.tv_sec - inicio.tv_sec;
      Ciclistas[Ciclista].tempo += (atualTime.tv_usec - inicio.tv_usec) / 1e6;
      
      pthread_mutex_lock(&semVolta[volta]);
      pilhas[volta] = push(pilhas[volta], Ciclista);
      pthread_mutex_unlock(&semVolta[volta]);
    }
    pista[m][n] = 0;
  }

  pthread_mutex_unlock(&sem[m][n]);
}

void * thread(void * id) {
  int Ciclista = *((int *) id);
  int count = 10;
  int timeRemaining = 0;
  int executionTime = 0;

  while (1) {
    switch (Ciclistas[Ciclista].v) {
      case 1:
        timeRemaining = (maxV ? 6 : 2);
        break;
      case 2:
        timeRemaining = (maxV ? 3 : 1);
        break;
      default:
        timeRemaining = 1;
        break;
    }

    while (timeRemaining != 0) {
      
      timeRemaining--;
      pthread_barreira_wait(&barreira);
      if (timeRemaining == 0)
        changePosition(Ciclista);
      continua[Ciclista] = 0;

      pthread_barreira_wait(&barreira);

      while (!continua[Ciclista])
        usleep(100);

      if (Ciclistas[Ciclista].quebrado || Ciclistas[Ciclista].desclassificado)
        pthread_exit(NULL);

      if (!executionTime && maxV) {
        executionTime = 1;
        timeRemaining *= 3;
      }
    }

    
    count--;
  }

  return NULL;
}

void judge(int restantes) {
  int lap = 1, m, n;
  int atual;
  int quebradoProbability;
  int someoneHasLeft;
  int lapCompleted;

  while (restantes > 1) {
    maxV = 0;
    for (int i = 0; i < n; i++) {
      if (Ciclistas[i].v == 3)
        maxV = 1;
    }

    if (maxV)
      usleep(20000);
    else
      usleep(60000);

    pthread_barreira_wait(&barreira);
    pthread_barreira_wait(&barreira);

    if (debug)
      printpista();

    someoneHasLeft = 0;

    if (restantes > 5) {
      for (int i = 0; i < n; i++) {
        quebradoProbability = (rand() % 100) + 1;

        if (Ciclistas[i].volta == 0 || Ciclistas[i].quebrado || Ciclistas[i].desclassificado || Ciclistas[i].terminou)
          continue;
        
        if (quebradoProbability <= 5 && Ciclistas[i].volta % 6 == 0 && Ciclistas[i].n == 0) {
          Ciclistas[i].quebrado = 1;

          printf("Ciclista %d quebrou!! ", Ciclistas[i].id);
          printf("Ele estava na volta %d\n", Ciclistas[i].volta);

          quebrados += 1;
          restantes--;

          pista[Ciclistas[i].m][Ciclistas[i].n] = 0;

          someoneHasLeft = 1;

          if (restantes == 5)
            break;
        }
      }
    }

    lapCompleted = 1;

    for (int i = 0; i < n; i++)
      if (!Ciclistas[i].quebrado && !Ciclistas[i].desclassificado && Ciclistas[i].volta < lap) {
        lapCompleted = 0;
        break;
      }
    

    if (lapCompleted) {
      printf("\n\nVOLTA %d\n", lap);
      printpilha(pilhas[lap]);

      if (lap % 2 == 0){
        while (Ciclistas[top(pilhas[lap])].quebrado || Ciclistas[top(pilhas[lap])].desclassificado)
          pilhas[lap] = pop(pilhas[lap]);

        if (Ciclistas[top(pilhas[lap])].n == 0) {
          atual = top(pilhas[lap]);
          pilhas[lap] = pop(pilhas[lap]);
          Ciclistas[atual].desclassificado = 1;
          m = Ciclistas[atual].m;
          n = Ciclistas[atual].n;

          printf("Ciclista %d foi embora!! ", atual + 1);
          printf("Tempo final ciclista %d: %lf\n", atual + 1, Ciclistas[atual].tempo);         
          restantes--;

          pista[m][n] = 0;
    
          someoneHasLeft = 1;
        }
      }
      lap += 1;
    }

    if (someoneHasLeft) {
      if (restantes > 1) {
        pthread_barreira_destroy(&barreira);
        pthread_barreira_init(&barreira, NULL, restantes + 1);
      }
      else {
        lap -= 1;
        printf("CICLISTA VENCEDOR: %d\n", top(pilhas[lap]) + 1);
        Ciclistas[top(pilhas[lap])].desclassificado = 1;
      }
    }

    for (int i = 0; i < n; i++)
      continua[i] = 1;
  }

  pthread_barreira_destroy(&barreira);
  printf("CICLISTAS QUEBRADOS: %d\n", quebrados);
}


int main(int argc, char ** argv) {
  int *ids;

  d = atoi(argv[1]);
  k = atoi(argv[2]);
  controle = atoi(argv[3]);

  for (int i = 4; i < argc; i++) {
    if (argv[i][0] == '-') 
      debug = 1;
  }

  srand(time(NULL));
  gettimeofday(&inicio, NULL);

  
  pilhas = malloc(2 * (k + 1) * sizeof(No *));
  Ciclistas = malloc(k * sizeof(Ciclista));
  threads = malloc(k * sizeof(pthread_t));
  
  ids = malloc(k * sizeof(int));
  continua = malloc(k * sizeof(int));

  pthread_barreira_init(&barreira, NULL, k + 1);


  for (int i = 0; i < 2 * (k + 1); i++)
    pilhas[i] = NULL;

  for (int i = 0; i < k; i++)
    continua[i] = 0;

  mutex();
  geraPista();

  if (debug)
    imprimePista();

  for (int i = 0; i < k; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, thread, &ids[i]);
  }

  judge(k);

  printf("\nFIM DA CORRIDA!!!\n");

  if (debug)
    imprimePista();

  printf("\nCLASSIFICAÇÃO FINAL:\n");
  mergeSort(ids, 0, k - 1, Ciclistas);
  printposicao(ids, k - 1);

  free(sem);
  free(semVolta);
  free(ciclistasRestantes);
  free(barreira);
  free(threads);
  free(pilhas);
  free(Ciclistas);

  return 0;
}
