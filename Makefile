###############
#- VARIABLES -#
###############

# Compilador y opciones de compilación
CC = gcc
CFLAGS = -Wall -Wpedantic -pthread

# Carpeta con las cabeceras
HEADERS_DIR = .

# Opción de compilación que indica dónde están los archivos .h
INCLUDES = -I$(HEADERS_DIR)

# Archivos de cabecera para generar dependencias
HEADERS = $(HEADERS_DIR)/producer_consumer.h $(HEADERS_DIR)/shared_stack.h

# Fuentes con las funcionalidades básicas del stack y del productor/consumidor (implementaciones de los .h)
COMMON = $(HEADERS:.h=.c)

# Fuentes
SRC1 = ejercicio1.c
SRCS = ejercicio2.c ejercicio3.c ejercicio4.c ejercicio5.c

# Objetos
OBJ1 = $(SRC1:.c=.o)
OBJS = $(SRCS:.c=.o)
OBJS_COMMON = $(COMMON:.c=.o)

# Ejecutables o archivos de salida
EXE1 = $(SRC1:.c=)
EXES = $(SRCS:.c=)
OUT = $(EXE1) $(EXES)

############
#- REGLAS -#
############

# Regla por defecto: compila todos los ejecutables
all: $(OUT)

# Genera el ejecutable del ejercicio 1, dependencia de su objeto.
$(EXE1): $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $<

# Genera los ejecutables restantes, cada uno depende de sus objetos respectivos y los objetos comunes.
$(EXES): %: %.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ 

# Genera los ficheros objeto .o necesarios, dependencia de sus respectivos .c y todas las cabeceras.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $< $(INCLUDES)


# Borra todos los resultados de la compilación (prerrequisito: cleanobj)
clean: cleanobj
	rm -f $(OUT)

# Borra todos los ficheros objeto del directorio actual y todos sus subdirectorios
cleanobj:
	find . -name "*.o" -delete
