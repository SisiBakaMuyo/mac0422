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
#include "ep1.h"

FILE *f;
float QUANTUM = 0.5;
int CONTADOR_LINHA, F_LINHA = 0;
int mudancasContexto = 0, threadsFinalizadas = 0;
struct timeval iniciandoTempo;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t varCondicao = PTHREAD_COND_INITIALIZER;

/* pilha */
int pilhaVazia(pilha *p){
	if(p->topo == 0) return 1;
	return 0;
}

pilha *criaPilha(int MAX){
	pilha *p;
	p = malloc (sizeof(pilha));
	p->v = malloc (MAX * sizeof(processo));
	p->topo = 0;
	p->max = MAX;
	return p;
}

void insereOrdenado(pilha *p, processo *x){
    int i, j, ok = 0;

	if (p->topo < p->max) {
        if (p->topo != 0) {
    		for (i = 0; i < p->topo && ok == 0; i++) {
                if (p->v[i]->dt <= x->dt) {
                    for (j = p->topo; j >= i; j--) {
                        p->v[j+1] = p->v[j];
                    }
                    p->v[i] = x;
                    ok = 1;
                }
            }
            if (!ok) {
                p->v[p->topo] = x;
            }
        }
        else { 
            p->v[0] = x;
        }
        (p->topo)++;
	}
    else
        printf("pilha cheia!\n");
}

processo *desempilha(pilha *p){
	if (pilhaVazia(p)){
		return NULL;
	}
	(p->topo)--;
	return (p->v[ p->topo ]);
}

processo *topoPilha(pilha *p) {
    if (p->topo == 0)
        return NULL;
    return p->v[p->topo - 1];
}

void destroiPilha(pilha *p){
	while ((p->topo) != 0) {
        free(p->v[p->topo - 1]);
        (p->topo)--;
    }
	free(p);
}

/* fila */
int filaVazia(fila *f) {
	if (f->tam == 0) return 1;
	return 0;
}

fila *criaFila(int MAX) {
	fila *f;
	f = malloc (sizeof(fila));
	f->v = malloc (MAX * sizeof(processo));
	f->frente = 0;
	f->tam = 0;
	f->max = MAX;

	return f;
}

void insere(fila *f, processo *x) {
	int i = (f->frente + f->tam) % f->max;

	f->v[i] = x;
	(f->tam)++;
}

processo *remove(fila *f) {
	processo *p;

	if (filaVazia(f))
		return NULL;

	p = f->v[f->frente];
	(f->frente) = ((f->frente) + 1) % f->max;
	(f->tam)--;

	return p;
}

void destroiFila(fila *f) {
	while ((f->tam) != 0) {
        free(f->v[((f->frente + f->tam) % f->max) - 1]);
        (f->tam)--;
    }
	free(f);
}

/* processos */
float escolheQuantum(processo *p) {
	float t = getTempoExecucao();

	if (p->et < 2 * QUANTUM)
		return p->et + 0.1;
	if (p->et + t < p->deadline - 5)
		return QUANTUM;
	if (p->et + t >= p->deadline)
		return p->et + 0.1;
	if (p->t0 + p->dt >= p->deadline)
		return p->dt;
		
	return QUANTUM;
}

float getTempoExecucao(){
	/*
	*struct timeval {
	*	long tv_sec;  ->  seconds 
	*	long tv_usec; ->   microseconds 
	*};
	*/
	struct timeval tv, t;
	gettimeofday(&tv, NULL);

	t = iniciandoTempo;
	t.tv_usec /= 10000;
	tv.tv_usec /= 10000;

	tv.tv_sec -= t.tv_sec;
	if (tv.tv_usec > t.tv_usec)
		tv.tv_usec -= t.tv_usec;
	else
		tv.tv_usec += 1 - t.tv_usec;

	return (float)(tv.tv_sec + (tv.tv_usec/100.0) - 0.01);
}

