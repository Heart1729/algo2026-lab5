#pragma once
#include "./vector/generic.h"

typedef struct {
    int doc_id; 
} PostingEntry;

Vector* createPostingList(void);
void    appendPosting(Vector* list, int doc_id);
Vector* clonePostingList(const Vector* list);
