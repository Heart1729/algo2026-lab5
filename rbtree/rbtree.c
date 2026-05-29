#define _GNU_SOURCE
#include "rbtree.h"
#include "../vector/generic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../posting.h"
#include <stdbool.h>

RBTree* createRBTree() {

    RBTree* tree = malloc(sizeof(RBTree));
    if (tree == NULL) {
        printf("Не удалось выделить память под дерево");
        return NULL;
    }

    RBNode* sentinel = malloc(sizeof(RBNode));
    if (sentinel == NULL) {
        free(tree);
        printf("Не удалось выделить память под узел-страж");
        return NULL;
    }

    sentinel->color = RB_BLACK;
    sentinel->key = NULL;
    sentinel->left = sentinel;
    sentinel->right = sentinel;
    sentinel->parent = sentinel;
    sentinel->postings = NULL;

    tree->nil = sentinel;
    tree->size = 0;
    tree->root = sentinel;

    return tree;
}

static void freeRBNodeRecursive(RBTree* tree, RBNode* node) {

    if (node->left != tree->nil) {
        freeRBNodeRecursive(tree, node->left);
    }

    if (node->right != tree->nil) {
        freeRBNodeRecursive(tree, node->right);
    }

    vectorFree(node->postings);
    free(node->key);
    free(node);
}

void freeRBTree(RBTree* tree) {
    if (tree->root != tree->nil) {
        freeRBNodeRecursive(tree, tree->root);
    }

    free(tree->nil);
    free(tree);
}

static void left_rotation(RBTree* tree, RBNode* node) {

    RBNode* parent = node->parent;
    RBNode* right_child = node->right;
    RBNode* left_grandchild = right_child->left;

    node->right = left_grandchild;
    if (left_grandchild != tree->nil) {
        left_grandchild->parent = node;
    }

    right_child->left = node;
    node->parent = right_child;

    right_child->parent = parent;

    if (parent == tree->nil) {
        tree->root = right_child;
    }
    else if (node == parent->left) {
        parent->left = right_child;
    }
    else {
        parent->right = right_child;
    }
}

static void right_rotation(RBTree* tree, RBNode* node) {

    RBNode* parent = node->parent;
    RBNode* left_child = node->left;
    RBNode* right_grandchild = left_child->right;

    node->left = right_grandchild;
    if (right_grandchild != tree->nil) {
        right_grandchild->parent = node;
    }

    left_child->right = node;
    node->parent = left_child;

    left_child->parent = parent;

    if (parent == tree->nil) {
        tree->root = left_child;
    }
    else if (parent->left == node) {
        parent->left = left_child;
    }
    else {
        parent->right = left_child;
    }
}

static void RBInsertFixup(RBTree* tree, RBNode* node) {

    while (node->parent->color == RB_RED) {

        RBNode* parent = node->parent;
        RBNode* grandfather = parent->parent;
        RBNode* uncle = NULL;

        if (grandfather->left == parent) {
            uncle = grandfather->right;
        }
        else {
            uncle = grandfather->left;
        }

        if (uncle->color == RB_RED) {
            parent->color = RB_BLACK;
            uncle->color = RB_BLACK;
            grandfather->color = RB_RED;
            node = grandfather;
        }
        else {
            bool LL = false;
            bool LR = false;
            bool RR = false;
            bool RL = false;

            if (grandfather->left == parent && parent->left == node) {
                LL = true;
            }
            else if (grandfather->left == parent && parent->right == node) {
                LR = true;
            }
            else if (grandfather->right == parent && parent->right == node) {
                RR = true;
            }
            else {
                RL = true;
            }

            if (LL) {
                right_rotation(tree, grandfather);
                parent->color = RB_BLACK;
                grandfather->color = RB_RED;
            }
            else if (LR) {
                left_rotation(tree, parent);
                right_rotation(tree, grandfather);
                node->color = RB_BLACK;
                grandfather->color = RB_RED;
            }
            else if (RR) {
                left_rotation(tree, grandfather);
                parent->color = RB_BLACK;
                grandfather->color = RB_RED;
            }
            else if (RL) {
                right_rotation(tree, parent);
                left_rotation(tree, grandfather);
                node->color = RB_BLACK;
                grandfather->color = RB_RED;
            }
            break;
        }
    }

    tree->root->color = RB_BLACK;
}

void rbInsert(RBTree* tree, const char* key, int doc_id) {

    Vector* found = rbSearch(tree, key);
    if (found != NULL) {
        if (found->size > 0) {
            PostingEntry* last = getVectorItem(found, found->size - 1);
            if (last->doc_id == doc_id) {
                return;
            }
        }
        appendPosting(found, doc_id);
        return;
    }

    RBNode* new_node = malloc(sizeof(RBNode));
    if (new_node == NULL) {
        printf("Не удалось выделить память под узел\n");
        return;
    }

    new_node->color = RB_RED;
    new_node->left = tree->nil;
    new_node->right = tree->nil;

    char* new_key = strdup(key);
    if (new_key == NULL) {
        free(new_node);
        printf("Не удалось выделить память под ключ\n");
        return;
    }

    new_node->key = new_key;

    Vector* posting_list = createPostingList();
    if (posting_list == NULL) {
        free(new_key);
        free(new_node);
        return;
    }

    appendPosting(posting_list, doc_id);
    new_node->postings = posting_list;


    RBNode* node_now = tree->root;
    RBNode* future_parent = tree->nil;
    while (node_now != tree->nil) {

        future_parent = node_now;

        if (strcmp(new_node->key, node_now->key) > 0) {
            node_now = node_now->right;
        }
        else {
            node_now = node_now->left;
        }
    }

    new_node->parent = future_parent;
    tree->size++;

    if (future_parent == tree->nil) {
        new_node->color = RB_BLACK;
        tree->root = new_node;
        return;
    }

    if (strcmp(new_node->key, future_parent->key) > 0) {
        future_parent->right = new_node;
    }
    else {
        future_parent->left = new_node;
    }

    RBInsertFixup(tree, new_node);
}

static void rbTraverseRecursive(const RBTree* tree,
    const RBNode* node_now,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
    ) {

    if (node_now->left != tree->nil) {
        rbTraverseRecursive(tree, node_now->left, visit, ctx);
    }

    visit(node_now->key, node_now->postings, ctx);

    if (node_now->right != tree->nil) {
        rbTraverseRecursive(tree, node_now->right, visit, ctx);
    }
}

void rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
    ) {

    if (tree->root == tree->nil) {
        printf("Дерево пустое\n");
    }
    else {
        rbTraverseRecursive(tree, tree->root, visit, ctx);
    }
}

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
