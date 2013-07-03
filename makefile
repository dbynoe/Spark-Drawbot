all: drawbot

drawbot:
	$(MAKE) -C daemon


clean:
	$(MAKE) -C daemon clean
