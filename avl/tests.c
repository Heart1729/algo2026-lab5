#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "avl.h"


void log_result(const char* task, const char* message) {
    printf("[%s] %s\n", task, message);
}

void test_createAVLTree(void)
{
    log_result("test_createAVLTree", "Начало тестирования");
    AVLTree* avl_tree = createAVLTree();
    assert(avl_tree != NULL);
    assert(avl_tree->root == NULL);
    assert(avl_tree->size == 0);

    log_result("test_createAVLTree", "Тесты успешно пройдены!");
}

void test_createAVLNode(void)
{
    log_result("test_createAVLNode", "Начало тестирования");

    const char* key = "123";
    int doc_id = 52;
    const char* title = "avl_node";
    AVLNode* node = createAVLNode(key, doc_id, title);

    assert(node != NULL);
    assert(strcmp(node->key, key) == 0);
    assert(node->height == 1);
    assert(node->left == NULL);
    assert(node->right == NULL);
    assert(node->postings->size == 1);

    log_result("test_createAVLNode", "Тесты успешно пройдены!");
}

int main() {
    test_createAVLTree();
    test_createAVLNode();

    return 0;
}