#include "levenshtein.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int minInt(int a, int b)
{
    return (a < b) ? a : b;
}

static int minOfThree(int a, int b, int c)
{
    return minInt(minInt(a, b), c);
}

static int **allocateTable(int rows, int cols)
{
    int **dp = malloc(sizeof(int*) * rows);
    if (dp == NULL) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        dp[i] = malloc(sizeof(int) * cols);

        if (dp[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(dp[j]);
            }

            free(dp);
            return NULL;
        }
    }

    return dp;
}

static void freeTable(int **table, int rows)
{
    if (table == NULL) {
        return;
    }

    for (int i = 0; i < rows; i++) {
        free(table[i]);
    }

    free(table);
}

// Для отладки — возвращает имя операции
const char *operationName(OperationType type)
{
    switch (type)
    {
        case OP_NONE:    return "NONE";
        case OP_INSERT:  return "INSERT";
        case OP_DELETE:  return "DELETE";
        case OP_REPLACE: return "REPLACE";
        default:         return "UNKNOWN";
    }
}


int levenshteinDistance(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL) {
        return -1;
    }

    size_t rows = strlen(s1) + 1;
    size_t cols = strlen(s2) + 1;

    int **dp = allocateTable(rows, cols);
    if (dp == NULL) {
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i == 0 || j == 0) {
                dp[i][j] = i + j;
                continue;
            }

            if (s1[i-1] == s2[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = 1 + minOfThree(dp[i-1][j], dp[i][j-1], dp[i-1][j-1]);
            }
        }
    }

    int result = dp[rows-1][cols-1];
    // printEditTable(dp, s1, s2);
    freeTable(dp, rows);
    return result;
}

EditResult *levenshteinWithOperations(const char *s1, const char *s2)
{
    if (s1 == NULL || s2 == NULL) {
        return NULL;
    }

    EditResult *result = malloc(sizeof(EditResult));
    if (result == NULL) {
        return NULL;
    }

    size_t rows = strlen(s1) + 1;
    size_t cols = strlen(s2) + 1;

    int **dp = allocateTable(rows, cols);
    if (dp == NULL) {
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i == 0 || j == 0) {
                dp[i][j] = i + j;
                continue;
            }

            if (s1[i-1] == s2[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = 1 + minOfThree(dp[i-1][j], dp[i][j-1], dp[i-1][j-1]);
            }
        }
    }

    result->distance = dp[rows-1][cols-1];
    int i = rows - 1; int j = cols - 1;

    result->operations = malloc((rows + cols) * sizeof(EditOperation));
    if (result->operations == NULL) {
        freeTable(dp, rows);
        free(result);
        return NULL;
    }

    EditOperation *stack = malloc((rows + cols) * sizeof(EditOperation));
    int size_stack = 0;
    
    while (i > 0 || j > 0)
    {
        EditOperation operation;
        if (i > 0 && j > 0 && s1[i-1] == s2[j-1] && dp[i][j] == dp[i-1][j-1])
        {
            operation.type = OP_NONE;
            operation.position = i - 1;
            stack[size_stack] = operation;
            i--;
            j--;
            continue;
        }
        
        if (i > 0 && j > 0 && dp[i][j] == dp[i-1][j-1] + 1)
        {
            operation.type = OP_REPLACE;
            operation.position = i - 1;
            operation.oldChar = s1[i-1];
            operation.newChar = s2[j-1];
            stack[size_stack] = operation;
            size_stack++;
            i--;
            j--;
            continue;
        }
        
        if (i > 0 && dp[i][j] == dp[i-1][j] + 1)
        {
            operation.type = OP_DELETE;
            operation.position = i - 1;
            operation.oldChar = s1[i-1];
            stack[size_stack] = operation;
            size_stack++;
            i--;
            continue;
        }
        
        if (j > 0 && dp[i][j] == dp[i][j-1] + 1)
        {
            operation.type = OP_INSERT;
            operation.position = i;
            operation.newChar = s2[j-1];
            stack[size_stack] = operation;
            size_stack++;
            j--;
            continue;
        }
    }
    
    result->operationCount = size_stack;
    for (int i = 0; i < size_stack; i++)
    {
        result->operations[i] = stack[size_stack - 1 - i];
    }
    
    free(stack);
    freeTable(dp, rows);
    
    return result;
}


