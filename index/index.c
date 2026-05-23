#include "index.h"
#include "../avl/avl.h"
#include "../btree/btree.h"
#include "../rbtree/rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_LINE_LEN 65536
#define MAX_TOKENS 4096
#define MAX_TOKEN_LEN 256


Index* createIndex(TreeType type)
{
    Index* idx = (Index*)malloc(sizeof(Index));

    if (!idx) {
        printf("Problem in 'avlSearch'\n"
               "-> Memory for Index wasn't allocated.\n");
        return NULL;
    }

    idx->type = type;
    
    if (type == TREE_AVL) 
        idx->tree = createAVLTree();

    else if (type == TREE_RB) 
        idx->tree = createRBTree();

    else if (type == TREE_BTREE)
        idx->tree = createBTree();

    return idx;
}


void insertTerm(Index* idx, const char* term, int doc_id, const char* title)
{
    if (!idx) {
        printf("Problem in 'insertTerm'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    if (idx->type == TREE_AVL) 
        avlInsert(idx->tree, term, doc_id, title);

    else if (idx->type == TREE_RB) 
        rbInsert(idx->tree, term, doc_id, title);

    else if (idx->type == TREE_BTREE)
        btreeInsert(idx->tree, term, doc_id, title);

    return;
}


Vector*  lookupTerm(const Index* idx, const char* term)
{
    if (!idx) {
        printf("Problem in 'lookupTerm'\n"
               "-> Given Index is NULL.\n");
        return NULL;
    }

    if (idx->type == TREE_AVL) 
        return avlSearch(idx->tree, term);

    else if (idx->type == TREE_RB) 
        return rbSearch(idx->tree, term);

    else if (idx->type == TREE_BTREE)
        return btreeSearch(idx->tree, term);
}


void indexDocument(Index* idx, int doc_id, const char* title,
                       const char** tokens, int n_tokens)
{
    if (!idx) {
        printf("Problem in 'indexDocument'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    for (int i = 0; i < n_tokens; i++) {
        const char* term = tokens[i];
        insertTerm(idx, term, doc_id, title);
    }

    return;
}


void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx)
{
    if (!idx) {
        printf("Problem in 'traverseIndex'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    if (idx->type == TREE_AVL)
        avlTraverse(idx->tree, visit, ctx);

    else if (idx->type == TREE_RB)
        rbTraverse(idx->tree, visit, ctx);

    else if (idx->type == TREE_BTREE)
        btreeTraverse(idx->tree, visit, ctx);

    return;
}


void freeIndex(Index* idx)
{
    if (!idx) {
        printf("Problem in 'freeIndex'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    if (idx->type == TREE_AVL)
        freeAVLTree(idx->tree);

    else if (idx->type == TREE_RB)
        freeRBTree(idx->tree);

    else if (idx->type == TREE_BTREE)
        freeBTree(idx->tree);

    free(idx);
    return;
}


static void saveNode(const char* key, Vector* postings, void* ctx)
{
    FILE* file = (FILE*)ctx;

    int key_len = strlen(key) + 1;
    int vector_len = postings->size;

    fwrite(&key_len, sizeof(int), 1, file);
    fwrite(&key, sizeof(char), key_len, file);

    fwrite(&vector_len, sizeof(int), 1, file);

    for (int i = 0; i < vector_len; i++) {
        PostingEntry* entry = getVectorItem(postings, i);
        fwrite(entry, sizeof(PostingEntry), 1, file);
    }
}


void saveIndex(const Index* idx, const char* path)
{
    if (!idx) {
        printf("Problem in 'saveIndex'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    FILE* file = fopen(path, "wb");

    if (!file) {
        printf("Problem in 'saveIndex'\n"
               "-> File not found.\n");
        return;
    }

    int total_words;
    
    if (idx->type == TREE_AVL)
        total_words = ((AVLTree*)idx->tree)->size;

    else if (idx->type == TREE_RB)
        total_words = ((RBTree*)idx->tree)->size;

    else if (idx->type == TREE_BTREE)
        total_words = ((BTree*)idx->tree)->size;

    fwrite(&(idx->type), sizeof(TreeType), 1, file);
    fwrite(&total_words, sizeof(int), 1, file);

    traverseIndex(idx, saveNode, file);

    fclose(file);
    return;
}


Index* loadIndex(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (!file) {
        printf("Problem in 'saveIndex'\n"
               "-> File not found.\n");
        return NULL;
    }

    TreeType type;
    int flag = fread(&type, sizeof(TreeType), 1, file);
    if (flag != 1) {
        printf("Problem in 'loadIndex'\n"
               "-> Reader error.\n");
        fclose(file);
        return NULL;
    }

    Index* idx = createIndex(type);

    int total_words;
    int flag = fread(&total_words, sizeof(int), 1, file);
    if (flag != 1) {
        printf("Problem in 'loadIndex'\n"
               "-> Reader error.\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < total_words; i++) 
    {
        int key_len;
        int flag = fread(&key_len, sizeof(int), 1, file);
        if (flag != 1) {
            printf("Problem in 'loadIndex'\n"
                   "-> Reader error.\n");
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        char* key = (char*)malloc(key_len * sizeof(char));
        int flag = fread(&key, sizeof(char), key_len, file);
        if (flag != 1) {
            printf("Problem in 'loadIndex'\n"
                   "-> Reader error.\n");
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        int vector_len;
        int flag = fread(&vector_len, sizeof(int), 1, file);
        if (flag != 1) {
            printf("Problem in 'loadIndex'\n"
                   "-> Reader error.\n");
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        for (int j = 0; j < vector_len; j++) 
        {
            PostingEntry entry;
            int flag = fread(&entry, sizeof(PostingEntry), 1, file);
            if (flag != 1) {
                printf("Problem in 'loadIndex'\n"
                       "-> Reader error.\n");
                fclose(file);
                freeIndex(idx);
                return NULL;
            }

            insertTerm(idx, key, entry.doc_id, entry.title);
        }

        free(key);
    }

    fclose(file);
    return idx;
}


// Вспомогательная функция для парсинга одной JSON-строки
static int parse_json_line(char* line, int* doc_id, char* title, char tokens[MAX_TOKENS][MAX_TOKEN_LEN], int* n_tokens) {
    *n_tokens = 0;
    
    char* id_ptr = strstr(line, "\"doc_id\":");
    if (!id_ptr) return 0;
    id_ptr += 9;
    while (*id_ptr == ' ' || *id_ptr == ':' || *id_ptr == '"') id_ptr++;
    *doc_id = atoi(id_ptr);

    char* title_ptr = strstr(line, "\"title\":");
    if (!title_ptr) return 0;
    title_ptr += 8;
    while (*title_ptr == ' ' || *title_ptr == ':' || *title_ptr == '"') title_ptr++;
    
    int t_idx = 0;
    while (*title_ptr != '"' && *title_ptr != '\0' && t_idx < MAX_TITLE_LEN - 1) {
        title[t_idx++] = *title_ptr++;
    }
    title[t_idx] = '\0';

    char* tokens_ptr = strstr(line, "\"tokens\":");
    if (!tokens_ptr) return 0;
    tokens_ptr += 9;
    
    while (*tokens_ptr != '[' && *tokens_ptr != '\0') tokens_ptr++;
    if (*tokens_ptr == '[') tokens_ptr++;

    while (*tokens_ptr != ']' && *tokens_ptr != '\0' && *n_tokens < MAX_TOKENS) {
        if (*tokens_ptr == '"') {
            tokens_ptr++;
            int tok_len = 0;
            while (*tokens_ptr != '"' && *tokens_ptr != '\0' && tok_len < MAX_TOKEN_LEN - 1) {
                tokens[*n_tokens][tok_len++] = *tokens_ptr++;
            }
            tokens[*n_tokens][tok_len] = '\0';
            (*n_tokens)++;
        }
        tokens_ptr++;
    }

    return (*n_tokens > 0);
}

void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    printf("[Index Mode] Initializing tree context...\n");

    Index* idx = createIndex(type);
    if (!idx) {
        fprintf(stderr, "Fatal error: Cannot allocate Index structure.\n");
        return;
    }

    FILE* file = fopen(data_path, "r");
    if (!file) {
        perror("Fatal error: Cannot open raw data file");
        freeIndex(idx);
        return;
    }

    char* line = (char*)malloc(MAX_LINE_LEN);
    if (!line) {
        fprintf(stderr, "Fatal error: Out of memory for line buffer.\n");
        fclose(file);
        freeIndex(idx);
        return;
    }

    char tokens_storage[MAX_TOKENS][MAX_TOKEN_LEN];
    const char* tokens_ptrs[MAX_TOKENS];

    int indexed_docs = 0;

    while (fgets(line, MAX_LINE_LEN, file)) {
        int doc_id = 0;
        char title[MAX_TITLE_LEN] = {0};
        int n_tokens = 0;

        if (!parse_json_line(line, &doc_id, title, tokens_storage, &n_tokens)) {
            continue;
        }

        for (int i = 0; i < n_tokens; i++) {
            tokens_ptrs[i] = tokens_storage[i];
        }

        indexDocument(idx, doc_id, title, tokens_ptrs, n_tokens);
        indexed_docs++;

        if (indexed_docs % 100 == 0) {
            printf("Indexed %d documents...\n", indexed_docs);
        }
    }

    printf("Indexing finished. Total successfully processed documents: %d\n", indexed_docs);

    saveIndex(idx, idx_path);

    free(line);
    fclose(file);
    freeIndex(idx);
}
