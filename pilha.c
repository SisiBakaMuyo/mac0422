#include "pilha.h"


int pilhaVazia(pilha *p){
	if(p->topo == 0) return 1;
	return 0;
}

pilha *criaPilha(int MAX){
	pilha *p;
	p = malloc (sizeof(pilha));
	p->v = malloc (MAX * sizeof(process));
	p->topo = 0;
	p->max = MAX;
	return p;
}

void insereOrdenado(pilha *p, process *x){
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

void empilha(pilha *p, process *x){
	if (p->topo < p->max){
		p->v[ p->topo ] = x;
		(p->topo)++;
	}
}

process *removeI(pilha *p, int i){
	if (!pilhaVazia(p)){
		process *x;
		int j;
		x = p->v[i];
		for (j = i; j < p->topo - 1; j++){
			p->v[j] = p->v[j + 1];
		}
		(p->topo)--;
		return x;
	}
	else
		printf("pilha vazia!\n");
	return NULL;
}

process *desempilha(pilha *p){
	if (pilhaVazia(p)){
		return NULL;
	}
	(p->topo)--;
	return (p->v[ p->topo ]);
}

process *topoPilha(pilha *p) {
    if (p->topo == 0)
        return NULL;
    return p->v[p->topo - 1];
}

void imprimePilha(pilha *p){
    int i;
    for (i = 0; i < p->topo; i++)
        printf("%s ", p->v[i]->name);
    printf("\n");
}

void destroiPilha(pilha *p){
	while ((p->topo) != 0) {
        free(p->v[p->topo - 1]);
        (p->topo)--;
    }
	free(p);
}
