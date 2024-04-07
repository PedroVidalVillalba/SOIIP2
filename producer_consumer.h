/**
 * Sistemas Operativos 2. Práctica 2.
 * Librería con las cabeceras de las funciones genéricas de
 * productor y consumidor usando un stack compartido controlado
 * por semáforos.
 *
 * @date 07/04/2024
 * @authors Falgueras Casarejos, Yago
 * @authors Vidal Villalba, Pedro
 */

#ifndef PRODUCER_CONSUMER
#define PRODUCER_CONSUMER

#include "shared_stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/* Definir constantes y macros para la impresión */
#define PRODUCER_COLOR_1  "\x1b[36m"
#define CONSUMER_COLOR_1  "\x1b[31m"
#define PRODUCER_COLOR_2  "\x1b[32m"
#define CONSUMER_COLOR_2  "\x1b[33m"
#define NO_COLOR        "\x1b[0m"
#define bold(text) "\x1b[1m" text "\x1b[22m"

/* Variables globales con los formatos de color para productor y consumidor */
extern char* producer_color;
extern char* consumer_color;

#define producer_printf(format, ...) ( printf("%s [%d] " format NO_COLOR, producer_color, getpid(), ##__VA_ARGS__) )
#define consumer_printf(format, ...) ( printf("%s [%d] " format NO_COLOR, consumer_color, getpid(), ##__VA_ARGS__) )

/*** 
 * No se incluyen funciones de producer y consumer como tal, ya que estas podrían
 * cambiar entre ejercicios. Sí se incluyen las cabeceras de las funciones genéricas,
 * que se mantienen iguales entre los diferentes apartados.
 ***/

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

#endif //PRODUCER_CONSUMER
