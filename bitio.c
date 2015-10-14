/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : bitio.c
 *   Authors : David Costa and Pietro De Rosa
 *
 ***************************************************************************/
/***************************************************************************
 *                             INCLUDED FILES
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bitio.h"

/***************************************************************************
 *                                CONSTANTS
 ***************************************************************************/
#define BIT_IO_BUFFER 4096

/***************************************************************************
 *                            TYPE DEFINITIONS
 ***************************************************************************/
struct bitFILE{
	FILE *file;     /* file to (from) write (read) */
	int mode;       /* the mode (READ or WRITE) */
	int bytepos;    /* actual byte's position in the buffer */
	int bitpos;     /* last bit's position in the byte */
    int read;       /* # of bytes read from the file and stored in buffer */
	unsigned char *buffer; /* bits buffer */
};

/***************************************************************************
 *					          BIT OF FUNCTION
 * 	Name        : bitof - computes the minimum # of bits needed to represent
 *                an integer
 * 	Parameters  : n	- integer number to represent
 * 	Returned    : min # of bits needed to represent the input
 ***************************************************************************/
int bitof(int n){
	return (int)(ceil(log(n)/log(2)));
}

/***************************************************************************
 *						  BIT I/O FEOF FUNCTION
 * 	Name        : bitIO_feof - allows to check if a read mode opened bitFILE
 *                has the end-of-file indicator set.
 * 	Parameters  : bitF - bitFILE opened in read mode
 * 	Returned    : 0 if EOF is not set, 1 if EOF is set
 ***************************************************************************/
int bitIO_feof(struct bitFILE *bitF)
{
    if (feof(bitF->file) && bitF->bytepos == bitF->read)
        return 1;
    return 0;
}

/***************************************************************************
 *						  BIT I/O FERR FUNCTION
 * 	Name        : bitIO_ferror - allows to check if a read mode opened bitFILE
 *                has the error indicator set.
 * 	Parameters  : bitF - bitFILE opened in read mode
 * 	Returned    : 0 if the flag is not set, non-zero value if it is set
 ***************************************************************************/
int bitIO_ferror(struct bitFILE *bitF)
{
    return (ferror(bitF->file));
}

/***************************************************************************
 *						 WRITE BUFFER FUNCTION
 * 	Name        : write_buffer - writes the used bytes of the buffer in the
 *                file opened in write mode. It is used either by the 
 *                'bitIO_write' function to flush the whole buffer, either 
 *                by the 'bitIO_close' one to write the remained bytes in 
 *                the file.
 * 	Parameters  : bitF - bitFILE opened in write mode
 ***************************************************************************/
void write_buffer(struct bitFILE *bitF){

	int ret;

	/* write data */
	ret = fwrite(bitF->buffer, 1, bitF->bytepos, bitF->file);
	/* check for errors on writing */
	if(ret != bitF->bytepos)
		return;
	/* clear the buffer */
	bitF->bytepos = 0;
	bitF->bitpos = 0;
	memset(bitF->buffer, 0, BIT_IO_BUFFER);
}

/***************************************************************************
 *                      READ BUFFER FUNCTION
 * 	Name        : read_buffer - reads at most BIT_IO_BUFFER bytes from the
 *                bitFILE and copy them into the buffer. It is used only by
 *                the 'bitIO_read' function, once the last bit in the buffer
 *                has been read.
 * 	Parameters  : bitF - bitFILE opened in write mode
 ***************************************************************************/
void read_buffer(struct bitFILE *bitF){

	/* read data */
	bitF->read = fread(bitF->buffer, 1, BIT_IO_BUFFER, bitF->file);
	/* check for errors */
	if(bitF->read < BIT_IO_BUFFER && ferror(bitF->file))
		return;
	/* clear variables */
	bitF->bytepos = 0;
	bitF->bitpos = 0;
}


/***************************************************************************
 *						BIT I/O OPEN FUNCTION
 * 	Name        : bitIO_open - open the file specified by 'path' in write or
 *                read mode, depending on the value of 'mode' parameter.
 * 	Parameters  : path - path to the file to open
 * 				  mode - specify the mode: read(BIT_IO_R) or write(BIT_IO_W)
 * 	Returned    : bitFILE just opened in the specified mode
 ***************************************************************************/
struct bitFILE* bitIO_open(const char *path, int mode){

	struct bitFILE *bitF;

	/* errors handler */
	if(mode!=BIT_IO_W && mode!=BIT_IO_R)
	{
		printf("Invalid mode detected.\n");
		return NULL;
	}
	if(path == NULL)
		return NULL;

	/* initialize structure */
	bitF = (struct bitFILE*)calloc(1, sizeof(struct bitFILE));
	bitF->mode = mode;
	bitF->bytepos = 0;
	bitF->bitpos = 0;
	bitF->buffer = (unsigned char*)calloc(BIT_IO_BUFFER, sizeof(unsigned char));
	memset(bitF->buffer, 0, BIT_IO_BUFFER);

	/* open file */
    /*read binary mode */
	if(bitF->mode == BIT_IO_R)
	{
		if((bitF->file = fopen(path, "rb")) == NULL)
			return NULL;
            
		/* fill the buffer for the 1st time */
		read_buffer(bitF);
	}
    /* write mode */
	else if((bitF->file = fopen(path, "w")) == NULL)
		return NULL;

	return bitF;
}

