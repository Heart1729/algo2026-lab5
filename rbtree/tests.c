#include <assert.h>
#include <stdio.h>
#include "rbtree.h"
#include "../vector/generic.h"

static RBNode* rbSearchNode(const RBTree* tree, const char* key) {
    RBNode* current = tree->root;
    while (current != tree->nil) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            return current;
        }
        if (cmp < 0) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }
    return NULL;
}

struct OrderCheckCtx {
    const char* last_key;
    bool is_sorted;
};

void checkAlphabeticalOrder(const char* key, Vector* postings, void* ctx) {
    struct OrderCheckCtx* context = (struct OrderCheckCtx*)ctx;
    if (context->last_key != NULL) {
        if (strcmp(context->last_key, key) >= 0) {
            context->is_sorted = false;
        }
    }
    context->last_key = key;
}

int main() {
    printf("Запуск тестов\n");

    RBTree* tree = createRBTree();
    assert(tree != NULL);
    assert(tree->size == 0);

    assert(tree->root == tree->nil);
    assert(tree->root->color == RB_BLACK);
    assert(tree->root->key == NULL);
    assert(tree->root->left == tree->nil);
    assert(tree->root->right == tree->nil);
    assert(tree->root->parent == tree->nil);
    assert(tree->root->postings == NULL);
    printf("Тест 1 пройден\n");


    char word1[] = "aaa";
    char word2[] = "bbb";
    int docid1 = 1;
    int docid2 = 2;
    rbInsert(tree, word1, docid1);
    rbInsert(tree, word2, docid2);
    assert(tree->size == 2);

    assert(tree->root->left == tree->nil);
    assert(strcmp(tree->root->key, word1) == 0);
    assert(strcmp(tree->root->right->key, word2) == 0);
    assert(tree->root->color == RB_BLACK);
    assert(tree->root->right->color == RB_RED);
    assert(tree->root->right->right == tree->root->right->left);
    assert(tree->root->right->right == tree->nil);
    assert(rbSearch(tree, word1) == tree->root->postings);
    assert(rbSearch(tree, word2) == tree->root->right->postings);
    printf("Тест 2 пройден\n");


    rbInsert(tree, "aaa", 3);

    assert(tree->size == 2);
    Vector* aaa_postings = rbSearch(tree, "aaa");
    assert(aaa_postings != NULL);
    assert(aaa_postings->size == 2);
    printf("Тест 3 пройден\n");

    // Исходно: aaa(Ч) -> bbb(К)
    rbInsert(tree, "000", 4);
    // Теперь: aaa(Ч) имеет двух детей: 000(К) и bbb(К)

    // Вставляем "ccc" (правый сын для "bbb").
    // Это запускает Случай 1: папа и дядя должны стать черными, дед (aaa) — красным,
    // но так как дед — корень, в конце он снова станет черным.
    rbInsert(tree, "ccc", 5);

    RBNode* node_aaa = tree->root;
    RBNode* node_000 = rbSearchNode(tree, "000");
    RBNode* node_bbb = rbSearchNode(tree, "bbb");
    RBNode* node_ccc = rbSearchNode(tree, "ccc");

    assert(strcmp(node_aaa->key, "aaa") == 0);
    assert(node_aaa->left == node_000);
    assert(node_aaa->right == node_bbb);
    assert(node_aaa->parent == tree->nil);

    assert(node_bbb->parent == node_aaa);
    assert(node_bbb->right == node_ccc);
    assert(node_bbb->left == tree->nil);

    assert(node_aaa->color == RB_BLACK);
    assert(node_000->color == RB_BLACK);
    assert(node_bbb->color == RB_BLACK);
    assert(node_ccc->color == RB_RED);

    printf("Тест 4 пройден\n");


    RBTree* rotation_tree = createRBTree();
    assert(rotation_tree != NULL);

    //Черный корень
    rbInsert(rotation_tree, "10", 1);

    //Правый красный сын
    rbInsert(rotation_tree, "20", 1);

    // Вставляем "30" — это создает линию из красных узлов справа (10 -> 20 -> 30).
    // Случай RR. Должен произойти левый поворот вокруг "10".
    rbInsert(rotation_tree, "30", 1);

    RBNode* node_10 = rbSearchNode(rotation_tree, "10");
    RBNode* node_20 = rbSearchNode(rotation_tree, "20");
    RBNode* node_30 = rbSearchNode(rotation_tree, "30");

    assert(node_10 != NULL && node_20 != NULL && node_30 != NULL);

    assert(rotation_tree->root == node_20);
    assert(node_20->color == RB_BLACK);
    assert(node_20->parent == rotation_tree->nil);

    assert(node_20->left == node_10);
    assert(node_10->parent == node_20);
    assert(node_10->color == RB_RED);

    assert(node_20->right == node_30);
    assert(node_30->parent == node_20);
    assert(node_30->color == RB_RED);

    rbInsert(rotation_tree, "25", 1);

    RBNode* node_25 = rbSearchNode(rotation_tree, "25");
    assert(node_25 != NULL);

    // "10" и "30" должны стать черными, "25" красный, "20" корень (черный)
    assert(node_10->color == RB_BLACK);
    assert(node_30->color == RB_BLACK);
    assert(node_25->color == RB_RED);

    // Случай RL.
    // Вставляем "28". Она пойдет направо от "25".
    // Структура справа от корня: 30(Ч) -> 25(К) -> 28(К). Дядя для "28" — это правый сын "30".
    rbInsert(rotation_tree, "28", 1);

    RBNode* node_28 = rbSearchNode(rotation_tree, "28");
    assert(node_28 != NULL);

    // После двойного поворота (сначала правый вокруг 30, затем левый вокруг деда)
    // Узел "28" должен встать на место бывшего отца ("25" уходит влево, "30" уходит вправо).
    assert(node_28->color == RB_BLACK);
    assert(node_28->parent == node_20);
    assert(rotation_tree->root->right == node_28);

    assert(node_28->left == node_25);
    assert(node_25->parent == node_28);
    assert(node_25->color == RB_RED);

    assert(node_28->right == node_30);
    assert(node_30->parent == node_28);
    assert(node_30->color == RB_RED);

    assert(node_25->left == rotation_tree->nil);
    assert(node_25->right == rotation_tree->nil);
    assert(node_30->left == rotation_tree->nil);
    assert(node_30->right == rotation_tree->nil);

    assert(rotation_tree->root == node_20);
    assert(node_20->color == RB_BLACK);

    freeRBTree(rotation_tree);
    printf("Тест 5 пройден\n");

    RBTree* left_rotation_tree = createRBTree();
    assert(left_rotation_tree != NULL);

    rbInsert(left_rotation_tree, "33", 1);

    rbInsert(left_rotation_tree, "22", 1);

    // Cоздается линия из красных узлов слева (33 -> 22 -> 11).
    // Cлучай LL. Правый поворот вокруг "33".
    rbInsert(left_rotation_tree, "11", 1);

    RBNode* node_11 = rbSearchNode(left_rotation_tree, "11");
    RBNode* node_22 = rbSearchNode(left_rotation_tree, "22");
    RBNode* node_33 = rbSearchNode(left_rotation_tree, "33");

    assert(left_rotation_tree->root == node_22);
    assert(node_22->color == RB_BLACK);
    assert(node_22->parent == left_rotation_tree->nil);

    assert(node_22->left == node_11);
    assert(node_11->parent == node_22);
    assert(node_11->color == RB_RED);

    assert(node_22->right == node_33);
    assert(node_33->parent == node_22);
    assert(node_33->color == RB_RED);


    // Вставляем "15" — для узла "11" это правый сын.
    // Перекраска, так как дядя "33" красный.
    rbInsert(left_rotation_tree, "15", 1);

    RBNode* node_15 = rbSearchNode(left_rotation_tree, "15");
    assert(node_15 != NULL);
    assert(node_11->color == RB_BLACK);
    assert(node_33->color == RB_BLACK);
    assert(node_15->color == RB_RED);

    // Вставляем "13". Она пойдет налево от "15".
    // Структура слева от корня: 11(Ч) -> 15(К) -> 13(К).
    // Дядя для "13" — это левый сын "11" (nil).
    // LR. Сначала правый поворот вокруг 15, затем левый вокруг деда 11.
    rbInsert(left_rotation_tree, "13", 1);

    RBNode* node_13 = rbSearchNode(left_rotation_tree, "13");
    assert(node_13 != NULL);


    assert(node_13->color == RB_BLACK);
    assert(node_13->parent == node_22);
    assert(left_rotation_tree->root->left == node_13);

    assert(node_11->color == RB_RED);
    assert(node_13->left == node_11);
    assert(node_11->parent == node_13);
    assert(node_11->left == left_rotation_tree->nil);
    assert(node_11->right == left_rotation_tree->nil);

    assert(node_15->color == RB_RED);
    assert(node_13->right == node_15);
    assert(node_15->parent == node_13);
    assert(node_15->left == left_rotation_tree->nil);
    assert(node_15->right == left_rotation_tree->nil);

    assert(left_rotation_tree->root == node_22);
    assert(node_22->color == RB_BLACK);
    assert(node_22->parent == left_rotation_tree->nil);

    assert(node_22->right == node_33);
    assert(node_33->color == RB_BLACK);
    assert(node_33->parent == node_22);
    assert(node_33->left == left_rotation_tree->nil);
    assert(node_33->right == left_rotation_tree->nil);

    freeRBTree(left_rotation_tree);
    printf("Тест 6 пройден\n");



    assert(rbSearch(tree, "missing_word") == NULL);
    assert(rbSearch(tree, "bb") == NULL);
    printf("Тест 7 пройден\n");

    // tree содержит ключи: "000", "aaa", "bbb", "ccc"
    struct OrderCheckCtx context = { .last_key = NULL, .is_sorted = true };
    rbTraverse(tree, checkAlphabeticalOrder, &context);

    assert(context.is_sorted == true);
    printf("Тест 8 пройден\n");

    freeRBTree(tree);

    return 0;
}
