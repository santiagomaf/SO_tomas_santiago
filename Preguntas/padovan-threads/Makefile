# Makefile para compilar p1-202420.c con pthreads

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread -lm

TARGET = p1-202420
SRC = p1-202420.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) -s 15 -f 0 -e 10 -p 1

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
