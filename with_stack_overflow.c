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
	int mem_size;
	int mem_start;
	int PID;
	int prioritate;
	int timp_executie;
	int cuantum_timp;
	int timp_executat;
	int stack_size;
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
		return NULL;
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
		printf ("Nu s-a putut aloca celula!\n");
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


void intrQ(Queue * *Q, void* p, size_t d)
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
	proces_nou->stack_size = 0;
	return proces_nou;
}

int determinarePID(int** vect_PID) {
	int i = 1;
	for (i; i < 32768; i++)
		if ((*vect_PID)[i] == 0) {
			(*vect_PID)[i] = 1;
			return i;
		}
	return -1;
}

void adaugaMemorie(Memorie** memorie, proces** proces_nou, int* vect_PID);

void defragmentare(Memorie **memorie, int *vect_PID) {
	Memorie* memorie_aux = (Memorie*)calloc(1, sizeof(Memorie));
	int i;
	for (i = 0; i < 32768; i++) {
		if (vect_PID[i] == 1) {
			Memorie* aux = (*memorie);
			while (aux != NULL) {
				if (aux->info->PID == i) {
					adaugaMemorie(&memorie_aux, &(aux->info), vect_PID);
					break;
				}	
				aux = aux->urm;
			}
		}
	}
	(*memorie) = memorie_aux;
}

