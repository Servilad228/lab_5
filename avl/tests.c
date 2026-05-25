#include "avl.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
// команда запуска: gcc -o tests.exe avl/tests.c avl/avl.c posting.c ../lab3/vector/generic.c
//создание и свобода
void testEmptyTree(void) {
    AVLTree* tree = createAVLTree();
    assert(tree != NULL);
    assert(tree->root == NULL);
    assert(tree->size == 0);
    freeAVLTree(tree);
    printf("testEmptyTree: OK\n");
}

//вставка и посик
void testSingleInsert(void) {
    AVLTree* tree = createAVLTree();
    avlInsert(tree, "cat", 7, "A small animal");
    
    assert(tree->size == 1);
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "cat") == 0);
    assert(tree->root->height == 1);
    
    Vector* result = avlSearch(tree, "cat");
    assert(result != NULL);
    assert(result->size == 1);
    
    assert(avlSearch(tree, "dog") == NULL);
    
    freeAVLTree(tree);
    printf("testSingleInsert: OK\n");
}

//вставка с разн ключами
void testMultipleInsert(void) {
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "red", 1, "Red color");
    avlInsert(tree, "blue", 2, "Blue color");
    avlInsert(tree, "green", 3, "Green color");
    avlInsert(tree, "black", 4, "Black color");
    avlInsert(tree, "white", 5, "White color");
    
    assert(tree->size == 5);
    assert(avlSearch(tree, "red") != NULL);
    assert(avlSearch(tree, "blue") != NULL);
    assert(avlSearch(tree, "green") != NULL);
    assert(avlSearch(tree, "black") != NULL);
    assert(avlSearch(tree, "white") != NULL);
    
    freeAVLTree(tree);
    printf("testMultipleInsert: OK\n");
}

//добавление в сущ ключ
void testDuplicateKey(void) {
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "box", 11, "Big box");
    avlInsert(tree, "box", 12, "Small box");
    avlInsert(tree, "box", 13, "Old box");
    
    assert(tree->size == 1);
    
    Vector* result = avlSearch(tree, "box");
    assert(result != NULL);
    assert(result->size == 3);
    
    freeAVLTree(tree);
    printf("testDuplicateKey: OK\n");
}

//пустое дерево
void testSearchEmptyTree(void) {
    AVLTree* tree = createAVLTree();
    
    assert(avlSearch(tree, "car") == NULL);
    
    freeAVLTree(tree);
    printf("testSearchEmptyTree: OK\n");
}
int main(void) {
    testEmptyTree();
    testSingleInsert();
    testMultipleInsert();
    testDuplicateKey();
    testSearchEmptyTree();
    printf("\nPASSED\n");
    return 0;
}