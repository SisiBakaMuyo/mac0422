/* Isis Ardisson Logullo 7577410 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "ep2.h"


int d, k;
char *controle;
int *continua;
int debug = 0;
int quebrados = 0;
int maxV = 0;
int *pista[10];

struct timeval inicio;

No **pilhas;
Ciclista *Ciclistas;
pthread_t *threads;
pthread_barrier_t barreira;
pthread_mutex_t *sem[10], *ciclistasRestantes, *semVolta, mutexProb;


//Pilha
int pilhaVazia(No *pilha) {
  return pilha == NULL;
}

int tamanhoPilha(No *pilha) {
  if (pilhaVazia(pilha))
    return 0;

  return pilha->tam;
}

No *push(No *pilha, int numero) {
  No *no = malloc(sizeof(No));

  no->tam = tamanhoPilha(pilha) + 1;
  no->numero = numero;
  no->prox = pilha;
  
  return no;
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
  if (!pilhaVazia(pilha))
    return pilha->numero;

  return -1;
}

void imprimePilha(No *pilha) {
  No *atual = pilha;
  int tam = tamanhoPilha(pilha);
  int *posicao = malloc(tam * sizeof(int));

  for (int i = 0; i < tam; i++) {
    posicao[i] = atual->numero;
    atual = atual->prox;
  }

  for (int j = tam - 1, p = 1; j >= 0; j--, p++) {
    printf("%dº lugar -> %d ciclista\n", p, posicao[j] + 1);
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
  Ciclistas[id].numero = numero;
  Ciclistas[id].m = m;
  Ciclistas[id].n = n;
  Ciclistas[id].v = 1;
  Ciclistas[id].terminou = 0;
  Ciclistas[id].volta = 0;
  Ciclistas[id].quebrado = 0;
  Ciclistas[id].desclassificado = 0;

}

void geraPista() {
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
  

  pthread_mutex_init(&mutexProb, NULL);


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

  printf("%dº lugar -> ciclista %d : %lf segundos\n", posicao, Ciclistas[v[b]].numero, Ciclistas[v[b]].tempo);

  for (i = b - 1; i >= 0; i--) {
    if (!Ciclistas[v[i]].quebrado) {
      posicao++;
      printf("%dº lugar -> ciclista %d : %lf segundos\n", posicao, Ciclistas[v[i]].numero, Ciclistas[v[i]].tempo);
    }
  }

  for (i = b - 1; i >= 0; i--) {
    if (Ciclistas[v[i]].quebrado) {
      printf("Ciclista %d ***Quebrou*** : %lf segundos\n", Ciclistas[v[i]].numero, Ciclistas[v[i]].tempo);
    }
  }
}

void imprimePista() {
  fprintf(stderr, "\n\n");

  for (int j = 0; j < d; j++) {
  
    for (int i = 0; i < 10; i++) {
    
      if (pista[i][j] == 0)
        fprintf(stderr, "  .|  ");
      else if (pista[i][j] < 10)
        fprintf(stderr, "  %d|  ", pista[i][j]);
      else
        fprintf(stderr, " %d|  ", pista[i][j]);
    }

    fprintf(stderr, "\n");
  }

  fprintf(stderr, "\n\n");
}

//Corrida
int geraProbabilidades(int numero) {
  int prob;
  int valor;

  pthread_mutex_lock(&mutexProb);
  valor = (rand() % 100) + 1;
  pthread_mutex_unlock(&mutexProb);

  switch (Ciclistas[numero].v) {

    case 1:

      prob = 25;
      break;

    case 2:

      prob = 55;
      break;

    default:

      break;

  }

  if (valor > prob)
    return 2;
  else
    return 1;
}

void mudaPosicao(int Ciclista) {
  int m, n, mudanca = 0, volta;
  struct timeval tempoAtual;

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
    mudanca = 1;

    pthread_mutex_unlock(&sem[m][(n + 1) % d]);
  }else {
    
    pthread_mutex_unlock(&sem[m][(n + 1) % d]);

    for (int i = m + 1; i < 10 && !mudanca; i++) {

      pthread_mutex_lock(&sem[i][n]);
      pthread_mutex_lock(&sem[i][(n + 1) % d]);

      if (pista[i][n] == 0 && pista[i][(n + 1) % d] == 0) {

        Ciclistas[Ciclista].m = i;
        Ciclistas[Ciclista].n = (n + 1) % d;
        pista[i][(n + 1) % d] = Ciclista + 1;
        mudanca = 1;

      }

      pthread_mutex_unlock(&sem[i][(n + 1) % d]);
      pthread_mutex_unlock(&sem[i][n]);
    }
  }

  if (mudanca) {

    if ((n + 1) % d == 0) {

      Ciclistas[Ciclista].volta++;
      volta = Ciclistas[Ciclista].volta;
      Ciclistas[Ciclista].v = geraProbabilidades(Ciclista);

      gettimeofday(&tempoAtual, NULL);

      Ciclistas[Ciclista].tempo = tempoAtual.tv_sec - inicio.tv_sec;
      Ciclistas[Ciclista].tempo += (tempoAtual.tv_usec - inicio.tv_usec) / 1e6;
      
      pthread_mutex_lock(&semVolta[volta]);
      pilhas[volta] = push(pilhas[volta], Ciclista);
      pthread_mutex_unlock(&semVolta[volta]);
    }

    pista[m][n] = 0;
  }

  pthread_mutex_unlock(&sem[m][n]);
}

void *thread(void *numero) {
  int cont = 10;
  int tempoExecucao = 0;
  int tempoRestante = 0;
  int Ciclista = *((int *) numero);

  while (1) {

    switch (Ciclistas[Ciclista].v) {

      case 1:

        tempoRestante = (maxV ? 6 : 2);
        break;

      case 2:

        tempoRestante = (maxV ? 3 : 1);
        break;

      default:

        tempoRestante = 1;
        break;

    }

    while (tempoRestante != 0) {
      
      tempoRestante--;
      pthread_barrier_wait(&barreira);

      if (tempoRestante == 0)
        mudaPosicao(Ciclista);

      continua[Ciclista] = 0;
      pthread_barrier_wait(&barreira);

      while (!continua[Ciclista])
        usleep(100);

      if (Ciclistas[Ciclista].quebrado || Ciclistas[Ciclista].desclassificado)
        pthread_exit(NULL);

      if (!tempoExecucao && maxV) {

        tempoExecucao = 1;
        tempoRestante *= 3;

      }
    }

    
    cont--;
  }

  return NULL;
}

void decidePosicao(int restantes) {
  int atual;
  int sobrou;
  int probQuebrar;
  int volta = 1, m, n;
  int voltaCompleta;

  while (restantes > 1) {

    maxV = 0;

    if (maxV)
      usleep(20000);
    else
      usleep(60000);

    pthread_barrier_wait(&barreira);
    pthread_barrier_wait(&barreira);

    if (debug)
      imprimePista();

    sobrou = 0;

    if (restantes > 5) {

      for (int i = 0; i < k; i++) {

        probQuebrar = (rand() % 100) + 1;

        if (Ciclistas[i].volta == 0 || Ciclistas[i].quebrado || Ciclistas[i].desclassificado || Ciclistas[i].terminou)
          continue;
        
        if (probQuebrar <= 5 && Ciclistas[i].volta % 6 == 0 && Ciclistas[i].n == 0) {

          Ciclistas[i].quebrado = 1;

          if (!debug)
            printf("Ciclista %d ***Quebrou***\n", Ciclistas[i].numero);

          quebrados += 1;
          restantes--;

          pista[Ciclistas[i].m][Ciclistas[i].n] = 0;
          sobrou = 1;

          if (restantes == 5)
            break;
        }
      }
    }

    voltaCompleta = 1;

    for (int i = 0; i < k; i++)

      if (!Ciclistas[i].quebrado && !Ciclistas[i].desclassificado && Ciclistas[i].volta < volta) {

        voltaCompleta = 0;
        break;

      }
    

    if (voltaCompleta) {

      if (!debug)
        printf("\n\nVolta: %d\n", volta);

      if (strcmp("e", controle) == 0 && !debug)
        imprimePilha(pilhas[volta]);

      if (volta % 2 == 0){

        while (Ciclistas[top(pilhas[volta])].quebrado || Ciclistas[top(pilhas[volta])].desclassificado)
          pilhas[volta] = pop(pilhas[volta]);

        if (Ciclistas[top(pilhas[volta])].n == 0) {

          atual = top(pilhas[volta]);
          pilhas[volta] = pop(pilhas[volta]);

          Ciclistas[atual].desclassificado = 1;
          m = Ciclistas[atual].m;
          n = Ciclistas[atual].n;

          if (!debug) {

            printf("Ciclista %d eliminado -> ", atual + 1);
            printf("Tempo: %lf\n", Ciclistas[atual].tempo);  

          }

          restantes--;
          pista[m][n] = 0;
          sobrou = 1;

        }
      }

      volta += 1;
    }

    if (sobrou) {

      if (restantes > 1) {

        pthread_barrier_destroy(&barreira);
        pthread_barrier_init(&barreira, NULL, restantes + 1);

      }else {

        volta -= 1;

        printf("\n***Vencedor***: %d\n", top(pilhas[volta]) + 1);

        Ciclistas[top(pilhas[volta])].desclassificado = 1;

      }
    }

    for (int i = 0; i < k; i++)
      continua[i] = 1;
  }

  pthread_barrier_destroy(&barreira);
  printf("Qtd de quebrados: %d\n", quebrados);

}

int main(int argc, char ** argv) {
  int *numeros;

  d = atoi(argv[1]);
  k = atoi(argv[2]);
  controle = argv[3];

  for (int i = 4; i < argc; i++) {
    if (argv[i][0] == '-') 
      debug = 1;
  }

  srand(time(NULL));
  gettimeofday(&inicio, NULL);

  
  pilhas = malloc(2 * (k + 1) * sizeof(No *));
  Ciclistas = malloc(k * sizeof(Ciclista));
  threads = malloc(k * sizeof(pthread_t));
  
  numeros = malloc(k * sizeof(int));
  continua = malloc(k * sizeof(int));

  pthread_barrier_init(&barreira, NULL, k + 1);


  for (int i = 0; i < 2 * (k + 1); i++)
    pilhas[i] = NULL;

  for (int i = 0; i < k; i++)
    continua[i] = 0;

  mutex();
  geraPista();

  if (debug)
    imprimePista();

  for (int i = 0; i < k; i++) {

    numeros[i] = i;
    pthread_create(&threads[i], NULL, thread, &numeros[i]);

  }

  decidePosicao(k);

  if (debug)
    imprimePista();

  printf("\nPodio:\n");

  mergeSort(numeros, 0, k - 1, Ciclistas);
  imprimePosicao(numeros, k - 1);

  free(semVolta);
  free(ciclistasRestantes);
  free(threads);
  free(pilhas);
  free(Ciclistas);

  return 0;
}
