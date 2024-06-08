/* Isis Ardisson Logullo - 7577410*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ep3.h"


int fat[8*MAPSIZE];
int desperdicado;
int livres = 8*MAPSIZE;
int qtdArq = 0, qtdDir = 0; 
unsigned char bitmap[MAPSIZE]; 
FILE *unidade;


float timedifference_msec(struct timeval t0, struct timeval t1){
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

void addDiretorio(){
  qtdDir++;
  fseek(unidade, QTDDIRSEEK, SEEK_SET);
  fwrite(&qtdDir, sizeof(int), 1, unidade);
}

void removeDiretorio(){
  qtdDir--;
  fseek(unidade, QTDDIRSEEK, SEEK_SET);
  fwrite(&qtdDir, sizeof(int), 1, unidade);
}

void addArquivo(){
  qtdArq++;
  fseek(unidade, QTDARQSEEK, SEEK_SET);
  fwrite(&qtdArq, sizeof(int), 1, unidade);
}

void removeArquivo(){
  qtdArq--;
  fseek(unidade, QTDARQSEEK, SEEK_SET);
  fwrite(&qtdArq, sizeof(int), 1, unidade);
}

char **geraTokens(char *str, char *separator) {
  char **tokens = NULL;
  int i;
  
  tokens = malloc(sizeof (char*) * MAXCHAR);

  tokens[0] = strtok(str, separator);

  for(i = 1; tokens[i - 1] != NULL; i++) {
    tokens[i] = strtok(NULL, separator);
  }
  return tokens;
}

unsigned char setBit(int posicao, unsigned char byte, int bit) {
	unsigned char setado;

	setado = 1;
	setado <<= posicao;

	if (bit == 0) setado = ~setado;

	if (bit == 1) setado = setado|byte;
	else setado = setado&byte;

	return setado;
}

void setBloco(int bloco, int bit){
	int i, j;
	i = bloco/8;
	j = bloco%8;

	bitmap[i] = setBit(j, bitmap[i], bit);
	fseek(unidade, i, SEEK_SET);
	fwrite(&bitmap[i], sizeof(bitmap[i]), 1, unidade);
}

int devolveBit(int posicao, unsigned char byte) {
	byte >>= posicao;
	return byte%2;
}

int procuraBloco(){
  int i, j, livre = -1;
  for (i = 0; (i < MAPSIZE) && (livre < 0); i++){
    for (j = 0; (j < 8) && (livre < 0); j++){
      if (devolveBit(j, bitmap[i]) == 0)
        livre = 8*i+j;
    }
  }
  return livre;
}

int quantidadeBlocos(int bloco0){
	int i = 0;

	while (bloco0 != -1){
		bloco0 = fat[bloco0];
		i++;
	}

	return i;
}

void setFAT(int conteudo, int posicao){
	fat[posicao] = conteudo;
	fseek(unidade, BLOCKSIZE+4*posicao, SEEK_SET);
	fwrite(&fat[posicao], sizeof(int), 1, unidade);
}

void imprimeBitmap (){
	int i, j;
	for(i = 0; i < MAPSIZE; i++){
		for(j = 0; j < 8; j++)
			printf("%d", devolveBit(j, bitmap[i]));
	}
	printf("\n");

}

Arquivo leArquivo(int i){
  Arquivo arq;

  fseek(unidade, i, SEEK_SET);
  fread(&arq, sizeof(Arquivo), 1, unidade);
  return arq;
}

int getEndArquivoRec(char **paradas, int i, Arquivo diretorio, int endereco,  int option){
  Arquivo arq;
  int tamanho, bloco;
  int j = 0;

  diretorio.instanteAcessado = time(NULL);

  if (diretorio.diretorio >=0)  {
    diretorio.instanteModificado = diretorio.instanteAcessado;
  }

  fseek(unidade, endereco, SEEK_SET);
  fwrite(&diretorio, sizeof(Arquivo), 1, unidade);

  switch (option){
    case FINAL:
      if(paradas[i] == NULL) return endereco;
      break;
    case PAI:
      if(paradas[i+1] == NULL) return endereco;
      break;
    default:
      break;
  }

  bloco = diretorio.bloco;

  fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);

  tamanho = diretorio.diretorio;

  while (j < tamanho){

    if(j > ARQPERBLOCK) {
      j = 0;
      tamanho -= ARQPERBLOCK;
      bloco = fat[bloco];

      fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
    }

    fread(&arq, sizeof(Arquivo), 1, unidade);

    if (strcmp(paradas[i], arq.nome) == 0) {
        return getEndArquivoRec(paradas, i+1, arq, bloco*BLOCKSIZE + j*sizeof(Arquivo), option);
    }

    j++;

  }

  return -1;
}

int getArquivo(char *caminho, int option){
  Arquivo root;
  char **paradas = NULL, *path;
  int end;

  path = malloc(sizeof(char)*MAXCHAR);
  strcpy(path, caminho);

  fseek(unidade, ROOTSEEK, SEEK_SET);
  fread(&root, sizeof(Arquivo), 1, unidade);

  paradas = geraTokens(path, "/");
  end = getEndArquivoRec(paradas, 0, root, ROOTSEEK, option);

  free(paradas);
  free(path);

  return end;
}

void mostraArquivo(char *caminho){
  Arquivo arq;
  int i, tamanho, bloco;
  char c;

  i = getArquivo(caminho, FINAL);

  if(i == -1) {
    printf("ERRO - Arquivo nao encontrado.\n");
    return;
  }

  arq = leArquivo(i);
  tamanho = arq.tamBytes;
  bloco = arq.bloco;

  while(bloco != -1){
    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    bloco = fat[bloco];

    for(i = 0; i < tamanho && i < BLOCKSIZE; i++){
      fread(&c, sizeof(char), 1, unidade);
      printf("%c", c);
    }

    tamanho -= BLOCKSIZE;
  }

  printf("\n");
}

void copiaArquivo(char *origem, char *destino){
  Arquivo dir, novo, arq;
  FILE *file_origem = NULL;
  int i, bloco, tamanho;
  int newblock = FALSE;
  int end, arquivos;
  char c, **paradas = NULL;

  file_origem = fopen(origem, "r");

  if(file_origem == NULL){
    printf("ERRO - Arquivo nao foi aberto");
    return;
  }

  fseek(file_origem, 0, SEEK_END);
  tamanho = ftell(file_origem);

  end = getArquivo(destino, FINAL);
  dir = leArquivo(end);

  if(end == -1){
    printf("ERRO - Nao existe esse diretorio.\n");
    return;
  }
  else if(dir.diretorio < 0){
    printf("ERRO- O destino precisa ser um diretorio.\n");
    return;
  }

  bloco = dir.bloco;

  i = 1;
  while(fat[bloco] != -1){
   bloco = fat[bloco];
   i++;
  }

  if(dir.diretorio == ARQPERBLOCK*i) {
    newblock = TRUE;
  }

  if (tamanho/BLOCKSIZE + newblock > livres){
    printf("ERRO - Falta espaço no bloco.\n");
    fclose(file_origem);
    return;
  }

  if(newblock != 0){

    newblock = procuraBloco();
    setBloco(newblock, 1);
    livres--;
    setFAT(newblock, fat[bloco]);
    setFAT(-1, fat[newblock]);
    bloco = newblock;
    desperdicado += (BLOCKSIZE - sizeof(Arquivo));

  } else {
    desperdicado -= sizeof(Arquivo);
  }

  paradas = geraTokens(origem, "/");

  for(i = 0; paradas[i+1] != NULL; i++);

  strcpy(novo.nome, paradas[i]);

  novo.tamBytes = tamanho;
  novo.instanteCriado = time(NULL);
  novo.instanteModificado = novo.instanteCriado;
  novo.instanteAcessado = novo.instanteCriado;
  novo.bloco = procuraBloco(bitmap);
  novo.diretorio = -1;

  if(novo.tamBytes%BLOCKSIZE != 0) {
    desperdicado += BLOCKSIZE - (novo.tamBytes%BLOCKSIZE);
  }

  if(newblock != 0){
    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

    fwrite(&novo, sizeof(novo), 1, unidade);

  } else{

    bloco = dir.bloco;
    arquivos = dir.diretorio;

    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

    for(i = 0; i < arquivos; i++){

      if (i > ARQPERBLOCK){
        bloco = fat[bloco];
        fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
        i = 0;
        arquivos -= ARQPERBLOCK;
      }

      fread(&arq, sizeof(Arquivo), 1, unidade);

      if(strlen(arq.nome) == 0){ 
        break;
      }
    }

    fseek(unidade, bloco*BLOCKSIZE + i*sizeof(Arquivo), SEEK_SET);
    fwrite(&novo, sizeof(novo), 1, unidade);
  }

  dir.tamBytes += sizeof(Arquivo);
  dir.diretorio++;

  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(dir), 1, unidade);

  bloco = novo.bloco;

  setBloco(novo.bloco, 1);
  livres--;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
  fseek(file_origem, 0, SEEK_SET);

  for(i = 0; i < tamanho; i++){

    if(i >= BLOCKSIZE){
      tamanho -= BLOCKSIZE;
      i = 0;
      newblock = procuraBloco(bitmap);

      setFAT(newblock, bloco);
      setFAT(-1, newblock);
      setBloco(newblock, 1);
      livres--;
      bloco = newblock;

      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&c, sizeof(char), 1, file_origem);
    fwrite(&c, sizeof(char), 1, unidade);
  }

  addArquivo();
  fseek(unidade, WASTESEEK, SEEK_SET);

  fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);

  fclose(file_origem);
}

void tocaArquivo(char *caminho){
  Arquivo arq, dir;
  char **paradas = NULL;
  char str[MAXCHAR];
  int end, arquivos, bloco, i, j, endvago = -1, newblock;

  end = getArquivo(caminho, PAI);
  dir = leArquivo(end);
  paradas = geraTokens(caminho, "/");

  for(i = 0; paradas[i+1] != NULL; i++);

  strcpy(str, paradas[i]);

  bloco = dir.bloco;
  arquivos = dir.diretorio;

  j = 0;
  i = 0;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

  while (i < arquivos){

    if (j > ARQPERBLOCK){
      j = 0;
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&arq, sizeof(Arquivo), 1, unidade);

    if(strlen(arq.nome) != 0){

      if (strcmp(arq.nome, str) == 0){

        arq.instanteAcessado = dir.instanteAcessado;

        fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
        fwrite(&arq, sizeof(Arquivo), 1, unidade);

        return;
      }

      i++;

    } else if (endvago == -1) {
        endvago = bloco*BLOCKSIZE + j*sizeof(Arquivo);
    }

    j++;

  }

  strcpy(arq.nome, str);

  arq.tamBytes = 0;
  arq.instanteCriado = dir.instanteAcessado;
  arq.instanteModificado = arq.instanteCriado;
  arq.instanteAcessado = arq.instanteCriado;
  arq.diretorio = -1;

  if ((j == ARQPERBLOCK) && (endvago != -1)){ 

    if (livres < 2){
      printf("ERRO - Falta espaco no bloco.\n");
      return;
    }

    newblock = procuraBloco();

    setFAT(newblock, bloco);
    setFAT(-1, newblock);
    setBloco(newblock, 1);

    livres--;
    bloco = newblock;

    fseek(unidade, newblock*BLOCKSIZE, SEEK_SET);

    desperdicado += (BLOCKSIZE - sizeof(Arquivo));

  } else {

    if (livres < 1){
      printf("ERRO - Falta espaco no bloco.\n");
      return;
    }

    if (endvago == -1)  {
        fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
    } else {
        fseek(unidade, endvago, SEEK_SET);
    }

    desperdicado -= sizeof(Arquivo);

  }

  dir.diretorio++;
  dir.tamBytes += sizeof(Arquivo);

  newblock = procuraBloco();

  arq.bloco = newblock;

  fwrite(&arq, sizeof(Arquivo), 1, unidade);
  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(Arquivo), 1, unidade);

  setBloco(newblock, 1);

  livres--;

  addArquivo();

  fseek(unidade, WASTESEEK, SEEK_SET);
  fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);

}

void listaArquivo(char *caminho){
  int tamanho, endereco, bloco, i, j;
  Arquivo dir, file;

  endereco = getArquivo (caminho, FINAL);
  dir = leArquivo(endereco);
  bloco = dir.bloco;

  tamanho = dir.diretorio;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

  if(dir.diretorio == 0) {
    (printf("Diretorio vazio.\n"));
  } else {
    printf ("eh diretorio  |    nome    | tamanho em bytes | ultima modificacao\n");
  }

  j = 0;
  i = 0;

  while(i < tamanho){

    if(j >= ARQPERBLOCK ){
      j = 0;
      bloco = fat[bloco];

      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&file, sizeof(Arquivo), 1, unidade);

    if(strlen(file.nome) != 0){

      if(file.diretorio >=0)  {
        printf("      *       |");
      } else {
        printf("              |");
      }

      printf("%12s|", file.nome);
      printf("%18d|", file.tamBytes);
      printf(" ");
      printf("%s", ctime(&file.instanteModificado));

      i++;
    }

    j++;
  }
}

void apagaArquivo(char *caminho){
  Arquivo arq, dir;
  char **paradas = NULL;
  char str[MAXCHAR];
  int end, arquivos, bloco, i, j, aux;

  end = getArquivo(caminho, PAI);
  dir = leArquivo(end);
  paradas = geraTokens(caminho, "/");

  for(i = 0; paradas[i+1] != NULL; i++);

  strcpy(str, paradas[i]);

  bloco = dir.bloco;
  arquivos = dir.diretorio;

  j = 0;
  i = 0;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

  while (i < arquivos){

    if (j > ARQPERBLOCK){
      j = 0;
      bloco = fat[bloco];

      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&arq, sizeof(Arquivo), 1, unidade);

    if(strlen(arq.nome) != 0){

      if (strcmp(arq.nome, str) == 0){

        memcpy(arq.nome, "\0", 1);

        desperdicado += sizeof(Arquivo);

        dir.diretorio--;
        dir.tamBytes -= sizeof(Arquivo);

        removeArquivo();

        if(arq.tamBytes%BLOCKSIZE != 0) {
            desperdicado -= BLOCKSIZE - (arq.tamBytes%BLOCKSIZE);
        }

        fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
        fwrite(&arq, sizeof(Arquivo), 1, unidade);

        for (bloco = arq.bloco; bloco != -1; bloco = aux){

          aux = fat[bloco];
          setFAT(-1, bloco);
          setBloco(bloco, 0);
          livres++;
          
          fseek(unidade, end, SEEK_SET);
          fwrite(&dir, sizeof(Arquivo), 1, unidade);

          fseek(unidade, WASTESEEK, SEEK_SET);
          fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
          fwrite(&livres, sizeof(livres), 1, unidade);
        }

        return;
      }

      i++;
    }

    j++;
  }
}

void criaDir(char *caminho){
  Arquivo dir, novo, arq;
  int i, arquivos, end, bloco, newblock = FALSE;
  char **paradas = NULL;

  end = getArquivo(caminho, PAI);
  dir = leArquivo(end);

  if(end == -1){
    printf("Diretorio de destino inexistente.\n");
    free(paradas);

    return;
  } else if(dir.diretorio < 0){
    printf("O destino deve ser um diretorio.\n");
    free(paradas);

    return;
  }

  bloco = dir.bloco;
  i = 1;

  while(fat[bloco] != -1){
   bloco = fat[bloco];
   i++;
  }

  if(dir.diretorio == ARQPERBLOCK*i) {
    newblock = TRUE;
  }

  if(newblock != 0){

    if(livres == 0) return;

    newblock = procuraBloco();
    setBloco(newblock, 1);
    livres--;

    setFAT(newblock, fat[bloco]);
    setFAT(-1, fat[newblock]);

    bloco = newblock;

    desperdicado += (BLOCKSIZE - sizeof(Arquivo));
  } else {
    desperdicado -= sizeof(Arquivo);
  }

  desperdicado += BLOCKSIZE;

  paradas = geraTokens(caminho, "/");

  for(i = 0; paradas[i+1] != NULL; i++);

  strcpy(novo.nome, paradas[i]);

  novo.tamBytes = 0;
  novo.instanteCriado = time(NULL);
  novo.instanteModificado = novo.instanteCriado;
  novo.instanteAcessado = novo.instanteCriado;
  novo.bloco = procuraBloco(bitmap);
  novo.diretorio = 0;

  if(newblock != 0){

    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    fwrite(&novo, sizeof(novo), 1, unidade);

  } else{

    bloco = dir.bloco;
    arquivos = dir.diretorio;

    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

    for(i = 0; i < arquivos; i++){

      if (i > ARQPERBLOCK){
        bloco = fat[bloco];
        fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
        i = 0;
        arquivos -= ARQPERBLOCK;
      }

      fread(&arq, sizeof(Arquivo), 1, unidade);

      if(strlen(arq.nome) == 0){ 
        break;
      }
    }

    fseek(unidade, bloco*BLOCKSIZE + i*sizeof(Arquivo), SEEK_SET);
    fwrite(&novo, sizeof(novo), 1, unidade);

  }

  dir.tamBytes += sizeof(Arquivo);
  dir.diretorio++;

  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(dir), 1, unidade);

  bloco = novo.bloco;
  setBloco(novo.bloco, 1);
  livres--;

  addDiretorio();
  fseek(unidade, WASTESEEK, SEEK_SET);

  fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);
}

void removeDiretorioirRec(int end){
  Arquivo arq, dir;
  int arquivos, bloco, i, j, aux;

  dir = leArquivo(end);
  bloco = dir.bloco;
  
  if (dir.diretorio >= 0){ 

    bloco = dir.bloco;
    arquivos = dir.diretorio;

    j = 0;
    i = 0;

    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

    while (i < arquivos){

      if (j > ARQPERBLOCK){
        j = 0;
        bloco = fat[bloco];
        fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
      }

      fread(&arq, sizeof(Arquivo), 1, unidade);

      if (strlen(arq.nome) != 0){ 

        if (arq.diretorio >= 0) { 

          printf("*%s\n", arq.nome);
          memcpy(arq.nome, "\0", 1);
          removeDiretorio();

          desperdicado -= (quantidadeBlocos(arq.bloco)*BLOCKSIZE - arq.diretorio*sizeof(Arquivo));

          fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
          fwrite(&arq, sizeof(Arquivo), 1, unidade);

          removeDiretorioirRec(bloco*BLOCKSIZE + j*sizeof(Arquivo));

        } else { 

          printf("%s\n", arq.nome);
          memcpy(arq.nome, "\0", 1);

          removeArquivo();

          if (arq.tamBytes%BLOCKSIZE != 0) {
            desperdicado -= BLOCKSIZE - (arq.tamBytes%BLOCKSIZE);
          }

          fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
          fwrite(&arq, sizeof(Arquivo), 1, unidade);

          removeDiretorioirRec(bloco*BLOCKSIZE + j*sizeof(Arquivo));

        }

        i++;
      }

      j++;
    }

  }

  for (bloco = dir.bloco; bloco != -1; bloco = aux){

    aux = fat[bloco];
    setFAT(-1, bloco);
    setBloco(bloco, 0);
    livres++;

  }
  
  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(Arquivo), 1, unidade);
  
  fseek(unidade, WASTESEEK, SEEK_SET);
  fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);

  return;
}

void removeDiretorioir(char *caminho){
  Arquivo arq, dir;
  char **paradas = NULL;
  char str[MAXCHAR];
  int end, arquivos, bloco, i, j;

  end = getArquivo(caminho, PAI);
  dir = leArquivo(end);
  paradas = geraTokens(caminho, "/");
  
  for(i = 0; paradas[i+1] != NULL; i++);

  strcpy(str, paradas[i]);

  bloco = dir.bloco;
  arquivos = dir.diretorio;

  j = 0;
  i = 0;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

  while (i < arquivos){

    if (j > ARQPERBLOCK){
      j = 0;
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&arq, sizeof(Arquivo), 1, unidade);

    if(strlen(arq.nome) != 0){

      if (strcmp(arq.nome, str) == 0){

        printf("Lista de arquivos deletados (diretorios marcados com '*'):\n");
        printf("*%s\n", arq.nome);
        memcpy(arq.nome, "\0", 1);

        desperdicado += sizeof(Arquivo);
        dir.tamBytes -= sizeof(Arquivo);

        removeDiretorio();

        desperdicado -= quantidadeBlocos(arq.bloco)*BLOCKSIZE - arq.diretorio*sizeof(Arquivo);

        fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
        fwrite(&arq, sizeof(Arquivo), 1, unidade);

        fseek(unidade, end, SEEK_SET);
        fwrite(&dir, sizeof(Arquivo), 1, unidade);

        fseek(unidade, WASTESEEK, SEEK_SET);
        fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);
        fwrite(&livres, sizeof(livres), 1, unidade);

        removeDiretorioirRec(bloco*BLOCKSIZE + j*sizeof(Arquivo));

        return;
      }

      i++;
    }

    j++;
  }
}


int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  int i;
  time_t rawtime;
  int montado = FALSE;
  Arquivo novo;

  unidade = NULL;
  rawtime = time(NULL);
  printf("%s", ctime(&rawtime));

  while(1) {

    snprintf(shell_prompt, sizeof(shell_prompt), "{ep3}: ");
    input = readline(shell_prompt);

    add_history(input);
    argv = geraTokens(input, " ");

    /*monta arquivo*/
  	if (strcmp(argv[0], "monta") == 0) {

      if(montado == FALSE){

          if (unidade != NULL){
            fclose (unidade);
          }
          unidade = fopen(argv[1], "r+");

          if (unidade == NULL){ 
            unidade = fopen(argv[1], "w+");

            if (unidade == NULL) {
              printf ("ERRO: Unidade nao foi criada.\n");
              return -1;
            }

            printf("Unidade nao encontrada. Criando nova unidade...");

            bitmap[0] = 0;
            bitmap[0] = setBit(0, bitmap[0], 1);
            livres--;

            fwrite (&bitmap[0], sizeof(bitmap[0]), 1, unidade);

            for (i = 1; i < MAPSIZE; i++) { 
              bitmap[i] = 0;
              fwrite (&bitmap[i], sizeof(bitmap[i]), 1, unidade);
            }

            desperdicado = BLOCKSIZE - MAPSIZE - 4 - 4 - 4 - 4; 

            for(; i < 100000000; i++){ 
              fwrite (&bitmap[1], sizeof(bitmap[1]), 1, unidade);
            }

            for(i = 0; i < MAPSIZE*8; i++){ 
              setFAT(-1, i);
            }

            for(i = 1; i < 26; i++){ 
              setBloco(i, 1);
              livres--;
            }


            memcpy(novo.nome, "root\0", 5);

            novo.tamBytes = 0;
            novo.instanteCriado = time(NULL);
            novo.instanteModificado = novo.instanteCriado;
            novo.instanteAcessado = novo.instanteCriado;
            novo.diretorio = 0;
            novo.bloco = 26;

            qtdDir++;

            desperdicado -= sizeof(Arquivo);

            setBloco(26, 1);
            livres--;

            fseek(unidade, MAPSIZE, SEEK_SET);
            fwrite(&desperdicado, sizeof(desperdicado), 1, unidade);  
            fwrite(&livres, sizeof(livres), 1, unidade);  /*4*/
            fwrite(&qtdDir, sizeof(qtdDir), 1, unidade);      /*8*/
            fwrite(&qtdArq, sizeof(qtdArq), 1, unidade);      /*12*/
            fwrite(&novo, sizeof(Arquivo), 1, unidade);   /*16*/

            printf("Unidade criada com sucesso!\n");

          } else {

            printf("Reiniciando unidade criada anteriormente.\n");
            fseek(unidade, 0, SEEK_SET);

            for(i = 0; i < MAPSIZE; i++) fread(&bitmap[i], sizeof(char), 1, unidade);

            fread(&desperdicado, sizeof(desperdicado), 1, unidade);  
            fread(&livres, sizeof(livres), 1, unidade);  /*4*/
            fread(&qtdDir, sizeof(qtdDir), 1, unidade);      /*8*/
            fread(&qtdArq, sizeof(qtdArq), 1, unidade);      /*12*/

            fseek(unidade, BLOCKSIZE, SEEK_SET);

            for (i = 0; i < MAPSIZE*8; i++) {
              fread(&fat[i], sizeof(int), 1, unidade);
            }

          }

          montado = TRUE;
      } else {
        printf("Eh necessario desmontar o sistema de arquivos atual para montar outro.\n");
      }

    //copia origem destino
  	} else if (strcmp(argv[0], "copia") == 0){

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else if (argv[1] == NULL) {
        printf("copia: insira a origem.\n");
      } else if (argv[2] == NULL) {
        printf("copia: insira o destino.\n");
      } else {
        struct timeval t0;
        struct timeval t1;
        float elapsed;
        gettimeofday(&t0, 0);
        copiaArquivo(argv[1], argv[2]);
        gettimeofday(&t1, 0);
        elapsed = timedifference_msec(t0, t1);
        printf("Copia executou em %f millisegundos.\n", elapsed);
      }

    //criadir diretorio
    } else if (strcmp(argv[0], "criadir") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {
        struct timeval t0;
        struct timeval t1;
        float elapsed;
        gettimeofday(&t0, 0);
        criaDir(argv[1]);
        gettimeofday(&t1, 0);
        elapsed = timedifference_msec(t0, t1);
        printf("Copia executou em %f millisegundos.\n", elapsed);
      }

    //apagadir diretorio
  	} else if (strcmp(argv[0], "apagadir") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {
        struct timeval t0;
        struct timeval t1;
        float elapsed;
        gettimeofday(&t0, 0);
        removeDiretorioir(argv[1]);
        gettimeofday(&t1, 0);
        elapsed = timedifference_msec(t0, t1);
        printf("Copia executou em %f millisegundos.\n", elapsed);
      }

    //mostra arquivo
    } else if (strcmp(argv[0], "mostra") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else if (argv[1] == NULL) {
        printf("mostra: insira o caminho do arquivo.\n");
      } else {
        mostraArquivo(argv[1]);
      }

    //toca arquivo
    } else if (strcmp(argv[0], "toca") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else if (argv[1] == NULL) {
        printf("toca: insira o caminho do arquivo.\n");
      } else {
        tocaArquivo(argv[1]);
      }

    //apaga arquivo
  	} else if (strcmp(argv[0], "apaga") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else if (argv[1] == NULL) {
        printf("apaga: insira o caminho do arquivo.\n");
      } else {
        struct timeval t0;
        struct timeval t1;
        float elapsed;
        gettimeofday(&t0, 0);
        apagaArquivo(argv[1]);
        gettimeofday(&t1, 0);
        elapsed = timedifference_msec(t0, t1);
        printf("Copia executou em %f millisegundos.\n", elapsed);
      }

    //lista diretorio
  	} else if (strcmp(argv[0], "lista") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else if (argv[1] == NULL) {
        printf("lista: insira o caminho do diretorio.\n");
      } else {
        listaArquivo(argv[1]);
      }

    //atualizadb
  	} else if (strcmp(argv[0], "atualizadb") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {}

    //busca string
    } else if (strcmp(argv[0], "busca") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {}

    //status  
  	} else if (strcmp(argv[0], "status") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {
        printf("Quantidade de diretorios: %d\n", qtdDir);
        printf("Quantidade de arquivos (nao diretorios): %d\n", qtdArq);
        printf("Espaco livre: %d bytes\n", (livres*BLOCKSIZE));
        printf("Espaco desperdicado: %d bytes\n", desperdicado);
      }

    //desmonta
  	} else if (strcmp(argv[0], "desmonta") == 0) {

      if(montado == FALSE) {
        printf("Eh necessario montar uma unidade antes.\n");
      } else {
        montado = FALSE;
        printf("Unidade desmontada com sucesso.\n");
      }

    //sai
  	}
    else if (strcmp(argv[0], "sai") == 0) {
  		printf("Exit.\n");
  		break;
  	} else {
        printf("Lista de comandos:\n");
        printf("monta <arquivo>    			-- monta o sistema de arquivos contido em <arquivo>\n");
        printf("copia <origem> <destino>   	-- cria uma copia do arquivo <origem> em <destino>\n");
        printf("criadir <diretorio>   		-- cria o diretorio <diretorio>\n");
        printf("apagadir <diretorio>        -- remove o diretorio <diretorio>\n");
        printf("mostra <arquivo> 			-- mostra o conteudo de <arquivo>\n");
        printf("toca <arquivo> 			    -- atualiza o ultimo acesso de <arquivo> para o instante atual");
        printf("apaga <arquivo> 			-- remove o arquivo <arquivo>");
        printf("lista <diretorio> 			-- lista tudo abaixo do diretorio <diretorio>");
        printf("atualizadb                  -- cria banco de dados com arvore de arquivos e diretorios do sistema de arquivos");
        printf("busca <string>          	-- busca no banco de dados criado <string> nos nomes");
        printf("status						-- imprime informacoes do sistema de arquivos");
        printf("desmonta					-- desmonta o sistema de arquivos");
        printf("sai               			-- encerra o programa\n");
		}

    if(argv != NULL){
      free(argv);
      argv = NULL;
    }
  }

  if(input != NULL)
    free(input);

  if(argv != NULL)
  	free(argv);

  if(unidade != NULL)
    fclose(unidade);

  return 0;
}
