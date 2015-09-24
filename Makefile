CC = gcc

all: main

main: lz77.c
	$(CC) -o lz77 lz77.c 

.PHONY: clean
clean:
	-rm -f lz77
