#ifndef PILHA_H
#define PILHA_H

#include "process.h"

typedef struct{
	process **v;
	int topo, max;
} pilha;

int pilhaVazia(pilha *p);

pilha *criaPilha(int MAX);

void insereOrdenado(pilha *p, process *x);

void empilha(pilha *p, process *x);

process *removeI(pilha *p, int i);

process *desempilha(pilha *p);

process *topoPilha(pilha *p);

void imprimePilha(pilha *p);

void destroiPilha(pilha *p);

#endif
