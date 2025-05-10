/* Isis Ardisson Logullo 7577410 */

typedef struct ciclista {
  int numero;
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
  int numero;
  int tam;
  struct no *prox;
} No;


//Pilha
int pilhaVazia(No *pilha);

int tamanhoPilha(No *pilha);

No *push(No *pilha, int numero);

No *pop(No *pilha);

int top(No *pilha);

void imprimePilha(No *pilha);

//MergeSort
void merge(int *v, int p, int q, int r, Ciclista *ciclistas);

void mergeSort(int *v, int p, int r, Ciclista *ciclistas);

//Inicializacoes
void insereCiclista(int numero, int m, int n);

void geraPista();

void mutex();

//Impressoes
void imprimePosicao(int *v, int b);

void imprimePista();

//Corrida
int geraProbabilidades(int numero);

void mudaPosicao(int Ciclista);

void *thread(void *numero);

void decidePosicao(int restantes);




