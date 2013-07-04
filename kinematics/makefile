CC=comp
CCFLAGS=--compile
LDFLAGS=

SOURCES=$(wildcard *.c)
COMPONENTS=$(SOURCES:.c=.ko)

all: $(COMPONENTS)

.PHONY: install

install: CCFLAGS = --install
install: $(COMPONENTS)

%.ko: %.c
	$(CC) $(CCFLAGS) $<

clean:
	rm -f *.ko
