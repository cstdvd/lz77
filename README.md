# LZ77
Lempel, Ziv Encoding and Decoding

## Description
LZ77 is a lossless data compression algorithm published by Abraham Lempel 
and Jacob Ziv in 1977. It is a dictionary coder and maintains a sliding window 
during compression.

The sliding window is divided in two parts: 
- Search-Buffer (dictionary - encoded data)
- Lookahead (uncompressed data).

LZ77 algorithms achieve compression by addressing byte sequences from former contents 
instead of the original data. All data will be coded in the same form (called token):
- Address to already coded contents; 
- Sequence length; 
- First deviating symbol.

The window is contained in a fixed size buffer.

The match between SB and LA is made by a binary tree, implemented in an array.

Reading and writing on the encoded file it is made via the bitIO library, implemented in the project. It allows to read and to write on the file bit-per-bit instead byte-per-byte as usual.

## Usage
Syntax:
```
./lz77 <options>
```
Options:  
```
-c: compression mode
-d: decompression mode
-i <filename>: input file
-o <filename>: output file
-l <value>: lookahead size (default 15)
-s <value>: searchbuffer size (default 4095)
-h: help
```
The *lookahead* and *searchbuffer* sizes are optional. If the two options are not set, default values are used.
