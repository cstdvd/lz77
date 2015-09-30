/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : tree.h
 *   Authors : David Costa and Pietro De Rosa
 *
 ***************************************************************************/
#ifndef tree_h
#define tree_h
/***************************************************************************
 *                            TYPE DEFINITIONS
 * Nodes are composed by a sequence of bytes, length of the sequence,
 * absolute offset of the sequence in the window and the node's children
 ***************************************************************************/
struct node;

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
void insert(struct node **tree, unsigned char *seq, int off, int len);
int* find(struct node *tree, unsigned char *window, int index, int size, int win_size);
void delete(struct node **tree, unsigned char *seq, int len, int sb, int win_size);
void printtree(struct node *tree);
void freetree(struct node **tree);
#endif
