CC = gcc
CFLAGS = -Wall -Werror -O2

all: lz77

lz77: lz77.o tree.o
	$(CC) -o lz77 lz77.o tree.o

lz77.o: lz77.c tree.h
	$(CC) $(CFLAGS) -c lz77.c

tree.o: tree.c tree.h
	$(CC) $(CFLAGS) -c tree.c

.PHONY: clean

clean:
	-rm -f *.o lz77
