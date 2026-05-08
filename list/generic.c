#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


GenericList* createList(size_t elem_size)
{
    if (elem_size == 0)
        return NULL;

    GenericList* list = (GenericList*)malloc(sizeof(GenericList));

    if (!list)  {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    list->head = NULL;
    list->elem_size = elem_size;

    return list;
}


struct Node* createNode(const void* data, const size_t data_size) { 
    if (!data)
        return NULL;

    struct Node* new = (struct Node*)malloc(sizeof(struct Node));

    if (!new) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    new->data = malloc(data_size);

    if (!new->data) {
        printf("Memory allocation failed!\n");
        free(new);
        exit(EXIT_FAILURE);
    }              

    memcpy(new->data, data, data_size);
    new->next = NULL;

    return new;
}


void appendItem(GenericList* list, void* data)
{
    if (!list || !data)
        return;

    Node* new_node = createNode(data, list->elem_size);

    if (!list->head)
        list->head = new_node;

    else {
        Node* current = list->head;
        
        while (current->next)
            current = current->next;

        current->next = new_node;
    }
}


int findItem(GenericList* list, void* value, EqualsFunc cmp)
{
    if (!list || !list->head || !value)
        return -1;

    Node* current = list->head;
    int counter = 0;

    while (current) {
        if (cmp(current->data, value))
            return counter;
        
        current = current->next;
        counter++;
    }

    return -1;
}


void* popItem(GenericList* list, size_t index)
{
    if (!list)
        return NULL;

    if (index < 0 || index >= listLength(list))
        return NULL;

    Node* current = list->head;
    Node* previous = NULL;

    for (size_t i = 0; i < index; i++) {
        previous = current;
        current = current->next;
    }

    if (!previous)
        list->head = current->next;
    else 
        previous->next = current->next;

    void* data_copy = malloc(list->elem_size);

    if (!data_copy) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(data_copy, current->data, list->elem_size);

    free(current->data);
    free(current);

    return data_copy;
}


void freeList(GenericList* list)
{
    if (!list)
        return;

    Node* current = list->head;

    while (current) { 
        Node* temp_next = current->next;
        free(current->data);
        free(current);
        current = temp_next;
    }

    free(list);
}

unsigned int listLength(GenericList* list)
{
    if (!list || !list->head)
        return 0;

    unsigned int count = 0;
    Node* current = list->head;

    while (current) {
        count++;
        current = current->next;
    }

    return count;
}
