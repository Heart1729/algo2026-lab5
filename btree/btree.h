#pragma once

#include "../posting.h"
#include "../levenshtein/levenshtein.h"

#define BTREE_T        3
#define BTREE_MAX_KEYS (2 * BTREE_T - 1)
#define BTREE_MAX_CH   (2 * BTREE_T)

typedef struct BTreeNode {
    char*             keys[BTREE_MAX_KEYS];
    Vector*           postings[BTREE_MAX_KEYS];
    struct BTreeNode* children[BTREE_MAX_CH];
    int               n;
    int               is_leaf;
} BTreeNode;

typedef struct {
    char    term[256];
    int     distance;
    Vector* postings;
} BTreeFuzzyCandidate;

typedef struct {
    BTreeNode* root;
    int        size;
} BTree;

BTree*  createBTree(void);
void    freeBTree(BTree* tree);
void   freeBTreeNode(BTreeNode* node);

BTreeNode *createBTreeNode(const char* keys, Vector *postings, int is_leaf);
                            
void    btreeInsert(BTree* tree, const char* key, int doc_id);
Vector* btreeSearch(const BTree *tree, const char* key);
void    btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);

Vector* btreeFuzzyFindCandidates(const BTree* tree, const char* term, int max_distance);
void freeFuzzyCandidates(Vector* candidates);
