/**
 * Sistemas Operativos 2. Práctica 2.
 * Librería con las implementaciones de las funciones genéricas de
 * para crear, representar y destruir un stack compartido controlado
 *
 * @date 07/04/2024
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


void create_stack(Stack* stack, int size, const char* name) {
    char mutex_name[256] = {0};
    char full_name[256] = {0};
    char empty_name[256] = {0};

    /* Crear el buffer en una zona de memoria compartida */
    stack->buffer = (int *) mmap(NULL, size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (stack->buffer == MAP_FAILED) fail("Error al crear el buffer compartido");
    stack->size = size;
    stack->count = 0;   /* El buffer se inicia vacío */

    /* Guardar el nombre del stack */
    stack->name = (char *) calloc(strlen(name) + 1, sizeof(char)); 
    memcpy(stack->name, name, strlen(name) * sizeof(char));
    /* Crear los semáforos asociados al uso del buffer */
    /* Contruir los nombres de cada semáforo a partir del nombre del stack */
    sprintf(mutex_name, "/mutex_%s", name);
    sprintf(full_name, "/full_%s", name);
    sprintf(empty_name, "/empty_%s", name);

    /* Eliminar los semáforos que pudiese haber previamente con el nombre que se va a utilizar */
    sem_unlink(mutex_name);
    sem_unlink(full_name);
    sem_unlink(empty_name);

    /* Crear los semáforos con sus valores iniciales */
    stack->mutex = sem_open(mutex_name, O_CREAT, 0700, 1);
    stack->full = sem_open(full_name, O_CREAT, 0700, 0);
    stack->empty = sem_open(empty_name, O_CREAT, 0700, size);

    /* Alojar memoria para la representación textual del stack */
    /* Tres caracteres por item (2 del número + barra de separación/corchete final), corchete inicial y \0 final */
    stack->representation = (char *) calloc(3 * size + 2, sizeof(char));    
}


void delete_stack(Stack* stack) {
    char mutex_name[256] = {0};
    char full_name[256] = {0};
    char empty_name[256] = {0};

    /* Liberar el buffer en memoria compartida */
	if (munmap(stack->buffer, stack->size * sizeof(int))) fail("Fallo al liberar el buffer");
    stack->buffer = NULL;
    stack->size = -1;   /* Marcar con valores de error */
    stack->count = -1;

    /* Cerrar semáforos */
	sem_close(stack->mutex);
	sem_close(stack->full);
	sem_close(stack->empty);

    /* Contruir los nombres de cada semáforo a partir del nombre del stack */
    sprintf(mutex_name, "/mutex_%s", stack->name);
    sprintf(full_name, "/full_%s", stack->name);
    sprintf(empty_name, "/empty_%s", stack->name);

    /* Eliminar semáforos */
    sem_unlink(mutex_name);
    sem_unlink(full_name);
    sem_unlink(empty_name);

    /* Liberar las cadenas de texto */
    free(stack->name);
    stack->name = NULL;
    free(stack->representation);
    stack->representation = NULL;
}


void update_representation(Stack* stack) {
    int i;

    stack->representation[0] = '[';
    /* Imprimir los items en el stack hasta count */
    for (i = 0; i < stack->count; i++) {
        sprintf(stack->representation + 1 + 3 * i, "%2d|", stack->buffer[i]);
    }
    /* Rellenar con huecos el resto de espacios */
    for (; i < stack->size; i++) {
        sprintf(stack->representation + 1 + 3 * i, "  |");
    }
    stack->representation[3 * stack->size] = ']';
}

