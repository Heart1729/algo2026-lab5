#include "avl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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

AVLTree* createAVLTree(void)
{
    AVLTree* avl_tree = malloc(sizeof(AVLTree));
    if (avl_tree == NULL) {
        printf("Не удалось выделить память под дерево");
        return NULL;
    }

    avl_tree->root = NULL;
    avl_tree->size = 0;

    return avl_tree;
}

AVLNode* createAVLNode(const char* key, int doc_id, const char* title)
{
    if (key == NULL) {
        return NULL;
    }

    AVLNode* node = malloc(sizeof(AVLNode));
    if (node == NULL) {
        printf("Не удалось выделить память под узел");
        return NULL;
    }

    node->key = malloc(strlen(key) + 1);
    if (node->key == NULL) {
        free(node);
        printf("Не удалось выделить память под ключ");
        return NULL;
    }
    strcpy(node->key, key);
    
    node->height = 1;
    node->postings = createPostingList();
    if (node->postings == NULL) {
        free(node->key);
        free(node);
        printf("Не удалось выделить память под Posting list");
        return NULL;
    }

    node->left = NULL;
    node->right = NULL;
    appendPosting(node->postings, doc_id, title);

    return node;
}

static void freeNode(AVLNode* node)
{
    if (node == NULL) {
        return;
    }
    
    free(node->key);
    if (node->postings != NULL) {
        vectorFree(node->postings);
    }

    freeNode(node->left);
    freeNode(node->right);

    free(node);
}

void freeAVLTree(AVLTree* tree)
{
    if (tree == NULL) {
        return;
    }

    freeNode(tree->root);
    free(tree);
}

static int height(AVLNode* node)
{
    if (node == NULL) {
        return 0;
    }

    return node->height;
}

static int balanceNode(AVLNode* node)
{
    if (node == NULL) {
        return 0;
    }

    return height(node->left) - height(node->right);
}

static int max(int a, int b)
{
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

static AVLNode* rotateLeft(AVLNode* a)
{
    AVLNode* b = a->right;
    a->right = b->left;
    b->left = a;
    
    a->height = max(height(a->left), height(a->right)) + 1;
    b->height = max(height(b->left), height(b->right)) + 1;

    return b;
}

static AVLNode* rotateRight(AVLNode* a)
{
    AVLNode* b = a->left;
    a->left = b->right;
    b->right = a;
    
    a->height = max(height(a->left), height(a->right)) + 1;
    b->height = max(height(b->left), height(b->right)) + 1;

    return b;
}

static AVLNode* bigRotateLeft(AVLNode* node)
{
    node->right = rotateRight(node->right);
    return rotateLeft(node);
}

static AVLNode* bigRotateRight(AVLNode* node)
{
    node->left = rotateLeft(node->left);
    return rotateRight(node);
}

static AVLNode* insertNode(AVLNode* node, const char* key, int doc_id, const char* title, int* created)
{
    if (node == NULL) {
        *created = 1;
        return createAVLNode(key, doc_id, title);
    }
    
    int cmp = strcmp(key, node->key);
    
    if (cmp < 0) {
        node->left = insertNode(node->left, key, doc_id, title, created);
    } else if (cmp > 0) {
        node->right = insertNode(node->right, key, doc_id, title, created);
    }
    
    node->height = 1 + max(height(node->left), height(node->right));
    int balance = balanceNode(node);
    
    if (balance < -1 && balanceNode(node->right) <= 0) {
        return rotateLeft(node);
    }

    if (balance > 1 && balanceNode(node->left) >= 0) {
        return rotateRight(node);
    }

    if (balance < -1 && balanceNode(node->right) == 1) {
        return bigRotateLeft(node);
    }

    if (balance > 1 && balanceNode(node->left) == -1) {
        return bigRotateRight(node);
    }
    
    return node;
}

void avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title)
{
    if (tree == NULL || key == NULL || title == NULL) {
        return;
    }
    
    Vector* postings = avlSearch(tree, key);
    
    if (postings != NULL) {
        for (size_t i = 0; i < postings->size; i++) {
            PostingEntry* entry = getVectorItem(postings, i);
            if (entry && entry->doc_id == doc_id) {
                strncpy(entry->title, title, MAX_TITLE_LEN - 1);
                entry->title[MAX_TITLE_LEN - 1] = '\0';
                return;
            }
        }
        appendPosting(postings, doc_id, title);
        return;
    }

    int created = 0;
    tree->root = insertNode(tree->root, key, doc_id, title, &created);
    
    if (created) {
        tree->size++;
    }
}

static void traverseNode(AVLNode* node, void (*visit)(const char* key, Vector* postings, void* ctx), void* ctx)
{
    if (node == NULL) {
        return;
    }
    
    traverseNode(node->left, visit, ctx);
    visit(node->key, node->postings, ctx);
    traverseNode(node->right, visit, ctx);
}

void avlTraverse(const AVLTree* tree, 
                 void (*visit)(const char* key, Vector* postings, void* ctx),
                 void* ctx)
{
    if (tree == NULL || visit == NULL) {
        return;
    }

    traverseNode(tree->root, visit, ctx);
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

