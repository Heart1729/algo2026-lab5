#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// Подключаем необходимые заголовочные файлы вашего проекта
#include ".vector/generic.h"
#include "index.h"
#include "posting.h"
#include "search.h"

/* 
   Внимание: Чтобы протестировать функции со спецификатором static (например, pairwiseIntersect), 
   этот тестовый код рекомендуется вставить в самый конец вашего файла реализации search.c 
   или временно убрать ключевое слово static у тестируемых функций.
*/

// Вспомогательная функция для быстрой сборки тестового вектора PostingEntry
static Vector* create_test_posting_vector(int* ids, int count) {
    Vector* v = createVector(sizeof(PostingEntry));
    assert(v != NULL);
    for (int i = 0; i < count; i++) {
        PostingEntry entry;
        entry.id = ids[i];
        // Инициализация остальных полей структуры PostingEntry, если они есть
        int res = appendVectorItem(v, &entry);
        assert(res == 0);
    }
    return v;
}

// Тест 1: Проверка базового пересечения двух списков (pairwiseIntersect)
void test_pairwiseIntersect_base() {
    int ids1[] = {1, 3, 5, 7, 9};
    int ids2[] = {2, 3, 7, 10};
    
    Vector* list1 = create_test_posting_vector(ids1, 5);
    Vector* list2 = create_test_posting_vector(ids2, 4);
    
    // Исправленная функция должна вернуть элементы с id 3 и 7
    Vector* result = pairwiseIntersect(list1, list2);
    
    assert(result != NULL);
    assert(result->size == 2);
    
    PostingEntry* e1 = (PostingEntry*)getVectorItem(result, 0);
    PostingEntry* e2 = (PostingEntry*)getVectorItem(result, 1);
    
    assert(e1 != NULL && e1->id == 3);
    assert(e2 != NULL && e2->id == 7);
    
    vectorFree(list1);
    vectorFree(list2);
    vectorFree(result);
    printf("[PASS] test_pairwiseIntersect_base\n");
}

// Тест 2: Пересечение списков без общих элементов
void test_pairwiseIntersect_no_match() {
    int ids1[] = {1, 2, 3};
    int ids2[] = {4, 5, 6};
    
    Vector* list1 = create_test_posting_vector(ids1, 3);
    Vector* list2 = create_test_posting_vector(ids2, 3);
    
    Vector* result = pairwiseIntersect(list1, list2);
    
    assert(result != NULL);
    assert(result->size == 0);
    
    vectorFree(list1);
    vectorFree(list2);
    vectorFree(result);
    printf("[PASS] test_pairwiseIntersect_no_match\n");
}

// Тест 3: Множественное пересечение массивов (intersectPostings)
void test_intersectPostings() {
    int ids1[] = {1, 2, 3, 4};
    int ids2[] = {2, 3, 4, 5};
    int ids3[] = {3, 4, 5, 6};
    
    Vector* lists[3];
    lists[0] = create_test_posting_vector(ids1, 4);
    lists[1] = create_test_posting_vector(ids2, 4);
    lists[2] = create_test_posting_vector(ids3, 4);
    
    // Общие элементы для всех трех списков: 3 и 4
    Vector* result = intersectPostings(lists, 3);
    
    assert(result != NULL);
    assert(result->size == 2);
    
    PostingEntry* e1 = (PostingEntry*)getVectorItem(result, 0);
    PostingEntry* e2 = (PostingEntry*)getVectorItem(result, 1);
    
    assert(e1->id == 3);
    assert(e2->id == 4);
    
    vectorFree(lists[0]);
    vectorFree(lists[1]);
    vectorFree(lists[2]);
    // В текущей архитектуре intersectPostings возвращает новый выделенный вектор,
    // созданный на последней итерации цикла pairwiseIntersect. Освобождаем его.
    vectorFree(result); 
    
    printf("[PASS] test_intersectPostings\n");
}

// Тест 4: Проверка подсчета слов в строке поискового запроса
void test_count_words() {
    assert(count_words("hello") == 1);
    assert(count_words("hello world") == 2);
    assert(count_words("   multiple   spaces   here  ") == 3);
    assert(count_words("") == 0);
    assert(count_words(" \t\n ") == 0);
    printf("[PASS] test_count_words\n");
}

// Тест 5: Ограничение выборки до Топ-10 элементов
void test_createIntersectTop10() {
    // Создаем вектор из 15 элементов
    Vector* large_vector = createVector(sizeof(PostingEntry));
    for (int i = 0; i < 15; i++) {
        PostingEntry e;
        e.id = i * 10;
        appendVectorItem(large_vector, &e);
    }
    
    Vector* top10 = createIntersectTop10(large_vector);
    
    assert(top10 != NULL);
    assert(top10->size == 10);
    
    // Проверяем граничные элементы (0-й и 9-й)
    PostingEntry* first = (PostingEntry*)getVectorItem(top10, 0);
    PostingEntry* last = (PostingEntry*)getVectorItem(top10, 9);
    
    assert(first->id == 0);
    assert(last->id == 90);
    
    vectorFree(large_vector);
    vectorFree(top10);
    printf("[PASS] test_createIntersectTop10\n");
}

// Тест 6: Создание структуры SearchResult
void test_createSearchResult() {
    SearchResult* res = createSearchResult(42, "Test Document Title", 100);
    
    assert(res != NULL);
    assert(res->doc_id == 42);
    assert(strcmp(res->title, "Test Document Title") == 0);
    assert(res->score == 100);
    
    free(res);
    printf("[PASS] test_createSearchResult\n");
}

// Запуск всех разработанных тестов
void run_all_search_tests() {
    printf("=== СТАРТ ТЕСТИРОВАНИЯ МОДУЛЯ ПОИСКА ===\n");
    test_pairwiseIntersect_base();
    test_pairwiseIntersect_no_match();
    test_intersectPostings();
    test_count_words();
    test_createIntersectTop10();
    test_createSearchResult();
    printf("=== ВСЕ ТЕСТЫ УСПЕШНО ПРОЙДЕНЫ ===\n");
}


int main() {
    printf("Начинаем тестирование!\n");

    run_all_search_tests();

    printf("Тестирование пройдено!\n");

    return 0;
}