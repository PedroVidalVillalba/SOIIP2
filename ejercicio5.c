/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 5
 * @date 05/04/2024
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
#include <errno.h>
#include "shared_stack.h"
#include "producer_consumer.h"

#define SIZE_1 8
#define SIZE_2 10
#define NUM_ITER 20

/* Variables globales con los formatos de color para productor y consumidor */
char* producer_color;
char* consumer_color;


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
    Stack stack_1, stack_2;
    int iter = 0;
    struct timeval tv;

    /* Crear los stacks compartidos */
    create_stack(&stack_1, SIZE_1, "stack_1");
    create_stack(&stack_2, SIZE_2, "stack_2");

	if (!fork()) { /* Código del hijo */
        /* Aleatorizar la semilla para los valores aleatorios generados */
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);

        /* Ajustar los colores para la impresión */
        consumer_color = CONSUMER_COLOR_1;
        producer_color = PRODUCER_COLOR_2;

        while(iter < NUM_ITER){
            consumer(&stack_1);
            producer(&stack_2);

            iter++;
        }
	} else { /* Código del padre */
        /* Aleatorizar la semilla para los valores aleatorios generados */
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);

        /* Ajustar los colores para la impresión */
        producer_color = PRODUCER_COLOR_1;
        consumer_color = CONSUMER_COLOR_2;

        while(iter < NUM_ITER){
            producer(&stack_1);
            consumer(&stack_2);

            iter++;
        }
	}

    /* Eliminar los stacks compartidos */
    delete_stack(&stack_1);
    delete_stack(&stack_2);

    exit(EXIT_SUCCESS);
}


void producer(Stack* stack) {
	int item;
    int i, r;

    sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
    r = random() % 3 + 1;           /* Generar un número aleatorio entre 1 y 3 para insertar items */
    for (i = 0; i < r; i++) {
        item = produce_item();          /* Generar el siguiente elemento */
        /* Intentar disminuir el contador de posiciones vacías; si valía 0, retornar para consumir */
        if (sem_trywait(stack->empty) && errno == EAGAIN) return;         
        sem_wait(stack->mutex);         /* Entra en la región crítica */
        insert_item(stack, item);       /* Poner item en el stack */
        sem_post(stack->mutex);         /* Sale de la región crítica */
        sem_post(stack->full);          /* Incrementar el contador de posiciones ocupadas */

        /* Como ya aumentamos el contador, no tenemos que sumarle 1 para pasarlo a indexado en 1 */
        producer_printf("Añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer (%s): %s\n", item, stack->count, stack->name, stack->representation);
    }
}

void consumer(Stack* stack) {
	int item;
	int total;
    int i, r;

    sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
    r = random() % 3 + 1;           /* Generar un número aleatorio entre 1 y 3 para consumir items */
    for (i = 0; i < r; i++) {
        /* Intentar disminuir el contador de posiciones ocupadas; si valía 0, retornar para producir */
        if (sem_trywait(stack->full) && errno == EAGAIN) return;         
        sem_wait(stack->mutex);                 /* Entra en la región crítica */
        item = remove_item(stack, &total);      /* Generar el siguiente elemento */
        sem_post(stack->mutex);                 /* Sale de la región crítica */
        sem_post(stack->empty);                 /* Incrementar el contador de posiciones vacías */
        consume_item(stack, item, total);       /* Imprimir elemento */
    }
}

