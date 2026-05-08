#ifndef VECTOR_GENERIC_H
#define VECTOR_GENERIC_H


#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "../comparators.h"

#define MIN_SIZE 1


typedef struct {
  void *data;       
  size_t elem_size; 
  size_t size;     
  size_t capacity;
} Vector;


Vector *createVector(size_t elem_size);

int appendVectorItem(Vector *vector, void *el);

void *getVectorItem(Vector *vector, size_t index);

int setVectorItem(Vector *vector, size_t index, void *value);

void *popVectorItem(Vector *vector, size_t index);

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp);

int vectorFree(Vector *vector);

#endif