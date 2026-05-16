#include "../vector/generic.h"
#include "rbtree.h"

Vector* rbSearch(const RBTree* tree, const char* key) {
    RBNode *node = tree->root;
    while (node != tree->nil) {
        int compare = strcmp(key, node->key);
        if (compare == 0) {
            return node->postings;
        }
        else if (compare < 0) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }

    return NULL;
}
