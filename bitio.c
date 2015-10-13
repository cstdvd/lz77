/*
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : bitio.c
 *   Authors : David Costa and Pietro De Rosa
 *
 */



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
	FILE *file; // file to (from) write (read)
	int mode; // specify the mode (READ or WRITE)
	int bytepos; // store the actual byte's position in the buffer
	int bitpos; // store the last bit's position in the byte
	unsigned char *buffer; // bits buffer
};



/***************************************************************************
 *                                FUNCTIONS
 ***************************************************************************/
/*					          BIT OF FUNCTION
 *
 * 	name:			bitof
 * 	description:	It is an utility function which computes the minimum #
 * 					of bits needed to represent an integer given as input.
 * 	parameters:		n	- integer number to represent
 * 	returned:		min # of bits needed to represent the input
 */
int bitof(int n){
	return (int)(ceil(log(n)/log(2)));
}



/*								BIT I/O FEOF FUNCTION
 *
 * 	name:			bitIO_feof
 * 	description:	It allows to check if a read mode opened bitFILE has
 * 					the end-of-file indicator set.
 * 	parameters:		bitF	- bitFILE opened in read mode
 * 	returned:		0, EOF is not set
 * 					non-zero value, EOF is set
 */
int bitIO_feof(struct bitFILE *bitF)
{
	return feof(bitF->file);
}



/*								WRITE BUFFER FUNCTION
 *
 * 	name:			write_buffer
 * 	description:	It writes the utilized bytes of the buffer in the file
 * 					opened in write mode. It is used either by the 'bitIO_write'
 * 					function to flush the whole buffer, either by the
 * 					'bitIO_close' one to write the remained bytes in the file.
 * 	parameters:		bitF	- bitFILE opened in write mode
 */
void write_buffer(struct bitFILE *bitF){

	int ret;

	// write data
	ret = fwrite(bitF->buffer, 1, bitF->bytepos, bitF->file);
	// check for errors on writing
	if(ret != bitF->bytepos)
	{
		perror("Error on write_buffer.\n");
		exit(EXIT_FAILURE);
	}
	// clear the buffer
	bitF->bytepos = 0;
	bitF->bitpos = 0;
	memset(bitF->buffer, 0, BIT_IO_BUFFER);
}



/*									READ BUFFER FUNCTION
 *
 * 	name:			read_buffer
 * 	description:	It reads at most BIT_IO_BUFFER bytes from the bitFILE and
 * 					copy them into the buffer. It is used only by the 'bitIO_read'
 * 					function, once the last bit in the buffer has been read.
 * 	parameters:		bitF	- bitFILE opened in write mode
 */
void read_buffer(struct bitFILE *bitF){

	int ret;

	// read data
	ret = fread(bitF->buffer, 1, BIT_IO_BUFFER, bitF->file);
	// check for errors
	if(ret < BIT_IO_BUFFER && ferror(bitF->file))
	{
		 perror("Error reading file.\n");
		 exit(EXIT_FAILURE);
	}
	// clear variables
	bitF->bytepos = 0;
	bitF->bitpos = 0;
}



/*									BIT I/O OPEN FUNCTION
 *
 * 	name:			bitIO_open
 * 	description:	It tries to open the file specified by 'path' in write or read
 * 					mode, depending on the value of 'mode' parameter.
 * 					Finally, return the bitFILE just opened as result.
 * 	parameters:		path	- path to the file to open
 * 					mode	- specify the mode: read(BIT_IO_R) or write(BIT_IO_W)
 * 	returned:		bitFILE just opened in the specified mode
 */
struct bitFILE* bitIO_open(const char *path, int mode){

	struct bitFILE *bitF;

	// errors handler
	if(mode!=BIT_IO_W && mode!=BIT_IO_R)
	{
		printf("Invalid mode detected.\n");
		exit(1);
	}
	if(path == NULL)
	{
		printf("Invalid path.\n");
		exit(1);
	}

	// initialize structure
	bitF = (struct bitFILE*)calloc(1, sizeof(struct bitFILE));
	bitF->mode = mode;
	bitF->bytepos = 0;
	bitF->bitpos = 0;
	bitF->buffer = (unsigned char*)calloc(BIT_IO_BUFFER, sizeof(unsigned char));
	memset(bitF->buffer, 0, BIT_IO_BUFFER);

	// open file
		// read binary mode
	if(bitF->mode == BIT_IO_R)
	{
		if((bitF->file = fopen(path, "rb")) == NULL)
		{
			perror("Opening file to read");
			exit(EXIT_FAILURE);
		}
		// fill the buffer for the 1st time
		read_buffer(bitF);
	}
		// write mode
	else if((bitF->file = fopen(path, "w")) == NULL)
	{
		perror("Opening file to write");
		exit(EXIT_FAILURE);
	}

	return bitF;
}



