#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#define BLOCKSIZE 4000
#define MAPSIZE 3125
#define ARQPERBLOCK (BLOCKSIZE/sizeof(Arquivo))

#define WASTESEEK MAPSIZE
#define LIVRESSEEK (MAPSIZE+4)
#define QTDDIRSEEK (MAPSIZE+8)
#define QTDARQSEEK (MAPSIZE+12)
#define ROOTSEEK (MAPSIZE+16)

#define MAXCHAR 256
#define TRUE 1
#define FALSE 0

#define PAI 0
#define FINAL 1


typedef struct arquivo Arquivo;
struct arquivo
{
  char     	nome[13];
  int     	tamBytes;
  time_t 	instanteCriado;
  time_t 	instanteModificado;
  time_t 	instanteAcessado;
  int       bloco;
  int 		diretorio;
};

float timeDifferenceMsec(struct timeval t0, struct timeval t1);

void addD();

void removeD();

void addF();

void removeF();

char** geraTokens(char *str, char *separator);

unsigned char setBit(int posicao, unsigned char byte, int bit);

void setBloco(int bloco, int bit);

int devolveBit(int posicao, unsigned char byte);

int procuraBloco();

int quantidadeBlocos(int arquivos);

void setFAT(int conteudo, int posicao);

void imprimeBitmap();

Arquivo leArquivo(int i);

int getEndArquivoRec(char **paradas, int i, Arquivo diretorio, int endereco,  int option);

int getArquivo(char *caminho, int option);

void mostraArquivo(char *caminho);

void copiaArquivo(char *origem, char *destino);

void tocaArquivo(char *caminho);

void listaArquivo(char *caminho);

void apagaArquivo(char *caminho);

void criaDir(char *caminho);

void removeDirRec(int end);

void removeDir(char *caminho);

char* geradir ( char* dir, int num);

void rec30Dir (char *caminho);

void rec30Dir100Arq (char *caminho, char *origem);












