#define _GNU_SOURCE
#include "../vector/generic.h"
#include "index.h"
#include "../posting.h"
#include "search.h"
#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"
#include "cJSON.h"
#include <ctype.h>
#include <time.h>

// Попарное пересечение векторов из PostingEntry
static Vector *pairwiseIntersect(Vector *list1, Vector *list2) {

    if (list1 == NULL || list2 == NULL) {
        return NULL;
    }

    Vector *intersect = createVector(sizeof(PostingEntry));
    if (intersect == NULL) {
        return NULL;
    }

    int i = 0;
    int j = 0;
    while (i != list1->size && j != list2->size) {
        PostingEntry *posting1 = getVectorItem(list1, i);
        if (posting1 == NULL) {
            vectorFree(intersect);
            return NULL;
        }

        PostingEntry *posting2 = getVectorItem(list2, j);
        if (posting2 == NULL) {
            vectorFree(intersect);
            return NULL;
        }

        if (posting1->doc_id == posting2->doc_id) {
            if (appendVectorItem(intersect, posting1) != 0) {
                printf("Ошибка добавления PostingEntry в Vector!\n");
                vectorFree(intersect);
                return NULL;
            }

            i++;
            j++;
        }
        else if (posting1->doc_id < posting2->doc_id) {
            i++;
        }
        else {
            j++;
        }
    }

    return intersect;
}

static void *deepCopyData(void *data) {
    size_t size = sizeof(data);
    void *copy_data = (void *)malloc(size);

    if (copy_data == NULL) {
        return NULL;
    }

    memcpy(copy_data, data, size);
    
    return copy_data;
}

Vector* intersectPostings(Vector** lists, int n) {
    Vector *previous_intersect = createVector(sizeof(PostingEntry));
    if (previous_intersect == NULL) {
        return NULL;
    }

    for (int j = 0; j < lists[0]->size; j++) {
        void *prev_item = getVectorItem(lists[0], j);
        if (prev_item == NULL) {
            printf("Ошибка нахождения элемента в Vector lists[0]!\n");
            vectorFree(previous_intersect);

            return NULL;
        }

        if (appendVectorItem(previous_intersect, prev_item) != 0) {
            printf("Ошибка добавления элемента в Vector previous_intersect!\n");
            vectorFree(previous_intersect);

            return NULL;
        }
    }

    for (int i = 1; i < n; i++) {
        Vector *curr_list = createVector(sizeof(PostingEntry));
        if (curr_list == NULL) {
            vectorFree(previous_intersect);
            return NULL;
        }

        for (int j = 0; j < lists[i]->size; j++) {
            void *curr_item = getVectorItem(lists[i], j);
            if (curr_item == NULL) {
                printf("Ошибка нахождения элемента в Vector lists[i]!\n");
                vectorFree(previous_intersect);
                vectorFree(curr_list);

                return NULL;
            }

            if (appendVectorItem(curr_list, curr_item) != 0) {
                printf("Ошибка добавления элемента в Vector curr_list!\n");
                vectorFree(previous_intersect);
                vectorFree(curr_list);

                return NULL;
            }
        }

        Vector *curr_intersect = pairwiseIntersect(previous_intersect, curr_list);
        if (curr_intersect == NULL) {
            return NULL;
        }

        vectorFree(curr_list);
        vectorFree(previous_intersect);

        previous_intersect = curr_intersect;
    }

    return previous_intersect;
}

static SearchResults* createSearchResults(Vector *results, int total, double time_ms) {
    SearchResults* search_results = malloc(sizeof(SearchResults));
    if (search_results == NULL) {
        return NULL;
    }

    search_results->results = results;
    search_results->total = total;
    search_results->time_ms = time_ms;

    return search_results;
}

static SearchResult* createSearchResult(int doc_id, char *title, int score) {
    SearchResult* result = malloc(sizeof(SearchResult));
    if (result == NULL) {
        return NULL;
    }

    result->doc_id = doc_id;
    strncpy(result->title, title, sizeof(result->title) - 1);
    result->title[sizeof(result->title) - 1] = '\0';
    result->score = score;

    return result;
}

static int countWords(const char *str) {
    int count = 0;
    int in_word = 0;
    while (*str) {
        if (isspace((unsigned char)*str)) {
            in_word = 0;
        } 
        else {
            if (in_word == 0) {
                count++;
                in_word = 1;
            }
        }
        str++;
    }

    return count;
}

static Vector *createIntersectTop10(Vector *intersect) {
    Vector *intersect_top10 = createVector(intersect->elem_size);
    if (intersect_top10 == NULL) {
        return NULL;
    }

    for (int i = 0; i < 10; i++) {
        void *curr_item = getVectorItem(intersect, i);
        if (curr_item == NULL) {
            vectorFree(intersect_top10);
            printf("Ошибка получения элемента в Vector intersect!\n");

            return NULL;
        }

        if (appendVectorItem(intersect_top10, curr_item) != 0) {
            vectorFree(intersect_top10);
            printf("Ошибка добавления элемента в Vector intersect_top10!\n");

            return NULL;
        }
    }

    return intersect_top10;
}

