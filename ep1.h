/* Isis Ardisson Logullo - 7577410*/

#define MAX_LINE_SIZE 1024

typedef struct{
	float t0; 
	float dt; 
	float et; 
	float deadline;
	float quantum;
	char *nome;
	int i;
} processo;

typedef struct{
	float t0;
	float dt;
	float deadline;
	char *nome;
} linha;

typedef struct{
	processo **v;
	int topo, max;
} pilha;

typedef struct{
	processo **v;
	int frente, tam, max;
} fila;

/* pilha */
int pilhaVazia(pilha *p);

pilha *criaPilha(int MAX);

void insereOrdenado(pilha *p, processo *x);

processo *desempilha(pilha *p);

processo *topoPilha(pilha *p);

void destroiPilha(pilha *p);

/* fila */
int filaVazia(fila *f);

fila *criaFila(int MAX);

void insere(fila *f, processo *x);

processo *removeFila(fila *f);

void destroiFila(fila *f);

/* leitura */
linha *criaLinha(int n);

linha **leArquivo(char *nomeArquivo, int *contadorLinha);

/* processos */
float getTempoExecucao();

void *criaThread(void* arg);

void *criaThreadQuantum(void* arg);

processo *criaProcessoLinha(linha *l, int i, float quantum);

void shortestJobFirst(linha **dados);

void roundRobin(linha **dados);

float escolheQuantum(processo *p);

void escalonamentoComPrioridade(linha **dados);




