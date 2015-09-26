CC = gcc

all: main

main: lz77.c tree.c
	$(CC) -o lz77 lz77.c tree.c

.PHONY: clean
clean:
	-rm -f lz77
