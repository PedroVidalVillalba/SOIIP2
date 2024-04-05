/**
 * Sistemas Operativos 2. Práctica 2.
 * Ejercicio 5
 * @date 05/04/2024
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

#define BUFFER_PRODUCER 8
#define BUFFER_CONSUMER 10
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

/** Variables de cada buffer */

int* buffer_producer;
int count_producer;
sem_t* mutex_producer;
sem_t* full_producer;
sem_t* empty_producer;
char* representation_producer;

int* buffer_consumer;
int count_consumer;
sem_t* mutex_consumer;
sem_t* full_consumer;
sem_t* empty_consumer;
char* representation_consumer;


Stack stacks[NUM_BUFFERS];


int main(int argc, char** argv) {

    int iter=0;
    struct timeval tv;

    /* Crear los buffers compartidos */
    buffer_producer = (int *) mmap(NULL, BUFFER_PRODUCER * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    buffer_consumer = (int *) mmap(NULL, BUFFER_CONSUMER * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (buffer_producer == MAP_FAILED || buffer_consumer == MAP_FAILED) fail("Error al crear el buffer compartido");

    /* Eliminar los semáforos que pudiese haber previamente con el nombre que se va a utilizar */
    sem_unlink("/mutex_producer");
    sem_unlink("/full_producer");
    sem_unlink("/empty_producer");

    sem_unlink("/mutex_consumer");
    sem_unlink("/full_consumer");
    sem_unlink("/empty_consumer");


    /* Abrir los semáforos */
    mutex_producer = sem_open("/mutex_producer", O_CREAT, 0700, 1);
    full_producer = sem_open("/full_producer", O_CREAT, 0700, 0);
    empty_producer = sem_open("/empty_producer", O_CREAT, 0700, N);

    mutex_consumer = sem_open("/mutex_consumer", O_CREAT, 0700, 1);
    full_consumer = sem_open("/full_consumer", O_CREAT, 0700, 0);
    empty_consumer = sem_open("/empty_consumer", O_CREAT, 0700, N);


	if (fork()) { /* Código del hijo */
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);
        while(iter < NUM_ITER){
            consumer();

            iter++;
        }
	} else { /* Código del padre */
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);
		producer();
	}

	if (munmap(buffer_producer, BUFFER_PRODUCER * sizeof(int))) fail("Fallo al liberar el buffer");
    if (munmap(buffer_consumer, BUFFER_CONSUMER * sizeof(int))) fail("Fallo al liberar el buffer");

    /* Cerrar y eliminar semáforos */
	sem_close(mutex_producer);
	sem_close(full_producer);
	sem_close(empty_producer);

    sem_close(mutex_consumer);
    sem_close(full_consumer);
    sem_close(empty_consumer);

    sem_unlink("/mutex_producer");
    sem_unlink("/full_producer");
    sem_unlink("/empty_producer");

    sem_unlink("/mutex_consumer");
    sem_unlink("/full_consumer");
    sem_unlink("/empty_consumer");

    exit(EXIT_SUCCESS);
}


void producer() {
	int item;

    sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
    item = produce_item();          /* Generar el siguiente elemento */
    sem_wait(empty_producer);                /* Disminuir el contador de posiciones vacías */
    sem_wait(mutex_producer);                /* Entra en la región crítica */
    insert_item(item);              /* Poner item en el buffer */
    sem_post(mutex_producer);                /* Sale de la región crítica */
    sem_post(full_producer);                 /* Incrementar el contador de posiciones ocupadas */

    producer_printf("Se ha añadido el item   "bold("%2d")" a la posición  "bold("%d")". Buffer: %s\n", item, count_producer + 1, representation_producer);
}

void consumer() {
	int item;
	int total;

    sleep((int) random() % 4);      /* Espera aleatoria de 0, 1, 2 ó 3 segundos */
    sem_wait(full_consumer);                 /* Disminuir el contador de posiciones ocupadas */
    sem_wait(mutex_consumer);                /* Entra en la región crítica */
    item = remove_item(&total);     /* Generar el siguiente elemento */
    sem_post(mutex_consumer);                /* Sale de la región crítica */
    sem_post(empty_consumer);                /* Incrementar el contador de posiciones vacías */
    consume_item(item, total);      /* Imprimir elemento */


}

int produce_item() {
    return ((int) random() % 11);
}

void insert_item(int item) {
    sem_getvalue(full_producer, &count);
    buffer_producer[count] = item;
    represent_buffer(count + 1);
}

int remove_item(int* total) {
    int item;
    int i;

    sem_getvalue(full_consumer, &count);
    item = buffer_consumer[count];
    buffer_consumer[count] = 0; /* Eliminar el entero del buffer */
    *total = item;
    for (i = count - 1; i >= 0; i--) {
        *total += buffer_consumer[i];
    }
    represent_buffer(count);
    return item;
}

void consume_item(int item, int total) {
    consumer_printf("Se ha eliminado el item "bold("%2d")" de la posición "bold("%d")". Buffer: %s. Suma total anterior: "bold("%2d")".\n", item , count + 1, representation, total);
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
