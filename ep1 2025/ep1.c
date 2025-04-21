/* Isis Ardisson Logullo 7577410 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include "ep1.h"


int tamanho;
int escalonador;
int mudancaContexto;
long int tempoExecucao;
int cumpriu;
pthread_mutex_t *sem;
pthread_mutex_t *empty;
pthread_mutex_t *full;
Processo *processos;

//leitura
void leArquivo(FILE * arquivo, int max) {
  int i;

  while (!feof(arquivo)) {
    if (fscanf(arquivo, "%s %d %d %d", processos[tamanho].nome, &(processos[tamanho].t0), &(processos[tamanho].dt), &(processos[tamanho].deadline)) != 4) {
      continue;
    }
    processos[tamanho].dtt = 0;
    processos[tamanho].i = tamanho;

    tamanho++;
    if (tamanho == max) {
      pthread_mutex_t *aux_sem, *aux_empty, *aux_full;
      Processo *aux_processos;

      max *= 2;

      aux_processos = malloc(max * sizeof(Processo));
      aux_sem = malloc(max * sizeof(pthread_mutex_t));
      aux_empty = malloc(max * sizeof(pthread_mutex_t));
      aux_full = malloc(max * sizeof(pthread_mutex_t));

      for (i = 0; i < tamanho; i++) {
        aux_processos[i] = processos[i];
      }

      free(processos);
      free(sem);
      free(empty);
      free(full);

      processos = aux_processos;
      sem = aux_sem;
      empty = aux_empty;
      full = aux_full;
    }
  }
  
  fclose(arquivo);
}

//fila
void criaFila(Fila *fila) {
  fila->ini = fila->fim = NULL;
}

void insereFila(Fila *fila, Processo data) {
  No *novo;
  novo = malloc(sizeof(No));
  novo->anterior = NULL;
  novo->data = data;
  
  if (fila->fim != NULL) {
    novo->proximo = fila->fim;
    fila->fim->anterior = novo;
  }
  else {
    novo->proximo = NULL;
    fila->ini = novo;
  }

  fila->fim = novo;
}

int filaVazia(Fila *fila) {
  return fila->ini == NULL;
}

Processo removeFila(Fila *fila) {
  Processo data = fila->ini->data;

  if (fila->fim != fila->ini) {
    fila->ini = fila->ini->anterior;
    free(fila->ini->proximo);
    fila->ini->proximo = NULL;
  }
  else {
    free(fila->fim);
    fila->fim = fila->ini = NULL;
  }

  return data;
}

//heap
void insereHeap(Processo *heap, int *tamanho, Processo valor) {
  int i = *tamanho;
  *tamanho = *tamanho + 1;

  heap[i] = valor;

  while (i != 0 && heap[i].dt < heap[(i - 1)/2].dt) {
    Processo x = heap[i];
    heap[i] = heap[(i - 1)/2];
    heap[(i - 1)/2] = x;

    i = (i - 1)/2;
  }
}

void transformaHeap(Processo *heap, int *tamanho, int i) {
  int menor = i;
  
  if (2 * i + 1 < *tamanho && heap[2 * i + 1].dt < heap[i].dt)
    menor = 2 * i + 1;

  if (2 * i + 2 < *tamanho && heap[2 * i + 2].dt < heap[menor].dt)
    menor = 2 * i + 2;

  if (menor != i) {
    Processo x = heap[menor];
    heap[menor] = heap[i];
    heap[i] = x;

    transformaHeap(heap, tamanho, menor);
  }
}

void removeHeap(Processo *heap, int *tamanho) {
  if (*tamanho == 0) {
    printf("Vazio\n");
    return;
  }

  *tamanho = *tamanho - 1;
  heap[0] = heap[*tamanho];
  transformaHeap(heap, tamanho, 0);
}

//thread
void * thread(void * atual) {
  int at = *((int *) atual);
  int i = 0;
  
  if (escalonador != 1) {
    while (processos[at].dt > 0 || processos[at].dtt > 0) {
      pthread_mutex_lock(&empty[at]);

      usleep(tempoExecucao);
      if (tempoExecucao > processos[at].dtt) {
        processos[at].dt--;
        processos[at].dtt += 1000000 - tempoExecucao;
      }
      else {
        processos[at].dtt -= tempoExecucao;
      }

      i = 2 * 2;

      pthread_mutex_unlock(&full[at]);
    }
  }
  else {
    pthread_mutex_lock(&sem[at]);

    while (processos[at].dt > 0) {
      sleep(1);
      processos[at].dt--;
      i = 2 * 2;
    }
  }

  return NULL;
}

void pthread() {
  int i;

  for (i = 0; i < tamanho; i++) {
    if (pthread_create(&processos[i].thread, NULL, thread, (void *) &processos[i].i)) {
      printf("\n ERROR");
      exit(1);
    }
  }
}

//semaforo
void mutex() {
  int i;

  if (escalonador != 1) {
    for (i = 0; i < tamanho; i++) {
      pthread_mutex_init(&empty[i], NULL);
      pthread_mutex_init(&full[i], NULL);
      pthread_mutex_lock(&full[i]);
      pthread_mutex_lock(&empty[i]);
    }
  }
  else {
    for (i = 0; i < tamanho; i++) {
      pthread_mutex_init(&sem[i], NULL);
      pthread_mutex_lock(&sem[i]);
    }
  }
}

void freeMutex() {
  int i;

  if (escalonador != 1) {
    for (i = 0; i < tamanho; i++) {
      pthread_join(processos[i].thread, NULL);
      pthread_mutex_destroy(&empty[i]);
      pthread_mutex_destroy(&full[i]);
    }
  }
  else {
    for (i = 0; i < tamanho; i++) {
      pthread_join(processos[i].thread, NULL);
      pthread_mutex_destroy(&sem[i]);
    }
  }
}

//quantum
int comparaQuantum(suseconds_t inicio2, suseconds_t fim2, double inicio, double fim) {
  if (inicio < fim) return 1;
  
  return (inicio == fim && inicio2 < fim2);
}

//escalonador
void fcfs() {
  int i;
  int at;
  int rodando = 0;
  int t = 0;
  time_t inicio;

  mutex();
  pthread();

  at = -1;
  i = -1;
  inicio = time(NULL);

  while (1) {
    if (difftime(time(NULL), inicio) >= t) {
      while (processos[i+1].t0 == t) {
        i++;  
      }
      t++;
    }
    if (rodando && processos[at].dt == 0) {
      processos[at].tf = difftime(time(NULL),inicio);
      
      rodando = 0;
      if (at + 1 <= i) {
        at++;
        mudancaContexto += 1;
        pthread_mutex_unlock(&sem[at]);
        rodando = 1;
      }
    }
    if (!rodando) {
      if (at + 1 <= i) {
        at++;
        pthread_mutex_unlock(&sem[at]);
        rodando = 1;
      }
    }
    if (!rodando && at + 1 == tamanho) break;
  }

  freeMutex();
}

void srtn() {
  int i, j;
  int atual;
  int tamHeap = 0;
  int rodando = 0;
  int t = 0;
  time_t inicio;
  Processo heap[10000];

  mutex();
  pthread();

  inicio = time(NULL);
  i = 0;
  tempoExecucao = 1000000.0;

  while (1) {
    if ((int) difftime(time(NULL), inicio) >= t) {

      for (j = i; j < tamanho && processos[j].t0 == t; j++) {
        insereHeap(heap, &tamHeap, processos[j]);
      }

      i = j;
      if (rodando) {
        pthread_mutex_lock(&full[atual]);

        if (processos[atual].dt == 0) {

          rodando = 0;
          pthread_cancel(processos[atual].thread);
          processos[atual].tf = difftime(time(NULL),inicio);

          if (tamHeap > 0) {

            atual = heap[0].i;
            removeHeap(heap, &tamHeap);
            mudancaContexto += 1;
            rodando = 1;
          }
        }
        else if (tamHeap > 0 && heap[0].dt < processos[atual].dt) {

          int anterior = atual;
          atual = heap[0].i;
          removeHeap(heap, &tamHeap);
          insereHeap(heap, &tamHeap, processos[anterior]);
          mudancaContexto += 1;
        }

        pthread_mutex_unlock(&empty[atual]);
      }
      else {
        if (tamHeap > 0) {

          atual = heap[0].i;
          removeHeap(heap, &tamHeap);
          pthread_mutex_unlock(&empty[atual]);
          rodando = 1;
        }
      }

      t++;
    }

    if (tamHeap == 0 && !rodando && i == tamanho)
      break;
  }

  freeMutex();
}

void prioridade() {
  int i, j;
  int atual;
  int prioridade[tamanho];
  int rodando = 0;
  int t = 0;
  long int tt = 0;
  Fila fila;
  struct timeval inicio, actual;

  mutex();
  pthread();

  for (i = 0; i < tamanho; i++){
    prioridade[i] = 1;
  }

  i = 0;
  criaFila(&fila);
  tempoExecucao = 500000;
  gettimeofday(&inicio, NULL);

  while (1) {
    gettimeofday(&actual, NULL);

    if (comparaQuantum(tt, actual.tv_usec - inicio.tv_usec, t, difftime(actual.tv_sec, inicio.tv_sec))) {
      
      for (j = i; j < tamanho && processos[j].t0 == t; j++) {
        insereFila(&fila, processos[j]);
      }

      i = j;
      if (rodando) {
        pthread_mutex_lock(&full[atual]);

        if (processos[atual].dt == 0 && processos[atual].dtt == 0) {

          rodando = 0;
          pthread_cancel(processos[atual].thread);
          processos[atual].tf = difftime(time(NULL),inicio.tv_sec);
      
          if (!filaVazia(&fila)) {

            atual = removeFila(&fila).i;
            prioridade[atual] = prioridade[atual] + 1;
            rodando = 1;
            mudancaContexto += 1;
          }
        }
        else if (!filaVazia(&fila)) {

          int anterior = atual;
          atual = removeFila(&fila).i;
          prioridade[atual] = prioridade[atual] + 1;
          insereFila(&fila, processos[anterior]);
          mudancaContexto += 1;
        }

        pthread_mutex_unlock(&empty[atual]);
      }
      else {
        if (!filaVazia(&fila)) {

          atual = removeFila(&fila).i;
          prioridade[atual] = prioridade[atual] + 1;
          pthread_mutex_unlock(&empty[atual]);
          rodando = 1;
        }
        else if (i == tamanho) 
          break;
      }
      if (tempoExecucao + tt >= 1000000.0) {
        t++;
        tt += tempoExecucao - 1000000.0;
        // tt += prioridade[atual] * 10;
      }
      else {
        tt += tempoExecucao;
        // tt += prioridade[atual] * 10;
      }
    }
  }

  freeMutex();
}

int main(int argc, char **argv) {
  
  int i;
  int max = 10000;
  tamanho = 0;
  char * nomeArquivo;
  char * nomeArquivoSaida;
  FILE * arquivo;
  FILE * arquivoSaida;

  escalonador = atoi(argv[1]);
  nomeArquivo = argv[2];
  nomeArquivoSaida = argv[3];

  arquivo = fopen(nomeArquivo, "r");

  processos = malloc(max * sizeof(Processo));
  sem = malloc(max * sizeof(Processo));
  empty = malloc(max * sizeof(Processo));
  full = malloc(max * sizeof(Processo));
 
  leArquivo(arquivo, max);
  mudancaContexto = 0;

  switch(escalonador) {
    case 1:
      fcfs();
      break;
    case 2:
      srtn();
      break;
    case 3:
      prioridade();
      break;
    default:
      printf("ERROR\n");
      exit(1);
      break;
  }

  arquivoSaida = fopen(nomeArquivoSaida, "w");
  
  for (i = 0; i < tamanho; i++) {
    if (processos[i].tf <= processos[i].deadline)
		cumpriu = 1;
		else 
		cumpriu = 0;

    fprintf(arquivoSaida, "%s %d %d %d\n", processos[i].nome, processos[i].tf - processos[i].t0, processos[i].tf, cumpriu);
  }

  fprintf(arquivoSaida, "Quantidade de mudancas de contexto: %d\n", mudancaContexto);
  fclose(arquivoSaida);
  free(processos);

  return 0;
}
