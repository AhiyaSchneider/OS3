CC = gcc
CFLAGS = -g -pthread
TARGET = ex3.out
SOURCE = ex3.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f $(TARGET)
