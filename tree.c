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
 * Nodes are composed by a sequence of bytes, length of the sequence,
 * absolute offset of the sequence in the window and the node's children
 ***************************************************************************/
struct node{
    int len, off;
    int left, right;
};


int findFreeIndex(struct node *tree, int max)
{
    int i = 0;
    
    while (i <max && tree[i].off != -1)
        i++;
    
    return i;
}


void initialize(struct node *tree, int max)
{
    int i;
    
    for (i = 0; i < max; i++)
        tree[i].off = -1;
}

/***************************************************************************
 *                            INSERT FUNCTION
 * Name         : insert - insert a node in the tree
 * Parameters   : tree - pointer to the binary tree array
 *                root - index of the root in the array
 *                seq - sequence of bytes to be inserted in the new node
 *                off - offset of the sequence
 *                len - length of the sequence
 ***************************************************************************/
void insert(struct node *tree, int root, unsigned char *window, int off, int len, int max)
{
    /* create the new node and insert it in the tree */
    if (tree[root].off == -1){
        tree[root].off = off;
        tree[root].len = len;
        
        tree[root].left = -1;
        tree[root].right = -1;

        return;
    }
    
    /* call recursively the function until the correct position is not found */
    if (memcmp(&(window[off]), &(window[tree[root].off]), len) < 0){
        if (tree[root].left == -1)
            tree[root].left = findFreeIndex(tree, max);
        insert(tree, tree[root].left, window, off, len, max);
    }else{
        if (tree[root].right == -1)
            tree[root].right = findFreeIndex(tree, max);
        insert(tree, tree[root].right, window, off, len, max);
    }
}

/***************************************************************************
 *                            FIND FUNCTION
 * Name         : find - find the longest match in the tree
 * Parameters   : tree - binary search tree
 *                window - whole buffer
 *                index - starting index of the lookahead
 *                size - actual lookahead size
 * Returned     : best match's offset and length
 ***************************************************************************/
int* find(struct node *tree, unsigned char *window, int index, int size)
{
    /* variables */
    int i, ret, root = 0;
    int *off_len;
    
    off_len = (int*)malloc(sizeof(int) * 2);
    
    /* initialize as non-match values */
    off_len[0] = 0;
    off_len[1] = 0;
    
    /* flow the tree finding the longest match node */
    while (tree[root].off != -1){
        
        /* look for how many characters are equal between the lookahead and the node */
        for (i = 0; (ret = memcmp(&(window[index+i]), &(window[tree[root].off + i]), 1)) == 0 && i < size-1; i++){}
        
        /* if the new match is better than the previous one, save the token */
        if (i > off_len[1]){
            off_len[0] = index - tree[root].off;
            off_len[1] = i;
        }
        
        if (ret < 0 && tree[root].left != -1)
            root = tree[root].left;
        else if (ret > 0 && tree[root].right != -1)
            root = tree[root].right;
        else break;
    }
    
    return off_len;
}

/***************************************************************************
 *                           DELETE MINIMUM FUNCTION
 * Name         : deleteMin - find the minimum node in the tree, delete it
 *                and move its content in the correct position. It is called
 *                whenever a node to be deleted has both the children
 * Parameters   : tree - root of the binary tree
 *                elem - node to be deleted
 ***************************************************************************/
void deleteMin(struct node *tree, int del, int min, int *parentChild)
{
    /* call recursively the function util the minimum node is not found */
    if(tree[min].left != -1)
        deleteMin(tree, del, tree[min].left, &(tree[min].left));
    else{
        tree[del].off = tree[min].off;
        tree[del].len = tree[min].len;
        tree[min].off = -1;
        *parentChild = tree[min].right;
    }
}

/***************************************************************************
 *                            DELETE FUNCTION
 * Name         : delete - delete the node from the tree containing the
 *                specific sequence and offset
 * Parameters   : tree - root of the binary tree
 *                seq - sequence to be contained by the node
 *                len - length of the sequence
 *                sb - actual starting index of the search buffer
 *                win_size - whole window size
 ***************************************************************************/
void delete(struct node *tree, int *root, unsigned char *window, int len, int sb)
{
    /* variables */
    int ret, i;
    if (tree[*root].off != -1){
        ret = memcmp(&(window[sb]), &(window[tree[*root].off]), len);
        printf("Comparo \"");
        for (i = 0; i < len; i++)
            printf("%c", window[sb+i]);
        printf("\" con \"");
        for (i = 0; i < len; i++)
            printf("%c", window[tree[*root].off + i]);
        printf("\", off: %d\n", tree[*root].off);
        
        /* smaller node */
        if(ret < 0)
            delete(tree, &(tree[*root].left), window, len, sb);
        /* greater node */
        else if(ret > 0)
            delete(tree, &(tree[*root].right), window, len, sb);
        /* match for the sequence but not for the offset */
        else if(tree[*root].off != sb)
            delete(tree, &(tree[*root].right), window, len, sb);
        /* match, just right child */
        else if (tree[*root].left == -1){
            tree[*root].off = -1;
            *root = tree[*root].right;
        /* match, just left child */
        }else if (tree[*root].right == -1){
            tree[*root].off = -1;
            *root = tree[*root].left;
        /* match, both children */
        }else{
            deleteMin(tree, *root, tree[*root].right, &(tree[*root].right));
        }
    }
}

/***************************************************************************
 *                         UPDATE OFFSET FUNCTION
 * Name         : updateOffset - update the offset when the buffer is shifted
 * Parameters   : tree - binary tree
 *                n - value to be subtracted
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
    if (tree[root].left != -1)
        printtree(tree, tree[root].left);
    printf("off %d in %d\n", tree[root].off, root);
    if (tree[root].right != -1)
        printtree(tree, tree[root].right);
}
