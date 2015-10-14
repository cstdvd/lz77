/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : lz77.h
 *   Authors : David Costa and Pietro De Rosa
 *
 ***************************************************************************/

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
#ifndef lz77_h
#define lz77_h
void encode(FILE *file, struct bitFILE *out, int la, int sb);
void decode(struct bitFILE *file, FILE *out);
#endif
