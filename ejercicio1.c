/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 1
 * @date 12/03/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
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

/*** Estructuras y funciones asociadas a la creación y destrucción del stack,
 * así como su presentación textual ***/
/**
 * Estructura en la que incluir toda la información relevante de un buffer
 * compartido sin semáforos.
 */
typedef struct {
    int* buffer;            /* Buffer en memoria compartida donde guardar los datos */
    int size;               /* Tamaño del buffer */
    int* count;             /* Posición del último elemento que hay actualmente en el buffer (variable compartida) */
    char* representation;   /* Representación imprimible de los elementos del stack */
} Stack;

/**
 * Crea un nuevo Stack en memoria compartida, de tamaño <u>size</u>.
 *
 * @param stack Dirección en la que guardar el nuevo Stack.
 * @param size  Tamaño del buffer.
 */
void create_stack(Stack* stack, int size);

/**
 * Elimina un Stack de memoria, liberando el buffer de memoria compartida,
 * su representación textual y el contador compartido.
 *
 * @param stack Puntero al Stack a destruir.
 */
void delete_stack(Stack* stack);

/**
 * Actualiza la representación textual del <u>stack</u>.
 *
 * @param stack Stack a actualizar.
 * @param count Número de elementos a incluir en la representación.
 */
void update_representation(Stack* stack, int count);

/*** Funciones genéricas del productor y consumidor ***/

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

/*** Funciones específicas para el trabajo del productor y consumidor ***/

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

    create_stack(&stack, N);

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
		item = produce_item();          /* Generar el siguiente elemento */
		while (*(stack->count) == N);            /* Si el buffer está lleno, hacer espera activa */
		insert_item(stack, item);              /* Poner item en el buffer */
        sleep((int) random() % 4);  /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		(*(stack->count))++;                     /* Incrementar la cuenta de items en el buffer */

        producer_printf("Añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer: %s\n", item, *(stack->count), stack->representation);
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
		while (*(stack->count) == 0);            /* Si el buffer está vacío, hacer espera activa */
		item = remove_item(stack, &total);     /* Generar el siguiente elemento */
        sleep((int) random() % 4);  /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		(*(stack->count))--;                     /* Decrementar la cuenta de items en el buffer */
		consume_item(stack, item, total);      /* Imprimir elemento */

		iter++;
	}
}

int produce_item() {
	return ((int) random() % 11);
}

void insert_item(Stack* stack, int item) {
    /* Almacenar el número de items actualmente en el stack, según lo ve el proceso que inserta,
     * en el campo count del stack */
    stack->buffer[*(stack->count)] = item;  /* Insertar el item y aumentar el contador */
    update_representation(stack, *(stack->count) + 1);           /* Guardar la representación del buffer que el productor ve en este momento */
}

int remove_item(Stack* stack, int* total) {
	int item;
	int i;

	item = stack->buffer[*(stack->count) - 1];
	stack->buffer[*(stack->count) - 1] = 0; /* Eliminar el entero del buffer */
	*total = item;
	for (i = *(stack->count) - 1; i >= 0; i--) {
		*total += stack->buffer[i];
	}

    update_representation(stack, *(stack->count) - 1);
	return item;
}

void consume_item(Stack* stack, int item, int total) {
    consumer_printf("Eliminado el item "bold("%2d")" de la posición "bold("%d")". Buffer: %s. Suma total anterior: "bold("%2d")".\n",
                    item , *(stack->count) + 1, stack->representation, total);
}


void create_stack(Stack* stack, int size) {
    /* Crear el buffer en una zona de memoria compartida */
    stack->buffer = (int *) mmap(NULL, size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (stack->buffer == MAP_FAILED) fail("Error al crear el buffer compartido");
    stack->size = size;
    /* Crear un espacio compartido para almacenar la cuenta */
    stack->count = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (stack->count == MAP_FAILED) fail("Error al crear el contador compartido");

    /* Alojar memoria para la representación textual del stack */
    /* Tres caracteres por item (2 del número + barra de separación/corchete final), corchete inicial y \0 final */
    stack->representation = (char *) calloc(3 * size + 2, sizeof(char));
}


void delete_stack(Stack* stack) {
    /* Liberar el buffer en memoria compartida */
    if (munmap(stack->buffer, stack->size * sizeof(int))) fail("Fallo al liberar el buffer");
    stack->buffer = NULL;
    stack->size = -1;   /* Marcar con valores de error */
    /* Liberar la cuenta compartida */
    if (munmap(stack->count, sizeof(int))) fail("Fallo al liberar el contador");
    stack->count = NULL;

    /* Liberar la representación de texto */
    free(stack->representation);
    stack->representation = NULL;
}


void update_representation(Stack* stack, int count) {
    int i;

    stack->representation[0] = '[';
    /* Imprimir los items en el stack hasta count */
    for (i = 0; i < count; i++) {
        sprintf(stack->representation + 1 + 3 * i, "%2d|", stack->buffer[i]);
    }
    /* Rellenar con huecos el resto de espacios */
    for (; i < stack->size; i++) {
        sprintf(stack->representation + 1 + 3 * i, "  |");
    }
    stack->representation[3 * stack->size] = ']';
}

