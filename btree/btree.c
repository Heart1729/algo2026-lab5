#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

BTree *createBTree(void) {
    BTree *tree = (BTree*)malloc(sizeof(BTree));
    if (tree == NULL) {
        printf("BTree was not allocated...\n");
        return NULL;
    }

    tree->size = 0;
    tree->root = NULL;

    return tree;
}

void freeBTreeNode(BTreeNode *node) {
    if (node == NULL) {
        printf("NULL node cannot be free...\n");
        return;
    }

    if (node->is_leaf == 0) {
        for (int i = 0; i <= node->n; i++) {
            freeBTreeNode(node->children[i]);
        }
    }

    for (int i = 0; i < node->n; i++) {
        free(node->keys[i]);
        vectorFree(node->postings[i]);
    }
    
    free(node);
}

void freeBTree(BTree *tree) {
    freeBTreeNode(tree->root);
    free(tree);
}

BTreeNode *createBTreeNode(const char* key, Vector *postings, int is_leaf) {
    BTreeNode *node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (node == NULL) {
        printf("Node was not allocated...\n");
        return NULL;
    }

    for (int i = 0; i < BTREE_MAX_KEYS; i++) {
        node->keys[i] = NULL;
        node->postings[i] = NULL;
    }

    for (int i = 0; i < BTREE_MAX_CH; i++) {
        node->children[i] = NULL;
    }

    node->n = 0;
    node->is_leaf = is_leaf;

    if (key != NULL) {
        node->keys[0] = strdup(key);
        node->postings[0] = postings;
        node->n = 1;
    }

    return node;
}

static void splitChild(BTreeNode* parent, int index) {
    BTreeNode *child = parent->children[index];
    BTreeNode *newNode = createBTreeNode(NULL, NULL, child->is_leaf);

    newNode->n = BTREE_MAX_KEYS / 2;

    for (int i = 0; i < BTREE_MAX_KEYS / 2; i++) {
        newNode->keys[i] = child->keys[i + BTREE_MAX_KEYS / 2 + 1];
        newNode->postings[i] = child->postings[i + BTREE_MAX_KEYS / 2 + 1];
    }

    if (child->is_leaf == 0) {
        for (int i = 0; i < BTREE_MAX_CH / 2; i++) {
            newNode->children[i] = child->children[i + BTREE_MAX_CH/ 2];
        }
    }   
    
    child->n = BTREE_MAX_KEYS / 2;
    parent->keys[index] = child->keys[BTREE_MAX_KEYS / 2];
    parent->postings[index] = child->postings[BTREE_MAX_KEYS / 2];

    for (int i = BTREE_MAX_KEYS / 2; i < BTREE_MAX_KEYS; i++) {
        child->keys[i] = NULL;
        child->postings[i] = NULL;
    }

    for (int i = parent->n; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newNode;

    for (int i = parent->n - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->postings[i + 1] = parent->postings[i];
    }

    parent->n += 1;
}

static void insertNonFull(BTreeNode *node, const char* key, Vector *postings) {
    int i = node->n - 1;

    if (node->is_leaf == 1) {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            node->keys[i+1] = node->keys[i];
            node->postings[i+1] = node->postings[i];
            i--;
        }
        node->keys[i + 1] = strdup(key);
        node->postings[i + 1] = postings;
        node->n += 1;
    }
    else {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            i -= 1;
        }
        i += 1;

        if (node->children[i]->n == BTREE_MAX_KEYS) {
            splitChild(node, i);

            if (strcmp(key, node->keys[i]) > 0) {
                i += 1;
            }
        }

        insertNonFull(node->children[i], key, postings);
    }
}

static int findKeyIndex(const BTreeNode *node, const char *key) {
    int left = 0;
    int right = node->n;

    while (left < right) {
        int mid = left + (right - left) / 2;

        if (strcmp(node->keys[mid], key) < 0) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }

    return left;
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    if (tree == NULL) {
        printf("Tree is NULL\n");
        return;
    }

    if (tree->root != NULL) {
        Vector* existing = btreeSearch(tree, key);
        if (existing != NULL) {
            appendPosting(existing, doc_id, title);
            return;
        }
    }

    Vector *postings = createPostingList();
    appendPosting(postings, doc_id, title);

    if (tree->root == NULL) {
        tree->root = createBTreeNode(key, postings, 1);
        tree->root->is_leaf = 1;
        tree->size += 1;
    }

    else {
        if (tree->root->n == BTREE_MAX_KEYS) {
            BTreeNode *newRoot = createBTreeNode(NULL, NULL, 0);
            newRoot->children[0] = tree->root;
            newRoot->is_leaf = 0;

            splitChild(newRoot, 0);
            tree->root = newRoot;
        }

        insertNonFull(tree->root, key, postings);
        tree->size += 1;
    }
}

static Vector* searchNode(const BTreeNode* node, const char* key) {
    if (node == NULL) {
        printf("Node is NULL...\n");
        return NULL;
    }
    int i = findKeyIndex(node, key);

    if (i < node->n && strcmp(node->keys[i], key) == 0) {
        return node->postings[i];
    }

    if (node->is_leaf) {
        return NULL;
    }

    return searchNode(node->children[i], key);
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    if (tree == NULL) {
        printf("Tree is NULL...");

        return NULL;
    }

    return searchNode(tree->root, key);
}

static void traverseNode(
    BTreeNode* node,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (node == NULL) {
        printf("Node is NULL\n");
        return;
    }

    for (int i = 0; i < node->n; i++) {
        if (!node->is_leaf)
            traverseNode(node->children[i], visit, ctx);

        visit(node->keys[i], node->postings[i], ctx);
    }

    if (!node->is_leaf) {
        traverseNode(node->children[node->n], visit, ctx);
    }
}

void    btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (tree == NULL || tree->root == NULL) {
        printf("Tree is NULL or root is NULL\n");
        return;
    }

    traverseNode(tree->root, visit, ctx);
}
