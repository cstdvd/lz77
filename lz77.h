#include <stdlib.h>
#include <stdio.h>

#define LA_SIZE 5
#define SB_SIZE 30

struct token{
	int off, n;
	char next;
};

struct token* match(void *sb, void *la);
