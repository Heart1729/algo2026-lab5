#include "avl.h"

AVLTree* createAVLTree(void)
{
    AVLTree* tree = (AVLTree*)malloc(sizeof(AVLTree));
    
    if (!tree) {
        printf("Problem in 'createAVL'\n-> Tree wasn't allocated.\n");
        return NULL;
    }

    tree->root = NULL;

    return tree;
}


static void freeAVLNode(AVLNode* node)
{
    if (!node)
        return;

    freeAVLNode(node->left);
    freeAVLNode(node->right);

    free(node->key);
    vectorFree(node->postings);
    free(node);

    return;
}

void freeAVLTree(AVLTree* tree)
{
    if (!tree) {
        printf("Problem in 'freeAVLTree'\n-> Given tree is NULL.\n");
        return;
    }

    if (!tree->root) {
        printf("Problem in 'freeAVLTree'\n-> Given tree->root is NULL.\n");
        return;
    }

    freeAVLNode(tree->root);
    free(tree);

    return;
}

Vector* avlSearch(const AVLTree* tree, const char* key)
{
    if (!tree) {
        printf("Problem in 'avlSearch'\n-> Given tree is NULL.\n");
        return NULL;
    }

    AVLNode* cur_node = tree->root;

    while (cur_node) {
        int cmp = strcmp(key, cur_node->key);

        if (cmp < 0)
            cur_node = cur_node->left;
        
        else if (cmp > 0)
            cur_node = cur_node->right;

        else
            return cur_node->postings;
    }

    return NULL;
}