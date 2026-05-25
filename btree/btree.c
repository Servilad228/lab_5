#include "btree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* copyString(const char* s) {
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s);
    char* copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    strcpy(copy, s);
    return copy;
}

static BTreeNode* createBTreeNode(int is_leaf) {
    BTreeNode* node = malloc(sizeof(BTreeNode));

    if (node == NULL) {
        return NULL;
    }

    node->n = 0;
    node->is_leaf = is_leaf;

    for (int i = 0; i < BTREE_MAX_KEYS; i++) {
        node->keys[i] = NULL;
        node->postings[i] = NULL;
    }

    for (int i = 0; i < BTREE_MAX_CH; i++) {
        node->children[i] = NULL;
    }

    return node;
}

BTree* createBTree(void) {
    BTree* tree = malloc(sizeof(BTree));

    if (tree == NULL) {
        return NULL;
    }

    tree->root = createBTreeNode(1); //по сути листок
    tree->size = 0;

    return tree;
}

static int findKeyIndex(BTreeNode* node, const char* key, int* found) {
    int i = 0;

    while (i < node->n && strcmp(key, node->keys[i]) > 0) { // проходимся по всем ключам и попадаем в нужный промежуток
        i++;
    }

    if (i < node->n && strcmp(key, node->keys[i]) == 0) {
        *found = 1;
    } else {
        *found = 0;
    }

    return i;
}

static Vector* btreeSearchNode(BTreeNode* node, const char* key) {
    if (node == NULL || key == NULL) {
        return NULL;
    }

    int found = 0;
    int i = findKeyIndex(node, key, &found);

    if (found) {
        return node->postings[i]; // если мы прям нашли в текущем Node, то берём
    }

    if (node->is_leaf) { // если у node нет детей, то ничего не нашли
        return NULL; 
    }

    return btreeSearchNode(node->children[i], key); // рекурсивно переходим к ребёнку с нужным промежутком
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    if (tree == NULL || tree->root == NULL || key == NULL) {
        return NULL;
    }

    return btreeSearchNode(tree->root, key);
}

static void btreeSplitChild(BTreeNode* parent, int child_index) {
    BTreeNode* full_child = parent->children[child_index]; // полный ребёнок, который делим
    BTreeNode* right_child = createBTreeNode(full_child->is_leaf); // новый правый узел

    right_child->n = BTREE_T - 1;

    // переносим правую половину ключей из full_child в right_child
    for (int j = 0; j < BTREE_T - 1; j++) {
        right_child->keys[j] = full_child->keys[j + BTREE_T];
        right_child->postings[j] = full_child->postings[j + BTREE_T];

        full_child->keys[j + BTREE_T] = NULL;
        full_child->postings[j + BTREE_T] = NULL;
    }

    // если это не лист, переносим правую половину детей
    if (!full_child->is_leaf) {
        for (int j = 0; j < BTREE_T; j++) {
            right_child->children[j] = full_child->children[j + BTREE_T];
            full_child->children[j + BTREE_T] = NULL;
        }
    }

    // в полном узле остаётся только левая половина, теперь это левый узел
    full_child->n = BTREE_T - 1;

    // в parent освобождаем место под нового ребёнка
    for (int j = parent->n; j >= child_index + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }

    parent->children[child_index + 1] = right_child;

    // в parent освобождаем место под средний ключ
    for (int j = parent->n - 1; j >= child_index; j--) {
        parent->keys[j + 1] = parent->keys[j];
        parent->postings[j + 1] = parent->postings[j];
    }

    // поднимаем средний ключ из full_child в parent
    parent->keys[child_index] = full_child->keys[BTREE_T - 1];
    parent->postings[child_index] = full_child->postings[BTREE_T - 1];

    full_child->keys[BTREE_T - 1] = NULL;
    full_child->postings[BTREE_T - 1] = NULL;

    parent->n++;
}

