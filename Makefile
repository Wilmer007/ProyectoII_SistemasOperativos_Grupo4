# ============================================================
#  Makefile  —  Proyecto II: MMU
#  Sistemas Operativos I | Seccion 852 | UNITEC-CEUTEC
# ============================================================

CC     = gcc
CFLAGS = -Wall -Wextra -g
TARGET = mmu

# Agregar el .c de cada modulo conforme se vaya integrando
SRCS = main.c \
       listas.c
       # procesos.c        <- Integrante 2 (quitar # cuando este listo)
       # planificador.c    <- Integrante 2
       # configSO.c        <- Integrante 3
       # emulacion.c       <- Integrante 4 (7 algoritmos)
       # visualizacion.c   <- Integrante 5

all: $(TARGET)

$(TARGET): $(SRCS)
       $(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
       @echo "Compilacion exitosa -> ./$(TARGET)"

run: all
       ./$(TARGET)

clean:
       rm -f $(TARGET)
