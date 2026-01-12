CC     = gcc
CFLAGS = -O2 -lX11 -lXft -lfontconfig -I/usr/include/freetype2 
PREFIX = /usr

all: clock 

clock: clock.c 
	@$(CC) $< -o $@ $(CFLAGS) 

install: clock
	@mkdir -p $(PREFIX)/bin
	@cp clock $(PREFIX)/bin

clean:
	@rm -f clock
