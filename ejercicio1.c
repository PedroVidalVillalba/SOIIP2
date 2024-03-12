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

#define N 8
#define NUM_ITER 1<<5

#define PRODUCER_COLOR  "\x1b[34m"
#define CONSUMER_COLOR  "\x1b[31m"
#define NO_COLOR        "\x1b[0m"

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


int* buffer;
int* count;

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

	while(iter < NUM_ITER){
		item = produce_item();      /* Generar el siguiente elemento */
		while (*count == N);        /* Si el buffer está lleno, hacer espera activa */
		insert_item(item);          /* Poner item en el buffer */
		(*count)++;                 /* Incrementar la cuenta de items en el buffer */

		producer_printf("Se ha añadido el item %d a la posición %d\n", item, *count);
		iter++;
		usleep(100000);
	}
}

void consumer() {
	int item;
	int total;
	int iter = 0;

	while(iter < NUM_ITER){
		while (*count == 0);        /* Si el buffer está vacío, hacer espera activa */
		item = remove_item(&total);       /* Generar el siguiente elemento */
		(*count)--;                 /* Decrementar la cuenta de items en el buffer */
		consume_item(item, total);         /* Imprimir elemento */

		consumer_printf("Se ha eliminado el item %d de la posición %d\n", item, *count+1);
		iter++;
		usleep(500000);
	}
}

int produce_item() {
	return (rand() % 11);
}

void insert_item(int item) {
	buffer[*count] = item;
}

int remove_item(int* total) {
	int item;
	int i;

	item = buffer[*count - 1];
	buffer[*count] = 0; /* Eliminar el entero del buffer */
	*total = item;
	for (i = *count - 2; i >= 0; i--) {
		*total += buffer[i];
	}
	return item;
}

void consume_item(int item, int total) {
	printf("Entero leído: %d. Suma total: %d\n", item , total);
}