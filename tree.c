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
        /*printf("Inserito ");
        for (j = 0; j < len; j++)
            printf("%c", window[tree[root].off + j]);
        printf(" off:%d in %d\n", tree[root].off, root);*/

        return;
    }
    
    /* call recursively the function until the correct position is not found */
    if (memcmp(&(window[off]), &(window[tree[root].off]), len) < 0){
        if (tree[root].left == -1)
            tree[root].left = findFreeIndex(tree, max);
        //printf("Inserisco a sinistra: %d\n", tree[root].left);
        insert(tree, tree[root].left, window, off, len, max);
    }else{
        if (tree[root].right == -1)
            tree[root].right = findFreeIndex(tree, max);
        //printf("Inserisco a destra: %d\n", tree[root].right);
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
int* find(struct node *tree, int root, unsigned char *window, int index, int size)
{
    /* variables */
    int i, j, ret;
    int *off_len;
    
    off_len = (int*)malloc(sizeof(int) * 2);
    
    /* initialize as non-match values */
    off_len[0] = 0;
    off_len[1] = 0;
    
    j = root;
    
    /* flow the tree finding the longest match node */
    while (tree[j].off != -1){
        
        /* look for how many characters are equal between the lookahead and the node */
        for (i = 0; (ret = memcmp(&(window[index+i]), &(window[tree[j].off + i]), 1)) == 0 && i < size-1; i++){}
        
        /* if the new match is better than the previous one, save the token */
        if (i > off_len[1]){
            off_len[0] = index - tree[j].off;
            off_len[1] = i;
        }
        
        if (ret < 0 && tree[j].left != -1)
            j = tree[j].left;
        else if (ret > 0 && tree[j].right != -1)
            j = tree[j].right;
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
        //printf("Sposto %d in %d\n", min, del);
        tree[del].off = tree[min].off;
        tree[del].len = tree[min].len;
        tree[min].off = -1;
        *parentChild = tree[min].right;
        tree[min].right = -1;
        tree[min].left = -1;
    }
}

/***************************************************************************
 *                            DELETE FUNCTION
 * Name         : delete - delete the node from the tree containing the
 *                specific sequence and offset
 * Parameters   : tree - binary tree
 *                root - root of the binary tree
 *                window - buffer
 *                len - length of the sequence
 *                sb - actual starting index of the search buffer
 ***************************************************************************/
void delete(struct node *tree, int *root, unsigned char *window, int len, int sb)
{
    /* variables */
    int ret, i, *parentChild = NULL;
    
    i = *root;
    //printf("Parto da %d\n", *root);
    
    while (tree[i].off != -1){
        ret = memcmp(&(window[sb]), &(window[tree[i].off]), len);
        /*printf("Comparo");
        for (j = 0; j < len; j++)
            printf("%c", window[sb+j]);
        printf("\" con \"");
        for (j = 0; j < len; j++)
            printf("%c", window[tree[i].off + j]);
        printf(" off1: %d off2: %d ret: %d\n", sb, tree[i].off, ret);*/
        
        if (ret < 0){
            parentChild = &(tree[i].left);
            i = tree[i].left;
            //printf("Vado a sinistra: %d\n", i);
        }else if (ret > 0){
            parentChild = &(tree[i].right);
            i = tree[i].right;
            //printf("Vado a destra: %d\n", i);
        }else if (tree[i].off != sb){
            parentChild = &(tree[i].right);
            i = tree[i].right;
            //printf("Vado a destra: %d\n", i);
        }else if (tree[i].left == -1){
            //printf("Elimino %d\n", tree[i].off);
            tree[i].off = -1;
            if (parentChild != NULL)
                *parentChild = tree[i].right;
            else{
                *root = tree[i].right;
                //printf("Root off: %d in %d\n", tree[*root].off, *root);
            }
            tree[i].right = -1;
            tree[i].left = -1;
            return;
        }else if (tree[i].right == -1){
            //printf("Elimino %d\n", tree[i].off);
            tree[i].off = -1;
            if (parentChild != NULL)
                *parentChild = tree[i].left;
            else{
                *root = tree[i].left;
                //printf("Root off: %d in %d\n", tree[*root].off, *root);
            }
            tree[i].right = -1;
            tree[i].left = -1;
            return;
        }else{
            //printf("Elimino %d\n", tree[i].off);
            deleteMin(tree, i, tree[i].right, &(tree[i].right));
            return;
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
