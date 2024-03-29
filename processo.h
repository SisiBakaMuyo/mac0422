#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>

typedef struct{
	float t0; 
	float dt; 
	float et; 
	float deadline;
	float quantum;
	char *name;
	int i;
} process;

#endif