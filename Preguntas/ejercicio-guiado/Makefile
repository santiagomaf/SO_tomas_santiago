CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -pthread

all: ejercicios test_exec

ejercicios: ejercicios.c
	$(CC) $(CFLAGS) -o ejercicios ejercicios.c

test_exec: test_exec.c
	$(CC) $(CFLAGS) -o test_exec test_exec.c $(LDFLAGS)

run: ejercicios
	./ejercicios

clean:
	rm -f ejercicios test_exec

.PHONY: all run clean
