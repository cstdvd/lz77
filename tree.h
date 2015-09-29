#ifndef tree_h
#define tree_h
typedef struct node{
    unsigned char *seq;
    int len, off;
    struct node *left, *right;
} node;

void insert(struct node **tree, unsigned char *seq, int off, int len);
struct node *find(struct node *tree, unsigned char *seq, int len);
void delete(struct node **tree, unsigned char *seq, int len, int sb, int win_size);
void printtree(struct node *tree);
#endif /* tree_h */
