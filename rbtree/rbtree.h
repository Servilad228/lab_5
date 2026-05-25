#ifndef RBTREE_H
#define RBTREE_H
#pragma once

#include "../posting.h"

typedef enum { RB_RED, RB_BLACK } RBColor;

typedef struct RBNode {
    char*           key;
    RBColor         color;
    Vector*         postings;
    struct RBNode*  left;
    struct RBNode*  right;
    struct RBNode*  parent;
} RBNode;

typedef struct {
    RBNode* root;
    RBNode* nil;
    int     size;
} RBTree;

RBTree* createRBTree(void);
void    freeRBTree(RBTree* tree);
void    freeRBNode(RBTree* tree, RBNode* node);


void    rbInsert(RBTree* tree, const char* key, int doc_id, const char* title);
Vector* rbSearch(const RBTree* tree, const char* key);
void    rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);
void    rbInorderTraversal(RBNode* node, RBNode* nil,
                          void (*visit)(const char* key, Vector* postings, void* ctx),
                          void* ctx);

// Вспомогательные функции
RBNode* createRBNode(RBTree* tree, const char* key);
void    rbInsertFixup(RBTree* tree, RBNode* z);
void    rbLeftRotate(RBTree* tree, RBNode* x);
void    rbRightRotate(RBTree* tree, RBNode* y);
void    rbTransplant(RBTree* tree, RBNode* u, RBNode* v);

// Для отладки
void    rbPrintTree(RBTree* tree);
void    rbPrintNode(RBNode* node, RBNode* nil, int depth, int isLeft);

#endif // RBTREE_H
