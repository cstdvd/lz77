/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : lz77.c
 *   Authors : David Costa and Pietro De Rosa
 *
 ***************************************************************************/

/***************************************************************************
 *                             INCLUDED FILES
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"
#include "tree.h"

/***************************************************************************
 *                                CONSTANTS
 ***************************************************************************/
#define LA_SIZE 15      /* lookahead size */
#define SB_SIZE 4095    /* search buffer size */
#define N 2
#define WINDOW_SIZE ((SB_SIZE + LA_SIZE) * N)

/***************************************************************************
 *                            TYPE DEFINITIONS
 *
 * Each token is composed by a backward offset, the match's length and the
 * next character in the lookahead.
 * Offset : [0, SB_SIZE]            Length : [0, LA_SIZE]
 ***************************************************************************/
struct token{
    int off, len;
    char next;
};

typedef enum{
    ENCODE,
    DECODE
} MODES;

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
void encode(FILE *file, FILE *out);
void decode(FILE *file, FILE *out);

void writecode(struct token t, FILE *out);
struct token readcode(FILE *file);

struct token match(struct node *tree, unsigned char *window, int la, int la_size);

/***************************************************************************
 *                            USER INTERFACE
 * Syntax: ./lz77 <options>
 * Options: -c: compression mode
 *          -d: decompression mode
 *          -i <filename>: input file
 *          -o <filename>: output file
 *          -h: help
 ***************************************************************************/
int main(int argc, char *argv[])
{
	/* variables */
    int opt;
    FILE *file = NULL, *out = NULL;
    MODES mode;
	
    /* default mode */
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
                        fclose(file);
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
            
            case 'h':       /* help */
                printf("Usage: lz77 <options>\n");
                printf("  -c : Encode input file to output file.\n");
                printf("  -d : Decode input file to output file.\n");
                printf("  -i <filename> : Name of input file.\n");
                printf("  -o <filename> : Name of output file.\n");
                printf("  -h : Command line options.\n\n");
                break;
            
        }
    }
    
    /* validate command line */
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

/***************************************************************************
 *                            ENCODE FUNCTION
 * Name         : encode - compress file
 * Parameters   : file - file to encode
 *                out - compressed file
 ***************************************************************************/
void encode(FILE *file, FILE *out)
{
    /* variables */
    int i, count = 0, k;
    int c;
    struct node *tree = NULL;
    struct token t;
    unsigned char *window, *seq;
    int la_size, sb_size = 0;    /* actual lookahead and search buffer size */
    
    int sb_index = 0, la_index = 0;
    window = (unsigned char*)malloc(WINDOW_SIZE);
    
    /* fill the lookahead with the first LA_SIZE bytes or until EOF is reached */
    for(la_size = 0; la_size < LA_SIZE; la_size++){
        if((c = getc(file)) != EOF){
            window[la_size] = c;
        }else
            break;
    }
    
    /* find the longest match of the lookahead in the tree*/
    t = match(tree, window, la_index, la_size);

    /* write the token in the output file */
    writecode(t, out);
    count++;
    
    
    /* cycle from the 2nd iteration until the end */
	while(la_size > 0){
		
        /* read as many bytes as matched in the previuos itaration */
        for(i = 0; i < t.len + 1; i++){
            c = getc(file);
            if(c != EOF){
                window[(la_index+la_size)%(WINDOW_SIZE)] = c;
                
                seq = (unsigned char*)malloc(la_size);
                for(k = 0; k < la_size; k++)
                    memcpy(seq+k, &(window[(la_index+k)%WINDOW_SIZE]), 1);
                
                /* insert a new node in the tree */
                insert(&tree, seq, la_index, la_size);
                free(seq);
                la_index = (la_index + 1) % (WINDOW_SIZE);
                
                /* if search buffer's length is max, the oldest node is removed from the tree */
                if(sb_size == SB_SIZE){
                    delete(&tree, window, la_size, sb_index, WINDOW_SIZE);
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                }else
                    sb_size++;
            }
            else{
                /* case where we hit EOF before filling lookahead */
                seq = (unsigned char*)malloc(la_size);
                for(k = 0; k < la_size; k++)
                    memcpy(seq+k, &(window[(la_index+k)%WINDOW_SIZE]), 1);
                
                /* insert a new node in the tree */
                insert(&tree, seq, la_index, la_size);
                free(seq);
                
                la_index = (la_index + 1) % (WINDOW_SIZE);
                
                /* if search buffer's length is max, the oldest node is removed from the tree */
                if(sb_size == SB_SIZE){
                    delete(&tree, window, la_size, sb_index, WINDOW_SIZE);
                    sb_index = (sb_index + 1) % (WINDOW_SIZE);
                }else
                    sb_size++;
                
                la_size--;
            }
        }
        
        if(la_size > 0){
            //free(t);
            /* find the longest match of the lookahead in the tree*/
            t = match(tree, window, la_index, la_size);

            /* write the token in the output file */
            writecode(t, out);
            count++;
        }
	}
    
    freetree(&tree);
    free(window);
}

