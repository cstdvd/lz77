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
 ***************************************************************************/
struct node;

struct ret{
    int off, len;
};

/***************************************************************************
 *                         FUNCTIONS DECLARATION
 ***************************************************************************/
struct node *createTree(int size);
void destroyTree(struct node *tree);
void insert(struct node *tree, int *root, unsigned char *window, int off, int len, int max);
struct ret find(struct node *tree, int root, unsigned char *window, int index, int size);
void delete(struct node *tree, int *root, unsigned char *window, int abs_sb, int max);
void updateOffset(struct node *tree, int n, int max);
void printtree(struct node *tree, int root);
#endif
