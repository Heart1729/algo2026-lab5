#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "avl.h"
#include "../posting.h"
#include "../vector/generic.h"

static void log_result(const char* task, const char* message) {
    printf("[%s] %s\n", task, message);
}

void test_createAVLTree(void)
{
    log_result("test_createAVLTree", "Начало тестирования");

    AVLTree* avl_tree = createAVLTree();
    assert(avl_tree != NULL);
    assert(avl_tree->root == NULL);
    assert(avl_tree->size == 0);

    freeAVLTree(avl_tree);
    log_result("test_createAVLTree", "Тесты успешно пройдены!");
}

void test_createAVLNode(void)
{
    log_result("test_createAVLNode", "Начало тестирования");

    const char* key = "123";
    int doc_id = 52;
    // ИСПРАВЛЕНО: удалили текстовый заголовок из вызова
    AVLNode* node = createAVLNode(key, doc_id);

    assert(node != NULL);
    assert(strcmp(node->key, key) == 0);
    assert(node->height == 1);
    assert(node->left == NULL);
    assert(node->right == NULL);
    assert(node->postings->size == 1);
    
    vectorFree(node->postings);
    free((char*)node->key);
    free(node);
    log_result("test_createAVLNode", "Тесты успешно пройдены!");
}

void test_insertAVLNode(void)
{
    log_result("test_insertAVLNode", "Начало тестирования");

    AVLTree* avl_tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовый заголовок
    avlInsert(avl_tree, "parent", 52);
    
    assert(avl_tree->size == 1);
    assert(avl_tree->root != NULL);
    assert(strcmp(avl_tree->root->key, "parent") == 0);
    assert(avl_tree->root->postings->size == 1);
    assert(avl_tree->root->height == 1);

    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(avl_tree, "a", 67);
    avlInsert(avl_tree, "z", 6767);

    assert(avl_tree->size == 3);
    assert(strcmp(avl_tree->root->left->key, "a") == 0);
    assert(avl_tree->root->left->postings->size == 1);
    assert(strcmp(avl_tree->root->right->key, "z") == 0);
    assert(avl_tree->root->right->postings->size == 1);
    assert(avl_tree->root->height == 2);

    freeAVLTree(avl_tree);
    log_result("test_insertAVLNode", "Тесты успешно пройдены!");
}

void test_addPostind(void)
{
    log_result("test_addPostind", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовый заголовок
    avlInsert(tree, "key", 67);
    assert(tree->size == 1);
    assert(tree->root->postings->size == 1);
    
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(tree, "key", 6767);
    avlInsert(tree, "key", 676767);
    assert(tree->size == 1);
    assert(tree->root->postings->size == 3);

    PostingEntry* entry1 = getVectorItem(tree->root->postings, 0);
    PostingEntry* entry2 = getVectorItem(tree->root->postings, 1);
    PostingEntry* entry3 = getVectorItem(tree->root->postings, 2);
    
    assert(entry1->doc_id == 67);
    assert(entry2->doc_id == 6767);
    assert(entry3->doc_id == 676767);
    
    // Повторный doc_id не должен дублироваться в списке постингов
    avlInsert(tree, "key", 67);
    assert(tree->size == 1);
    assert(tree->root->postings->size == 3);
    assert(entry1->doc_id == 67);
    
    freeAVLTree(tree);
    log_result("test_addPostind", "Тесты успешно пройдены!");
}

void test_avlSearch(void)
{
    log_result("test_avlSearch", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    Vector* result_empty = avlSearch(tree, "empty");
    assert(result_empty == NULL);

    // ИСПРАВЛЕНО: удалили текстовый заголовок
    avlInsert(tree, "key", 67);
    Vector* result = avlSearch(tree, "key");
    assert(result != NULL);
    assert(result->size == 1);

    PostingEntry* entry = getVectorItem(result, 0);
    assert(entry->doc_id == 67);

    Vector* result2 = avlSearch(tree, "keykey");
    assert(result2 == NULL);

    freeAVLTree(tree);
    log_result("test_avlSearch", "Тесты успешно пройдены!");
}

void test_rotateLeft(void)
{
    log_result("test_rotateLeft", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(tree, "a", 1);
    avlInsert(tree, "b", 2);
    avlInsert(tree, "c", 3);

    assert(strcmp(tree->root->key, "b") == 0);
    assert(strcmp(tree->root->left->key, "a") == 0);
    assert(strcmp(tree->root->right->key, "c") == 0);

    freeAVLTree(tree);
    log_result("test_rotateLeft", "Тесты успешно пройдены!");
}

void test_rotateRight(void)
{
    log_result("test_rotateRight", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(tree, "c", 1);
    avlInsert(tree, "b", 2);
    avlInsert(tree, "a", 3);

    assert(strcmp(tree->root->key, "b") == 0);
    assert(strcmp(tree->root->left->key, "a") == 0);
    assert(strcmp(tree->root->right->key, "c") == 0);

    freeAVLTree(tree);
    log_result("test_rotateRight", "Тесты успешно пройдены!");
}

void test_bigRotateLeft(void)
{
    log_result("test_bigRotateLeft", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(tree, "a", 1);
    avlInsert(tree, "c", 2);
    avlInsert(tree, "b", 3);

    assert(strcmp(tree->root->key, "b") == 0);
    assert(strcmp(tree->root->left->key, "a") == 0);
    assert(strcmp(tree->root->right->key, "c") == 0);

    freeAVLTree(tree);
    log_result("test_bigRotateLeft", "Тесты успешно пройдены!");
}

void test_bigRotateRight(void)
{
    log_result("test_bigRotateRight", "Начало тестирования");

    AVLTree* tree = createAVLTree();
    // ИСПРАВЛЕНО: удалили текстовые заголовки
    avlInsert(tree, "c", 1);
    avlInsert(tree, "a", 2);
    avlInsert(tree, "b", 3);

    assert(strcmp(tree->root->key, "b") == 0);
    assert(strcmp(tree->root->left->key, "a") == 0);
    assert(strcmp(tree->root->right->key, "c") == 0);

    freeAVLTree(tree);
    log_result("test_bigRotateRight", "Тесты успешно пройдены!");
}

static int checkHeight(AVLNode* node)
{
    if (node == NULL) {
        return 0;
    }

    return node->height;
}

static bool checkBalacne(AVLNode* node)
{
    if (node == NULL) {
        return true;
    }

    bool check_left = checkBalacne(node->left);
    bool check_right = checkBalacne(node->right);
    if (check_left && check_right) {
        return abs(checkHeight(node->left) - checkHeight(node->right)) <= 1;
    }

    return false;
}

void test_AVLtree(int count_nodes)
{
    log_result("test_AVLtree", "Начало тестирования");
    printf("Кол-во узлов: %d\n", count_nodes);

    AVLTree* tree = createAVLTree();
    char key[256];
    
    for (int i = 0; i < count_nodes; i++) {
        sprintf(key, "%d", i);
        // ИСПРАВЛЕНО: удалили лишний строковый аргумент
        avlInsert(tree, key, i);
    }
    
    assert(tree->size == count_nodes);
    assert(checkBalacne(tree->root) == true);

    freeAVLTree(tree);
    log_result("test_AVLtree", "Тесты успешно пройдены!");
}


int main() {
    test_createAVLTree();
    test_createAVLNode();
    test_insertAVLNode();
    test_addPostind();
    test_avlSearch();
    test_rotateLeft();
    test_rotateRight();
    test_bigRotateLeft();
    test_bigRotateRight();
    test_AVLtree(100);
    test_AVLtree(10000);
    test_AVLtree(1000000); 

    return 0;
}
