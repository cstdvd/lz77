#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"

#define LA_SIZE 16
#define SB_SIZE 4095
#define N 4
#define WINDOW_SIZE (SB_SIZE + LA_SIZE) * N

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

struct token* match(unsigned char *window, int sb, int sb_size, int la, int la_size);



int main(int argc, char *argv[])
{
	// vars
    int opt;
    FILE *file, *out;
    MODES mode;
	
    // default mode
    mode = ENCODE;
    
    while ((opt = getopt(argc, argv, "cdi:o:h")) != -1)
    {
        switch(opt)
        {
            case 'c':       /* compression mode */
                mode = ENCODE;
                break;
            
            case 'd':       /* decompression mode */
                mode = DECODE;
                break;
            
            case 'i':       /* input file name */
                if (file != NULL){
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(file);
                
                    if (out != NULL){
                        fclose(out);
                    }
                
                    exit(EXIT_FAILURE);
                }else if ((file = fopen(optarg, "rb")) == NULL){
                    perror("Opening inFile");
                
                    if (out != NULL){
                        fclose(out);
                    }
                
                    exit(EXIT_FAILURE);
                }
                break;
            
            case 'o':       /* output file name */
                if (out != NULL){
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    fclose(out);
                
                    if (file != NULL){
                        fclose(out);
                    }
                
                    exit(EXIT_FAILURE);
                }else if ((out = fopen(optarg, "w")) == NULL){
                    perror("Opening outFile");
                
                    if (file != NULL){
                        fclose(file);
                    }
                
                    exit(EXIT_FAILURE);
                }
                break;
            
            case 'h':
                printf("Usage: lz77 <options>\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -i <filename> : Name of input file.\n");
                printf("  -o <filename> : Name of output file.\n");
                printf("  -h : Command line options.\n\n");
                break;
            
        }
    }
    
    if (file == NULL){
        fprintf(stderr, "Input file must be provided\n");
        
        if (out != NULL){
            fclose(out);
        }
        
        exit (EXIT_FAILURE);
    }else if (out == NULL)
    {
        fprintf(stderr, "Output file must be provided\n");
        
        if (file != NULL){
            fclose(file);
        }
        
        exit (EXIT_FAILURE);
    }
    
    if (mode == ENCODE){
        encode(file, out);
    }else{
        decode(file, out);
    }
    
    fclose(file);
    fclose(out);
    return 0;
}


void encode(FILE *file, FILE *out)
{
    int i, ret;
    int c;
    struct token *t = NULL;
    unsigned char *window;
    int la_size, sb_size = 0;    // actual lookahead size
    
    int sb_index = 0, la_index = 0;
    window = (unsigned char*)malloc(WINDOW_SIZE);
    
    for(la_size = 0; la_size < LA_SIZE; la_size++){
        if((c = getc(file)) != EOF){
            window[la_size] = c;
        }else
            break;
    }
    t = match(window, sb_index, sb_size, la_index, la_size);

    writecode(t, out);
    
	while(la_size > 0){
		
        for(i = 0; i < t->len + 1; i++){
            c = getc(file);
            if(c != EOF){
                window[(la_index+la_size)%(WINDOW_SIZE)] = c;
                la_index = (la_index + 1) % (WINDOW_SIZE);
                if(sb_size == SB_SIZE)
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                else
                    sb_size++;
            }
            else{ // caso in cui EOF compaia prima del riempimento del lookahead
                la_index = (la_index + 1) % (WINDOW_SIZE);
                if(sb_size == SB_SIZE)
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                else
                    sb_size++;
                
                la_size--;
            }
        }
        
        if(la_size > 0){
            t = match(window, sb_index, sb_size, la_index, la_size);

            writecode(t, out);
            free(t);
        }
	}
    
    free(window);
}

void decode(FILE *file, FILE *out)
{
    struct token *t;
    int front = 0, back = 0, off;
    unsigned char *buffer;
    
    buffer = (unsigned char*)malloc(WINDOW_SIZE);
    
    while(feof(file) == 0)
    {
        t = readcode(file);
        if(t == NULL)
            break;
        
        while(t->len > 0)
        {
            off = (back - t->off >= 0) ? (back - t->off) : (back + (WINDOW_SIZE) - t->off);
            buffer[back] = buffer[off];
            putc(buffer[back], out);
            if(back == front)
                front = (front + 1) % (WINDOW_SIZE);
            back = (back + 1) % (WINDOW_SIZE);
            
            t->len--;
        }
        buffer[back] = t->next;
        putc(buffer[back], out);
        if(back == front)
            front = (front + 1) % (WINDOW_SIZE);
        back = (back + 1) % (WINDOW_SIZE);
    }
    
    free(buffer);
}

struct token* match(unsigned char *window, int sb, int sb_size, int la, int la_size)
{	
	struct token *t;
    int i, j, c = 0;
	
	t = (struct token*)malloc(sizeof(struct token));
	t->off = 0;
	t->len = 0;
	t->next = window[la];

	
	while(c < sb_size){
	   
		for(i = 0; window[(sb+i)%(WINDOW_SIZE)] == window[(la+i)%(WINDOW_SIZE)]; i++){
            if((i >= la_size-1) || (c+i >= SB_SIZE))
                break;
		}
        
        // nel caso in cui il match continui nel lookahead buffer
        if((c+i >= sb_size) && (i < la_size-1)){
            for(j = la; window[j%(WINDOW_SIZE)] == window[(la+i)%(WINDOW_SIZE)]; i++){
                j++;
                if(i >= la_size-1)
                    break;
            }
        }
		if(i > t->len){
			t->off = sb_size - c;
			t->len = i;
			t->next = window[(la+i)%(WINDOW_SIZE)];
		}
		sb = (sb + 1) % (WINDOW_SIZE);
        c++;
	}
	return t;
}

void writecode(struct token *t, FILE *out) //  off = 12 bits, len = 4 bits, next = 8 bits
{
    unsigned char code[3];
    int i;
    
    code[0] = (unsigned char)t->off;
    code[1] = (unsigned char)((t->off >> 8) & 0x0f);
    code[1] = code[1] | (unsigned char)((t->len << 4) & 0xf0);
    code[2] = t->next;
    
    for(i = 0; i < 3; i++)
        putc(code[i], out);
}

struct token *readcode(FILE *file)
{
    struct token *t;
    unsigned char code[3];
    int ret;
    
    t = (struct token*)malloc(sizeof(struct token));
    
    ret = fread(code, 1, 3, file);
    if(feof(file) == 1)
        return NULL;
    if(ret < 3){
        perror("Error reading file.");
        exit(1);
    }
    
    t->off = (((int)code[1] & 0x0000000f) << 8) | (int)code[0];
    t->len = ((int)code[1] & 0x000000f0) >> 4;
    t->next = code[2];
    
    //printf("\n<%d, %d, %c>\n", t->off, t->len, t->next);

    return t;
}
