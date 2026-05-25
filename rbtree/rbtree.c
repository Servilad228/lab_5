#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Создание нового узла
RBNode* createRBNode(RBTree* tree, const char* key) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    if (!node) return NULL;
    
    node->key = (char*)malloc(strlen(key) + 1);
    if (!node->key) {
        free(node);
        return NULL;
    }
    strcpy(node->key, key);
    
    node->color = RB_RED;  // Новый узел всегда красный
    node->postings = createPostingList();
    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;
    
    return node;
}

// Создание дерева
RBTree* createRBTree(void) {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    if (!tree) return NULL;
    
    tree->nil = (RBNode*)malloc(sizeof(RBNode));
    if (!tree->nil) {
        free(tree);
        return NULL;
    }
    
    tree->nil->color = RB_BLACK;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->nil->parent = NULL;
    tree->nil->key = NULL;
    tree->nil->postings = NULL;
    
    tree->root = tree->nil;
    tree->size = 0;
    
    return tree;
}

// Левый поворот
void rbLeftRotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    
    if (y->left != tree->nil) {
        y->left->parent = x;
    }
    
    y->parent = x->parent;
    
    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    y->left = x;
    x->parent = y;
}

// Правый поворот
void rbRightRotate(RBTree* tree, RBNode* y) {
    RBNode* x = y->left;
    y->left = x->right;
    
    if (x->right != tree->nil) {
        x->right->parent = y;
    }
    
    x->parent = y->parent;
    
    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    
    x->right = y;
    y->parent = x;
}

// Восстановление свойств красно-черного дерева после вставки
void rbInsertFixup(RBTree* tree, RBNode* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;  // дядя
            
            if (y->color == RB_RED) {
                // Случай 1: дядя красный
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                // Случай 2: дядя черный
                if (z == z->parent->right) {
                    // Случай 2: z - правый ребенок
                    z = z->parent;
                    rbLeftRotate(tree, z);
                }
                // Случай 3: z - левый ребенок
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rbRightRotate(tree, z->parent->parent);
            }
        } else {
            // Симметричный случай для правого родителя
            RBNode* y = z->parent->parent->left;  // дядя
            
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbRightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rbLeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

// Вставка узла
void rbInsert(RBTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !key) return;
    
    RBNode* existing = NULL;
    RBNode* current = tree->root;
    
    while (current != tree->nil) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            existing = current;
            break;
        }
        current = (cmp < 0) ? current->left : current->right;
    }
    
    if (existing) {
        // Ключ уже существует - добавляем в posting list
        appendPosting(existing->postings, doc_id, title);
        return;
    }
    
    RBNode* z = createRBNode(tree, key);
    if (!z) return;
    
    appendPosting(z->postings, doc_id, title);
    
    // Вставка в дерево
    RBNode* y = tree->nil;
    RBNode* x = tree->root;
    
    while (x != tree->nil) {
        y = x;
        if (strcmp(z->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    
    z->parent = y;
    
    if (y == tree->nil) {
        tree->root = z;
    } else if (strcmp(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }
    
    z->left = tree->nil;
    z->right = tree->nil;
    z->color = RB_RED;
    
    rbInsertFixup(tree, z);
    tree->size++;
}

// Поиск ключа
Vector* rbSearch(const RBTree* tree, const char* key) {
    if (!tree || !key) return NULL;
    
    RBNode* current = tree->root;
    
    while (current != tree->nil) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            return clonePostingList(current->postings);

        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return NULL;
}

// Рекурсивный обход дерева
void rbInorderTraversal(RBNode* node, RBNode* nil,
                        void (*visit)(const char* key, Vector* postings, void* ctx),
                        void* ctx) {
    if (node == nil) return;
    
    rbInorderTraversal(node->left, nil, visit, ctx);
    visit(node->key, node->postings, ctx);
    rbInorderTraversal(node->right, nil, visit, ctx);
}

// Обход дерева
void rbTraverse(const RBTree* tree,
                void (*visit)(const char* key, Vector* postings, void* ctx),
                void* ctx) {
    if (!tree || !visit) return;
    rbInorderTraversal(tree->root, tree->nil, visit, ctx);
}

// Освобождение узла
void freeRBNode(RBTree* tree, RBNode* node) {
    if (!tree || !node || node == tree->nil) return;
    
    freeRBNode(tree, node->left);
    freeRBNode(tree, node->right);
    
    if (node->key) free(node->key);
    if (node->postings) vectorFree(node->postings);
    free(node);
}

// Освобождение дерева
void freeRBTree(RBTree* tree) {
    if (!tree) return;
    
    freeRBNode(tree, tree->root);
    if (tree->nil) free(tree->nil);
    free(tree);
}

void rbPrintNode(RBNode* node, RBNode* nil, int depth, int isLeft) {
    if (node == nil) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s%s (%s)\n", 
           isLeft ? "L: " : "R: ",
           node->key,
           node->color == RB_RED ? "RED" : "BLACK");
    
    rbPrintNode(node->left, nil, depth + 1, 1);
    rbPrintNode(node->right, nil, depth + 1, 0);
}

void rbPrintTree(RBTree* tree) {
    if (!tree) return;
    printf("=== RBTree (size=%d) ===\n", tree->size);
    if (tree->root != tree->nil) {
        printf("ROOT: %s (%s)\n", 
               tree->root->key,
               tree->root->color == RB_RED ? "RED" : "BLACK");
        rbPrintNode(tree->root->left, tree->nil, 1, 1);
        rbPrintNode(tree->root->right, tree->nil, 1, 0);
    }
    printf("===================\n");
}