#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Celula {
	struct celula* urm;
	void* info;
}celula;

typedef struct celulaStiva {
	struct celStiva* urm;
	int info;
}celStiva;

typedef struct Stiva {
	size_t dim;
	celStiva* top;
}Stack;

typedef struct Proces {
	int PID;
	int prioritate;
	int timp_executie;
	int cuantum_timp;
	int mem_size;
	int timp_executat;
	int mem_start;
	Stack* stiva;
}proces;

typedef struct Coada {
	celula* front, * rear;
	size_t dim;
}Queue;

typedef struct Memorie {
	proces* info;
	struct Memorie* urm;
}Memorie;

////////////////////////////////////////////////////////////////////////////////

int vidaQ(void* Q)
{
	if (((Queue*)Q)->front == NULL && ((Queue*)Q)->rear == NULL) return 1;
	return 0;
}

void* initQ(size_t dim) {
	Queue* Q;
	Q = (Queue*)calloc(1, dim);
	Q->dim = dim;
	return (void*)Q;
}

proces* alocareProces() {
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

celStiva* alocareCelStiva(void* p, size_t dim) {
	celStiva* aux = (celStiva*)calloc(1, sizeof(celStiva));
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return NULL;
	}
	aux->urm = NULL;
	aux->info = malloc(dim);
	if (aux->info == NULL) {
		printf("Nu s-a putut aloca informatia!\n");
		return NULL;
	}
	memcpy(aux->info, &p, dim);
	return aux;
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

celula* extrQ(void* Q) {
	if (((Queue*)Q)->front == ((Queue*)Q)->rear)
	{
		celula* aux = ((Queue*)Q)->front;
		((Queue*)Q)->front = NULL;
		((Queue*)Q)->rear = NULL;
		return aux;
	}
	celula* aux = ((Queue*)Q)->front;
	((Queue*)Q)->front = aux->urm;
	return aux;
}

celula* topQ(void* Q) {
	return ((Queue*)Q)->front;
}


void intrQ(Queue **Q, void* p, size_t d)
{
	celula* aux;
	aux = alocareCelula(p, d);
	if (aux == NULL) {
		printf("Nu s-a putut aloca celula!\n");
		return;
	}

	if ((*Q)->front == NULL && (*Q)->rear == NULL)
	{
		(*Q)->front = aux;
		(*Q)->rear = aux;
	}
	else
	{
		(*Q)->rear->urm = aux;
		(*Q)->rear = aux;
	}
}

proces* setProcesDate(proces * proces_nou, int PID, int mem_size, int exec_time, int prioritate, int cuantum_timp) {
	proces_nou->mem_size = mem_size;
	proces_nou->timp_executie = exec_time;
	proces_nou->prioritate = prioritate;
	proces_nou->PID = PID;
	proces_nou->timp_executat = 0;
	proces_nou->cuantum_timp = cuantum_timp;
	return proces_nou;
}

int determinarePID(int** vect_PID) {
	int i = 1;
	for (i; i < 32768; i++)
		if ((*vect_PID)[i] == 0) {
			(*vect_PID)[i] = 1;
			return i;
		}
}

void adaugaMemorie(Memorie** memorie, proces** proces_nou) {
	if ((*memorie)->info == NULL)
		(*memorie)->info = (*proces_nou);
	else if ((*memorie)->urm == NULL) {
		(*proces_nou)->mem_start = (*memorie)->info->mem_size;
		(*memorie)->urm = (Memorie*)calloc(1, sizeof(Memorie));
		(*memorie)->urm->info = (*proces_nou);
	}
	else {
		Memorie* aux = (*memorie);
		while (aux->urm != NULL) {
			if ((*proces_nou)->mem_size <= (aux->urm->info->mem_start - (aux->info->mem_size + aux->info->mem_start))) {
				Memorie* swap = (Memorie*)calloc(1, sizeof(Memorie));
				swap->info = proces_nou;
				swap->info->mem_start = aux->info->mem_size;
				swap->urm = aux->urm;
				aux->urm = swap;
				return;
			}
			aux = aux->urm;
		}
		if (aux->urm == NULL) {
			Memorie* mem_nou = (Memorie*)calloc(1, sizeof(Memorie));
			(*proces_nou)->mem_start = aux->info->mem_size + aux->info->mem_start;
			mem_nou->info = (*proces_nou);
			aux->urm = mem_nou;
		}
	}
}
Queue* add(Queue **Q, Memorie **memorie, int mem_size, int exec_time, int prioritate, int cuantum_timp, int** vect_PID, int* stare_running, celula * *running) {

	if (*stare_running == 0) {
		(*stare_running) = 1;
		int PID = determinarePID(vect_PID);
		proces* proces_nou = alocareProces();
		proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);
		proces_nou->mem_start = 0;
		(*running) = alocareCelula(proces_nou, sizeof(proces));
		adaugaMemorie(memorie, &proces_nou);
		printf("Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
		return (*Q);
	}

	celula* aux = (*Q)->front;

	int PID = determinarePID(vect_PID);

	proces* proces_nou = alocareProces();
	proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);
	celula* cel = alocareCelula(proces_nou, sizeof(proces));

	adaugaMemorie(memorie, &proces_nou);

	Queue* coada_noua = initQ(sizeof(Queue));

	int introdus = 0, aux_null = 0;
	if (aux == NULL) {
		intrQ(&coada_noua, proces_nou, sizeof(proces));
		aux_null = 1;
	}
	else {
		while (aux != NULL) {
			if (prioritate > ((proces*)aux->info)->prioritate) {
				introdus = 1;
				intrQ(&coada_noua, proces_nou, sizeof(proces));
			}
			else if (prioritate == ((proces*)aux->info)->prioritate) {
				if (exec_time < ((proces*)aux->info)->timp_executie) {
					introdus = 1;
					intrQ(&coada_noua, proces_nou, sizeof(proces));
				}
				else if (exec_time == ((proces*)aux->info)->timp_executie) {
					if (PID < ((proces*)aux->info)->PID) {
						introdus = 1;
						intrQ(&coada_noua, proces_nou, sizeof(proces));
					}
					else {
						intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
						aux = aux->urm;
						continue;
					}
				}
				else {
					intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
					aux = aux->urm;
					continue;
				}
			}
			else intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			if (introdus == 1) break;
			aux = aux->urm;
		}
		while (aux != NULL) {
			intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			aux = aux->urm;
		}
	}
	if (introdus == 0 && aux_null == 0) intrQ(&coada_noua, proces_nou, sizeof(proces));

	printf("Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
	return coada_noua;
}

void afisare(Queue * Q) {
	Queue* aux_n = Q;
	celula* aux = aux_n->front;

	while (aux != NULL) {
		printf("PID:%d Timp de executie:%d Prioritate:%d\n", (((proces*)aux->info)->PID), (((proces*)aux->info)->timp_executie), (((proces*)aux->info)->prioritate));
		aux = aux->urm;
	}
}

void get(int PID, Queue * coada_asteptare, celula * running, Queue * finished) {
	if (PID == ((proces*)running->info)->PID)
		printf("Process %d is running (remaining_time = %d).\n", PID, ((proces*)running->info)->timp_executie);
	else {
		celula* aux = coada_asteptare->front;
		while (aux != NULL) {
			if (PID == ((proces*)aux->info)->PID) {
				printf("Process %d is waiting (remaining_time = %d).\n", PID, ((proces*)aux->info)->timp_executie);
				return;
			}
			aux = aux->urm;
		}
		aux = finished->front;
		while (aux != NULL) {
			if (PID == ((proces*)aux->info)->PID) {
				printf("Process %d is finished.\n", PID);
				return;
			}
			aux = aux->urm;
		}
		printf("Process %d not found.\n", PID);
	}
}

void print_waiting(Queue * coada_asteptare) {
	celula* aux = coada_asteptare->front;
	printf("Waiting queue:\n[");
	if (aux != NULL) {
		while (aux->urm != NULL) {
			printf("(%d: priority = %d, remaining_time = %d),\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executie);
			aux = aux->urm;
		}
		printf("(%d: priority = %d, remaining_time = %d)]\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executie);
	}
	else printf("]\n");
}


void print_finished(Queue* coada_finished) {
	celula* aux = coada_finished->front;
	printf("Finished queue:\n[");

	if (aux != NULL) {
		while (aux->urm != NULL) {
			printf("(%d: priority = %d, executed_time: %d),\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executat);
			aux = aux->urm;
		}
		printf("(%d: priority = %d, executed_time: %d)]\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executat);
	}
	else printf("]\n");
}

void introducereQ(Queue** Q, proces* pr) {
	celula* aux = (*Q)->front;

	proces* proces_nou = pr;
	celula* cel = alocareCelula(proces_nou, sizeof(proces));

	int prioritate = pr->prioritate;
	int exec_time = pr->timp_executie;
	int PID = pr->PID;

	Queue* coada_noua = initQ(sizeof(Queue));

	int introdus = 0, aux_null = 0;
	if (aux == NULL) {
		intrQ(&coada_noua, proces_nou, sizeof(proces));
		aux_null = 1;
	}
	else {
		while (aux != NULL) {
			if (prioritate > ((proces*)aux->info)->prioritate) {
				introdus = 1;
				intrQ(&coada_noua, proces_nou, sizeof(proces));
			}
			else if (prioritate == ((proces*)aux->info)->prioritate) {
				if (exec_time < ((proces*)aux->info)->timp_executie) {
					introdus = 1;
					intrQ(&coada_noua, proces_nou, sizeof(proces));
				}
				else if (exec_time == ((proces*)aux->info)->timp_executie) {
					if (PID < ((proces*)aux->info)->PID) {
						introdus = 1;
						intrQ(&coada_noua, proces_nou, sizeof(proces));
					}
					else {
						intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
						aux = aux->urm;
						continue;
					}
				}
				else {
					intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
					aux = aux->urm;
					continue;
				}
			}
			else intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			if (introdus == 1) break;
			aux = aux->urm;
		}
		while (aux != NULL) {
			intrQ(&coada_noua, (proces*)aux->info, sizeof(proces));
			aux = aux->urm;
		}
	}
	if (introdus == 0 && aux_null == 0) intrQ(&coada_noua, proces_nou, sizeof(proces));
	(*Q) = coada_noua;
}

void run(int timp, int cuantum_timp, Queue **coada_asteptare, celula **running, Queue **coada_finished) {

	celula* aux = (*running);
	celula* aux_nou = (*running);

	while (timp > 0) {
		if (cuantum_timp > ((proces*)aux->info)->cuantum_timp) {
			if (timp > ((proces*)aux->info)->cuantum_timp) {
				((proces*)aux->info)->timp_executie -= ((proces*)aux->info)->cuantum_timp;
				timp -= ((proces*)aux->info)->cuantum_timp;
				((proces*)aux->info)->cuantum_timp = cuantum_timp;
				if (((proces*)aux->info)->timp_executie > 0) {
					aux_nou = aux;
					if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
						aux = extrQ(*coada_asteptare);
						introducereQ(coada_asteptare, (proces*)aux_nou->info, sizeof(proces));
					}
				}
				else {
					intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
					aux = extrQ(*coada_asteptare);
				}
			}
			else if (timp == ((proces*)aux->info)->cuantum_timp) {
				((proces*)aux->info)->timp_executie -= ((proces*)aux->info)->cuantum_timp;
				((proces*)aux->info)->cuantum_timp = cuantum_timp;
				if (((proces*)aux->info)->timp_executie > 0) {
					aux_nou = aux;
					if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
						aux = extrQ(*coada_asteptare);
						introducereQ(coada_asteptare, (proces*)aux_nou->info, sizeof(proces));
					}
				}
				else {
					intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
					aux = extrQ(*coada_asteptare);
				}
			}
			else {
				((proces*)aux->info)->timp_executie -= timp;
				((proces*)aux->info)->cuantum_timp -= timp;
				if (((proces*)aux->info)->timp_executie > 0) {
					aux_nou = aux;
					if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
						aux = extrQ(*coada_asteptare);
						introducereQ(coada_asteptare, (proces*)aux_nou->info, sizeof(proces));
					}
				}
				else {
					intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
					aux = extrQ(*coada_asteptare);
				}
			}
		}
		if (timp < cuantum_timp) {
			if (timp - (((proces*)aux->info)->timp_executie) == 0) {
				((proces*)aux->info)->timp_executie = 0;
				((proces*)aux->info)->timp_executat += timp;
				intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
				aux = extrQ(*coada_asteptare);
				break;
			}
			else if (timp - (((proces*)aux->info)->timp_executie) < 0) {
				((proces*)aux->info)->timp_executie -= timp;
				((proces*)aux->info)->timp_executat += timp;
				((proces*)aux->info)->cuantum_timp -= timp;
				break;
			}
			else {
				((proces*)aux->info)->timp_executat += ((proces*)aux->info)->timp_executie;
				timp -= ((proces*)aux->info)->timp_executie;
				((proces*)aux->info)->timp_executie = 0;
				intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
				aux = extrQ(*coada_asteptare);
			}
		}
 		else if (timp >= cuantum_timp) {
			if (cuantum_timp - (((proces*)aux->info)->timp_executie) == 0) {
				((proces*)aux->info)->timp_executie = 0;
				((proces*)aux->info)->timp_executat += cuantum_timp;
				((proces*)aux->info)->cuantum_timp = cuantum_timp;
				intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
				aux = extrQ(*coada_asteptare);
				timp -= cuantum_timp;
			}
			else if (cuantum_timp - (((proces*)aux->info)->timp_executie) < 0) {
				((proces*)aux->info)->timp_executie -= cuantum_timp;
				((proces*)aux->info)->timp_executat += cuantum_timp;
				((proces*)aux->info)->cuantum_timp = cuantum_timp;
				timp -= cuantum_timp;
				aux_nou = aux;
				if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
					aux = extrQ(*coada_asteptare);
					introducereQ(coada_asteptare, (proces*)aux_nou->info, sizeof(proces));
				}
			}
			else {
				((proces*)aux->info)->timp_executat += ((proces*)aux->info)->timp_executie;
				timp -= ((proces*)aux->info)->timp_executie;
				((proces*)aux->info)->timp_executie = 0;
				intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
				aux = extrQ(*coada_asteptare);
			}
		}
	}
	(*running) = aux;
}

void* initS(size_t dim) {
	Stack* S;
	S = (Stack*)calloc(1, dim);
	S->dim = dim;
	return (void*)S;
}

void finish(Queue * coada_asteptare, celula * running) {
	int total_time = 0;
	total_time += ((proces*)running->info)->timp_executie;
	celula* aux = coada_asteptare->front;
	while (aux != NULL) {
		total_time += ((proces*)aux->info)->timp_executie;
		aux = aux->urm;
	}
	printf("Total time: %d\n", total_time);
}



void pushS(Memorie** memorie, int PID, int data) {

	Memorie* aux = (*memorie);

	while (aux != NULL) {
		if (aux->info->PID == PID) {
			celStiva* informatie = (celStiva*)calloc(1, sizeof(celStiva));
			informatie->info = data;
			if (aux->info->stiva->top == NULL) {
				aux->info->stiva->top = informatie;
			}
			else{
				informatie->urm = aux->info->stiva->top;
				aux->info->stiva->top = informatie;
			}
			break;
		}
		aux = aux->urm;
	}
	if (aux == NULL) printf("PID %d not found.\n", PID);
}

void printStack(Memorie** memorie, int PID) {

	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (aux->info->PID == PID) {
			if (aux->info->stiva->top == NULL) printf("Empty stack PID %d.\n", PID);
			else {
				printf("Stack of PID %d: ", PID);
				celStiva* info = aux->info->stiva->top;
				while (info->urm != NULL) {
					printf("%d ", (int)info->info);
					info = info->urm;
				}
				printf("%d.\n", (int)info->info);
			}	
			break;
		}
		aux = aux->urm;
	}
}

