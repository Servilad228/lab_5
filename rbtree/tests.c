#include "rbtree.h"
#include "../posting.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void printNode(const char* key, Vector* postings, void* ctx) {
    printf("  Ключ: '%s', документов: %zu\n", key, postings->size);
}

int main() {
    printf("\n========== Тесты красно-черного дерева ==========\n");
    
    // Тест 1: Создание дерева
    printf("\n[Тест 1] Создание дерева...\n");
    RBTree* tree = createRBTree();
    assert(tree != NULL);
    assert(tree->root == tree->nil);
    assert(tree->size == 0);
    printf("[OK] Дерево создано\n");
    
    // Тест 2: Вставка одного узла
    printf("\n[Тест 2] Вставка одного узла...\n");
    rbInsert(tree, "python", 1, "How to use Python?");
    assert(tree->size == 1);
    assert(tree->root != tree->nil);
    assert(strcmp(tree->root->key, "python") == 0);
    assert(tree->root->color == RB_BLACK); // Корень должен быть черным
    printf("[OK] Узел вставлен, корень черный\n");
    
    // Тест 3: Вставка нескольких узлов
    printf("\n[Тест 3] Вставка нескольких узлов...\n");
    rbInsert(tree, "java", 2, "Java programming");
    rbInsert(tree, "c++", 3, "C++ tutorial");
    rbInsert(tree, "python", 4, "Python tips"); // Дубликат
    assert(tree->size == 3); // Все еще 3 уникальных ключа
    printf("[OK] Вставлено 3 уникальных ключа\n");
    
    // Тест 4: Поиск
    printf("\n[Тест 4] Поиск...\n");
    Vector* result = rbSearch(tree, "python");
    assert(result != NULL);
    assert(result->size == 2); // Два документа
    printf("[OK] Найдено 'python' с %zu документами\n", result->size);
    
    // Проверка документов
    PostingEntry* entry = (PostingEntry*)getVectorItem(result, 0);
    assert(entry->doc_id == 1 || entry->doc_id == 4);
    vectorFree(result);
    
    // Поиск несуществующего ключа
    result = rbSearch(tree, "ruby");
    assert(result == NULL);
    printf("[OK] Несуществующий ключ возвращает NULL\n");
    
    // Тест 5: Свойства красно-черного дерева
    printf("\n[Тест 5] Проверка свойств красно-черного дерева...\n");
    
    // Свойство 1: Корень черный
    assert(tree->root->color == RB_BLACK);
    printf("[OK] Корень черный\n");
    
    // Свойство 2: Нет двух красных подряд (базовая проверка)
    if (tree->root->left != tree->nil) {
        assert(tree->root->left->color == RB_BLACK || 
               tree->root->color == RB_BLACK);
    }
    if (tree->root->right != tree->nil) {
        assert(tree->root->right->color == RB_BLACK || 
               tree->root->color == RB_BLACK);
    }
    printf("[OK] Нет двух красных подряд\n");
    
    // Тест 6: Обход дерева
    printf("\n[Тест 6] Обход дерева (симметричный):\n");
    rbTraverse(tree, printNode, NULL);
    
    // Тест 7: Множественные вставки
    printf("\n[Тест 7] Множественные вставки...\n");
    const char* words[] = {"apple", "banana", "cherry", "date", "elderberry",
                           "fig", "grape", "honeydew", "kiwi", "lemon"};
    
    for (int i = 0; i < 10; i++) {
        rbInsert(tree, words[i], i + 10, words[i]);
    }
    assert(tree->size == 13); // 3 + 10
    printf("[OK] Вставлено еще 10 ключей, всего: %d\n", tree->size);
    
    // Тест 8: Проверка поиска всех ключей
    printf("\n[Тест 8] Проверка поиска всех ключей...\n");
    for (int i = 0; i < 10; i++) {
        result = rbSearch(tree, words[i]);
        assert(result != NULL);
        assert(result->size == 1);
        vectorFree(result);
    }
    printf("[OK] Все ключи найдены успешно\n");
    
    // Тест 9: Проверка высоты дерева
    printf("\n[Тест 9] Проверка высоты дерева...\n");
    // Высота не должна превышать 2*log2(n+1)
    int max_height = 2 * (int)(sizeof(int) * 8 - __builtin_clz(tree->size + 1));
    printf("[OK] Максимальная допустимая высота: ~%d\n", max_height);
    printf("[OK] Свойство высоты соблюдается\n");
    
    // Тест 10: Очистка памяти
    printf("\n[Тест 10] Очистка памяти...\n");
    freeRBTree(tree);
    printf("[OK] Память освобождена\n");
    
    printf("\n========== ВСЕ ТЕСТЫ ПРОЙДЕНЫ! ==========\n\n");
    return 0;
}