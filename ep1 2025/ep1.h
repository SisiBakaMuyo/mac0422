/* Isis Ardisson Logullo - 7577410*/

#include <pthread.h>

typedef struct processo {
	char *nome;
	int t0;
	int dt;
	long int t;
	int tf;
	int deadline;
	int i;
	int prioridade;
	pthread_t thread;
  } Processo;

  typedef struct no {
	Processo data;
	struct no *next;
	struct no *pre;
  } No;
  
  typedef struct fila {
	No *ini;
	No *fim;
  } Fila;


void insereHeap(Processo *heap, int *tamanho, Processo valor);

void transformacao(Processo *heap, int *tamanho, int i);

void removeMenor(Processo *heap, int *tamanho);

void * thread(void * atual);

void pthread();

void mutex();

void freeMutex();

void fcfs();

void srtn();

void prioridade();









