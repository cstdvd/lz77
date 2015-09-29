#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"
#include "tree.h"

#define LA_SIZE 15
#define SB_SIZE 4095
#define N 2
#define WINDOW_SIZE ((SB_SIZE + LA_SIZE) * N)

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

struct token* match(struct node *tree, unsigned char *window, int la, int la_size);


int main(int argc, char *argv[])
{
	// vars
    int opt;
    FILE *file = NULL, *out = NULL;
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
    int i, ret, h, count = 0, k;
    int c;
    struct node *tree = NULL;
    struct token *t = NULL;
    unsigned char *window, *seq;
    int la_size, sb_size = 0;    // actual lookahead size
    
    int sb_index = 0, la_index = 0;
    window = (unsigned char*)malloc(WINDOW_SIZE);
    
    for(la_size = 0; la_size < LA_SIZE; la_size++){
        if((c = getc(file)) != EOF){
            window[la_size] = c;
        }else
            break;
    }
    t = match(tree, window, la_index, la_size);
    printf("<%d, %d, %c>\n\n", t->off, t->len, t->next);

    writecode(t, out);
    count++;
    
	while(la_size > 0){
		
        for(i = 0; i < t->len + 1; i++){
            c = getc(file);
            if(c != EOF){
                window[(la_index+la_size)%(WINDOW_SIZE)] = c;
                
                seq = (unsigned char*)malloc(la_size);
                for(k = 0; k < la_size; k++)
                    memcpy(seq+k, &(window[(la_index+k)%WINDOW_SIZE]), 1);
                insert(&tree, seq, la_index, la_size);
                free(seq);
                la_index = (la_index + 1) % (WINDOW_SIZE);
                
                if(sb_size == SB_SIZE){
                    delete(&tree, window, la_size, sb_index, WINDOW_SIZE);
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                }else
                    sb_size++;
            }
            else{ // caso in cui EOF compaia prima del riempimento del lookahead
                seq = (unsigned char*)malloc(la_size);
                for(k = 0; k < la_size; k++)
                    memcpy(seq+k, &(window[(la_index+k)%WINDOW_SIZE]), 1);
                insert(&tree, seq, la_index, la_size);
                free(seq);
                
                la_index = (la_index + 1) % (WINDOW_SIZE);
                
                if(sb_size == SB_SIZE){
                    delete(&tree, window, la_size, sb_index, WINDOW_SIZE);
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                }else
                    sb_size++;
                
                la_size--;
            }
        }
        
        if(la_size > 0){
            free(t);
            t = match(tree, window, la_index, la_size);
            printf("<%d, %d, %c>\n\n", t->off, t->len, t->next);

            writecode(t, out);
            count++;
        }
	}
    
    printf("numero token: %d\n", count);
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

struct token* match(struct node *tree, unsigned char *window, int la, int la_size)
{
    struct token *t;
    struct node *elem;
    int i = 0, j, ret;
    
    t = (struct token*)malloc(sizeof(struct token));
    t->off = 0;
    t->len = 0;
    t->next = window[la];
    
    //elem = find(tree, &(window[la]), la_size);
    elem = tree;
    while (elem != NULL){
        /*printf("Trovato: ");
        for(j = 0; j < la_size; j++)
            printf("%x ", elem->seq[j]);
        printf(" per ");
        for(j = 0; j < la_size; j++)
            printf("%x ", window[(la+j)%WINDOW_SIZE]);
        printf("\n");*/
        for (i = 0; (ret = memcmp(&(window[(la+i)%WINDOW_SIZE]), &(elem->seq[i]), 1)) == 0 && i < la_size-1; i++){}
        
        if (i > t->len){
            t->off = (la > elem->off) ? (la - elem->off) : (la + WINDOW_SIZE - elem->off);
            t->len = i;
            t->next = window[(la+i)%(WINDOW_SIZE)];
        }
        
        if (ret < 0)
            elem = elem->left;
        else if (ret > 0)
            elem = elem->right;
        else break;
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
