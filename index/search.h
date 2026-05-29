#pragma once

#include "index.h"
#include "../vector/generic.h"

#define MAX_TITLE_LEN 256

typedef struct {
    int  doc_id;
    char title[MAX_TITLE_LEN];
    int  score;
} SearchResult;

typedef struct {
    Vector* results;   /* elements: SearchResult, топ-10 */
    int     total;     /* всего найдено документов       */
    double  time_ms;
} SearchResults;

static void *deepCopyData(void *data);
static Vector *pairwiseIntersect(Vector *list1, Vector *list2);
Vector*        intersectPostings(Vector** lists, int n);
static SearchResults* createSearchResults(Vector *results, int total, double time_ms);
static SearchResult* createSearchResult(int doc_id, char *title, int score);
static int countWords(const char *str);
static Vector *createIntersectTop10(Vector *intersect);
SearchResults* search(Index* idx, const char* query);
void printResultsText(const SearchResults* sr);
void printResultsJSON(const SearchResults* sr);
void           freeSearchResults(SearchResults* sr);