void printEditTable(int **dp, const char *s1, const char *s2)
{
    printf("Строка 1: '%s'\n", s1);
    printf("Строка 2: '%s'\n\n", s2);
    printf("Таблица DP:\n");

    size_t rows = strlen(s1) + 2;
    size_t cols = strlen(s2) + 2;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i == 0 && j == 0) {
                printf("   ");
            } else if ((i == 0 && j == 1) || (i == 1 && j == 0)) {
                printf("'' ");
            } else if (i == 0 && j >= 2) {
                printf(" %c ", s2[j-2]);
            } else if (i >= 2 && j == 0) {
                printf(" %c ", s1[i-2]);
            } else {
                printf(" %d ", dp[i-1][j-1]);
            }
            if (j == (cols - 1)) {
                printf("\n");
            }
        }
    }
    printf("\n");
}

void printOperations(EditResult *result, const char *s1, const char *s2)
{
    printf("Расстояние: %d\n\nОперации (%d шт.):\n", result->distance, result->operationCount);
    for (int i = 0; i < result->operationCount; i++) {
        EditOperation operation = result->operations[i];

        switch (operation.type) {
        case OP_NONE:
            break;

        case OP_REPLACE:
            printf("Заменить '%c' на '%c' в позиции %d\n", operation.oldChar, operation.newChar, operation.position);
            break;

        case OP_DELETE:
            printf("Удалить '%c' в позицию %d\n", operation.oldChar, operation.position);
            break;

        case OP_INSERT:
            printf("Вставить '%c' в позицию %d\n", operation.newChar, operation.position);
            break;

        default:
            break;
        }
    }
    printf("\n");
}

void printTransformation(EditResult *result, const char *s1, const char *s2)
{
    printf("Начало: '%s'\n", s1);

    size_t len_s1 = strlen(s1);
    size_t len_s2 = strlen(s2);

    char *temp = malloc((sizeof(char)) * (len_s1 + len_s2));
    if (temp == NULL) {
        return;
    }
    strcpy(temp, s1);

    int change = 0;
    size_t curent_len = len_s1;
    for (int i = 0; i < result->operationCount; i++) {
        EditOperation operation = result->operations[i];
        int position = operation.position + change;

        switch (operation.type) {
        case OP_NONE:
            break;

        case OP_REPLACE:
            temp[position] = operation.newChar;
            printf("Шаг %d: '%s   (REPLACE '%c' на '%c' в позиции %d)\n",
                   i + 1, temp, operation.oldChar, operation.newChar, position);
            break;

        case OP_DELETE:
            for (int j = position; j < curent_len; j++) {
                temp[j] = temp[j + 1];
            }
            curent_len--;
            change--;

            printf("Шаг %d: '%s' (DELETE '%c' из позиции %d)\n",
                   i + 1, temp, operation.oldChar, position);
            break;

        case OP_INSERT:
            for (int j = curent_len; j >= position; j--) {
                temp[j + 1] = temp[j];
            }
            temp[position] = operation.newChar;
            curent_len++;
            change++;

            printf("Шаг %d: '%s' (INSERT '%c' в позицию %d)\n",
                   i + 1, temp, operation.newChar, position);
            break;

        default:
            break;
        }
    }
    printf("Итог: %s\n", temp);
}

char **findSimilarWords(const char *word, char **dictionary, int dictSize,
                        int maxDistance, int *resultCount)
{
    if (word  == NULL || dictionary  == NULL|| resultCount == NULL || resultCount == NULL) {
        return NULL;
    }

    *resultCount = 0;
    char **result = malloc(sizeof(char*) * dictSize);
    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < dictSize; i++) {
        if (levenshteinDistance(word, dictionary[i]) <= maxDistance) {
            result[*resultCount] = malloc(strlen(dictionary[i]) + 1);
            
            if (result[*resultCount] == NULL) {
                for (int j = 0; j < *resultCount; j++) {
                    free(result[j]);
                }
                free(result);
                return NULL;
            }

            strcpy(result[*resultCount], dictionary[i]);
            (*resultCount)++;
        }
    }

   if (*resultCount == 0) {
        free(result);
        return NULL;
    }
    
    return result;
}

void freeSimilarWords(char **words, int count)
{
    if (words == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        free(words[i]);
    }

    free(words);
}


void freeEditResult(EditResult *result)
{
    if (result == NULL) {
        return;
    }

    if (result->operations != NULL) {
        free(result->operations);
    }

    free(result);
}

