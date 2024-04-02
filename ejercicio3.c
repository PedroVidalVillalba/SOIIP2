/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 3
 * @date 02/04/2024
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
#include <sys/time.h>

#define N 8
#define NUM_ITER 20

#define PRODUCER_COLOR  "\x1b[36m"
#define CONSUMER_COLOR  "\x1b[31m"
#define NO_COLOR        "\x1b[0m"
#define bold(text) "\x1b[1m" text "\x1b[22m"

#define fail(message)  { perror(message); exit(EXIT_FAILURE); }
#define producer_printf(format, ...) ( printf(PRODUCER_COLOR format NO_COLOR, ##__VA_ARGS__) )
#define consumer_printf(format, ...) ( printf(CONSUMER_COLOR format NO_COLOR, ##__VA_ARGS__) )


/* Adaptar la interfaz de las funciones al uso de hilos */
void* producer(void*);
void* consumer(void*);

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

void represent_buffer(char* representation, int size);

int* buffer;
int count_producer, count_consumer;     /* Desdoblamos las variables en las que guardamos la posición actual para cada hilo, por ser ahora compartidas */
sem_t* mutex;
sem_t* full;
sem_t* empty;
/* Desdoblamos también la representación que cada hilo tiene del buffer en un momento dado */
char representation_producer[3 * N + 2];
char representation_consumer[3 * N + 2];

int main(int argc, char** argv) {
    int status;
    pthread_t producer_thread, consumer_thread;
	/* Crear el buffer compartido; al usar hilos, puede estar simplemente como variable global en lugar de memoria compartida */
    buffer = (int *) calloc(N, sizeof(int));

	/* Eliminar los semáforos que pudiese haber previamente con el nombre que se va a utilizar */
	sem_unlink("/mutex");
	sem_unlink("/full");
	sem_unlink("/empty");

	/* Abrir los semáforos */
	mutex = sem_open("/mutex", O_CREAT, 0700, 1);
	full = sem_open("/full", O_CREAT, 0700, 0);
	empty = sem_open("/empty", O_CREAT, 0700, N);

    //Creación de los hilos
    status = pthread_create(&producer_thread, NULL, producer, NULL);
    //Comprobación de que se crea correctamente
    if (status != 0) {
        printf("Error en la creación del hilo. Status: %d\n", status);
    }

    status = pthread_create(&consumer_thread, NULL, consumer, NULL);
    //Comprobación de que se crea correctamente
    if (status != 0) {
        printf("Error en la creación del hilo. Status: %d\n", status);
    }

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    free(buffer);

	sem_close(mutex);
	sem_close(full);
	sem_close(empty);

	sem_unlink("/mutex");
	sem_unlink("/full");
	sem_unlink("/empty");

    exit(EXIT_SUCCESS);
}


void* producer(void * _) {
	int item;
	int iter = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
	while(iter < NUM_ITER){
		sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
		item = produce_item();          /* Generar el siguiente elemento */
		sem_wait(empty);                /* Disminuir el contador de posiciones vacías */
		sem_wait(mutex);                /* Entra en la región crítica */
		insert_item(item);              /* Poner item en el buffer */
		sem_post(mutex);                /* Sale de la región crítica */
		sem_post(full);                 /* Incrementar el contador de posiciones ocupadas */

        producer_printf("Se ha añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer: %s\n", item, count_producer + 1, representation_producer);

		iter++;
	}
    pthread_exit(NULL);
}

void* consumer(void* _) {
	int item;
	int total;
	int iter = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
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
    pthread_exit(NULL);
}

int produce_item() {
	return ((int) random() % 11);
}

void insert_item(int item) {
	sem_getvalue(full, &count_producer);
	buffer[count_producer] = item;
    represent_buffer(representation_producer, count_producer + 1);
}

int remove_item(int* total) {
	int item;
	int i;

	sem_getvalue(full, &count_consumer);
	item = buffer[count_consumer];
	buffer[count_consumer] = 0; /* Eliminar el entero del buffer */
	*total = item;
	for (i = count_consumer - 1; i >= 0; i--) {
		*total += buffer[i];
	}
    represent_buffer(representation_consumer, count_consumer);
	return item;
}

void consume_item(int item, int total) {
    consumer_printf("Se ha eliminado el item "bold("%2d")" de la posición "bold("%d")". Buffer: %s. Suma total anterior: "bold("%2d")".\n", item , count_consumer + 1, representation_consumer, total);
}

void represent_buffer(char* representation, int size) {
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
