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
 * Nodes are composed by the offset of the sequence in the buffer, length 
 * of the sequence, index of its parent in the tree, indices of its children
 * in the tree.
 ***************************************************************************/
struct node{
    int len, off;
    int parent;
    int left, right;
};

struct ret{
    int off, len;
};

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
void initialize(struct node *tree, int max);
void insert(struct node *tree, int *root, unsigned char *window, int off, int len, int max);
struct ret find(struct node *tree, int root, unsigned char *window, int index, int size);
void delete(struct node *tree, int *root, unsigned char *window, int abs_sb, int max);
void updateOffset(struct node *tree, int n, int max);
void printtree(struct node *tree, int root);
#endif
