CC     = gcc
CFLAGS = -Wall -Wextra -g
TARGET = mmu.exe

SRCS = main.c \
       listas.c \
       modulo2.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
	@echo Compilacion exitosa

run: all
	$(TARGET)

clean:
	del /f $(TARGET)

