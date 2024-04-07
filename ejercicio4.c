/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 4
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

#define N 8
#define NUM_ITER 20

#define PRODUCER_COLOR  "\x1b[36m"
#define CONSUMER_COLOR  "\x1b[31m"
#define NO_COLOR        "\x1b[0m"
#define bold(text) "\x1b[1m" text "\x1b[22m"

#define fail(message)  { perror(message); exit(EXIT_FAILURE); }
#define producer_printf(format, ...) ( printf(PRODUCER_COLOR "[%d] " format NO_COLOR, getpid(), ##__VA_ARGS__) )
#define consumer_printf(format, ...) ( printf(CONSUMER_COLOR "[%d] " format NO_COLOR, getpid(), ##__VA_ARGS__) )


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

/**
 * Genera un entero aleatorio entre 0 y 10.
 *
 * @return Entero entre 0 y 10
 */
int produce_item(void);

/**
 * Coloca el entero <u>item</u> en <u>stack</u>.
 *
 * @param stack Stack en el que colocar el item.
 * @param item	Entero a colocar en el stack.
 *
 */
void insert_item(Stack* stack, int item);

/**
 * Retira el último elemento del <u>stack</u>. Acumula en <u>total</u>
 * la suma de todos los valores contenidos en ese momento en el buffer.
 *
 * @param stack Stack del que retirar el item.
 * @param total	Variable de salida para contener la suma de todos los elementos del buffer.
 *
 * @return	Entero retirado del stack.
 */
int remove_item(Stack* stack, int* total);

/**
 * Muestra por pantalla el entero consumido <u>item</u>, así como el resultado
 * de la suma de los valores contenidos en el <u>stack</u> (<u>total</u>) y la 
 * representación del stack actualizado.
 *
 * @param stack Stack del que se leyó el valor.
 * @param item	Último valor leído del stack.
 * @param total	Suma de los valores contenidos en el stack.
 */
void consume_item(Stack* stack, int item, int total);



int main(int argc, char** argv) {
    Stack stack;
    int producers, consumers;
    int i;

    if (argc != 3) {
        printf("ERROR: Número de parámetros incorrecto.\nUso: %s <productores> <consumidores>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    producers = (int) strtol(argv[1], NULL, 10);
    consumers = (int) strtol(argv[2], NULL, 10);

    if (producers <= 0 || consumers <= 0) fail("El número de consumidores y productores debe ser positivo");

	/* Crear el stack compartido */
    create_stack(&stack, N, "stack");

    for (i = 0; i < producers; i++) {
        if (!fork()) {
            producer(&stack);
            goto clean;
        }
    }
    for (i = 0; i < consumers; i++) {
        if (!fork()) {
            consumer(&stack);
            goto clean;
        }
    }

clean:
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

