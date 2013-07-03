CC=gcc
CCFLAGS=-c -std=c99 -lm
LDFLAGS=

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
TARGET=drawbotd

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f $(TARGET) *.o
