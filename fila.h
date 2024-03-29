#ifndef FILA_H
#define FILA_H

#include "process.h"

typedef struct{
	process **v;
	int frente, tam, max;
} fila;

int filaVazia(fila *f);

fila *criaFila(int MAX);

void insere(fila *f, process *x);

process *removeFila(fila *f);

process *topoFila(fila *f);

void imprimeFila(fila *f);

void destroiFila(fila *f);

#endif