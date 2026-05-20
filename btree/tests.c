#include "btree.h"
#include <assert.h>
#include <string.h>

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
    appendPosting(postings, 1, "Матан");
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
    btreeInsert(tree, "кот", 1, "Матан");
    assert(tree->root != NULL);
    assert(tree->size == 1);
    assert(strcmp(tree->root->keys[0], "кот") == 0);
    freeBTree(tree);
    printf("TestInsertSingle success!\n");
}

void testInsertMultiple(void) {
    printf("TestInsertMultiple...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "кот", 1, "Матан");
    btreeInsert(tree, "бобёр", 2, "Линал");
    btreeInsert(tree, "собака", 3, "Математика");
    assert(tree->size == 3);
    freeBTree(tree);
    printf("TestInsertMultiple success!\n");
}

void testSearchExisting(void) {
    printf("TestSearchExisting...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "кот", 1, "Матан");
    btreeInsert(tree, "бобёр", 2, "Линал");
    Vector *result = btreeSearch(tree->root, "кот");
    assert(result != NULL);
    freeBTree(tree);
    printf("TestSearchExisting success!\n");
}

void testSearchNotExisting(void) {
    printf("TestSearchNotExisting...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "кот", 1, "Матан");
    Vector *result = btreeSearch(tree->root, "тигр");
    assert(result == NULL);
    freeBTree(tree);
    printf("TestSearchNotExisting success!\n");
}

void testInsertSplit(void) {
    printf("TestInsertSplit...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "бобёр",  1, "doc1");
    btreeInsert(tree, "кот",    2, "doc2");
    btreeInsert(tree, "лиса",   3, "doc3");
    btreeInsert(tree, "медведь",4, "doc4");
    btreeInsert(tree, "собака", 5, "doc5");
    btreeInsert(tree, "тигр",   6, "doc6");
    assert(tree->root->is_leaf == 0);
    assert(tree->size == 6);
    freeBTree(tree);
    printf("TestInsertSplit success!\n");
}

void testInsertOrder(void) {
    printf("TestInsertOrder...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "ббб", 1, "doc1");
    btreeInsert(tree, "ааа", 2, "doc2");
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ббб") == 0);
    freeBTree(tree);
    printf("TestInsertOrder success!\n");
}

void testInsertOrderReverse(void) {
    printf("TestInsertOrderReverse...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "ггг", 1, "doc1");
    btreeInsert(tree, "ввв", 2, "doc2");
    btreeInsert(tree, "ааа", 3, "doc3");
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ввв") == 0);
    assert(strcmp(tree->root->keys[2], "ггг") == 0);
    freeBTree(tree);
    printf("TestInsertOrderReverse success!\n");
}

void testInsertOrderMixed(void) {
    printf("TestInsertOrderMixed...\n");
    BTree *tree = createBTree();
    btreeInsert(tree, "ввв", 1, "doc1");
    btreeInsert(tree, "ааа", 2, "doc2");
    btreeInsert(tree, "ббб", 3, "doc3");
    assert(strcmp(tree->root->keys[0], "ааа") == 0);
    assert(strcmp(tree->root->keys[1], "ббб") == 0);
    assert(strcmp(tree->root->keys[2], "ввв") == 0);
    freeBTree(tree);
    printf("TestInsertOrderMixed success!\n");
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
    return 0;
}
