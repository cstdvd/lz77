/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : main.c
 *   Authors : David Costa and Pietro De Rosa
 *
 *                              DESCRIPTION
 *   LZ77 is a lossless data compression algorithms published by Abraham 
 *   Lempel and Jacob Ziv in 1977. It is a dictionary coder and maintains a
 *   sliding window during compression. The sliding window is divided in two
 *   parts: Search-Buffer (dictionary - encoded data) and Lookahead (uncomp-
 *   ressed data). LZ77 algorithms achieve compression by addressing byte 
 *   sequences from former contents instead of the original data. All data 
 *   will be coded in the same form (called token): -Address to already 
 *   coded contents; -Sequence length; -First deviating symbol.
 *   The window is contained in a fixed size buffer.
 *   The match between SB and LA is made by a binary tree, implemented in an
 *   array.
 ***************************************************************************/

/***************************************************************************
 *                             INCLUDED FILES
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "getopt.h"
#include "lz77.h"

/***************************************************************************
 *                            TYPE DEFINITIONS
 ***************************************************************************/
typedef enum{
    ENCODE,
    DECODE
} MODES;

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
                    goto error;
                }else if ((file = fopen(optarg, "rb")) == NULL){
                    perror("Opening input file");
                    goto error;
                }
                break;
                
            case 'o':       /* output file name */
                if (out != NULL){
                    fprintf(stderr, "Multiple output files not allowed.\n");
                    goto error;
                }else if ((out = fopen(optarg, "w")) == NULL){
                    perror("Opening output file");
                    goto error;
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
        goto error;
    }else if (out == NULL)
    {
        fprintf(stderr, "Output file must be provided\n");
        goto error;
    }
    
    if (mode == ENCODE){
        encode(file, out);
    }else{
        decode(file, out);
    }
    
    fclose(file);
    fclose(out);
    return 0;
    
    /* handle error */
error:
    if (file != NULL){
        fclose(file);
    }
    if (out != NULL){
        fclose(out);
    }
    exit (EXIT_FAILURE);
}