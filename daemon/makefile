CC=gcc
CCFLAGS=-c -std=c99 -lm
LDFLAGS=

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
TARGET=drawbotd

.PHONY: install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

install:

clean:
	rm -f $(TARGET) *.o