void pop(Memorie** memorie, int PID) {
	
	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (aux->info->PID == PID) {
			if (aux->info->stiva->top == NULL) printf("Empty stack PID %d.\n", PID);
			else {
				aux->info->stiva->top = aux->info->stiva->top->urm;
			}
			break;
		}
		aux = aux->urm;
	}
	if (aux == NULL) if (aux == NULL) printf("PID %d not found.\n", PID);
}

int main() {

	int cuantum_timp;
	FILE* intrare = fopen("input.txt", "r");
	fscanf(intrare, "%d", &cuantum_timp);

	Queue* coada_asteptare = initQ(sizeof(Queue));
	Queue* coada_finished = initQ(sizeof(Queue));
	int* vect_PID = (int*)calloc(32768, sizeof(int));
	celula* running = (celula*)calloc(1, sizeof(celula));
	Memorie* memorie = (Memorie*)calloc(1, sizeof(Memorie));

	int stare_running = 0;

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
			coada_asteptare = add(&coada_asteptare, &memorie, mem_size, exec_time, prioritate, cuantum_timp, &vect_PID, &stare_running, &running);
		}
		if (strcmp(p, "get") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			get(PID, coada_asteptare, running, coada_finished);
		}

		if (strcmp(p, "print") == 0) {
			p = strtok(NULL, " ");
			if (p[strlen(p) - 1] == '\n')
				p[strlen(p) - 1] = '\0';
			if (strcmp(p, "finished") == 0)
				print_finished(coada_finished);
			else if (strcmp(p, "waiting") == 0)
				print_waiting(coada_asteptare);
			else if (strcmp(p, "stack") == 0) {
				p = strtok(NULL, " ");
				int PID = atoi(p);
				printStack(&memorie, PID);
			}
		}
		if (strcmp(p, "run") == 0) {
			p = strtok(NULL, " ");
			int timp = atoi(p);
			run(timp, cuantum_timp, &coada_asteptare, &running, &coada_finished);
		}
		if (strcmp(p, "finish") == 0) {
			finish(coada_asteptare, running);
		}
		if (strcmp(p, "push") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			p = strtok(NULL, " ");
			int data = atoi(p);
			pushS(&memorie, PID, data);
		}
		if (strcmp(p, "pop") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			pop(&memorie, PID);
		}
	}
	return 0;
}
