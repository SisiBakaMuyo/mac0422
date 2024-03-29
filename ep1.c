/* Isis Ardisson Logullo - 7577410*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "leitorArquivo.h"
#include "pilha.h"
#include "fila.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

int LINE_COUNT, context_changes = 0, finished_thread = 0, F_LINE = 0;
float STDQUANTUM = 0.5;
struct timeval starting_time;
FILE *f;

float get_time(){
	struct timeval tv, temp;

	gettimeofday(&tv, NULL);

	temp = starting_time;
	temp.tv_usec /= 10000;
	tv.tv_usec /= 10000;

	tv.tv_sec -= temp.tv_sec;
	if (tv.tv_usec > temp.tv_usec)
		tv.tv_usec -= temp.tv_usec;
	else
		tv.tv_usec += 1 - temp.tv_usec;

	return (float)(tv.tv_sec + (tv.tv_usec/100.0) - 0.01);
}

void *newThread(void* arg) {
	process *p;
	float t0, t1;

	t0 = t1 = get_time();
	p = (process *) arg;

	while (p->et > 0) {
		t1 = get_time();
		p->et -= (double)(t1 - t0);
		t0 = t1;
	}

	t1 = get_time();
	fprintf(f, "%s %f %f\n", p->name, t1, t1 - p->t0);
	pthread_mutex_lock(&mutex);
	F_LINE++;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *newQuantumThread(void* arg) {
	process *p;
	float t0, t1, elapsed = 0.0;

	t0 = t1 = get_time();
	p = (process *) arg;

	while (elapsed < p->quantum && p->et > 0){
		t1 = get_time();
		elapsed += t1 - t0;
		p->et -= t1 - t0;
		t0 = t1;
	}

	if (elapsed >= p->quantum) {
		pthread_mutex_lock(&mutex);
		context_changes++;
		pthread_mutex_unlock(&mutex);

		pthread_exit(NULL);
	}

	t1 = get_time();
	fprintf(f, "%s %f %f\n", p->name, t1, t1 - p->t0);
	pthread_mutex_lock(&mutex);
	finished_thread = 1;
	F_LINE++;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

process *lineToProcess(linha *l, int index, float quantum) {
	process *p = malloc(sizeof(process));;

	p->t0 = l->t0;
	p->dt = l->dt;
	p->et = l->dt;
	p->deadline = l->deadline;
	p->quantum = quantum;
	p->name = l->nome;
	p->i = index;

	return p;
}

void shortestJobFirst(linha **dados){
	int i = 0, th, pros_done = 0;
	float cur_time;
	pthread_t *threads = malloc(LINE_COUNT * sizeof(pthread_t));
	process *top_pros, **pros = malloc(LINE_COUNT * sizeof(process*));
	pilha *job_order;

	job_order = criaPilha(LINE_COUNT); 

	while (pros_done < LINE_COUNT) {
		cur_time = get_time();
		top_pros = topoPilha(job_order);
		
		if (!pilhaVazia(job_order)) {
			top_pros = desempilha(job_order);
			if ((th = pthread_create(&threads[top_pros->i], NULL, newThread, (void *) top_pros)))
				printf("Failed to create thread %d\n", th);
			pthread_join(threads[top_pros->i], NULL);
			pros_done++;
		}

		while (i < LINE_COUNT && cur_time >= dados[i]->t0) {
			top_pros = topoPilha(job_order);
			pros[i] = lineToProcess(dados[i], i, dados[i]->dt + 1); 
			insereOrdenado(job_order, pros[i]);
			i++;
		}
	}

	free(threads);
	for (i = 0; i < LINE_COUNT; i++)
		free(pros[i]);
	free(pros);
	destroiPilha(job_order);
}


void roundRobin(linha **dados) {
	int i = 0, j, th, pros_done = 0;
	float cur_time;
	pthread_t *threads = malloc(LINE_COUNT * sizeof(pthread_t));
	process *top_pros;
	fila *jobs;

	jobs = criaFila(LINE_COUNT);

	while (pros_done < LINE_COUNT) {
		cur_time = get_time();

		while (i < LINE_COUNT && cur_time >= dados[i]->t0){
			insere(jobs, lineToProcess(dados[i], i, STDQUANTUM));
			i++;
		}

		if (!filaVazia(jobs)){
			
			for (j = jobs->tam; j > 0; j--) {
				
				top_pros = removeFila(jobs);
				if ((th = pthread_create(&threads[top_pros->i], NULL, newQuantumThread, (void *) top_pros)))
					printf("Failed to create thread %d\n", th);
				pthread_join(threads[top_pros->i], NULL);

				if (!finished_thread)
					insere(jobs, top_pros);
				else 
					pros_done++;
				finished_thread = 0;
			}
		}
	}
	destroiFila(jobs);
}


float decideQuantum(process *p) {
	float time = get_time();
	if (p->t0 + p->dt >= p->deadline)
		return p->dt;
	if (p->et + time >= p->deadline)
		return p->et + 0.1;
	if (p->et < 2 * STDQUANTUM)
		return p->et + 0.1;
	if (p->et + time < p->deadline - 5)
		return STDQUANTUM;
	return STDQUANTUM;
}


void escalonamentoComPrioridade(linha **dados) {
	int i = 0, j, th, pros_done = 0;
	float cur_time;
	pthread_t *threads = malloc(LINE_COUNT * sizeof(pthread_t));
	process *top_pros, *new_pros;
	fila *jobs;

	jobs = criaFila(LINE_COUNT);

	while (pros_done < LINE_COUNT) {
		cur_time = get_time();

		while (i < LINE_COUNT && cur_time >= dados[i]->t0) {
			new_pros = lineToProcess(dados[i], i, STDQUANTUM/2);
			new_pros->quantum = decideQuantum(new_pros);
			insere(jobs, new_pros);

			i++;
		}

		if (!filaVazia(jobs)){
			for (j = jobs->tam; j > 0; j--) {
				top_pros = removeFila(jobs);
				top_pros->quantum = decideQuantum(top_pros);

				if ((th = pthread_create(&threads[top_pros->i], NULL, newQuantumThread, (void *) top_pros)))
					printf("Failed to create thread %d\n", th);
				pthread_join(threads[top_pros->i], NULL);

				if (!finished_thread)
					insere(jobs, top_pros);
				else 
					pros_done++;
				finished_thread = 0;
			}
		}
	}
	destroiFila(jobs);
}


void simulador(linha **dados, int tipo){
	if (tipo == 1)
		shortestJobFirst(dados);
	else if (tipo == 2)
		roundRobin(dados);
	else
		escalonamentoComPrioridade(dados);
}


int main(int argc, char **argv){
	struct timeval tv;
	int i;
	linha **dados;

	gettimeofday(&tv, NULL);
	starting_time = tv;
	dados = readFile(argv[2], &LINE_COUNT);
	f = fopen(argv[3], "w");

	simulador(dados, atoi(argv[1]));

	for (i = 0; i < LINE_COUNT; i++)
		free(dados[i]);
	free(dados);

	fprintf(f, "%d", context_changes);
	fclose(f);

	return 0;
}
