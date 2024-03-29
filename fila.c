#include "fila.h"

int filaVazia(fila *f) {
	if (f->tam == 0) return 1;
	return 0;
}

fila *criaFila(int MAX) {
	fila *f;
	f = malloc (sizeof(fila));
	f->v = malloc (MAX * sizeof(process));
	f->frente = 0;
	f->tam = 0;
	f->max = MAX;

	return f;
}

void insere(fila *f, process *x) {
	int i = (f->frente + f->tam) % f->max;

	f->v[i] = x;
	(f->tam)++;
}

process *removeFila(fila *f) {
	process *p;

	if (filaVazia(f))
		return NULL;

	p = f->v[f->frente];
	(f->frente) = ((f->frente) + 1) % f->max;
	(f->tam)--;

	return p;
}

process *topoFila(fila *f) {
	return f->v[f->frente];
}

void imprimeFila(fila *f) {
	int i;
    for (i = 0; i < f->tam; i++)
        printf("%s ", f->v[((f->frente) + 1) % f->max]->name);
    printf("\n");
}

void destroiFila(fila *f) {
	while ((f->tam) != 0) {
        free(f->v[((f->frente + f->tam) % f->max) - 1]);
        (f->tam)--;
    }
	free(f);
}