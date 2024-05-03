// Isis Ardisson Logullo 7577410

typedef struct ciclista {
  int id;
  int posicaoColuna;
  int posicaoLinha;
  int velocidade;
  int volta;
  int eliminado;
  int terminou;
  int quebrado;
  double tempoExecucao;
} Ciclista;

// typedef struct queueNode {
//   int id;
//   struct queueNode *next;
// } QueueNode;

// typedef struct queue{
//    QueueNode *head;
//    QueueNode *tail;
// } Queue;

typedef struct stackNode {
  int id;
  int size;
  struct stackNode *next;
} StackNode;

// void enqueue(QueueNode *novo, int id);
// int dequeue();
// int queueSizeQueue(QueueNode *q);
// Queue *criaFila();
// void freeQueue();
// int queueEmpty();
// void printQueue(QueueNode *node);
// int primeiroElemento();

int stackEmpty(StackNode *stack);
int stackSize(StackNode *stack);
int top(StackNode *stack);
StackNode *push(StackNode *stack, int id);
StackNode *pop(StackNode *stack);
void printStack(StackNode *stack);
void merge(int *vetor, int a, int q, int b, Ciclista *ciclistas);
void mergeSort(int *vetor, int a, int b, Ciclista *ciclistas);
void printRank(int *vetor, int b);
int randomVelocidade(int id);
void mudaPosicao(int ciclista);
void * thread(void * id);
void printPista();
void juiz(int ciclistasRestantes);
void insereCiclistaPista(int numeroCiclistas, int linha, int coluna);
void inicioPista();
void inicioMutex();
void geraSaida(char *nome);
void freeMemoria(int *ids);