/***************************************************************************
 *						  BIT I/O CLOSE FUNCTION
 * 	Name        : bitIO_close - closes the bitFILE given as input and frees
 *                memory portions occupied by the structures. If the file is
 *                opened in write modecand there are pending bits, these are
 *                written in the buffer first.
 * 	Parameters  : bitF - bitFILE to close
 * 	Returned    : 0 if file closed successfully, -1 if error on inputs
 ***************************************************************************/
int bitIO_close(struct bitFILE *bitF){

	/* errors handler */
	if(bitF == NULL || bitF->file == NULL)
		return -1;

	/* write the unwritten bytes into the file */
	if(bitF->mode == BIT_IO_W)
	{
		if(bitF->bitpos > 0)
			(bitF->bytepos)++;
		write_buffer(bitF);
	}
	/* close the file */
	fclose(bitF->file);
	/* free memory */
	free(bitF->buffer);
	free(bitF);

	return 0;
}

/***************************************************************************
 *							BIT I/O WRITE FUNCTION
 * 	Name        : bitIO_write - writes the first 'nbit' pointed by 'info' in
 *                the bitFILE pointed by 'bitF'.
 * 	Parameters  : bitF - bitFILE opened in write mode
 * 				  info - buffer containing information to be written
 * 				  nbytes - number of bytes to be written from the buffer
 * 				  nbit - number of bits required to represent the information
 * 	Returned    : # bits written successfully, -1 if error on inputs
 ***************************************************************************/
int bitIO_write(struct bitFILE *bitF, void *info, int nbit){

	int i;
	int byte_pos = 0, bit_pos = 0; /* byte's position and bit's position */
	unsigned char mask; /* mask of bits */

	/* errors handler */
	if(bitF == NULL || bitF->file == NULL || bitF->mode != BIT_IO_W|| info == NULL || nbit < 0)
		return -1;

	for(i=0; i<nbit; i++)
	{
		/* get bit to write */
		mask = 1 << bit_pos;
        
        /* if it is a 1 set it, otherwise do nothing */
		if((*(unsigned char *)(info + byte_pos) & mask) != 0)
			bitF->buffer[bitF->bytepos] |= (1 << bitF->bitpos);
        
        /* update info to write variables */
		byte_pos = (bit_pos < 7)? byte_pos : (byte_pos + 1);
		bit_pos = (bit_pos < 7)? (bit_pos + 1) : 0;
        
        /* update bitF structure */
		bitF->bytepos = (bitF->bitpos < 7)? bitF->bytepos : (bitF->bytepos + 1);
		bitF->bitpos = (bitF->bitpos <7)? (bitF->bitpos + 1) : 0;
        
		/* check if a write_buffer must be done */
		if(bitF->bytepos == BIT_IO_BUFFER)
			write_buffer(bitF);
		/* check for writing errors */		
		if(bitIO_ferror(bitF) != 0)
			break;
	}

	return i;
}

/***************************************************************************
 *								BIT I/O READ FUNCTION
 * 	Name        : bitIO_read - reads the first 'nbit' from the bitFILE and
 *                puts them in the buffer.
 *				  If an error occurs, or the end of the file is
 *      		  reached, the return value is a short item count (or zero).
 *			  	  bitIO_read() does not distinguish between end-of-file and
 *				  error, and callers must use 'bitIO_feof' and 'bitIO_ferror'
 *				  to determine which occurred.
 * 	Parameters  : bitF - bitFILE opened in read mode
 * 				  info - buffer where read bits are put
 * 				  info_s - size of the 'info' buffer in bytes
 * 				  nbit - number of bits to read
 * 	Returned    :	# bits read successfully, -1 if error on inputs
 ***************************************************************************/
int bitIO_read(struct bitFILE *bitF, void *info, int info_s, int nbit){

	int i;
	int byte_pos = 0, bit_pos = 0; /* byte's position and bit's position */
	unsigned char mask; /* mask of bits */

	/* errors handler */
	if(bitF == NULL || bitF->file == NULL || bitF->mode != BIT_IO_R || info == NULL || info_s <= 0 || nbit < 0)
		return -1;

	/* clear the 'info' buffer */
	memset(info, 0, info_s);
    
	/* begin the reading */
	for(i=0; i<nbit && (bitIO_feof(bitF) != 1); i++)
	{
		/* get bit to read */
		mask = 1 << bitF->bitpos;
        
        /* if it is a 1 set it, otherwise do nothing */
		if((bitF->buffer[bitF->bytepos] & mask) != 0)
			*(unsigned char *)(info + byte_pos) |= (1 << bit_pos);
        
		/* update buffers */
        /* update info to write variables */
		byte_pos = (bit_pos < 7)? byte_pos : (byte_pos + 1);
		bit_pos = (bit_pos < 7)? (bit_pos + 1) : 0;
        
        /* update bitF structure */
		bitF->bytepos = (bitF->bitpos < 7)? bitF->bytepos : (bitF->bytepos + 1);
		bitF->bitpos = (bitF->bitpos <7)? (bitF->bitpos + 1) : 0;

		/* check if it read all bits from the file and, if it is the case, 
           it reads new bytes from the file */
		if(bitF->bytepos == BIT_IO_BUFFER)
			read_buffer(bitF);
		/* check for reading errors */
		if(bitIO_ferror(bitF) != 0)
			break;
	}

	return i;
}
