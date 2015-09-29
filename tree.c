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
 *                            INSERT FUNCTION
 * Name         : insert - insert a node in the tree
 * Parameters   : tree - root of the binary tree
 *                seq - sequence of bytes to be inserted in the new node
 *                off - offset of the sequence
 *                len - length of the sequence
 ***************************************************************************/
void insert(struct node **tree, unsigned char *seq, int off, int len)
{
    /* variables */
    struct node * elem;
    int i;

    /* create the new node and insert it in the tree */
    if (*tree == NULL){
        elem = (struct node*)malloc(sizeof(struct node));
        elem->off = off;
        elem->len = len;
        elem->seq = (unsigned char*)malloc(len);
        memcpy(elem->seq, seq, len);
        
        *tree = elem;
        (*tree)->left = (*tree)->right = NULL;

        return;
    }
    
    /* call recursively the function until the correct position is not found */
    if (memcmp(seq, (*tree)->seq, len) < 0)
        insert(&((*tree)->left), seq, off, len);
    else
        insert(&((*tree)->right), seq, off, len);
    
}

/***************************************************************************
 *                           DELETE MINIMUM FUNCTION
 * Name         : deleteMin - find the minimum node in the tree, delete it
 *                and move its content in the correct position. It is called
 *                whenever a node to be deleted has both the children
 * Parameters   : tree - root of the binary tree
 *                elem - node to be deleted
 ***************************************************************************/
void deleteMin(struct node **tree, struct node *elem)
{
    /* variables */
    struct node *tmp;
    int i;
    
    /* call recursively the function util the minimum node is not found */
    if((*tree)->left != NULL)
        deleteMin(&((*tree)->left), elem);
    else{
        free(elem->seq);
        elem->seq = (unsigned char*)malloc((*tree)->len);
        memcpy(elem->seq, (*tree)->seq, (*tree)->len);
        elem->len = (*tree)->len;
        elem->off = (*tree)->off;
        tmp = (*tree);
        (*tree) = (*tree)->right;
        
        free(tmp->seq);
        free(tmp);
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
void delete(struct node **tree, unsigned char *seq, int len, int sb, int win_size)
{
    /* variables */
    int ret, i;
    struct node *tmp;
    
    if ((*tree) != NULL){
        for(i = 0; i < len && (ret = memcmp(&(seq[(sb+i)%win_size]), &((*tree)->seq[i]), 1)) == 0; i++){}
        
        /* smaller node */
        if(ret < 0)
            delete(&((*tree)->left), seq, len, sb, win_size);
        /* greater node */
        else if(ret > 0)
            delete(&((*tree)->right), seq, len, sb, win_size);
        /* match for the sequence but not for the offset */
        else if((*tree)->off != sb)
            delete(&((*tree)->right), seq, len, sb, win_size);
        /* match, just right son*/
        else if ((*tree)->left == NULL){
            tmp = (*tree);
            (*tree) = (*tree)->right;
            free(tmp->seq);
            free(tmp);
        /* match, just left son */
        }else if ((*tree)->right == NULL){
            tmp = (*tree);
            (*tree) = (*tree)->left;
            free(tmp->seq);
            free(tmp);
        /* match, both children */
        }else
            deleteMin(&((*tree)->right), (*tree));
    }
}

/***************************************************************************
 *                           PRINT TREE FUNCTION
 * Name         : printtree - print the whole tree in increasing order
 * Parameters   : tree - root of the binary tree
 * Just for debug
 ***************************************************************************/
void printtree(struct node *tree)
{
    int i;
    if (tree == NULL)
        return;
    printtree(tree->left);
    for(i = 0; i < tree->len; i++)
        printf("%c", tree->seq[i]);
    printf("\toff %d\n", tree->off);
    printtree(tree->right);
}

/***************************************************************************
 *                           FREE TREE FUNCTION
 * Name         : freetree - delete the whole tree from memory
 * Parameters   : tree - root of the binary tree
 ***************************************************************************/
void freetree(struct node **tree)
{
    if (*tree == NULL)
        return;
    if ((*tree)->left != NULL)
        freetree(&((*tree)->left));
    if ((*tree)->right != NULL)
        freetree(&((*tree)->right));
    
    free((*tree)->seq);
    free(*tree);
    (*tree) = NULL;
}
