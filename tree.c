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
    struct node * elem;
    int i;

    if (*tree == NULL){
        elem = (struct node*)malloc(sizeof(struct node));
        elem->off = off;
        elem->len = len;
        elem->seq = (unsigned char*)malloc(len);
        memcpy(elem->seq, seq, len);
        
        *tree = elem;
        (*tree)->left = (*tree)->right = NULL;
        /*printf("Inserisco ");
        for(i = 0; i < elem->len; i++)
            printf("%c", elem->seq[i]);
        printf("\t offset = %d\n", elem->off);*/
        return;
    }
    if (memcmp(seq, (*tree)->seq, len) < 0){
        //printf("inserisco sottoalbero sinistro\n");
        insert(&((*tree)->left), seq, off, len);
    }else{
        //printf("inserisco sottoalbero destro\n");
        insert(&((*tree)->right), seq, off, len);
    }
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
    int i;
    
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
        /*printf("nodo min -> ");
        for(i = 0; i < elem->len; i++)
            printf("%c", elem->seq[i]);
        printf("|%d\n", elem->off);*/
    }
}

void delete(struct node **tree, unsigned char *seq, int len, int sb, int win_size)
{
    int ret, i;
    struct node *tmp;
    
    if ((*tree) != NULL){
        /*printf("Confronto ");
        for(i = 0; i < len; i++)
            printf("%c", seq[(sb+i)%win_size]);
        printf(" con il nodo ");
        for(i = 0; i < (*tree)->len; i++)
            printf("%c", (*tree)->seq[i]);
        printf("\n");*/
        for(i = 0; i < len && (ret = memcmp(&(seq[(sb+i)%win_size]), &((*tree)->seq[i]), 1)) == 0; i++){}
        if(ret < 0){
            //printf("ret < 0\n");
            delete(&((*tree)->left), seq, len, sb, win_size);
        }else if(ret > 0){
            //printf("ret > 0\n");
            delete(&((*tree)->right), seq, len, sb, win_size);
        }else if((*tree)->off != sb){
            //printf("ret = 0 & off(%d) != sb(%d)\n", (*tree)->off, sb);
            delete(&((*tree)->right), seq, len, sb, win_size);
        }else if ((*tree)->left == NULL){
            /*printf("Elimino ");
            for(i = 0; i < (*tree)->len; i++)
                printf("%c", (*tree)->seq[i]);
            printf("\n");*/
            tmp = (*tree);
            (*tree) = (*tree)->right;
            free(tmp->seq);
            free(tmp);
        }else if ((*tree)->right == NULL){
            /*printf("Elimino ");
            for(i = 0; i < (*tree)->len; i++)
                printf("%c", (*tree)->seq[i]);
            printf("\n");*/
            tmp = (*tree);
            (*tree) = (*tree)->left;
            free(tmp->seq);
            free(tmp);
        }else{
            /*printf("Elimino ");
            for(i = 0; i < (*tree)->len; i++)
                printf("%c", (*tree)->seq[i]);
            printf("\n");*/
            deleteMin(&((*tree)->right), (*tree));
        }
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

