/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 3
 * @date 02/04/2024
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

/* Variables globales con los formatos de color para productor y consumidor */
char* producer_color = PRODUCER_COLOR_1;
char* consumer_color = CONSUMER_COLOR_1;


/* Adaptar la interfaz de las funciones de productor y consumidor al uso de hilos */
/**
 * Produce un item y lo coloca en <u>stack</u>.
 *
 * @param stack Stack en el que colocar el item producido.
 */
void* producer(void* stack);

/**
 * Consume un item de <u>stack</u>.
 *
 * @param stack Stack del que consumir el item.
 */
void* consumer(void* stack);

/**
 * Copia la información de un stack a otro, compartiendo el mismo buffer
 * y semáforos, pero separando la representación textual y el contador.
 *
 * @param destiny   Puntero a la posición de memoria en la que copiar el stack.
 * @param source    Puntero al stack a ser copiado.
 */
void clone_stack(Stack* destiny, Stack* source);



int main(int argc, char** argv) {
    int status;
    pthread_t producer_thread, consumer_thread; 
    /* Al tratarse de hilos, ahora las variables son compartidas, así que tenemos que desdoblar la información de cada stack que ve cada hilo */
    Stack producer_stack, consumer_stack;

    create_stack(&producer_stack, N, "stack");
    clone_stack(&consumer_stack, &producer_stack);

    //Creación de los hilos
    status = pthread_create(&producer_thread, NULL, producer, &producer_stack);
    //Comprobación de que se crea correctamente
    if (status != 0) {
        printf("Error en la creación del hilo. Status: %d\n", status);
    }

    status = pthread_create(&consumer_thread, NULL, consumer, &consumer_stack);
    //Comprobación de que se crea correctamente
    if (status != 0) {
        printf("Error en la creación del hilo. Status: %d\n", status);
    }

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    delete_stack(&producer_stack);
    /* El stack clonado tiene las mismas variables que el original excepto la representación, que es la única que debe liberarse a parte*/
    free(consumer_stack.representation);    

    exit(EXIT_SUCCESS);
}


void* producer(void * stack_) {
    Stack* stack = (Stack *) stack_;
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
		insert_item(stack, item);       /* Poner item en el buffer */
		sem_post(stack->mutex);         /* Sale de la región crítica */
		sem_post(stack->full);          /* Incrementar el contador de posiciones ocupadas */

        /* Como ya aumentamos el contador, no tenemos que sumarle 1 para pasarlo a indexado en 1 */
        producer_printf("Añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer (%s): %s\n", item, stack->count, stack->name, stack->representation);

		iter++;
	}
    pthread_exit(NULL);
}

void* consumer(void* stack_) {
    Stack* stack = (Stack *) stack_;
	int item;
	int total;
	int iter = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
	while(iter < NUM_ITER){
		sleep((int) random() % 4);          /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		sem_wait(stack->full);              /* Disminuir el contador de posiciones ocupadas */
		sem_wait(stack->mutex);             /* Entra en la región crítica */
		item = remove_item(stack, &total);  /* Generar el siguiente elemento */
		sem_post(stack->mutex);             /* Sale de la región crítica */
		sem_post(stack->empty);             /* Incrementar el contador de posiciones vacías */
		consume_item(stack, item, total);   /* Imprimir elemento */

		iter++;
	}
    pthread_exit(NULL);
}

void clone_stack(Stack* destiny, Stack* source) {
    /* Copiar tal cual toda la información de la fuente al destino.
     * Con esto mantenemos el buffer, los semáfores, el nombre y la representación,
     * pero desacoplamos el contador (y el tamaño, aunque no vayamos a cambiarlo). */
    memcpy(destiny, source, sizeof(Stack)); 
    /* Reservar memoria para una representación textual independiente */
    destiny->representation = (char *) calloc(3 * destiny->size + 2, sizeof(char));
}
