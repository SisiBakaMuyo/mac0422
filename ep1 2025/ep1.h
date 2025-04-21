/* Isis Ardisson Logullo 7577410 */

#include <pthread.h>

typedef struct processo {
  int t0;
  int dt;
  int tf;
  int deadline;
  int i;
  long int dtt;
  char nome[32];
  pthread_t thread;
} Processo;

typedef struct no {
    Processo data;
    struct no *proximo;
    struct no *anterior;
} No;

typedef struct fila {
    No *ini;
    No *fim;
} Fila;


//fila
void criaFila(Fila *fila);

void insereFila(Fila *fila, Processo data);

int filaVazia(Fila *fila);

Processo removeFila(Fila *fila);

//heap
void insereHeap(Processo *heap, int *tamanho, Processo valor);

void transformacao(Processo *heap, int *tamanho, int i);

void removeMinimoHeap(Processo *heap, int *tamanho);

//thread
void * thread(void * atual);

void pthread();

//semaforo
void mutex();

void freeMutex();

//escalonador
void fcfs();

void srtn();

void prioridade();

