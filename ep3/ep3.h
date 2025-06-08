// Isis Ardisson Logullo nUSP: 7577410

#include <stdio.h>

// Manipulador de arquivo
void geraSaida(FILE *arquivoEntrada);

// Gerenciadores
long getCabecalho();

long contaEspaco(int posicao);

int getBit(int posicao);

void setBit(int posicao, int bit);

void compactar();

// Algoritmos
int firstFit(int tam);

int nextFit(int tam);

int bestFit(int tam);

int worstFit(int tam);

int setAlgoritmo(int algoritmo, int tam);

