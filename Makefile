CC = gcc
CFLAGS = -Wall -Werror -O2

all: lz77

lz77: main.o lz77.o tree.o
	$(CC) -o lz77 main.o lz77.o tree.o

main.o: main.c lz77.h
	$(CC) $(CFLAGS) -c main.c

lz77.o: lz77.c tree.h
	$(CC) $(CFLAGS) -c lz77.c

tree.o: tree.c tree.h
	$(CC) $(CFLAGS) -c tree.c

.PHONY: clean

clean:
	-rm -f *.o lz77
