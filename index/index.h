#pragma once

#include "../posting.h"
#include "../hash_table/generic.h"

typedef enum { TREE_AVL, TREE_RB, TREE_BTREE } TreeType;

typedef struct {
    void*    tree;
    TreeType type;
    HashTable* names; // doc_id -> title
} Index;

Index*   createIndex(TreeType type);
void     insertTerm(Index* idx, const char* term, int doc_id);
Vector*  lookupTerm(const Index* idx, const char* term);
void     indexDocument(Index* idx, int doc_id, const char* title,
                       const char** tokens, int n_tokens);
void     traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);
void     saveIndex(const Index* idx, const char* path);
Index*   loadIndex(const char* path);
void     freeIndex(Index* idx);
void     runIndex(TreeType type, const char* data_path, const char* idx_path);
void     runSearch(TreeType type, const char* idx_path, const char* query, int json_out);
