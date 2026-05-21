#include "avl.h"

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
