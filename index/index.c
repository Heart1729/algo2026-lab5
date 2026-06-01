#include "index.h"
#include "../avl/avl.h"
#include "../btree/btree.h"
#include "../rbtree/rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cJSON.h" 
#include "search.h"

#define MAX_LINE_LEN 65536
#define MAX_TOKENS 4096
#define MAX_TOKEN_LEN 256

#ifndef MAX_TITLE_LEN
#define MAX_TITLE_LEN 256
#endif

Index* createIndex(TreeType type)
{
    Index* idx = (Index*)malloc(sizeof(Index));

    if (!idx) {
        printf("Problem in 'createIndex'\n"
               "-> Memory for Index wasn't allocated.\n");
        return NULL;
    }

    idx->type = type;
    idx->names = createHashTable(sizeof(int), MAX_TITLE_LEN * sizeof(char));
    
    if (type == TREE_AVL) 
        idx->tree = createAVLTree();

    else if (type == TREE_RB) 
        idx->tree = createRBTree();

    else if (type == TREE_BTREE)
        idx->tree = createBTree();

    return idx;
}


void insertTerm(Index* idx, const char* term, int doc_id)
{
    if (!idx) {
        printf("Problem in 'insertTerm'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    if (idx->type == TREE_AVL) 
        avlInsert(idx->tree, term, doc_id);

    else if (idx->type == TREE_RB) 
        rbInsert(idx->tree, term, doc_id);

    else if (idx->type == TREE_BTREE)
        btreeInsert(idx->tree, term, doc_id);

    return;
}


Vector* lookupTerm(const Index* idx, const char* term)
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

    return NULL;
}


void indexDocument(Index* idx, int doc_id, const char* title,
                       const char** tokens, int n_tokens)
{
    if (!idx) {
        printf("Problem in 'indexDocument'\n"
               "-> Given Index is NULL.\n");
        return;
    }

    setItemHashTable(idx->names, &doc_id, (void*)title, HashInt, intEquals);

    for (int i = 0; i < n_tokens; i++) {
        const char* term = tokens[i];
        insertTerm(idx, term, doc_id);
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
    fwrite(key, sizeof(char), key_len, file);

    fwrite(&vector_len, sizeof(int), 1, file);

    for (int i = 0; i < vector_len; i++) {
        PostingEntry* entry = getVectorItem(postings, i);
        fwrite(entry, sizeof(PostingEntry), 1, file);
    }
}


void saveHashTableText(const HashTable* html, const char* path) {
    if (!html) return;

    FILE* file = fopen(path, "w");
    if (!file) {
        perror("Failed to open hash table file for text writing");
        return;
    }
    
    fprintf(file, "%zu\n", html->size);

    for (size_t i = 0; i < html->capacity; i++) {
        void* slot = getVectorItem(html->values, i);
        if (slot == NULL) continue;

        int* doc_id_ptr = (int*)slot;
        char* title_ptr = (char*)((char*)slot + html->key_size);

        if (*doc_id_ptr == 0 && strlen(title_ptr) == 0) {
            continue;
        }

        fprintf(file, "%d %s\n", *doc_id_ptr, title_ptr);
    }

    fflush(file);
    fclose(file);
}


void saveIndex(const Index* idx, const char* path)
{
    if (!idx) {
        printf("Problem in 'saveIndex'\n-> Given Index is NULL.\n");
        return;
    }

    FILE* file = fopen(path, "wb");
    if (!file) {
        printf("Problem in 'saveIndex'\n-> File not found.\n");
        return;
    }

    fwrite(&(idx->type), sizeof(TreeType), 1, file);

    int actual_docs = 0;
    if (idx->names != NULL) {
        for (size_t i = 0; i < idx->names->capacity; i++) {
            unsigned char* slot = (unsigned char*)getVectorItem(idx->names->values, i);
            if (slot == NULL) continue;

            if (slot[0] != SLOT_OCCUPIED) {
                continue;
            }
            actual_docs++;
        }
    }

    fwrite(&actual_docs, sizeof(int), 1, file);

    if (actual_docs > 0 && idx->names != NULL) {
        for (size_t i = 0; i < idx->names->capacity; i++) {
            unsigned char* slot = (unsigned char*)getVectorItem(idx->names->values, i);
            if (slot == NULL) continue;

            if (slot[0] != 1) { 
                continue;
            }

            int* doc_id_ptr = (int*)(slot + 1);
            char* title_ptr = (char*)(slot + 1 + idx->names->key_size);

            fwrite(doc_id_ptr, sizeof(int), 1, file);
            fwrite(title_ptr, sizeof(char), MAX_TITLE_LEN, file);
        }
    }

    int total_words = 0;
    if (idx->tree != NULL) {
        if (idx->type == TREE_AVL)
            total_words = ((AVLTree*)idx->tree)->size;
        else if (idx->type == TREE_RB)
            total_words = ((RBTree*)idx->tree)->size;
        else if (idx->type == TREE_BTREE)
            total_words = ((BTree*)idx->tree)->size;
    }

    fwrite(&total_words, sizeof(int), 1, file);

    traverseIndex(idx, saveNode, file);

    fflush(file);
    fclose(file);
}


HashTable* loadHashTableText(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("Failed to open hash table file for text reading");
        return NULL;
    }

    size_t total_elements = 0;
    if (fscanf(file, "%zu\n", &total_elements) != 1) {
        printf("Error reading HashTable text header.\n");
        fclose(file);
        return NULL;
    }

    HashTable* html = createHashTable(sizeof(int), sizeof(char) * MAX_TITLE_LEN);
    if (!html) {
        fclose(file);
        return NULL;
    }

    int temp_id;
    char temp_title[MAX_TITLE_LEN];

    for (size_t i = 0; i < total_elements; i++) {
        if (fscanf(file, "%d ", &temp_id) != 1) {
            break; 
        }

        if (fgets(temp_title, sizeof(temp_title), file) == NULL) {
            break;
        }

        size_t len = strlen(temp_title);
        if (len > 0 && temp_title[len - 1] == '\n') {
            temp_title[len - 1] = '\0';
        }

        setItemHashTable(html, &temp_id, temp_title, HashInt, intEquals);
    }

    fclose(file);
    return html;
}


Index* loadIndex(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Problem in 'loadIndex'\n-> File not found: %s\n", path);
        return NULL;
    }

    // 1. Читаем тип дерева
    TreeType type;
    if (fread(&type, sizeof(TreeType), 1, file) != 1) {
        printf("Problem in 'loadIndex'\n-> Reader error (TreeType).\n");
        fclose(file);
        return NULL;
    }

    // 2. Инициализируем индекс нужного типа
    Index* idx = createIndex(type);
    if (!idx) {
        fclose(file);
        return NULL;
    }

    // 3. Читаем количество сохраненных документов
    int total_docs;
    if (fread(&total_docs, sizeof(int), 1, file) != 1) {
        printf("Problem in 'loadIndex'\n-> Reader error (total_docs).\n");
        fclose(file);
        freeIndex(idx);
        return NULL;
    }

    // 4. Читаем документы и заполняем хэш-таблицу
    int temp_id;
    char temp_title[MAX_TITLE_LEN];
    for (int i = 0; i < total_docs; i++) {
        if (fread(&temp_id, sizeof(int), 1, file) != 1 ||
            fread(temp_title, sizeof(char), MAX_TITLE_LEN, file) != MAX_TITLE_LEN) {
            printf("Problem in 'loadIndex'\n-> Reader error (HashTable elements at index %d).\n", i);
            fclose(file);
            freeIndex(idx);
            return NULL;
        }
        setItemHashTable(idx->names, &temp_id, temp_title, HashInt, intEquals);
    }

    // 5. Читаем общее количество слов в дереве
    int total_words;
    if (fread(&total_words, sizeof(int), 1, file) != 1) {
        printf("Problem in 'loadIndex'\n-> Reader error (total_words).\n");
        fclose(file);
        freeIndex(idx);
        return NULL;
    }

   // 6. Восстанавливаем слова и постинг-листы в дереве
    for (int i = 0; i < total_words; i++) 
    {
        int key_len;
        if (fread(&key_len, sizeof(int), 1, file) != 1) {
            printf("Problem in 'loadIndex'\n-> Reader error (key_len at index %d).\n", i);
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        char* key = (char*)malloc((key_len + 1) * sizeof(char));
        if (!key) {
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        if (fread(key, sizeof(char), key_len, file) != key_len) {
            printf("Problem in 'loadIndex'\n-> Reader error (key string).\n");
            free(key);
            fclose(file);
            freeIndex(idx);
            return NULL;
        }
        key[key_len] = '\0';

        int vector_len;
        if (fread(&vector_len, sizeof(int), 1, file) != 1) {
            printf("Problem in 'loadIndex'\n-> Reader error (vector_len).\n");
            free(key);
            fclose(file);
            freeIndex(idx);
            return NULL;
        }

        if (vector_len > 0) {
            PostingEntry entry;
            if (fread(&entry, sizeof(PostingEntry), 1, file) != 1) {
                printf("Problem in 'loadIndex'\n-> Reader error (posting entry).\n");
                free(key); fclose(file); freeIndex(idx); return NULL;
            }
            
            insertTerm(idx, key, entry.doc_id);
            
            Vector* postings = lookupTerm(idx, key);
            
            for (int j = 1; j < vector_len; j++) {
                if (fread(&entry, sizeof(PostingEntry), 1, file) != 1) {
                    printf("Problem in 'loadIndex'\n-> Reader error (posting entry).\n");
                    free(key); fclose(file); freeIndex(idx); return NULL;
                }
                if (postings != NULL) {
                    appendVectorItem(postings, &entry);
                }
            }
        }

        free(key);
    }

    fclose(file);
    return idx;
}


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

        if (indexed_docs % 100000 == 0) {
            printf("Indexed %d documents...\n", indexed_docs);
        }
    }

    printf("Indexing finished. Total successfully processed documents: %d\n", indexed_docs);

    saveIndex(idx, idx_path);

    free(line);
    fclose(file);
}


void runSearch(TreeType type, const char* idx_path, const char* query, int json_out) {
    Index* idx = loadIndex(idx_path);
    if (!idx) { 
        fprintf(stderr, "Fatal error: Failed to load index from path: %s\n", idx_path); 
        exit(1); 
    }

    if (idx->type != type) {
        fprintf(stderr, "Warning: Loaded index type mismatch. Requested: %d, Found: %d\n", type, idx->type);
    }

    SearchResults* sr = search(idx, query);
    if (!sr) {
        fprintf(stderr, "Search failed or returned no memory allocation.\n");
        freeIndex(idx);
        return;
    }

    if (json_out) {
        printResultsJSON(sr);
    } else {
        printResultsText(sr);
    }

    freeSearchResults(sr);
    freeIndex(idx);
}
