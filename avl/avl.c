// Делайте на ваше усмотрение, главное, чтобы интерфейс был таким же, как в rbtree и btree, чтобы не менять код в main.c
// Ну и переиспользуйте код из предыдущих лабораторных, если он вам подходит
#include "avl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int maxInt(int a, int b) {
    return (a > b) ? a : b;
}

static int height(AVLNode* node) {
    return node ? node->height : 0;
}

static int balanceFactor(AVLNode* node) {
    if (!node) return 0;
    return height(node->left) - height(node->right);
}

static void updateHeight(AVLNode* node) {
    if (node) {
        node->height = 1 + maxInt(height(node->left), height(node->right));
    }
}

static AVLNode* createNode(const char* key, int doc_id, const char* title) {
    //создание узла
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) return NULL;

    node->key = strdup(key);
    if (!node->key) {
        free(node);
        return NULL;
    }

    node->postings = createPostingList();
    if (!node->postings) {
        free(node->key);
        free(node);
        return NULL;
    }

    appendPosting(node->postings, doc_id, title);
    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static void freeNode(AVLNode* node) {
    if (node) {
        free(node->key);
        if (node->postings) {
            vectorFree(node->postings);
        }
        free(node);
    }
}

//вращения
static AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

static AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

//балансировка
static AVLNode* balance(AVLNode* node) {
    if (!node) return NULL;

    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1) { //левое поддерево тяжелее
        //лево-прав вращ
        if (balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    if (bf < -1) { //правое поддерево тяжелее
        //право-лев вращ
        if (balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

//вставка
static AVLNode* insertNode(AVLNode* node, const char* key, int doc_id, 
                           const char* title, int* treeSize) {
    //новый узел создаем
    if (!node) {
        AVLNode* newNode = createNode(key, doc_id, title);
        if (newNode) (*treeSize)++;
        return newNode;
    }

    int cmp = strcmp(key, node->key);

    if (cmp < 0) {
        node->left = insertNode(node->left, key, doc_id, title, treeSize);
    } else if (cmp > 0) {
        node->right = insertNode(node->right, key, doc_id, title, treeSize);
    } else {
        //если ключ уже есть добавляем в posting list
        appendPosting(node->postings, doc_id, title);
        return node;
    }

    return balance(node);
}

AVLTree* createAVLTree(void) {
    //создаем дерево
    AVLTree* tree = (AVLTree*)malloc(sizeof(AVLTree));
    if (tree) {
        tree->root = NULL;
        tree->size = 0;
    }
    return tree;
}

void avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title) {
    //вставка в дерево с автобалансом
    if (!tree || !key || !title) return;
    tree->root = insertNode(tree->root, key, doc_id, title, &tree->size);
}

Vector* avlSearch(const AVLTree* tree, const char* key) {
    //ищет термин
    if (!tree || !key) return NULL;

    AVLNode* current = tree->root;
    while (current) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            return current->postings;
        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return NULL;
}


static void freeTreeRecursive(AVLNode* node) {
    if (!node) return;
    freeTreeRecursive(node->left);
    freeTreeRecursive(node->right);
    freeNode(node);
}

void freeAVLTree(AVLTree* tree) {
    if (!tree) return;
    freeTreeRecursive(tree->root);
    free(tree);
}

void avlTraverse(const AVLTree* tree,
                 void (*visit)(const char* key, Vector* postings, void* ctx),
                 void* ctx) {
    //обход лево-корнеь-право, с вызовом какой то функции
    if (!tree || !visit) return;
    
    AVLNode* stack[MAX_TREE_DEPTH];
    int top = -1;
    AVLNode* current = tree->root;

    while (current || top >= 0) {
        while (current) {
            stack[++top] = current;
            current = current->left;
        }
        
        current = stack[top--];
        visit(current->key, current->postings, ctx);
        current = current->right;
    }
}