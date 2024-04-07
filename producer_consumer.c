/**
 * Sistemas Operativos 2. Práctica 2.
 * Librería con las implementaciones de las funciones genéricas de
 * productor y consumidor usando un stack compartido controlado
 * por semáforos.
 *
 * @date 07/04/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#include "producer_consumer.h"
#include <semaphore.h>


int produce_item() {
    return ((int) random() % 11);
}


void insert_item(Stack* stack, int item) {
    /* Almacenar el número de items actualmente en el stack, según lo ve el proceso que inserta,
     * en el campo count del stack */
    sem_getvalue(stack->full, &(stack->count));
    stack->buffer[(stack->count)++] = item; /* Insertar el item y aumentar el contador */
    update_representation(stack);   /* Guardar la representación del buffer que el productor ve en este momento */
}


int remove_item(Stack* stack, int* total) {
    int item;
    int i;

    /* Almacenar el número de items actualmente en el stack, según lo ve el proceso que elimina,
     * en el campo count del stack */
    sem_getvalue(stack->full, &(stack->count));
    item = stack->buffer[stack->count];
    stack->buffer[stack->count] = 0; /* Eliminar el entero del buffer */
    /* Acumular la suma de los elementos del stack en total */
    *total = item;
    for (i = stack->count - 1; i >= 0; i--) {
        *total += stack->buffer[i];
    }
    update_representation(stack);

    return item;
}


void consume_item(Stack* stack, int item, int total) {
    consumer_printf("Eliminado el item "bold("%2d")" de la posición "bold("%d")". Buffer (%s): %s. Suma total anterior: "bold("%2d")".\n",
            item , stack->count + 1, stack->name, stack->representation, total);
}
