/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 2
 * @date 19/03/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
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

void print_buffer();

int* buffer;
int count;
sem_t* mutex;
sem_t* full;
sem_t* empty;

int main(int argc, char** argv) {
	/* Crear el buffer compartido */
	buffer = (int *) mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (buffer == MAP_FAILED) fail("Error al crear el buffer compartido");

	/* Eliminar los semáforos que pudiese haber previamente con el nombre que se va a utilizar */
	sem_unlink("/mutex");
	sem_unlink("/full");
	sem_unlink("/empty");

	/* Abrir los semáforos */
	mutex = sem_open("/mutex", O_CREAT, 0700, 1);
	full = sem_open("/full", O_CREAT, 0700, 0);
	empty = sem_open("/empty", O_CREAT, 0700, N);

	if (fork()) {
		consumer();
	} else {
		producer();
	}

	if (munmap(buffer, N * sizeof(int))) fail("Fallo al liberar el buffer");

	sem_close(mutex);
	sem_close(full);
	sem_close(empty);

	sem_unlink("/mutex");
	sem_unlink("/full");
	sem_unlink("/empty");
}


void producer() {
	int item;
	int iter = 0;

	srandom(time(NULL));
	while(iter < NUM_ITER){
		sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		item = produce_item();          /* Generar el siguiente elemento */
		sem_wait(empty);                /* Disminuir el contador de posiciones vacías */
		sem_wait(mutex);                /* Entra en la región crítica */
		insert_item(item);              /* Poner item en el buffer */
		sem_post(mutex);                /* Sale de la región crítica */
		sem_post(full);                 /* Incrementar el contador de posiciones ocupadas */

        producer_printf("Se ha añadido el item   "bold("%2d")" a la posición  "bold("%d")".                 Buffer: ", item, count + 1);
        print_buffer();

		iter++;
	}
}

void consumer() {
	int item;
	int total;
	int iter = 0;

	while(iter < NUM_ITER){
		sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		sem_wait(full);                 /* Disminuir el contador de posiciones ocupadas */
		sem_wait(mutex);                /* Entra en la región crítica */
		item = remove_item(&total);     /* Generar el siguiente elemento */
		sem_post(mutex);                /* Sale de la región crítica */
		sem_post(empty);                /* Incrementar el contador de posiciones vacías */
		consume_item(item, total);      /* Imprimir elemento */

		iter++;
	}
}

int produce_item() {
	return ((int) random() % 11);
}

void insert_item(int item) {
	sem_getvalue(full, &count);
	buffer[count] = item;
}

int remove_item(int* total) {
	int item;
	int i;

	sem_getvalue(full, &count);
	item = buffer[count];
	buffer[count] = 0; /* Eliminar el entero del buffer */
	*total = item;
	for (i = count - 1; i >= 0; i--) {
		*total += buffer[i];
	}
	return item;
}

void consume_item(int item, int total) {
	consumer_printf("Se ha eliminado el item "bold("%2d")" de la posición "bold("%d")". Suma total: "bold("%2d")". Buffer: ", item , count + 1, total);
    print_buffer();
}

void print_buffer() {
    int i;
    putchar('[');
    for (i = 0; i < count; i++) {
        printf(bold("%2d")"|", buffer[i]);
    }
    for (; i < N; i++) {
        printf("  |");
    }
    printf("\b]\n");
}
