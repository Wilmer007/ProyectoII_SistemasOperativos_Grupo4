CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src
SRC = src/main.c src/listas.c src/procesos.c src/planificador.c src/configSO.c src/emulacion.c src/visualizacion.c src/algoritmos.c
OBJ = $(SRC:.c=.o)
TARGET = SistemMMU

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
