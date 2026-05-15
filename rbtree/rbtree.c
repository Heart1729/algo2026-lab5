#include "../vector/generic.h"
#include "rbtree.h"

Vector* rbSearch(const RBTree* tree, const char* key) {
    RBNode *node = tree->root;
    while (node != tree->nil) {
        if (key == node->key) {
            return node->postings;
        }
        else if (key < node->key) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }

    return NULL;
}
