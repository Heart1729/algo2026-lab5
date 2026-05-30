// Тут все примерно, можете подстроить под себя, главное, чтобы интерфейс был таким же, как в avl и btree
#pragma once

#include "../posting.h"
#include "../levenshtein/levenshtein.h"

typedef enum { RB_RED, RB_BLACK } RBColor;

typedef struct RBNode {
    char*           key;
    RBColor         color;
    Vector*         postings;
    struct RBNode*  left;
    struct RBNode*  right;
    struct RBNode*  parent;
} RBNode;

typedef struct {
    char    term[256];
    int     distance;
    Vector* postings;
} RBFuzzyCandidate;

typedef struct {
    RBNode* root;
    RBNode* nil;   /* sentinel-узел (чёрный лист) */
    int     size;
} RBTree;

RBTree* createRBTree(void);
void    freeRBTree(RBTree* tree);

void    rbInsert(RBTree* tree, const char* key, int doc_id);
Vector* rbSearch(const RBTree* tree, const char* key);
void    rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);

Vector* rbFuzzyFindCandidates(const RBTree* tree, const char* term, int max_distance);
void freeRBFuzzyCandidates(Vector* candidates);
