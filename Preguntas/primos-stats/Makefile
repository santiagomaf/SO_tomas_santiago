CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread -lm

TARGET = p1-202520
SRC = p1-202520.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
