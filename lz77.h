#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"

#define LA_SIZE 16
#define SB_SIZE 4096

struct token{
	int off, len;
	char next;
};

void encode(FILE *file, FILE *out);
void decode(FILE *file);

void writecode(struct token *t, FILE *out);
struct token* match(int sb, int la);
