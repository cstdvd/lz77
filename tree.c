#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node{
    unsigned char *seq;
    int len, off;
    struct node *left, *right;
};

void insert(struct node **tree, unsigned char *seq, int off, int len)
{
    struct node *elem = (struct node*)malloc(sizeof(struct node));
    int i;
    elem->off = off;
    elem->len = len;
    elem->seq = (unsigned char*)malloc(len);
    memcpy(elem->seq, seq, len);

    if (*tree == NULL){
        *tree = elem;
        (*tree)->left = (*tree)->right = NULL;
        /*printf("Inserisco ");
        for(i = 0; i < elem->len; i++)
            printf("%c", elem->seq[i]);
        printf("\t off %d\n", elem->off);*/
        return;
    }
    if (memcmp(elem->seq, (*tree)->seq, elem->len) <= 0)
        insert(&((*tree)->left), seq, off, len);
    else
        insert(&((*tree)->right), seq, off, len);
}

struct node *find(struct node *tree, unsigned char *seq, int len)
{
    int ret;
    
    if (tree == NULL)
        return NULL;
    
    ret = memcmp(seq, tree->seq, len);
    
    if (ret == 0)
        return tree;
    else if (ret < 0){
        if (tree->left != NULL)
            return find(tree->left, seq, len);
        else
            return tree;
    }else{
        if (tree->right != NULL)
            return find(tree->right, seq, len);
        else
            return tree;
    }
}

void deleteMin(struct node **tree, struct node *elem)
{
    struct node *tmp;
    
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

void delete(struct node **tree, unsigned char *seq, int len)
{
    int ret;
    struct node *tmp;
    
    if ((*tree) != NULL){
        ret = memcmp(seq, (*tree)->seq, len);
    
        if(ret < 0)
            delete(&((*tree)->left), seq, len);
        else if(ret > 0)
            delete(&((*tree)->right), seq, len);
        else if ((*tree)->left == NULL){
            tmp = (*tree);
            (*tree) = (*tree)->right;
            free(tmp->seq);
            free(tmp);
        }else if ((*tree)->right == NULL){
            tmp = (*tree);
            (*tree) = (*tree)->left;
            free(tmp->seq);
            free(tmp);
        }else
            deleteMin(&((*tree)->right), (*tree));
    }
}

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

