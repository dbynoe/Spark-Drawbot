all: drawbot

drawbot: drawbot.c
	gcc -std=c99 -lm -o drawbot drawbot.c

clean:
	rm -f drawbot