static void btreeInsertNonFull(BTreeNode* node,
                               const char* key,
                               int doc_id,
                               const char* title,
                               int* inserted_new_key) {
    int found = 0;
    int i = findKeyIndex(node, key, &found);
    // если нашли в этом же узле, то просто добавляем текущий posting
    if (found) {
        appendPosting(node->postings[i], doc_id, title);
        *inserted_new_key = 0; // нового слова не появилось
        return;
    }
    // если это лист, то просто вставляем в нужный промежуток
    if (node->is_leaf) {
        for (int j = node->n - 1; j >= i; j--) {
            node->keys[j + 1] = node->keys[j];
            node->postings[j + 1] = node->postings[j];
        }

        node->keys[i] = copyString(key);
        node->postings[i] = createPostingList();
        appendPosting(node->postings[i], doc_id, title);

        node->n++;
        *inserted_new_key = 1; // новое слово появилось
        return;
    }
    // если это не лист, но ребёнок нужного промежутка переполнен, то нужно его сплитовать
    if (node->children[i]->n == BTREE_MAX_KEYS) {
        btreeSplitChild(node, i);

        int cmp = strcmp(key, node->keys[i]);
        // если так получилось, что мы подняли после сплита нужный key
        if (cmp == 0) {
            appendPosting(node->postings[i], doc_id, title);
            *inserted_new_key = 0; // нового слова не появилоь
            return;
        }
        // если больше -> i++, если меньше -> оставляем i
        if (cmp > 0) {
            i++;
        }
    }
    // если ребёнок не переполнен, то в него переходим (god damn рекурсия)
    btreeInsertNonFull(node->children[i], key, doc_id, title, inserted_new_key);
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    if (tree == NULL || tree->root == NULL || key == NULL || title == NULL) {
        return;
    }
    // тот же btreeInsertNonFull, но с обработкой корня
    int inserted_new_key = 0;
    BTreeNode* root = tree->root;
    
    if (root->n == BTREE_MAX_KEYS) {
        BTreeNode* new_root = createBTreeNode(0);

        new_root->children[0] = root;
        tree->root = new_root;

        btreeSplitChild(new_root, 0);

        btreeInsertNonFull(new_root, key, doc_id, title, &inserted_new_key);
    } else {
        btreeInsertNonFull(root, key, doc_id, title, &inserted_new_key);
    }

    if (inserted_new_key) {
        tree->size++;
    }
}
// рекурсивная подфункция
static void btreeTraverseNode(BTreeNode* node,
                              void (*visit)(const char* key, Vector* postings, void* ctx),
                              void* ctx) {
    if (node == NULL) {
        return;
    }
    // проходимся по первому ребёнку, потом по первому ключу итд 
    for (int i = 0; i < node->n; i++) {
        if (!node->is_leaf) {
            btreeTraverseNode(node->children[i], visit, ctx);
        }
        // callback-функция
        visit(node->keys[i], node->postings[i], ctx);
    }
    // потому что детей на 1 больше, чем ключей
    if (!node->is_leaf) {
        btreeTraverseNode(node->children[node->n], visit, ctx);
    }
}

void btreeTraverse(const BTree* tree,
                   void (*visit)(const char* key, Vector* postings, void* ctx),
                   void* ctx) {
    if (tree == NULL || tree->root == NULL || visit == NULL) {
        return;
    }

    btreeTraverseNode(tree->root, visit, ctx);
}

// рекурсивная подфункция для освобождения 
static void freeBTreeNode(BTreeNode* node) {
    if (node == NULL) {
        return;
    }

    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            freeBTreeNode(node->children[i]);
        }
    }

    for (int i = 0; i < node->n; i++) {
        free(node->keys[i]);
        vectorFree(node->postings[i]);
    }

    free(node);
}

void freeBTree(BTree* tree) {
    if (tree == NULL) {
        return;
    }

    freeBTreeNode(tree->root);
    free(tree);
}