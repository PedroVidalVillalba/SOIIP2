/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 2
 * @date 19/03/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/time.h>
#include "shared_stack.h"
#include "producer_consumer.h"

#define N 8
#define NUM_ITER 20

char* producer_color = PRODUCER_COLOR_1;
char* consumer_color = CONSUMER_COLOR_1;


/**
 * Produce un item y lo coloca en <u>stack</u>.
 *
 * @param stack Stack en el que colocar el item producido.
 */
void producer(Stack* stack);

/**
 * Consume un item de <u>stack</u>.
 *
 * @param stack Stack del que consumir el item.
 */
void consumer(Stack* stack);



int main(int argc, char** argv) {
    Stack stack;

	/* Crear el stack compartido */
    create_stack(&stack, N, "stack");

	if (fork()) {   /* Código del padre */
		consumer(&stack);
	} else {        /* Código del hijo */
		producer(&stack);
	}

    delete_stack(&stack);

    exit(EXIT_SUCCESS);
}


void producer(Stack* stack) {
	int item;
	int iter = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
	while(iter < NUM_ITER){
        sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
        item = produce_item();          /* Generar el siguiente elemento */
        sem_wait(stack->empty);         /* Disminuir el contador de posiciones vacías */
        sem_wait(stack->mutex);         /* Entra en la región crítica */
        insert_item(stack, item);       /* Poner item en el stack */
        sem_post(stack->mutex);         /* Sale de la región crítica */
        sem_post(stack->full);          /* Incrementar el contador de posiciones ocupadas */

        /* Como ya aumentamos el contador, no tenemos que sumarle 1 para pasarlo a indexado en 1 */
        producer_printf("Añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer (%s): %s\n", item, stack->count, stack->name, stack->representation);

		iter++;
	}
}

void consumer(Stack* stack) {
	int item;
	int total;
	int iter = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
	while(iter < NUM_ITER){
        sleep((int) random() % 4);              /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
        sem_wait(stack->full);                  /* Disminuir el contador de posiciones ocupadas */
        sem_wait(stack->mutex);                 /* Entra en la región crítica */
        item = remove_item(stack, &total);      /* Generar el siguiente elemento */
        sem_post(stack->mutex);                 /* Sale de la región crítica */
        sem_post(stack->empty);                 /* Incrementar el contador de posiciones vacías */
        consume_item(stack, item, total);       /* Imprimir elemento */

		iter++;
	}
}
