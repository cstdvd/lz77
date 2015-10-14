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
#include "bitio.h"
#include "tree.h"

/***************************************************************************
 *                                CONSTANTS
 ***************************************************************************/
#define DEFAULT_LA_SIZE 15      /* lookahead size */
#define DEFAULT_SB_SIZE 4095    /* search buffer size */
#define N 3
#define WINDOW_SIZE ((DEFAULT_SB_SIZE * N) + DEFAULT_LA_SIZE)
#define MAX_BIT_BUFFER 16

/***************************************************************************
 *                            TYPE DEFINITIONS
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
void writecode(struct token t, struct bitFILE *out);
struct token readcode(struct bitFILE *file);

struct token match(struct node *tree, int root, unsigned char *window, int la, int la_size);

/***************************************************************************
 *                            ENCODE FUNCTION
 * Name         : encode - compress file
 * Parameters   : file - file to encode
 *                out - compressed file
 ***************************************************************************/
void encode(FILE *file, struct bitFILE *out)
{
    /* variables */
    int i, root = -1;
    int eof;
    struct node *tree;
    struct token t;
    unsigned char *window;
    int la_size, sb_size = 0;    /* actual lookahead and search buffer size */
    int buff_size;
    int sb_index = 0, la_index = 0;
    int LA_SIZE = DEFAULT_LA_SIZE, SB_SIZE = DEFAULT_SB_SIZE;
    
    window = calloc(WINDOW_SIZE, sizeof(unsigned char));
    
    tree = createTree(SB_SIZE);
    
    /* write header */
    /*bitIO_write(out, &SB_SIZE, MAX_BIT_BUFFER);
    bitIO_write(out, &LA_SIZE, MAX_BIT_BUFFER);*/
    
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
    
    destroyTree(tree);
    free(window);
}

/***************************************************************************
 *                            DECODE FUNCTION
 * Name         : decode - decompress file
 * Parameters   : file - compressed file
 *                out - output file
 ***************************************************************************/
void decode(struct bitFILE *file, FILE *out)
{
    /* variables */
    struct token t;
    int back = 0, off;
    unsigned char *buffer;
    int SB_SIZE = DEFAULT_SB_SIZE;
    
    buffer = (unsigned char*)calloc(WINDOW_SIZE, sizeof(unsigned char));
    
    /* read header */
    /*bitIO_read(file, &SB_SIZE, sizeof(SB_SIZE), MAX_BIT_BUFFER);
    bitIO_read(file, &LA_SIZE, sizeof(LA_SIZE), MAX_BIT_BUFFER);*/
    
    while(bitIO_feof(file) == 0)
    {
        /* read the code from the input file */
        t = readcode(file);
        
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
void writecode(struct token t, struct bitFILE *out)
{
    bitIO_write(out, &t.off, bitof(DEFAULT_SB_SIZE));
    bitIO_write(out, &t.len, bitof(DEFAULT_LA_SIZE));
    bitIO_write(out, &t.next, 8);
}

/***************************************************************************
 *                          READCODE FUNCTION
 * Name         : readcode - read the token from the compressed file
 * Parameters   : file - compressed file
 * Returned     : t - reconstructed token
 ***************************************************************************/
struct token readcode(struct bitFILE *file)
{
    /* variables */
    struct token t;
    
    bitIO_read(file, &t.off, sizeof(t.off), bitof(DEFAULT_SB_SIZE));
    bitIO_read(file, &t.len, sizeof(t.len), bitof(DEFAULT_LA_SIZE));
    bitIO_read(file, &t.next, sizeof(t.next), 8);
    
    return t;
}