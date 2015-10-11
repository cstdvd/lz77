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
#include "tree.h"

/***************************************************************************
 *                                CONSTANTS
 ***************************************************************************/
#define LA_SIZE 15      /* lookahead size */
#define SB_SIZE 4095    /* search buffer size */
#define N 3
#define WINDOW_SIZE ((SB_SIZE * N) + LA_SIZE)

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

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
void writecode(struct token t, FILE *out);
struct token readcode(FILE *file);

struct token match(struct node *tree, int root, unsigned char *window, int la, int la_size);

/***************************************************************************
 *                            ENCODE FUNCTION
 * Name         : encode - compress file
 * Parameters   : file - file to encode
 *                out - compressed file
 ***************************************************************************/
void encode(FILE *file, FILE *out)
{
    /* variables */
    int i, root = -1;
    int eof;
    struct node tree[SB_SIZE];
    struct token t;
    unsigned char *window;
    int la_size, sb_size = 0, buff_size;    /* actual lookahead and search buffer size */
    
    int sb_index = 0, la_index = 0;
    
    window = calloc(WINDOW_SIZE, sizeof(unsigned char));
    
    /* fill the lookahead with the first LA_SIZE bytes or until EOF is reached */
    buff_size = fread(window, 1, WINDOW_SIZE, file);
    if(ferror(file)) {
        printf("Error loading the data in the window.\n");
        return;
   	}
    
    eof = feof(file);
    
    /* set lookahead's size */
    la_size = (buff_size > LA_SIZE) ? LA_SIZE : buff_size;
    
	while(buff_size > 0){
		
        /* find the longest match of the lookahead in the tree*/
        t = match(tree, root, window, la_index, la_size);
        
        /* write the token in the output file */
        writecode(t, out);
        
        /* read as many bytes as matched in the previuos iteration */
        for(i = 0; i < t.len + 1; i++){
            
            /* if search buffer's length is max, the oldest node is removed from the tree */
            if(sb_size == SB_SIZE){
                delete(tree, &root, window, sb_index, SB_SIZE);
                sb_index++;
            }else
                sb_size++;
            
            /* insert a new node in the tree */
            insert(tree, &root, window, la_index, la_size, SB_SIZE);
            la_index++;
            
            if (eof == 0){
                /* scroll backward the buffer when it is almost full */
                if (sb_index == SB_SIZE * (N - 1)){
                    memmove(window, &(window[sb_index]), sb_size+la_size);
                    
                    /* update the node's offset when the buffer is scrolled */
                    updateOffset(tree, sb_index, SB_SIZE);
                    
                    sb_index = 0;
                    la_index = sb_size;
                    
                    /* read from file */
                    buff_size += fread(&(window[sb_size+la_size]), 1, WINDOW_SIZE-(sb_size+la_size), file);
                    if(ferror(file)) {
                        printf("Error loading the data in the window.\n");
                        return;
                    }
                    eof = feof(file);
                }
            }
            
            buff_size--;
            /* case where we hit EOF before filling lookahead */
            la_size = (buff_size > LA_SIZE) ? LA_SIZE : buff_size;
        }
	}
    
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
    int back = 0, off;
    unsigned char buffer[WINDOW_SIZE];
    
    while(feof(file) == 0)
    {
        /* read the code from the input file */
        t = readcode(file);
        if(t.off == -1)
            break;
        
        if(back + t.len > WINDOW_SIZE - 1){
            memcpy(buffer, &(buffer[back - SB_SIZE]), SB_SIZE);
            back = SB_SIZE;
        }
        
        /* reconstruct the original byte*/
        while(t.len > 0)
        {
            off = back - t.off;
            buffer[back] = buffer[off];
            
            /* write the byte in the output file*/
            putc(buffer[back], out);
            
            back++;
            t.len--;
        }
        buffer[back] = t.next;
        
        /* write the byte in the output file*/
        putc(buffer[back], out);
        
        back++;
    }
    
}

/***************************************************************************
 *                            MATCH FUNCTION
 * Name         : match - find the longest match and create the token
 * Parameters   : tree - binary search tree
 *                root - index of the root
 *                window - pointer to the buffer
 *                la - starting index of the lookahead
 *                la_size - actual lookahead size
 * Returned     : token of the best match
 ***************************************************************************/
struct token match(struct node *tree, int root, unsigned char *window, int la, int la_size)
{
    /* variables */
    struct token t;
    struct ret r;
    
    /* find the longest match */
    r = find(tree, root, window, la, la_size);
    
    /* create the token */
    t.off = r.off;
    t.len = r.len;
    t.next = window[la+r.len];
    
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
    ret = (int)fread(code, 1, 3, file);
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