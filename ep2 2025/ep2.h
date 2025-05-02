/* Isis Ardisson Logullo 7577410 */

typedef struct ciclista {
  int id;
  int m; //linha
  int n; //coluna
  int v; //velocidade
  int terminou;
  int volta;
  int quebrado;
  int desclassificado;
  double tempo;
} Ciclista;

typedef struct no {
  int id;
  int tam;
  struct no *prox;
} No;

