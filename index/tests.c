#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"
#include "../levenshtein/levenshtein.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>


typedef struct {
    double insert_time_ms;
    double search_time_ms;
} Result;

static void log_result(const char* task, const char* message) {
    printf("[%s] %s\n", task, message);
}

void testAVLSearch() {
    log_result("testAVLSearch", "Начало тестирования");
    
    AVLTree* tree = createAVLTree();
    const char* words[] = {"C++", "C", "C--", "python", "jawa", "javascript"};
    printf("Добавлены слова: 'C++', 'C', 'C--', 'python', 'jawa', 'javascript'\n");
    for (int i = 0; i < 6; i++) {
        avlInsert(tree, words[i], i + 1);
    }
    
    Vector* candidates1 = avlFuzzyFindCandidates(tree, "python", 0);
    assert(candidates1 != NULL);
    assert(candidates1->size == 1);
    
    AVLFuzzyCandidate* c = getVectorItem(candidates1, 0);
    assert(strcmp(c->candidate_key, "python") == 0);
    assert(c->distance == 0);
    
    printf("\nНайдено по запросу 'python': '%s' (distance=%d)\n\n", c->candidate_key, c->distance);
    
    Vector* candidates2 = avlFuzzyFindCandidates(tree, "C", 2);
    assert(candidates2 != NULL);
    assert(candidates2->size >= 2);
    
    printf("Найдено по запросу 'C'\n");
    for (size_t i = 0; i < candidates2->size; i++) {
        AVLFuzzyCandidate* c = (AVLFuzzyCandidate*)getVectorItem(candidates2, i);
        printf("  %zu. '%s' (distance=%d)\n", i+1, c->candidate_key, c->distance);
    }
    printf("\n");

    freeAVLFuzzyCandidates(candidates1);
    freeAVLFuzzyCandidates(candidates2);
    freeAVLTree(tree);
    log_result("testAVLSearch", "Тесты успешно пройдены!");
}

void testBTreeSearch() {
    log_result("testBTreeSearch", "Начало тестирования");
    
    BTree* tree = createBTree();
    const char* words[] = {"C++", "C", "C--", "python", "jawa", "javascript"};
    printf("Добавлены слова: 'C++', 'C', 'C--', 'python', 'jawa', 'javascript'\n");
    for (int i = 0; i < 6; i++) {
        btreeInsert(tree, words[i], i + 1);
    }
    
    Vector* candidates1 = btreeFuzzyFindCandidates(tree, "python", 0);
    assert(candidates1 != NULL);
    assert(candidates1->size == 1);
    
    BTreeFuzzyCandidate* c = getVectorItem(candidates1, 0);
    assert(strcmp(c->term, "python") == 0);
    assert(c->distance == 0);
    
    printf("\nНайдено по запросу 'python': '%s' (distance=%d)\n\n", c->term, c->distance);
    
    Vector* candidates2 = btreeFuzzyFindCandidates(tree, "C", 2);
    assert(candidates2 != NULL);
    assert(candidates2->size >= 2);
    
    printf("Найдено по запросу 'C':\n");
    for (size_t i = 0; i < candidates2->size; i++) {
        BTreeFuzzyCandidate* c2 = getVectorItem(candidates2, i);
        printf("  %zu. '%s' (distance=%d)\n", i+1, c2->term, c2->distance);
    }
    printf("\n");
    
    freeFuzzyCandidates(candidates1);
    freeFuzzyCandidates(candidates2);
    freeBTree(tree);
    log_result("testBTreeSearch", "Тесты успешно пройдены!");
}

