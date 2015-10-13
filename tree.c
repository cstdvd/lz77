/***************************************************************************
 *          Lempel, Ziv Encoding and Decoding
 *
 *   File    : tree.c
 *   Authors : David Costa and Pietro De Rosa
 *
 ***************************************************************************/

/***************************************************************************
 *                             INCLUDED FILES
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

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

/***************************************************************************
 *                          CREATE TREE FUNCTION
 * Name         : createTree - memory allocation for the tree array
 * Parameters   : size - maximum number of nodes in the tree
 * Returned     : pointer to the tree array
 ***************************************************************************/
struct node *createTree(int size)
{
    struct node *tree = calloc(size, sizeof(struct node));
    
    return tree;
}

/***************************************************************************
 *                        DESTROY TREE FUNCTION
 * Name         : destroyTree - memory deallocation of the tree array
 * Parameters   : tree - pointer to the tree array
 ***************************************************************************/
void destroyTree(struct node *tree)
{
    free(tree);
}

/***************************************************************************
 *                            INSERT FUNCTION
 * Name         : insert - insert a node in the tree
 * Parameters   : tree - pointer to the binary tree array
 *                root - index of the root in the array
 *                window - pointer to the buffer
 *                abs_off - absolute offset of the sequence
 *                len - length of the sequence
 *                max - size of the tree array
 ***************************************************************************/
void insert(struct node *tree, int *root, unsigned char *window, int abs_off, int len, int max)
{
    /* variables */
    int i, tmp;
    int off = abs_off % max;    /* from absolute index to relative index (array) */
    
    /* no root: the new node becomes the root */
    if (*root == -1){
        *root = off;
        tree[*root].parent = -1;
    }else{
        i = *root;
        
        while (1){
            tmp = i;
            if (memcmp(&(window[abs_off]), &(window[tree[i].off]), len) < 0){
                /* go to the left child */
                i = tree[i].left;
                if (i == -1){
                    /* set parent-child relation */
                    tree[tmp].left = off;
                    tree[off].parent = tmp;
                    
                    break;
                }
            }else{
                /* go to the right child */
                i = tree[i].right;
                if (i == -1){
                    /* set parent-child relation */
                    tree[tmp].right = off;
                    tree[off].parent = tmp;
                
                    break;
                }
            }
        }
    }
    
    /* set other parameters */
    tree[off].off = abs_off;
    tree[off].len = len;
    tree[off].left = -1;
    tree[off].right = -1;
}

/***************************************************************************
 *                            FIND FUNCTION
 * Name         : find - find the longest match in the tree
 * Parameters   : tree - pointer to the binary tree array
 *                root - index of the root in the array
 *                window - pointer to the buffer
 *                index - starting index of the lookahead
 *                size - actual lookahead size
 * Returned     : best match's offset and length
 ***************************************************************************/
struct ret find(struct node *tree, int root, unsigned char *window, int index, int size)
{
    /* variables */
    int i, j;
    struct ret off_len;
    
    /* initialize as non-match values */
    off_len.off = 0;
    off_len.len = 0;
    
    if (root == -1)
        return off_len;
    j = root;
    
    /* flow the tree finding the longest match node */
    while (1){
        
        /* look for how many characters are equal between the lookahead and the node */
        for (i = 0; window[index+i] == window[tree[j].off + i] && i < size-1; i++){}
        
        /* if the new match is better than the previous one, save the values */
        if (i > off_len.len){
            off_len.off = index - tree[j].off;
            off_len.len = i;
        }
        
        if (window[index+i] < window[tree[j].off + i] && tree[j].left != -1)
            j = tree[j].left;
        else if (window[index+i] > window[tree[j].off + i] && tree[j].right != -1)
            j = tree[j].right;
        else break;
    }
    
    return off_len;
}

/***************************************************************************
 *                           MINIMUM CHILD FUNCTION
 * Name         : minChild - find the minimum node in the tree and return
 *                its index, used whether a node to be deleted has both the
 *                children
 * Parameters   : tree - pointer to the binary tree
 *                elem - node to be deleted
 ***************************************************************************/
int minChild(struct node *tree, int index)
{
    int min = index;
    
    while (tree[min].left != -1)
        min = tree[min].left;
    
    return min;
}

/***************************************************************************
 *                            DELETE FUNCTION
 * Name         : delete - delete the node from the tree in the specific
 *                offset
 * Parameters   : tree - pointer to the binary tree
 *                root - index of the root of the binary tree
 *                window - pointer to the buffer
 *                abs_sb - actual starting index of the search buffer
 *                max - size of the tree array
 ***************************************************************************/
void delete(struct node *tree, int *root, unsigned char *window, int abs_sb, int max)
{
    /* variables */
    int parent, child, sb;
    
    sb = abs_sb % max;  /* from absolute index to relative index (array) */
    
    if (tree[sb].left == -1){
        /* the node to be deleted has not the left child */
        child = tree[sb].right;
        if (child != -1)
            tree[child].parent = tree[sb].parent;
        parent = tree[sb].parent;

    }else if (tree[sb].right == -1){
        /* the node to be deleted has not the right child */
        child = tree[sb].left;
        tree[child].parent = tree[sb].parent;
        parent = tree[sb].parent;
        
    }else{
        /* the node to be deleted has both the children: it will be replaced
           by the minimum child of its right subtree */
        child = minChild(tree, tree[sb].right);
        
        if (tree[child].parent == sb){
            /* just the left child has to be updated */
            parent = tree[sb].parent;
            tree[child].parent = parent;
            
        }else{
            /* also the right child has to be updated */
            parent = tree[child].parent;
            tree[parent].left = tree[child].right;
            if(tree[child].right != -1)
                tree[tree[child].right].parent = parent;
        
            tree[child].right = tree[sb].right;
            tree[child].parent = tree[sb].parent;
        
            if (tree[child].right != -1)
                tree[tree[child].right].parent = child;
        
            parent = tree[child].parent;
        }
        
        tree[child].left = tree[sb].left;
        if (tree[child].left != -1)
            tree[tree[child].left].parent = child;
    }
    
    /* set the parent's child or the child as the root */
    if (parent != -1){
        if (tree[parent].right == sb){
            tree[parent].right = child;
        }else{
            tree[parent].left = child;
        }
    }else
        *root = child;
    
}

/***************************************************************************
 *                         UPDATE OFFSET FUNCTION
 * Name         : updateOffset - update the offset when the buffer is shifted
 * Parameters   : tree - binary tree
 *                n - value to subtract
 *                max - length of the array
 ***************************************************************************/
void updateOffset(struct node *tree, int n, int max)
{
    int i;
    
    for (i = 0; i < max; i++){
        if (tree[i].off != -1)
            tree[i].off -= n;
    }
}

/***************************************************************************
 *                           PRINT TREE FUNCTION
 * Name         : printtree - print the whole tree in increasing order
 * Parameters   : tree - binary tree
 *                root - node's index
 * Just for debug
 ***************************************************************************/
void printtree(struct node *tree, int root)
{
    if (root == -1)
        return;
    
    if (tree[root].left != -1)
        printtree(tree, tree[root].left);
    printf("%d\n", tree[root].off);
    if (tree[root].right != -1)
        printtree(tree, tree[root].right);
}