SearchResults* search(Index* idx, const char* query) {
    clock_t start = clock();

    char *query_copy = strdup(query);
    if (query_copy == NULL) {
        return NULL;
    }

    int n = countWords(query);

    Vector **postings_lists = malloc(n * sizeof(Vector*));
    if (postings_lists == NULL) {
        free(query_copy);
        return NULL;
    }

    const char *delimeter = " \t\n";
    char* token = strtok(query_copy, delimeter);
    for (int i = 0; i < n; i++) {
        Vector *postings;
        if (idx->type == TREE_AVL) {
            postings = avlSearch(idx->tree, token);
            if (postings == NULL) {
                free(query_copy);
                free(postings_lists);
                printf("Ошибка поиска элемента в AVLTree!\n");

                return NULL;
            }
        }
        else if (idx->type == TREE_RB) {
            postings = rbSearch(idx->tree, token);
            if (postings == NULL) {
                free(query_copy);
                free(postings_lists);
                printf("Ошибка поиска элемента в RBTree!\n");

                return NULL;
            }
        }
        else if (idx->type == TREE_BTREE) {
            postings = btreeSearch(idx->tree, token);
            if (postings == NULL) {
                free(query_copy);
                free(postings_lists);
                printf("Ошибка поиска элемента в BTree!\n");

                return NULL;
            }
        }

        postings_lists[i] = postings;

        token = strtok(NULL, delimeter);
    }

    free(query_copy);

    Vector *intersect = intersectPostings(postings_lists, n);
    if (intersect == NULL) {
        free(postings_lists);
        printf("Ошибка пересечения векторов с PostingEntry!\n");

        return NULL;
    }

    free(postings_lists);

    int total = intersect->size;
    if (intersect->size > 10) {
        Vector *changed_intersect = createIntersectTop10(intersect);
        if (changed_intersect == NULL) {
            vectorFree(intersect);
            printf("Ошибка создания Vector intersect_top10!\n");

            return NULL;
        }

        vectorFree(intersect);
        intersect = changed_intersect;
    }

    clock_t end = clock();
    clock_t time_delta = end - start;
    double time_in_ms = (double)time_delta / CLOCKS_PER_SEC * 1000;

    Vector *intersect_changed = createVector(sizeof(SearchResult));
    if (intersect_changed == NULL) {
        free(postings_lists);
        vectorFree(intersect);

        return NULL;
    }

        for (int i = 0; i < intersect->size; i++) {
        PostingEntry *curr_item = getVectorItem(intersect, i);
        if (curr_item == NULL) {
            printf("Ошибка нахождения элемента в Vector intersect!\n");
            vectorFree(intersect_changed);
            vectorFree(intersect);
            return NULL;
        }

        void* res_ptr = getItemHashTable(idx->names, &(curr_item->doc_id), HashInt, intEquals);
        
        const char* real_title;
        if (res_ptr != NULL) {
            real_title = (const char*)res_ptr;
        } else {
            real_title = "Title not found in Document Store";
        }

        int score = n; 

        SearchResult *changed_item = createSearchResult(curr_item->doc_id, (char*)real_title, score);
        if (changed_item == NULL) {
            vectorFree(intersect_changed);
            vectorFree(intersect);
            return NULL;
        }

        if (appendVectorItem(intersect_changed, changed_item) != 0) {
            printf("Ошибка добавления элемента в Vector intersect_changed!\n");
            vectorFree(intersect_changed);
            vectorFree(intersect);
            free(changed_item);
            return NULL;
        }

        free(changed_item);
    }


    SearchResults *search_results = createSearchResults(intersect_changed, total, time_in_ms);
    if (search_results == NULL) {
        free(postings_lists);
        vectorFree(intersect);
        printf("Ошибка создания SearchResults!\n");

        return NULL;
    }

    return search_results;
}

void printResultsText(const SearchResults* sr) {
    if (sr == NULL || sr->results == NULL) {
        return;
    }

    printf("Время: %.1f мс | Найдено: %d документов\n\n", sr->time_ms, sr->total);

    for (int i = 0; i < 10; i++) {
        SearchResult* res = getVectorItem(sr->results, i);  
        if (res != NULL) {
            printf("%2d. [id=%d] %s\n", i + 1, res->doc_id, res->title);
        }
    }
}

void printResultsJSON(const SearchResults* sr) {
    if (!sr) return;

    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    cJSON_AddNumberToObject(root, "time_ms", sr->time_ms);
    cJSON_AddNumberToObject(root, "total", sr->total);

    cJSON *documents_array = cJSON_CreateArray();
    if (!documents_array) {
        cJSON_Delete(root);
        return;
    }

    // ИСПРАВЛЕНИЕ 1: Называем ключ "results", как того ждет app.py
    cJSON_AddItemToObject(root, "results", documents_array);

    size_t count = sr->results->size;
    for (size_t i = 0; i < count; i++) {
        SearchResult* item = getVectorItem(sr->results, i);
        if (!item) continue;

        cJSON *doc_obj = cJSON_CreateObject();
        if (!doc_obj) continue;

        cJSON_AddNumberToObject(doc_obj, "doc_id", item->doc_id);
        cJSON_AddStringToObject(doc_obj, "title", item->title);
        cJSON_AddNumberToObject(doc_obj, "score", item->score);

        cJSON_AddItemToArray(documents_array, doc_obj);
    }

    char *json_string = cJSON_Print(root);
    if (!json_string) {
        cJSON_Delete(root);
        return;
    }

    // ИСПРАВЛЕНИЕ 2: Печатаем прямо в stdout, чтобы Streamlit мог это прочитать
    printf("%s\n", json_string);
    fflush(stdout); // Принудительно выталкиваем буфер вывода

    cJSON_free(json_string);
    cJSON_Delete(root);
}

void freeSearchResults(SearchResults* sr) {
    vectorFree(sr->results);
    free(sr);
}
