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
#include <time.h>

#define N 8
#define NUM_ITER 20

#define PRODUCER_COLOR  "\x1b[36m"
#define CONSUMER_COLOR  "\x1b[31m"
#define NO_COLOR        "\x1b[0m"
#define bold(text) "\x1b[1m" text "\x1b[22m"

#define fail(message)  { perror(message); exit(EXIT_FAILURE); }
#define producer_printf(format, ...) ( printf(PRODUCER_COLOR format NO_COLOR, ##__VA_ARGS__) )
#define consumer_printf(format, ...) ( printf(CONSUMER_COLOR format NO_COLOR, ##__VA_ARGS__) )

void producer(void);
void consumer(void);

/**
 * Genera un entero aleatorio entre 0 y 10.
 * @return Entero entre 0 y 10
 */
int produce_item(void);

/**
 * Coloca el entero <u>item</u> en la posición del buffer
 * apuntada por count.
 * @param item	Entero a colocar en el buffer
 */
void insert_item(int item);

/**
 * Retira el último elemento del buffer. Acumula en <u>total</u>
 * la suma de todos los valores contenidos en ese momento en el buffer.
 * @param total	Variable de salida para contener la suma de todos los elementos del buffer.
 * @return	Entero en la posición count-1 del buffer
 */
int remove_item(int* total);

/**
 * Muestra por pantalla el entero <u>item</u>, así como el resultado
 * de la suma de los valores contenidos en el buffer (<u>total</u>).
 * @param item	Último valor leído del buffer
 * @param total	Suma de los valores contenidos en el buffer
 */
void consume_item(int item, int total);

void represent_buffer(int size);


int* buffer;
int* count;
char representation[3 * N + 2];

int main(int argc, char** argv) {
	/* Crear el buffer compartido */
	buffer = (int *) mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (buffer == MAP_FAILED) fail("Error al crear el buffer compartido");
	/* Crear un espacio compartido para almacenar la cuenta */
	count = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (count == MAP_FAILED) fail("Error al crear el contador compartido");

	if (fork()) {
		consumer();
	} else {
		producer();
	}

	if (munmap(buffer, N * sizeof(int))) fail("Fallo al liberar el buffer");
	if (munmap(count, sizeof(int))) fail("Fallo al liberar el contador");
}


void producer() {
	int item;
	int iter = 0;

	srandom(time(NULL));
	while(iter < NUM_ITER){
		item = produce_item();          /* Generar el siguiente elemento */
		while (*count == N);            /* Si el buffer está lleno, hacer espera activa */
		sleep((int) random() % 4);  /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		insert_item(item);              /* Poner item en el buffer */
		(*count)++;                     /* Incrementar la cuenta de items en el buffer */

        represent_buffer(*count);
        producer_printf("Se ha añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer: %s\n", item, *count, representation);
		iter++;
	}
}

void consumer() {
	int item;
	int total;
	int iter = 0;

	while(iter < NUM_ITER){
		while (*count == 0);            /* Si el buffer está vacío, hacer espera activa */
		sleep((int) random() % 4);  /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		item = remove_item(&total);     /* Generar el siguiente elemento */
		(*count)--;                     /* Decrementar la cuenta de items en el buffer */
        represent_buffer(*count);
		consume_item(item, total);      /* Imprimir elemento */

		iter++;
	}
}

int produce_item() {
	return ((int) random() % 11);
}

void insert_item(int item) {
	buffer[*count] = item;
}

int remove_item(int* total) {
	int item;
	int i;

	item = buffer[*count - 1];
	buffer[*count - 1] = 0; /* Eliminar el entero del buffer */
	*total = item;
	for (i = N; i >= 0; i--) {
		*total += buffer[i];
	}
	return item;
}

void consume_item(int item, int total) {
	consumer_printf("Se ha eliminado el item "bold("%2d")" de la posición "bold("%d")". Buffer: %s. Suma total anterior: "bold("%2d")".\n", item , *count + 1, representation, total);
}

void represent_buffer(int size) {
    int i;
    representation[0] = '[';
    for (i = 0; i < size; i++) {
        sprintf(representation + 1 + 3 * i, "%2d|", buffer[i]);
    }
    for (; i < N; i++) {
        sprintf(representation + 1 + 3 * i, "  |");
    }
    representation[3 * N] = ']';
}