/***************************************************************************
 *                            DECODE FUNCTION
 * Name         : decode - decompress file
 * Parameters   : file - compressed file
 *                out - output file
 ***************************************************************************/
void decode(FILE *file, FILE *out)
{
    /* variables */
    struct token t;
    int front = 0, back = 0, off;
    unsigned char *buffer;
    
    buffer = (unsigned char*)malloc(WINDOW_SIZE);
    
    while(feof(file) == 0)
    {
        /* read the code from the input file */
        t = readcode(file);
        if(t.off == -1)
            break;
        
        /* reconstruct the original byte*/
        while(t.len > 0)
        {
            off = (back - t.off >= 0) ? (back - t.off) : (back + (WINDOW_SIZE) - t.off);
            buffer[back] = buffer[off];
            
            /* write the byte in the output file*/
            putc(buffer[back], out);
            
            /* slide the circular array*/
            if(back == front)
                front = (front + 1) % (WINDOW_SIZE);
            back = (back + 1) % (WINDOW_SIZE);
            
            t.len--;
        }
        buffer[back] = t.next;
        
        /* write the byte in the output file*/
        putc(buffer[back], out);
        
        /* slide the circular array*/
        if(back == front)
            front = (front + 1) % (WINDOW_SIZE);
        back = (back + 1) % (WINDOW_SIZE);
    }
    
    free(buffer);
}

/***************************************************************************
 *                            MATCH FUNCTION
 * Name         : match - find the longest match and create the token
 * Parameters   : tree - binary search tree
 *                window - whole buffer
 *                la - starting index of the lookahead
 *                la_size - actual lookahead size
 * Returned     : token of the best match
 ***************************************************************************/
struct token match(struct node *tree, unsigned char *window, int la, int la_size)
{
    /* variables */
    struct token t;
    int *ret;
    
    /* find the longest match */
    ret = find(tree, window, la, la_size, WINDOW_SIZE);
    
    /* create the token */
    //t = (struct token*)malloc(sizeof(struct token));
    t.off = ret[0];
    t.len = ret[1];
    t.next = window[(la+ret[1])%WINDOW_SIZE];
    
    return t;
}

/***************************************************************************
 *                           WRITECODE FUNCTION
 * Name         : writecode - write the token in the output file
 * Parameters   : t - token to be written
 *                out - output file
 * Offset : 12 bits representation => [0, 4095]
 * Length : 4 bits representation => [0, 15]
 * Next char requires 8 bits
 * Total token's size: 12 + 4 + 8 = 24 bits = 3 bytes
 * 
 *     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |         offset        |lenght |   next char   |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***************************************************************************/
void writecode(struct token t, FILE *out)
{
    /* variables */
    unsigned char code[3];
    int i;
    
    /* use masks to selectivly set the bits */
    code[0] = (unsigned char)t.off;
    code[1] = (unsigned char)((t.off >> 8) & 0x0f);
    code[1] = code[1] | (unsigned char)((t.len << 4) & 0xf0);
    code[2] = t.next;
    
    /* write the code in the output file */
    for(i = 0; i < 3; i++)
        putc(code[i], out);
}

/***************************************************************************
 *                          READCODE FUNCTION
 * Name         : readcode - read the token from the compressed file
 * Parameters   : file - compressed file
 * Returned     : t - reconstructed token
 ***************************************************************************/
struct token readcode(FILE *file)
{
    /* variables */
    struct token t;
    unsigned char code[3];
    int ret;
    
    /* read code from file */
    ret = fread(code, 1, 3, file);
    if(feof(file) == 1){
        t.off = -1;
        return t;
    }
    if(ret < 3){
        perror("Error reading file.");
        exit(1);
    }
    
    /* use masks to selectivly set the bits */
    t.off = (((int)code[1] & 0x0000000f) << 8) | (int)code[0];
    t.len = ((int)code[1] & 0x000000f0) >> 4;
    t.next = code[2];
    
    return t;
}