/*										BIT I/O CLOSE FUNCTION
 *
 * 	name:			bitIO_close
 * 	description:	It closes the bitFILE given as input and frees memory portions
 * 					occupied by the structures. If the file is opened in write mode
 * 					and there are pending bits, these are written in the buffer first.
 * 	parameters:		bitF	- bitFILE to close
 * 	returned:		0, file closed successfully
 * 					-1, error on inputs
 */
int bitIO_close(struct bitFILE *bitF){

	// errors handler
	if(bitF == NULL || bitF->file == NULL)
		return -1;

	// write the unwritten bytes into the file
	if(bitF->mode == BIT_IO_W)
	{
		if(bitF->bitpos > 0)
			(bitF->bytepos)++;
		write_buffer(bitF);
	}
	// close the file
	fclose(bitF->file);
	// free memory
	free(bitF->buffer);
	free(bitF);

	return 0;
}



/*										BIT I/O WRITE FUNCTION
 *
 * 	name:			bitIO_write
 * 	description:	It writes the first 'nbit' pointed by 'info' in the bitFILE
 * 					pointed by 'bitF'.
 * 	parameters:		bitF	- bitFILE opened in write mode
 * 					info 	- buffer containing information to be written
 * 					nbytes	- number of bytes to be written from the buffer
 * 					nbit	- number of bits required to represent the information
 * 	returned:		0, write successfully
 * 					-1, error on inputs
 */
int bitIO_write(struct bitFILE *bitF, void *info, int nbit){

	int i;
	int byte_pos = 0, bit_pos = 0; // respectively, byte's position and bit's position (in the byte) in 'info'
	unsigned char mask; // mask of bits

	// errors handler
	if(bitF == NULL || bitF->file == NULL || bitF->mode != BIT_IO_W|| info == NULL || nbit < 0)
		return -1;

	for(i=0; i<nbit; i++)
	{
		// get bit to write
		mask = 1 << bit_pos;
			// if it is a 1 set it, otherwise do nothing
		if((*(unsigned char *)(info + byte_pos) & mask) != 0)
			bitF->buffer[bitF->bytepos] |= (1 << bitF->bitpos);
		//update buffers
			//update info to write variables
		byte_pos = (bit_pos < 7)? byte_pos : (byte_pos + 1);
		bit_pos = (bit_pos < 7)? (bit_pos + 1) : 0;
			// update bitF structure
		bitF->bytepos = (bitF->bitpos < 7)? bitF->bytepos : (bitF->bytepos + 1);
		bitF->bitpos = (bitF->bitpos <7)? (bitF->bitpos + 1) : 0;
		// check if a write_buffer must be done
		if(bitF->bytepos == BIT_IO_BUFFER)
			write_buffer(bitF);
	}

	return 0;
}



/*											BIT I/O READ FUNCTION
 *
 * 	name:			bitIO_read
 * 	description:	It reads the first 'nbit' from the bitFILE 'bitF' and puts them in the buffer
 * 					pointed by 'info' whose size is 'info_s'.
 * 	parameters:		bitF	- bitFILE opened in read mode
 * 					info 	- buffer where read bits are put
 * 					info_s	- size of the 'info' buffer in bytes
 * 					nbit	- number of bits to read
 * 	returned:		0, read successfully
 * 					-1, error on inputs
 */
int bitIO_read(struct bitFILE *bitF, void *info, int info_s, int nbit){

	int i;
	int byte_pos = 0, bit_pos = 0; // respectively, byte's position and bit's position (in the byte) in 'info'
	unsigned char mask; // mask of bits

	// errors handler
	if(bitF == NULL || bitF->file == NULL || bitF->mode != BIT_IO_R || info == NULL || info_s <= 0 || nbit < 0)
		return -1;

	// clear the 'info' buffer
	memset(info, 0, info_s);
	// begin the reading
	for(i=0; i<nbit; i++)
	{
		// get bit to read
		mask = 1 << bitF->bitpos;
			// if it is a 1 set it, otherwise do nothing
		if((bitF->buffer[bitF->bytepos] & mask) != 0)
			*(unsigned char *)(info + byte_pos) |= (1 << bit_pos);
		//update buffers
			//update info to write variables
		byte_pos = (bit_pos < 7)? byte_pos : (byte_pos + 1);
		bit_pos = (bit_pos < 7)? (bit_pos + 1) : 0;
			// update bitF structure
		bitF->bytepos = (bitF->bitpos < 7)? bitF->bytepos : (bitF->bytepos + 1);
		bitF->bitpos = (bitF->bitpos <7)? (bitF->bitpos + 1) : 0;
		// check if it read all bits from the file and, if it is the case, it reads new bytes from the file
		if(bitF->bytepos == BIT_IO_BUFFER)
			read_buffer(bitF);
	}

	return 0;
}
