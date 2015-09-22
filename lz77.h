#include <stdlib.h>
#include <stdio.h>

#define LA_SIZE 5
#define SB_SIZE 30

struct token{
	int off, len;
	char next;
};

struct token* match(int sb, int la);
