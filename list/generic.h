#ifndef GENERIC_LIST_H
#define GENERIC_LIST_H


#include <stddef.h>
#include "../comparators.h"

typedef struct Node
{
    void *data;
    struct Node *next;
} Node;

typedef struct
{
    Node *head;
    size_t elem_size;
} GenericList;


GenericList *createList(size_t elem_size);

void appendItem(GenericList *list, void *data);

int findItem(GenericList *list, void *value, EqualsFunc cmp);

void *popItem(GenericList *list, size_t index);

void freeList(GenericList *list);

unsigned int listLength(GenericList *list);

#endif