void adaugaMemorie(Memorie **memorie, proces **proces_nou, int *vect_PID) {

	if ((*memorie) == NULL) (*memorie) = (Memorie*)calloc(1, sizeof(Memorie));
	if ((*memorie)->info == NULL)
		(*memorie)->info = (*proces_nou);
	else if ((*memorie)->urm == NULL) {
		(*proces_nou)->mem_start = (*memorie)->info->mem_size;
		(*memorie)->urm = (Memorie*)calloc(1, sizeof(Memorie));
		(*memorie)->urm->info = (*proces_nou);
	}
	else {
		Memorie* aux = (*memorie);
		if (aux->info->mem_start != 0) {
			if ((*proces_nou)->mem_size <= aux->info->mem_start) {
				Memorie* swap = (Memorie*)calloc(1, sizeof(Memorie));
				swap->info = (*proces_nou);
				swap->urm = (*memorie);
				(*memorie) = swap;
				return;
			}
		}
		while (aux->urm != NULL) {
			if ((*proces_nou)->mem_size <= (aux->urm->info->mem_start - (aux->info->mem_size + aux->info->mem_start))) {
				Memorie* swap = (Memorie*)calloc(1, sizeof(Memorie));
				swap->info = (*proces_nou);
				swap->info->mem_start = aux->info->mem_start + aux->info->mem_size;
				(*proces_nou)->mem_start = swap->info->mem_start;
				swap->urm = aux->urm;
				aux->urm = swap;
				return;
			}
			else if ((aux->urm->info->mem_start - (aux->info->mem_size + aux->info->mem_start)) > 0 && (*proces_nou)->mem_size > (aux->urm->info->mem_start - (aux->info->mem_size + aux->info->mem_start))) {
				defragmentare(memorie, vect_PID);
				adaugaMemorie(memorie, proces_nou, vect_PID);
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

Queue* add(Queue **Q, Memorie **memorie, int *nr_elemente, int *memorie_totala, int mem_size, int exec_time, int prioritate, int cuantum_timp, int** vect_PID, int* stare_running, celula * *running, FILE* output) {

	if (mem_size == 108) {
		int i = 0;
	}
	if ((*memorie_totala) + mem_size <= 3145728) {
		(*memorie_totala) += mem_size;
		if (*stare_running == 0 || (*running) == NULL) {
			if ((*running) == NULL) (*Q) = (Queue*)calloc(1, sizeof(Queue));
			(*stare_running) = 1;
			int PID = determinarePID(vect_PID);
			proces* proces_nou = alocareProces();
			proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);
			proces_nou->mem_start = 0;
			(*running) = alocareCelula(proces_nou, sizeof(proces));
			adaugaMemorie(memorie, &proces_nou, (*vect_PID));
			(*nr_elemente) = 1;
			fprintf(output, "Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
			return (*Q);
		}

		int PID = determinarePID(vect_PID);

		proces* proces_nou = alocareProces();
		proces_nou = setProcesDate(proces_nou, PID, mem_size, exec_time, prioritate, cuantum_timp);

		adaugaMemorie(memorie, &proces_nou, (*vect_PID));
		(*nr_elemente)++;

		Queue* coada_noua = initQ(sizeof(Queue));
		celula* aux = (*Q)->front;
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

		fprintf(output, "Process created successfully: PID: %d, Memory starts at 0x%x.\n", proces_nou->PID, proces_nou->mem_start);
		return coada_noua;
	}
	else {
		int PID = determinarePID(vect_PID);
		fprintf(output, "Cannot reserve memory for PID %d.\n", PID);
		(*vect_PID)[PID] = 0;
		return (*Q);
	}
}

void get(int PID, Queue * coada_asteptare, celula * running, Queue * finished, FILE* output) {
	
	int afisat = 0;
	if (running!=NULL)
		if (PID == ((proces*)running->info)->PID) {
			fprintf(output, "Process %d is running (remaining_time: %d).\n", PID, ((proces*)running->info)->timp_executie);
			return;
		}
	celula* aux = coada_asteptare->front;
	while (aux != NULL) {
		if (PID == ((proces*)aux->info)->PID) {
			fprintf(output,  "Process %d is waiting (remaining_time: %d).\n", PID, ((proces*)aux->info)->timp_executie);
			afisat = 1;
		}
		aux = aux->urm;
	}
	aux = finished->front;
	while (aux != NULL) {
		if (PID == ((proces*)aux->info)->PID) {
			fprintf(output, "Process %d is finished.\n", PID);
			afisat = 1;
		}
		aux = aux->urm;
	}
	if(afisat == 0) fprintf(output,  "Process %d not found.\n", PID);
}

void print_waiting(Queue * coada_asteptare, FILE* output) {
	celula* aux = coada_asteptare->front;
	fprintf(output,  "Waiting queue:\n[");
	if (aux != NULL) {
		while (aux->urm != NULL) {
			fprintf(output,  "(%d: priority = %d, remaining_time = %d),\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executie);
			aux = aux->urm;
		}
		fprintf(output,  "(%d: priority = %d, remaining_time = %d)]\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executie);
	}
	else fprintf(output,  "]\n");
}


void print_finished(Queue * coada_finished, FILE* output) {
	celula* aux = coada_finished->front;
	fprintf(output,  "Finished queue:\n[");

	if (aux != NULL) {
		while (aux->urm != NULL) {
			fprintf(output,  "(%d: priority = %d, executed_time = %d),\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executat);
			aux = aux->urm;
		}
		fprintf(output,  "(%d: priority = %d, executed_time = %d)]\n", ((proces*)aux->info)->PID, ((proces*)aux->info)->prioritate, ((proces*)aux->info)->timp_executat);
	}
	else fprintf(output,  "]\n");
}

void remove_memorie(int PID, Queue** coada_asteptare, Memorie** memorie, int** vect_PID, int* nr_elemente) {
	Memorie* aux = (*memorie);
	(*vect_PID)[PID] = 0;
	(*nr_elemente)--;
	if ((*nr_elemente) <= 0) {
		(*memorie) = (Memorie*)calloc(1, sizeof(Memorie));
		(*vect_PID) = (int*)calloc(32768, sizeof(int));
		(*coada_asteptare) = (Queue*)calloc(1, sizeof(Queue));
	}
	else {
		if ((proces*)aux->info->PID == PID) {
			(*memorie) = (*memorie)->urm;
			return;
		}
		else {
			if (PID == ((proces*)aux->urm->info)->PID) {
				aux->urm = aux->urm->urm;
				return;
			}
			aux = aux->urm;
			while (aux->urm != NULL) {
				if (PID == ((proces*)aux->urm->info)->PID) {
					aux->urm = aux->urm->urm;
					return;
				}
				aux = aux->urm;
			}
			aux = NULL;
		}
	}
}

void run(int timp, int cuantum_timp, int *memorie_totala, Queue **coada_asteptare, celula **running, Queue **coada_finished, Memorie **memorie, int **vect_PID, int *nr_elemente) {

	celula* aux = (*running);
	celula* aux_nou = (*running);
	if (aux != NULL) {
		while (timp > 0) {
			if (((proces*)aux->info)->timp_executie < 300) {
				int i = 0;
			}
			if (aux != NULL) {
				if (((proces*)aux->info)->timp_executie < 2000) {
					int i = 0;
				}
				if (cuantum_timp > ((proces*)aux->info)->cuantum_timp) {
					if (timp > ((proces*)aux->info)->cuantum_timp) {
						((proces*)aux->info)->timp_executat += ((proces*)aux->info)->cuantum_timp;
						((proces*)aux->info)->timp_executie -= ((proces*)aux->info)->cuantum_timp;

						timp -= ((proces*)aux->info)->cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						if (((proces*)aux->info)->timp_executie > 0) {
							aux_nou = aux;
							if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
								aux = extrQ(*coada_asteptare);
								introducereQ(coada_asteptare, (proces*)aux_nou->info);
							}
						}
						else {
							(*memorie_totala) -= ((proces*)aux->info)->mem_size;
							remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
							intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
							aux = extrQ(*coada_asteptare);
							if ((*nr_elemente) == 0) {
								(*running) = NULL;
								return;
							}
						}
					}
					else if (timp == ((proces*)aux->info)->cuantum_timp) {
						((proces*)aux->info)->timp_executat += ((proces*)aux->info)->cuantum_timp;
						((proces*)aux->info)->timp_executie -= ((proces*)aux->info)->cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						if (((proces*)aux->info)->timp_executie > 0) {
							aux_nou = aux;
							if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
								aux = extrQ(*coada_asteptare);
								introducereQ(coada_asteptare, (proces*)aux_nou->info);
							}
						}
						else {
							(*memorie_totala) -= ((proces*)aux->info)->mem_size;
							remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
							intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
							aux = extrQ(*coada_asteptare);
							if ((*nr_elemente) == 0) {
								(*running) = NULL;
								return;
							}
						}
					}
					else {
						((proces*)aux->info)->timp_executat += timp;
						((proces*)aux->info)->timp_executie -= timp;
						((proces*)aux->info)->cuantum_timp -= timp;
						if (((proces*)aux->info)->timp_executie > 0) {
							aux_nou = aux;
							if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
								aux = extrQ(*coada_asteptare);
								introducereQ(coada_asteptare, (proces*)aux_nou->info);
							}
						}
						else {
							(*memorie_totala) -= ((proces*)aux->info)->mem_size;
							remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
							intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
							aux = extrQ(*coada_asteptare);
							if ((*nr_elemente) == 0) {
								(*running) = NULL;
								return;
							}
						}
					}
				}
				if (timp < cuantum_timp) {
					if (timp - (((proces*)aux->info)->timp_executie) == 0) {
						((proces*)aux->info)->timp_executie = 0;
						((proces*)aux->info)->timp_executat += timp;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);		
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
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
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
				}
				else if (timp >= cuantum_timp) {
					if (cuantum_timp - (((proces*)aux->info)->timp_executie) == 0) {
						((proces*)aux->info)->timp_executie = 0;
						((proces*)aux->info)->timp_executat += cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						timp -= cuantum_timp;
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
					}
					else if (cuantum_timp - (((proces*)aux->info)->timp_executie) < 0) {
						((proces*)aux->info)->timp_executie -= cuantum_timp;
						((proces*)aux->info)->timp_executat += cuantum_timp;
						((proces*)aux->info)->cuantum_timp = cuantum_timp;
						timp -= cuantum_timp;
						aux_nou = aux;
						if ((*coada_asteptare)->front != NULL && (*coada_asteptare)->rear != NULL) {
							aux = extrQ(*coada_asteptare);
							introducereQ(coada_asteptare, (proces*)aux_nou->info);
						}
					}
					else {
						((proces*)aux->info)->timp_executat += ((proces*)aux->info)->timp_executie;
						timp -= ((proces*)aux->info)->timp_executie;
						((proces*)aux->info)->timp_executie = 0;
						(*memorie_totala) -= ((proces*)aux->info)->mem_size;
						remove_memorie(((proces*)aux->info)->PID, coada_asteptare, memorie, vect_PID, nr_elemente);
						intrQ(coada_finished, (proces*)aux->info, sizeof(proces));
						aux = extrQ(*coada_asteptare);
						if ((*nr_elemente) == 0) {
							(*running) = NULL;
							return;
						}
						
					}
				}
			}
			else break;
		}	
		(*running) = aux;
	}
}

void* initS(size_t dim) {
	Stack* S;
	S = (Stack*)calloc(1, dim);
	S->dim = dim;
	return (void*)S;
}

void finish(Queue * coada_asteptare, celula * running, FILE* output) {
	int total_time = 0;
	total_time += ((proces*)running->info)->timp_executie;
	celula* aux = coada_asteptare->front;
	while (aux != NULL) {
		total_time += ((proces*)aux->info)->timp_executie;
		aux = aux->urm;
	}
	fprintf(output,  "Total time: %d\n", total_time);
}

void pushS(Memorie **memorie, int PID, int data, FILE* output) {

	Memorie* aux = (*memorie);

	while (aux != NULL) {
		if(aux->info!=NULL)
			if (aux->info->PID == PID) {
				if (sizeof(int) + aux->info->stack_size > aux->info->mem_size) {
					fprintf(output, "Stack overflow PID %d.\n", PID);
					return;
				}
				aux->info->stack_size += sizeof(int);
				celStiva* informatie = (celStiva*)calloc(1, sizeof(celStiva));
				informatie->info = data;
				if (aux->info->stiva->top == NULL) {
					aux->info->stiva->top = informatie;
				}
				else {
					informatie->urm = aux->info->stiva->top;
					aux->info->stiva->top = informatie;
				}
				break;
			}
		aux = aux->urm;
	}
	if (aux == NULL) fprintf(output,  "PID %d not found.\n", PID);
}

void printStack(Memorie **memorie, int PID, FILE* output) {

	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (aux->info->PID == PID) {
			if (aux->info->stiva->top == NULL) fprintf(output,  "Empty stack PID %d.\n", PID);
			else {
				int nr_elemente = 0;
				celStiva* cel = aux->info->stiva->top;
				while (cel != NULL) {
					nr_elemente++;
					cel = cel->urm;
				}
				fprintf(output, "Stack of PID %d: ", PID);
				while (nr_elemente) {
					cel = aux->info->stiva->top;
					int nr_aux = 0;
					while (nr_aux != nr_elemente - 1) {
						nr_aux++;
						cel = cel->urm;
					}
					if (nr_elemente != 1) fprintf(output, "%d ", cel->info);
					else fprintf(output, "%d.\n", cel->info);
					nr_elemente--;
				}
			}
			return;
		}
		aux = aux->urm;
	}
	fprintf(output, "PID %d not found.\n", PID);
}

void pop(Memorie **memorie, int PID, FILE* output) {

	Memorie* aux = (*memorie);
	while (aux != NULL) {
		if (aux->info->PID == PID) {
			if (aux->info->stiva->top == NULL) fprintf(output,  "Empty stack PID %d.\n", PID);
			else {
				aux->info->stiva->top = aux->info->stiva->top->urm;
				aux->info->stack_size -= sizeof(int);
			}
			break;
		}
		aux = aux->urm;
	}
	if (aux == NULL) if (aux == NULL) fprintf(output,  "PID %d not found.\n", PID);
}

int main() {

	int cuantum_timp, memorie_totala = 0, nr_elemente = 0;
	FILE* intrare = fopen("input.txt", "r");
	FILE* iesire = fopen("output.txt", "w");
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
			coada_asteptare = add(&coada_asteptare, &memorie, &nr_elemente, &memorie_totala, mem_size, exec_time, prioritate, cuantum_timp, &vect_PID, &stare_running, &running, iesire);
		}
		if (strcmp(p, "get") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			get(PID, coada_asteptare, running, coada_finished, iesire);
		}

		if (strcmp(p, "print") == 0) {
			p = strtok(NULL, " ");
			if (p[strlen(p) - 1] == '\n')
				p[strlen(p) - 1] = '\0';
			if (strcmp(p, "finished") == 0)
				print_finished(coada_finished, iesire);
			else if (strcmp(p, "waiting") == 0)
				print_waiting(coada_asteptare, iesire);
			else if (strcmp(p, "stack") == 0) {
				p = strtok(NULL, " ");
				int PID = atoi(p);
				printStack(&memorie, PID, iesire);
			}
		}
		if (strcmp(p, "run") == 0) {
			p = strtok(NULL, " ");
			int timp = atoi(p);
			if(coada_asteptare!=NULL)run(timp, cuantum_timp, &memorie_totala, &coada_asteptare, &running, &coada_finished, &memorie, &vect_PID, &nr_elemente);
		}
		if (strcmp(p, "finish\n") == 0) {
			finish(coada_asteptare, running, iesire);
			return 0;
		}
		if (strcmp(p, "push") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			p = strtok(NULL, " ");
			int data = atoi(p);
			pushS(&memorie, PID, data, iesire);
		}
		if (strcmp(p, "pop") == 0) {
			p = strtok(NULL, " ");
			int PID = atoi(p);
			pop(&memorie, PID, iesire);
		}
	}
	return 0;
}