void testRBSearch() {
    log_result("testRBSearch", "Начало тестирования");
    
    RBTree* tree = createRBTree();
    const char* words[] = {"C++", "C", "C--", "python", "jawa", "javascript"};
    printf("Добавлены слова: 'C++', 'C', 'C--', 'python', 'jawa', 'javascript'\n");
    for (int i = 0; i < 6; i++) {
        rbInsert(tree, words[i], i + 1);
    }
    
    Vector* candidates1 = rbFuzzyFindCandidates(tree, "python", 0);
    assert(candidates1 != NULL);
    assert(candidates1->size == 1);
    
    RBFuzzyCandidate* c = getVectorItem(candidates1, 0);
    assert(strcmp(c->term, "python") == 0);
    assert(c->distance == 0);
    
    printf("\nНайдено по запросу 'python': '%s' (distance=%d)\n\n", c->term, c->distance);
    
    Vector* candidates2 = rbFuzzyFindCandidates(tree, "C", 2);
    assert(candidates2 != NULL);
    assert(candidates2->size >= 2);
    
    printf("Найдено по запросу 'C':\n");
    for (size_t i = 0; i < candidates2->size; i++) {
        RBFuzzyCandidate* c2 = getVectorItem(candidates2, i);
        printf("  %zu. '%s' (distance=%d)\n", i+1, c2->term, c2->distance);
    }
    printf("\n");
    
    freeRBFuzzyCandidates(candidates1);
    freeRBFuzzyCandidates(candidates2);
    freeRBTree(tree);
    log_result("testRBSearch", "Тесты успешно пройдены!");
}

Result testInsert(void* tree, int tree_type, int count_words) {
    Result result = {0};
    clock_t start = clock();
    
    for (int i = 0; i < count_words; i++) {
        char word[50];
        sprintf(word, "word%d", i);
        
        switch (tree_type) {
            case 0:
                avlInsert(tree, word, i);
                break;
            case 1:
                rbInsert(tree, word, i);
                break;
            case 2:
                btreeInsert(tree, word, i);
                break;
        }
    }
    
    clock_t end = clock();
    result.insert_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    
    return result;
}

Result testSearch(void* tree, int tree_type, int num_queries, int max_distance) {
    Result result = {0};
    clock_t start = clock();
    
    for (int i = 0; i < num_queries; i++) {
        char query[50];
        sprintf(query, "word%d", rand() % 10000);
        
        Vector* candidates = NULL;
        switch (tree_type) {
            case 0:
                candidates = avlFuzzyFindCandidates(tree, query, max_distance);
                if (candidates) {
                    freeAVLFuzzyCandidates(candidates);
                }
                break;
            case 1:
                candidates = rbFuzzyFindCandidates(tree, query, max_distance);
                if (candidates) {
                    freeRBFuzzyCandidates(candidates);
                }
                break;
            case 2:
                candidates = btreeFuzzyFindCandidates(tree, query, max_distance);
                if (candidates) {
                    freeFuzzyCandidates(candidates);
                }
                break;
        }
    }
    
    clock_t end = clock();
    result.search_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    
    return result;
}

void compareAllTrees() { 
    const int sizes[] = {1000, 5000, 10000, 50000};
    const int num_sizes = 4;
    
    for (int s = 0; s < num_sizes; s++) {
        int count_words = sizes[s];
        
        printf("\nКол-во слов: %d\n", count_words);
        printf("|          |   Вставка(ms) |   Поиск(ms)   |\n");
        
        AVLTree* avl = createAVLTree();
        Result avl_insert = testInsert(avl, 0, count_words);
        Result avl_search = testSearch(avl, 0, 100, 2);
        
        RBTree* rb = createRBTree();
        Result rb_insert = testInsert(rb, 1, count_words);
        Result rb_search = testSearch(rb, 1, 100, 2);
        
        BTree* btree = createBTree();
        Result btree_insert = testInsert(btree, 2, count_words);
        Result btree_search = testSearch(btree, 2, 100, 2);
        
        printf("|  AVLTree |      %.2f     |     %.2f     |\n",
            avl_insert.insert_time_ms, avl_search.search_time_ms);
        printf("|  RBTree  |      %.2f     |     %.2f     |\n",
            rb_insert.insert_time_ms, rb_search.search_time_ms);
        printf("|   BTree  |      %.2f     |     %.2f     |\n",
            btree_insert.insert_time_ms, btree_search.search_time_ms);
        
        freeAVLTree(avl);
        freeRBTree(rb);
        freeBTree(btree);
    }
}

int main() {
    testAVLSearch();
    testBTreeSearch();
    testRBSearch();
    compareAllTrees();
    
    return 0;
}