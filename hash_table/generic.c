#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../vector/generic.h"
#include "generic.h"

int HashInt(const void *key)
{
    int number = *(const int*)key;
    const double A = 0.6180339887498948;
    double x = number * A;
    double frac = x - (int)x;
    int hash = frac * INT_MAX;
    return hash;
}

int HashString(const void *key)
{
    const char *str = (const char*)key;
    long long k = 0;
    int i = 0;
    while (str[i] != '\0') {
        k += (i + 1) * (unsigned char)str[i];
        i++;
    }
    return HashInt(&k);
}

HashTable *createHashTable(size_t key_size, size_t val_size) {
    HashTable* table = malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }

    table->key_size = key_size;
    table->val_size = val_size;
    table->size = 0;

    size_t elem_size = 1 + key_size + val_size;
    Vector* values = createVector(elem_size);
    if (values == NULL) {
        free(table);
        return NULL;
    }
    table->values = values;

    for (size_t i = 0; i < TABLE_MIN_SIZE; i++) {
        unsigned char* slot = calloc(1, elem_size);
        if (slot == NULL) {
            vectorFree(table->values);
            free(table);
            return NULL;
        }
        slot[0] = SLOT_EMPTY;
        appendVectorItem(table->values, slot);
        free(slot);
    }
    table->capacity = table->values->capacity;
    return table;
}

void setItemHashTable(HashTable *table, void *key, void *data, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || table->values == NULL) {
        return;
    }
    if (key == NULL || data == NULL) {
        return;
    }

    size_t index = hash(key) % table->capacity;

    for (int count = 0; count < table->capacity; count++) {
        unsigned char* slot = getVectorItem(table->values, index);
        if (slot[0] == SLOT_EMPTY || slot[0] == SLOT_DELETED) {
            memcpy(slot + 1, key, table->key_size);
            memcpy(slot + 1 + table->key_size, data, table->val_size);
            slot[0] = SLOT_OCCUPIED;
            table->size++;
            return;
        }
        if (slot[0] == SLOT_OCCUPIED && cmp(key, (void*)(slot + 1))) {
            memcpy(slot + 1 + table->key_size, data, table->val_size);
            return;
        }
        index++;
        if (index == table->capacity) {
            index = 0;
        }
    }
    if (table->size * 2 > table->capacity) {
        rehashHashTable(table, hash, cmp);
        setItemHashTable(table, key, data, hash, cmp);
    }
}

void rehashHashTable(HashTable *table, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL) {
        return;
    }

    size_t elem_size = table->val_size + table->key_size + 1;
    Vector* vector_new = createVector(elem_size);
    if (vector_new == NULL) {
        return;
    }
    for (int i = 0; i < table->capacity * 2; i++) {
        unsigned char* slot = calloc(1, elem_size);
        if (slot == NULL) {
            vectorFree(vector_new);
            return;
        }
        slot[0] = SLOT_EMPTY;
        appendVectorItem(vector_new, slot);
        free(slot);
    }
    table->capacity *= 2;
    Vector* vector_old = table->values;
    table->values = vector_new;

    for (int index = 0; index < table->capacity / 2; index++) {
        unsigned char* slot = getVectorItem(vector_old, index);
        if (slot[0] == SLOT_OCCUPIED) {
            void* key = slot + 1;
            void* value = slot + 1 + table->key_size;
            table->size--;
            setItemHashTable(table, key, value, hash, cmp);
        }
    }
    vectorFree(vector_old);
    return;
}

void *getItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL) {
        return NULL;
    }

    size_t index = hash(key) % table->capacity;
    for (int i = 0; i < table->capacity; i++) {
        unsigned char* ptr = getVectorItem(table->values, index);
        if (ptr[0] == SLOT_OCCUPIED && cmp(ptr + 1, key)) {
            void* value = ptr + 1 + table->key_size;
            return value;
        }
        index++;
        if (index == table->capacity) {
            index = 0;
        }
    }
    return NULL;
}

void *popItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    unsigned char* ptr = getItemHashTable(table, key, hash, cmp);
    if (ptr == NULL) {
        return NULL;
    }
    void* value = malloc(table->val_size);
    if (value == NULL) {
        return NULL;
    }
    memcpy(value, ptr, table->val_size);
    unsigned char* slot = (unsigned char*)ptr - table->key_size - 1;
    slot[0] = SLOT_DELETED;
    table->size--;
    return value;
}

unsigned long int getCollisionCount(HashTable *table, HashFunc hash)
{
    if (table == NULL || table->values == NULL) {
        return 0;
    }

    unsigned long int count = 0;
    for (size_t index = 0; table->capacity > index; index++) {
        unsigned char* ptr_now = getVectorItem(table->values, index);
        if (ptr_now[0] == SLOT_OCCUPIED) {
            void* key = ptr_now + 1;
            size_t true_index = hash(key) % table->capacity;
            if (true_index != index) {
                count++;
            }
        }
    }
    return count;
}

void freeHashTable(HashTable *table)
{
    if (table == NULL) {
        return;
    }
    vectorFree(table->values);
    free(table);
}