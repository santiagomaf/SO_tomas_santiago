CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -pthread

TARGET = monitoreo
SRC = monitoreo.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
