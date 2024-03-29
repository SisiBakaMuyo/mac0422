#ifndef FILEREADER_H
#define FILEREADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

typedef struct{
	float t0;
	float dt;
	float deadline;
	char *nome;
} linha;

linha *criaLinha(int n);

linha **leArquivo(char  *nome_arquivo, int *contador_linha);

#endif
