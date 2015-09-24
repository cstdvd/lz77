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

typedef enum
{
    ENCODE,
    DECODE
} MODES;

void encode(FILE *file, FILE *out);
void decode(FILE *file, FILE *out);

void writecode(struct token *t, FILE *out);
struct token *readcode(FILE *file);

struct token* match(int sb, int la);