void *criaThread(void* arg) {
	float t0, t1;
	processo *p = (processo *) arg;;

	t0 = t1 = getTempoExecucao();

	while (p->et > 0) {
		t1 = getTempoExecucao();
		p->et -= (double)(t1 - t0);
		t0 = t1;
	}

	t1 = getTempoExecucao();
	fprintf(f, "%s %f %f\n", p->nome, t1, t1 - p->t0);

	pthread_mutex_lock(&mutex);
	F_LINHA++;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *criaThreadQuantum(void* arg) {
	processo *p;
	float t0, t1, passado = 0.0;

	t0 = t1 = getTempoExecucao();
	p = (processo *) arg;

	while (passado < p->quantum && p->et > 0){
		t1 = getTempoExecucao();
		passado += t1 - t0;
		p->et -= t1 - t0;
		t0 = t1;
	}

	if (passado >= p->quantum) {
		pthread_mutex_lock(&mutex);
		mudancasContexto++;
		pthread_mutex_unlock(&mutex);

		pthread_exit(NULL);
	}

	t1 = getTempoExecucao();
	fprintf(f, "%s %f %f\n", p->nome, t1, t1 - p->t0);

	pthread_mutex_lock(&mutex);
	threadsFinalizadas = 1;
	F_LINHA++;
	pthread_mutex_unlock(&mutex);

	return NULL;
}

processo *criaProcessoLinha(linha *l, int i, float quantum) {
	processo *p = malloc(sizeof(processo));;

	p->t0 = l->t0;
	p->dt = l->dt;
	p->et = l->dt;
	p->deadline = l->deadline;
	p->quantum = quantum;
	p->nome = l->nome;
	p->i = i;

	return p;
}

void shortestJobFirst(linha **dados){
	float t;
	pilha *ordenador;
	int i = 0, posicao, processosFinalizados = 0;
	pthread_t *threads = malloc(CONTADOR_LINHA * sizeof(pthread_t));
	processo *processoTopo, **procs = malloc(CONTADOR_LINHA * sizeof(processo*));

	ordenador = criaPilha(CONTADOR_LINHA); 

	while (processosFinalizados < CONTADOR_LINHA) {
		t = getTempoExecucao();
		processoTopo = topoPilha(ordenador);
		
		if (!pilhaVazia(ordenador)) {
			processoTopo = desempilha(ordenador);
			if ((posicao = pthread_create(&threads[processoTopo->i], NULL, criaThread, (void *) processoTopo)))
				printf("Erro ao criar thread %d\n", posicao);

			pthread_join(threads[processoTopo->i], NULL);
			processosFinalizados++;
		}

		while (i < CONTADOR_LINHA && t >= dados[i]->t0) {
			processoTopo = topoPilha(ordenador);
			procs[i] = criaProcessoLinha(dados[i], i, dados[i]->dt + 1); 
			insereOrdenado(ordenador, procs[i]);
			i++;
		}
	}

	free(threads);

	for (i = 0; i < CONTADOR_LINHA; i++)
		free(procs[i]);

	free(procs);
	destroiPilha(ordenador);
}

void roundRobin(linha **dados) {
	float t;
	fila *procs;
	int i = 0, j;
	processo *processoTopo;
	int posicao, processosFinalizados = 0;
	pthread_t *threads = malloc(CONTADOR_LINHA * sizeof(pthread_t));

	procs = criaFila(CONTADOR_LINHA);

	while (processosFinalizados < CONTADOR_LINHA) {
		t = getTempoExecucao();

		while (i < CONTADOR_LINHA && t >= dados[i]->t0){
			insere(procs, criaProcessoLinha(dados[i], i, QUANTUM));
			i++;
		}

		if (!filaVazia(procs)){
			for (j = procs->tam; j > 0; j--) {
				processoTopo = remove(procs);

				if ((posicao = pthread_create(&threads[processoTopo->i], NULL, criaThreadQuantum, (void *) processoTopo)))
					printf("Erro ao criar thread %d\n", posicao);

				pthread_join(threads[processoTopo->i], NULL);

				if (!threadsFinalizadas)
					insere(procs, processoTopo);
				else 
					processosFinalizados++;

				threadsFinalizadas = 0;
			}
		}
	}
	destroiFila(procs);
}

void escalonamentoComPrioridade(linha **dados) {
	float t;
	fila *procs;
	int i = 0, j;
	processo *processoTopo, *novosProcs;
	int posicao, processosFinalizados = 0;
	pthread_t *threads = malloc(CONTADOR_LINHA * sizeof(pthread_t));

	procs = criaFila(CONTADOR_LINHA);

	while (processosFinalizados < CONTADOR_LINHA) {
		t = getTempoExecucao();

		while (i < CONTADOR_LINHA && t >= dados[i]->t0) {
			novosProcs = criaProcessoLinha(dados[i], i, QUANTUM/2);
			novosProcs->quantum = escolheQuantum(novosProcs);
			insere(procs, novosProcs);
			i++;
		}

		if (!filaVazia(procs)){
			for (j = procs->tam; j > 0; j--) {
				processoTopo = remove(procs);
				processoTopo->quantum = escolheQuantum(processoTopo);

				if ((posicao = pthread_create(&threads[processoTopo->i], NULL, criaThreadQuantum, (void *) processoTopo)))
					printf("Failed to create thread %d\n", posicao);

				pthread_join(threads[processoTopo->i], NULL);

				if (!threadsFinalizadas)
					insere(procs, processoTopo);
				else 
					processosFinalizados++;

				threadsFinalizadas = 0;
			}
		}
	}
	destroiFila(procs);
}

int main(int argc, char **argv){
	linha **dados;
	struct timeval tv;
	int i, escalonador;
	
	gettimeofday(&tv, NULL);
	iniciandoTempo = tv;
	dados = readFile(argv[2], &CONTADOR_LINHA);
	f = fopen(argv[3], "w");
	escalonador = atoi(argv[1]);

	if (escalonador == 1)
		shortestJobFirst(dados);
	else if (escalonador == 2)
		roundRobin(dados);
	else
		escalonamentoComPrioridade(dados);

	for (i = 0; i < CONTADOR_LINHA; i++)
		free(dados[i]);
	free(dados);

	fprintf(f, "%d", mudancasContexto);
	fclose(f);

	return 0;
}
