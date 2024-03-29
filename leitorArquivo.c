#include "leitorArquivo.h"


linha *criaLinha(int n) {
	linha *l;
	l = malloc(sizeof(linha));
	l->nome = malloc(n * sizeof(char));
	return l;
}

linha **leArquivo(char *nome_arquivo, int *contador_linha) {
	int elementos_linha = 1; 
	int i, k;
	char c;
	char buf[MAX_LINE_SIZE / 4];
	linha **dados;
	FILE *arquivo;

	dados = malloc(sizeof(linha*));
	dados[0] = crialinha(MAX_LINE_SIZE);

	arquivo = fopen(nome_arquivo, "r");

	if (!arquivo) {
		printf("Erro\n");
		return NULL;
	}

	i = k = 0; 

	while( (c = getc(arquivo)) != EOF ) {
		if (c == ' ') { 
			buf[i] = '\0';
			
			if (elementos_linha == 1)
				dados[k]->t0 = atof(buf);
			else if (elementos_linha == 2)
				dados[k]->dt = atof(buf);
			else 
				dados[k]->deadline = atof(buf);

			elementos_linha++;
			i = 0;
		}
		else if (c == '\n') { 
			buf[i] = '\0';
			strcpy(dados[k]->nome, buf);
			i = 0;
			elementos_linha = 1;
			k++;
			dados = realloc(dados, (k+1)*sizeof(linha));
			dados[k] = crialinha(MAX_LINE_SIZE / 4);
		}
		else{
			buf[i] = c;
			i++;
		}
	}

	buf[i] = '\0';
	strcpy(dados[k]->nome, buf);
	k++;
	*contador_linha = k;
	fclose(arquivo);

	return dados;
}
