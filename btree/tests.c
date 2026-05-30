#include "btree.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../posting.h"
#include "../vector/generic.h"

void testCreateBTree(void) {
    printf("TestCreateBTree...\n");
    BTree *tree = createBTree();
    assert(tree != NULL);
    assert(tree->root == NULL);
    assert(tree->size == 0);
    freeBTree(tree);
    printf("TestCreateBTree success!\n");
}

void testCreateBTreeNode(void) {
    printf("TestCreateBTreeNode...\n");
    Vector *postings = createPostingList();
    // ИСПРАВЛЕНО: удалили текстовый заголовок
    appendPosting(postings, 1);
    BTreeNode *node = createBTreeNode("кот", postings, 1);
    assert(node != NULL);
    assert(strcmp(node->keys[0], "кот") == 0);
    assert(node->is_leaf == 1);
    assert(node->n == 1);
    freeBTreeNode(node);
    printf("TestCreateBTreeNode success!\n");
}

void testInsertSingle(void) {
    printf("TestInsertSingle...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовый заголовок
    btreeInsert(tree, "кот", 1);
    assert(tree->root != NULL);
    assert(tree->size == 1);
    assert(strcmp(tree->root->keys[0], "кот") == 0);
    freeBTree(tree);
    printf("TestInsertSingle success!\n");
}

void testInsertMultiple(void) {
    printf("TestInsertMultiple...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "кот", 1);
    btreeInsert(tree, "бэкап", 2);
    btreeInsert(tree, "собака", 3);
    assert(tree->size == 3);
    freeBTree(tree);
    printf("TestInsertMultiple success!\n");
}

void testSearchExisting(void) {
    printf("TestSearchExisting...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "кот", 1);
    btreeInsert(tree, "бэкап", 2);
    Vector *result = btreeSearch(tree, "кот");
    assert(result != NULL);
    PostingEntry *e = (PostingEntry*)getVectorItem(result, 0);
    assert(e != NULL);
    assert(e->doc_id == 1); // Проверка только по doc_id
    freeBTree(tree);
    printf("TestSearchExisting success!\n");
}

void testSearchNotExisting(void) {
    printf("TestSearchNotExisting...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовый заголовок
    btreeInsert(tree, "кот", 1);
    Vector *result = btreeSearch(tree, "тигр");
    assert(result == NULL);
    freeBTree(tree);
    printf("TestSearchNotExisting success!\n");
}

void testInsertSplit(void) {
    printf("TestInsertSplit...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "бобёр", 1);
    btreeInsert(tree, "кот", 2);
    btreeInsert(tree, "лиса", 3);
    btreeInsert(tree, "медведь", 4);
    btreeInsert(tree, "собака", 5);
    btreeInsert(tree, "тигр", 6);
    assert(tree->root->is_leaf == 0);
    assert(tree->size == 6);
    freeBTree(tree);
    printf("TestInsertSplit success!\n");
}

void testInsertOrder(void) {
    printf("TestInsertOrder...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ббб", 1);
    btreeInsert(tree, "ааа", 2);
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ббб") == 0);
    freeBTree(tree);
    printf("TestInsertOrder success!\n");
}

void testInsertOrderReverse(void) {
    printf("TestInsertOrderReverse...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ггг", 1);
    btreeInsert(tree, "ввв", 2);
    btreeInsert(tree, "ааа", 3);
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ввв") == 0);
    assert(strcmp(tree->root->keys[2], "ггг") == 0);
    freeBTree(tree);
    printf("TestInsertOrderReverse success!\n");
}

void testInsertOrderMixed(void) {
    printf("TestInsertOrderMixed...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ввв", 1);
    btreeInsert(tree, "ааа", 2);
    btreeInsert(tree, "ббб", 3);
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ббб") == 0);
    assert(strcmp(tree->root->keys[2], "ввв") == 0);
    freeBTree(tree);
    printf("TestInsertOrderMixed success!\n");
}

void testDuplicateInsert(void) {
    printf("TestDuplicateInsert...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "кот", 1);
    btreeInsert(tree, "кот", 2);
    assert(tree->size == 1);
    Vector *result = btreeSearch(tree, "кот");
    assert(result != NULL);
    assert(result->size == 2);
    PostingEntry *e0 = (PostingEntry*)getVectorItem(result, 0);
    PostingEntry *e1 = (PostingEntry*)getVectorItem(result, 1);
    assert(e0->doc_id == 1); // ИСПРАВЛЕНО: проверка по doc_id
    assert(e1->doc_id == 2); // ИСПРАВЛЕНО: проверка по doc_id
    freeBTree(tree);
    printf("TestDuplicateInsert success!\n");
}

void testDoubleSplit(void) {
    printf("TestDoubleSplit...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ааа", 1);
    btreeInsert(tree, "ббб", 2);
    btreeInsert(tree, "ввв", 3);
    btreeInsert(tree, "ггг", 4);
    btreeInsert(tree, "ддд", 5);
    btreeInsert(tree, "еее", 6);
    btreeInsert(tree, "жжж", 7);
    btreeInsert(tree, "ззз", 8);
    btreeInsert(tree, "ийй", 9);
    btreeInsert(tree, "ккк", 10);
    btreeInsert(tree, "ллл", 11);
    
    assert(tree->root->is_leaf == 0);
    const char *keys[] = {"ааа","ббб","ввв","ггг","ддд","еее","жжж","ззз","ийй","ккк","ллл"};
    for (int i = 0; i < 11; i++) {
        Vector *r = btreeSearch(tree, keys[i]);
        assert(r != NULL);
        PostingEntry *ent = (PostingEntry*)getVectorItem(r, 0);
        assert(ent != NULL);
        assert(ent->doc_id == i + 1);
    }
    assert(tree->size == 11);
    freeBTree(tree);
    printf("TestDoubleSplit success!\n");
}

void testSearchAfterSplit(void) {
    printf("TestSearchAfterSplit...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "бобёр", 1);
    btreeInsert(tree, "кот", 2);
    btreeInsert(tree, "лиса", 3);
    btreeInsert(tree, "медведь", 4);
    btreeInsert(tree, "собака", 5);
    btreeInsert(tree, "тигр", 6);

    const char *present[] = {"бобёр","кот","лиса","медведь","собака","тигр"};
    for (int i = 0; i < 6; i++) {
        Vector *r = btreeSearch(tree, present[i]);
        assert(r != NULL);
        PostingEntry *ent = (PostingEntry*)getVectorItem(r, 0);
        assert(ent != NULL);
        assert(ent->doc_id == i + 1);
    }
    assert(btreeSearch(tree, "волк") == NULL);
    freeBTree(tree);
    printf("TestSearchAfterSplit success!\n");
}

static void collectKeysCallback(const char *key, Vector *postings, void *ctx) {
    (void)postings; // Игнорируем неиспользуемый параметр
    Vector *collected = (Vector*)ctx;
    char *dup = strdup(key);
    appendVectorItem(collected, &dup);
}

void testTraverse(void) {
    printf("TestTraverse...\n");
    BTree *tree = createBTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ввв", 1);
    btreeInsert(tree, "ааа", 2);
    btreeInsert(tree, "ббб", 3);

    Vector *collected = createVector(sizeof(char*));
    btreeTraverse(tree, collectKeysCallback, collected);

    assert(collected->size == 3);
    char *k0 = *(char**)getVectorItem(collected, 0);
    char *k1 = *(char**)getVectorItem(collected, 1);
    char *k2 = *(char**)getVectorItem(collected, 2);
    assert(strcmp(k0, "ааа") == 0);
    assert(strcmp(k1, "ббб") == 0);
    assert(strcmp(k2, "ввв") == 0);

    for (size_t i = 0; i < collected->size; i++) {
        char *s = *(char**)getVectorItem(collected, i);
        free(s);
    }
    vectorFree(collected);
    freeBTree(tree);
    printf("TestTraverse success!\n");
}

void testInsertSizeTracking(void) {
    printf("TestInsertSizeTracking...\n");
    BTree *tree = createBTree();
    assert(tree->size == 0);
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    btreeInsert(tree, "ааа", 1);
    assert(tree->size == 1);
    btreeInsert(tree, "ббб", 2);
    assert(tree->size == 2);
    btreeInsert(tree, "ааа", 3);
    assert(tree->size == 2);
    freeBTree(tree);
    printf("TestInsertSizeTracking success!\n");
}

int main(void) {
    testCreateBTree();
    testCreateBTreeNode();
    testInsertSingle();
    testInsertMultiple();
    testSearchExisting();
    testSearchNotExisting();
    testInsertSplit();
    testInsertOrder();
    testInsertOrderReverse();
    testInsertOrderMixed();
    testDuplicateInsert();
    testDoubleSplit();
    testSearchAfterSplit();
    testTraverse();
    testInsertSizeTracking();
    printf("=== ALL BTREE TESTS PASSED ===\n");
    return 0;
}
