/**
 * Sistemas Operativos 2. Práctica 2.
 * Librería con las cabeceras de las funciones genéricas de
 * para crear, representar y destruir un stack compartido controlado
 *
 * @date 07/04/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#ifndef SHARED_STACK
#define SHARED_STACK

#include <sys/types.h>
#include <semaphore.h>

#define fail(message)  { perror(message); exit(EXIT_FAILURE); }

typedef struct {
    int* buffer;
    int size;
    int count;
    sem_t* mutex;
    sem_t* full;
    sem_t* empty;
    char* name;
    char* representation;
} Stack;

/**
 * Crea un nuevo Stack en memoria compartida, de tamaño <u>size</u>,
 * y con sus correspondientes semafóros identificados por el nombre
 * (e.g., "/mutex_<name>").
 *
 * @param stack Dirección en la que guardar el nuevo Stack.
 * @param size  Tamaño del buffer.
 * @param name  Nombre con el que identificar a los semáforos del nuevo Stack.
 */
void create_stack(Stack* stack, int size, const char* name);

/**
 * Elimina un Stack de memoria, liberando el buffer de memoria compartida,
 * su representación textual y cerrando y destruyendo sus semáforos asociados.
 *
 * @param stack Puntero al Stack a destruir.
 */
void delete_stack(Stack* stack);

/**
 * Actualiza la representación textual del <u>stack</u>.
 *
 * @param stack Stack a actualizar.
 */
void update_representation(Stack* stack);

#endif //SHARED_STACK
