#include <stdlib.h>
#include <stdio.h>
#include "getopt.h"

#define LA_SIZE 5
#define SB_SIZE 30

struct token{
	int off, len;
	char next;
};

void encode(FILE *file);
void decode(FILE *file);

struct token* match(int sb, int la);

