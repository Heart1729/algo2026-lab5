#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#ifdef __APPLE__
#include <mach/mach.h>
#endif

#include "../posting.h"
#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"

#define VOCAB 30000
#define WORDS_PER_DOC 8
#define QUERIES 1000

unsigned long seed = 12345;
int myRand(int n) {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % n;
}

char words[VOCAB][16];
void makeWords(void) {
    for (int i = 0; i < VOCAB; i++) {
        sprintf(words[i], "word%d", i);
    }
}

/* текущее время в секундах */
double getTime(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1000000000.0;
}

double getMemoryMB(void) {
#ifdef __APPLE__
    mach_task_basic_info_data_t info;
    mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &size) == KERN_SUCCESS) {
        return info.resident_size / (1024.0 * 1024.0);
    }
    return 0.0;
#else
    struct rusage u;
    getrusage(RUSAGE_SELF, &u);

    return u.ru_maxrss / 1024.0;
#endif
}
int intersect(Vector *lists[], int count) {
    for (int i = 0; i < count; i++) {
        if (lists[i] == NULL || lists[i]->size == 0) {
            return 0;
        }
    }
    int answer = 0;
    Vector *first = lists[0];
    for (size_t i = 0; i < first->size; i++) {
        PostingEntry *e = getVectorItem(first, i);
        int inAll = 1;
        for (int j = 1; j < count; j++) {
            int found = 0;
            for (size_t m = 0; m < lists[j]->size; m++) {
                PostingEntry *e2 = getVectorItem(lists[j], m);
                if (e2->doc_id == e->doc_id) {
                    found = 1;
                    break;
                }
            }
            if (!found) { inAll = 0; break; }
        }
        if (inAll) answer++;
    }
    return answer;
}

void runTree(int type, int docs) {
    char *name;
    void *tree;
    if (type == 0) {
        name = "AVL";
        tree = createAVLTree();
    }
    else if (type == 1) {
        name = "Red-Black";
        tree = createRBTree();
    }
    else {
        name = "B-tree";
        tree = createBTree();
    }

    seed = 12345;

    double start = getTime();
    for (int d = 0; d < docs; d++) {
        for (int w = 0; w < WORDS_PER_DOC; w++) {
            int num = myRand(VOCAB);
            if (type == 0) {
                avlInsert(tree, words[num], d);
            }
            else if (type == 1) {
                rbInsert(tree, words[num], d);
            }
            else {
                btreeInsert(tree, words[num], d);
            }
        }
    }
    double indexTime = getTime() - start;

    double memory = getMemoryMB();

    printf("INDEX %s %d %.3f sec\n", name, docs, indexTime);
    printf("MEM %s %d %.1f MB\n", name, docs, memory);

    for (int len = 1; len <= 3; len++) {
        double total = 0;
        for (int q = 0; q < QUERIES; q++) {
            char *query[3];
            for (int w = 0; w < len; w++) {
                query[w] = words[myRand(VOCAB)];
            }
            double s = getTime();
            Vector *lists[3];
            for (int w = 0; w < len; w++) {
                if (type == 0) {
                    lists[w] = avlSearch(tree, query[w]);
                }
                else if (type == 1) {
                    lists[w] = rbSearch(tree, query[w]);
                }
                else {
                    lists[w] = btreeSearch(tree, query[w]);
                }
            }
            intersect(lists, len);
            total += getTime() - s;
        }
        double msPerQuery = total / QUERIES * 1000.0;
        printf("SEARCH %s %d words=%d %.4f ms\n", name, docs, len, msPerQuery);
    }

    if (type == 0) {
        freeAVLTree(tree);
    }
    else if (type == 1) {
        freeRBTree(tree);
    }
    else {
        freeBTree(tree);
    }
}

int main(int argc, char **argv) {
    makeWords();

    int counts[3] = { 50000, 200000, 500000 };
    int howMany = 3;
    if (argc > 1) {
        counts[0] = atoi(argv[1]);
        howMany = 1;
    }

    for (int i = 0; i < howMany; i++) {
        for (int type = 0; type < 3; type++) {
            runTree(type, counts[i]);
        }
    }
    return 0;
}
