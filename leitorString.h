#ifndef LEITORSTRING_H
#define LEITORSTRING_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char **leLinha(char *line, int *lin);

char *leString(char *line, int init);

void freeMatrix(char **matrix, int lin);

#endif