#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Celula{
	void* info;
	struct celula* urm;
}celula;

typedef struct Stiva {
	celula* top;
	size_t dim;
}Stack;

typedef struct Proces{
	int PID;
	int prioritate;
	int timp_executie;
	int mem_size;
	Stack* stiva;
}proces;

typedef struct Coada{
	celula* front, * rear;
	size_t dim;
}Queue;

////////////////////////////////////////////////////////////////////////////////

int vidaQ(void* Q)
{
	if (((Queue *)Q)->front == NULL && ((Queue *)Q)->rear == NULL) return 1;
	return 0;
}

void* initQ(size_t dim) {
	Queue* Q;
	Q = (Queue*)calloc(1, dim);
	Q->dim = dim;
	return (void*)Q;
}

proces* alocareProces(){
	proces* p;
	p = (proces*)calloc(1, sizeof(proces));
	if (p == NULL) {
		printf("Nu s-a putut aloca procesul!\n");
		return;
	}
	p->stiva = (Stack*)calloc(1, sizeof(Stack));
	if (!p->stiva) return NULL;
	p->stiva->dim = sizeof(Stack);
	p->stiva->top = NULL;
	return p;
}

celula* alocareCelula(void* p, size_t dim) {
	celula* aux;
	aux = (celula*)calloc(1, sizeof(celula));
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return;
	}
	aux->urm = NULL;
	aux->info = malloc(dim);
	if (aux->info == NULL) {
		printf("Nu s-a putut aloca informatia!\n");
		return;
	}
	memcpy(aux->info, p, dim);
	return aux;
}

celula* ExtrQ(void* Q)
{
	if (((Queue *)Q)->front == ((Queue *)Q)->rear)
	{
		celula*	aux = ((Queue *)Q)->front;
		((Queue *)Q)->front = NULL;
		((Queue *)Q)->rear = NULL;
		return aux;
	}
	celula*	aux = ((Queue *)Q)->front;
	((Queue *)Q)->front = aux->urm;
	return aux;
}


void* intrQ(void* Q, void* p, size_t d)
{
	celula* aux;
	aux = alocareCelula(p, d);
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return;
	}

	if (vidaQ(Q) == 1)
	{
		((Queue *)Q)->front = aux;
		((Queue *)Q)->rear = aux;
		return (void*)Q;
	}
	else
	{
		((Queue *)Q)->rear->urm = aux;
		((Queue *)Q)->rear = aux;
	}
	return (void*)Q;
}

proces* setProcesDate(proces* proces_nou, int mem_size, int exec_time, int prioritate) {
	proces_nou->mem_size = mem_size;
	proces_nou->timp_executie = exec_time;
	proces_nou->prioritate = prioritate;
	return proces_nou;
}

int main() {

	int T;
	FILE* intrare = fopen("input.txt", "r");
	fscanf(intrare, "%d", &T);
	Queue* q = initQ(sizeof(Queue));

	//proces *p = alocareProces();

	char optiune[50];
	
	while (!feof(intrare)) {
		fgets(optiune, 50, intrare);

		char* p = (char*)malloc(30);
		p = strtok(optiune, " ");
		if (strcmp(p, "add") == 0) {
			p = strtok(NULL, " ");
			int mem_size = atoi(p);
			p = strtok(NULL, " ");
			int exec_time = atoi(p);
			p = strtok(NULL, " ");
			int prioritate = atoi(p);
			
			proces* proces_nou = alocareProces();
			proces_nou = setProcesDate(proces_nou, mem_size, exec_time, prioritate);
			printf("%d\n", proces_nou->mem_size);
		}
	}
	return 0;
}
