

CC     = gcc
CFLAGS = -Wall -Wextra -g
TARGET = mmu

SRCS = main.c \
       listas.c \
       procesos.c \
       planificador.c \
       visualizacion.c \
       emulacion.c
       # configSO.c  <- Integrante 3 (quitar # cuando este listo)

all: $(TARGET)

$(TARGET): $(SRCS)
       $(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
       @echo "Compilacion exitosa -> ./$(TARGET)"

run: all
       ./$(TARGET)

clean:
       rm -f $(TARGET)
