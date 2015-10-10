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
void insert(struct node *tree, int *root, unsigned char *window, int abs_off, int len, int max)
{
    int i, tmp;
    int off = abs_off % max;
    //int k;
    
    if (*root == -1){
        *root = off;
        tree[*root].off = abs_off;
        tree[*root].len = len;
        tree[*root].left = -1;
        tree[*root].right = -1;
        tree[*root].parent = -1;
        /*printf("Inserisco %d in %d: ", abs_off, off);
        for (k = 0; k < len; k++)
            printf("%c", window[abs_off + k]);
        printf(" len %d\n", len);*/
        
        return;
    }
    
    i = *root;
    
    while (1){
        if (memcmp(&(window[abs_off]), &(window[tree[i].off]), len) < 0){
            tmp = i;
            i = tree[i].left;
            //printf("Vado a sinistra: %d\n", i);
            if (i == -1){
                tree[off].off = abs_off;
                tree[tmp].left = off;
                tree[off].parent = tmp;
                tree[off].len = len;
                tree[off].left = -1;
                tree[off].right = -1;
                /*printf("Inserisco %d in %d: ", abs_off, off);
                for (k = 0; k < len; k++)
                    printf("%c", window[abs_off + k]);
                printf(" len %d\n", len);*/
                
                break;
            }
        }else{
            tmp = i;
            i = tree[i].right;
            //printf("Vado a destra: %d\n", i);
            if (i == -1){
                tree[off].off = abs_off;
                tree[tmp].right = off;
                tree[off].parent = tmp;
                tree[off].len = len;
                tree[off].left = -1;
                tree[off].right = -1;
                /*printf("Inserisco %d in %d: ", abs_off, off);
                for (k = 0; k < len; k++)
                    printf("%c", window[abs_off + k]);
                printf(" len %d\n", len);*/
                
                break;
            }
        }
    }
    /* create the new node and insert it in the tree */
    
    /* call recursively the function until the correct position is not found */
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
struct ret find(struct node *tree, int root, unsigned char *window, int index, int size)
{
    /* variables */
    int i, j, ret;
    struct ret off_len;
    
    /* initialize as non-match values */
    off_len.off = 0;
    off_len.len = 0;
    
    if (root == -1)
        return off_len;
    j = root;
    
    /* flow the tree finding the longest match node */
    while (tree[j].off != -1){
        
        /* look for how many characters are equal between the lookahead and the node */
        for (i = 0; (ret = memcmp(&(window[index+i]), &(window[tree[j].off + i]), 1)) == 0 && i < size-1; i++){}
        
        /* if the new match is better than the previous one, save the token */
        if (i > off_len.len){
            off_len.off = index - tree[j].off;
            off_len.len = i;
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

int minChild(struct node *tree, int index)
{
    int min = index;
    
    while (tree[min].left != -1)
        min = tree[min].left;
    
    return min;
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
void delete(struct node *tree, int *root, unsigned char *window, int abs_sb, int max)
{
    /* variables */
    int parent, child, sb;
    //int k;
    
    sb = abs_sb % max;

    /*printf("Elimino %d in %d: ", abs_sb, sb);
    for (k = 0; k < tree[sb].len; k++)
        printf("%c", window[abs_sb + k]);
    printf("\n");*/
    
    if (tree[sb].left == -1){
        //printf("\tNo figlio sinistro\n");
        child = tree[sb].right;
        if (child != -1)
            tree[child].parent = tree[sb].parent;
        parent = tree[sb].parent;
        if (parent != -1){
            if (tree[parent].right == sb){
                tree[parent].right = child;
            }else{
                tree[parent].left = child;
            }
        }else
            *root = child;
    }else if (tree[sb].right == -1){
        //printf("\tNo figlio destro\n");
        child = tree[sb].left;
        tree[child].parent = tree[sb].parent;
        parent = tree[sb].parent;
        if (parent != -1){
            if (tree[parent].right == sb){
                tree[parent].right = child;
            }else{
                tree[parent].left = child;
            }
        }else
            *root = child;
    }else{
        //printf("\tEntrambi i figli: ");
        child = minChild(tree, tree[sb].right);
        //printf("sostituisco %d con %d\n", sb, child);
        
        if (tree[child].parent == sb){
            parent = tree[sb].parent;
            tree[child].parent = parent;
            tree[child].left = tree[sb].left;
            if (tree[child].left != -1)
                tree[tree[child].left].parent = child;
            if (parent != -1){
                if (tree[parent].right == sb){
                    tree[parent].right = child;
                }else{
                    tree[parent].left = child;
                }
            }else
                *root = child;
        }else{
            parent = tree[child].parent;
            tree[parent].left = tree[child].right;
            if(tree[child].right != -1)
                tree[tree[child].right].parent = parent;
        
            tree[child].left = tree[sb].left;
            tree[child].right = tree[sb].right;
            tree[child].parent = tree[sb].parent;
        
            if (tree[child].left != -1)
                tree[tree[child].left].parent = child;
            if (tree[child].right != -1)
                tree[tree[child].right].parent = child;
        
            parent = tree[child].parent;
            if (parent != -1){
                if (tree[parent].right == sb){
                    tree[parent].right = child;
                }else{
                    tree[parent].left = child;
                }
            }else
                *root = child;
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
    if (root == -1)
        return;
    
    if (tree[root].left != -1)
        printtree(tree, tree[root].left);
    printf("%d\n", tree[root].off);
    if (tree[root].right != -1)
        printtree(tree, tree[root].right);
}
