#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generic.h"


static bool needToResize(Vector* vector, bool* increase)
{
    if (vector->size == vector->capacity) {
        *increase = true;
        return true;
    }

    if (MIN_SIZE < vector->size && vector->size < vector->capacity / 5) {
        *increase = false;
        return true;
    }

    return false;
}


static int resize(Vector* vector, bool increase)
{
    size_t new_capacity;

    if (increase) {
        new_capacity = vector->capacity * 2;

        if (new_capacity == 0)
            new_capacity = MIN_SIZE; 
    } 
    
    else {
        new_capacity = vector->capacity / 2;

        if (new_capacity < MIN_SIZE)
            new_capacity = MIN_SIZE;
    }

    void* new_data = malloc(new_capacity * vector->elem_size);
    
    if (!new_data)
        return -1;

    if (vector->size > 0) 
        memcpy(new_data, vector->data, vector->size * vector->elem_size);

    free(vector->data);

    vector->data = new_data;
    vector->capacity = new_capacity;

    return 0;
}


Vector* createVector(size_t elem_size)
{
    if (elem_size == 0)      
        return NULL;

    Vector* vector = (Vector*)malloc(sizeof(Vector));      

    if (!vector)      
        return NULL;

    vector->data = malloc(MIN_SIZE * elem_size);      
    
    if (!vector->data) {      
        free(vector);      
        return NULL;
    }

    vector->elem_size = elem_size;      
    vector->size = 0;      
    vector->capacity = MIN_SIZE;      

    return vector;
}


int appendVectorItem(Vector* vector, void* el)
{
    if (!vector || !el)      
        return -1;

    bool increase;      

    if (needToResize(vector, &increase)) {      
        if (resize(vector, increase) != 0)       
            return -1;
    }

    void* elem_pointer = (char*)vector->data + vector->size * vector->elem_size;      

    memcpy(elem_pointer, el, vector->elem_size);      
    vector->size++;      

    return 0;
}


void* getVectorItem(Vector *vector, size_t index)
{
    if (!vector || index >= vector->size)      
        return NULL;

    void* elem_pointer = (char*)vector->data + index * vector->elem_size;      

    return elem_pointer;
}


int setVectorItem(Vector* vector, size_t index, void* value)
{
    if (!vector || !value || index >= vector->size)      
        return -1;

    void* elem_pointer = (char*)vector->data + index * vector->elem_size;      
    memcpy(elem_pointer, value, vector->elem_size);      

    return 0;
}


void *popVectorItem(Vector* vector, size_t index)
{
    if (!vector || index >= vector->size)      
        return NULL;

    void* copy = malloc(vector->elem_size);      
    if (!copy) 
        return NULL;      

    void* elem_to_remove = (char*)vector->data + index * vector->elem_size;      
    memcpy(copy, elem_to_remove, vector->elem_size);      

    for (size_t i = index + 1; i < vector->size; i++) { 
        void* dest = (char*)vector->data + (i - 1) * vector->elem_size;      
        void* src = (char*)vector->data + i * vector->elem_size;      
        memcpy(dest, src, vector->elem_size);      
    }

    vector->size--;      

    bool increase;      
    if (needToResize(vector, &increase))      
        resize(vector, increase);       

    return copy;
}


long int findVectorItem(Vector* vector, void* value, EqualsFunc cmp)
{
    if (!vector || !value)      
        return -1;

    for (long int i = 0; i < vector->size; i++) {       
        void* elem_pointer = (char*)vector->data + i * vector->elem_size;      
        if (cmp(elem_pointer, value))
            return i;
    }

    return -1;
}


int vectorFree(Vector* vector)
{
    if (!vector)      
        return -1;

    if (vector->data)      
        free(vector->data);      

    free(vector);      

    return 0;
}


Vector* createVectorBySize(size_t elem_size, size_t vector_size) {
    if (elem_size == 0 || vector_size == 0)      
        return NULL;

    Vector* vector = (Vector*)malloc(sizeof(Vector));      

    if (!vector)      
        return NULL;

    vector->data = malloc(vector_size * elem_size);      
    if (!vector->data) {      
        free(vector);      
        return NULL;
    }

    vector->elem_size = elem_size;      
    vector->size = 0;      
    vector->capacity = vector_size;

    return vector;
}
