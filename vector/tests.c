#include "generic.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../comparators.h"

int test_create_vector() {
    printf("Тестируем createVector...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    assert(vector != NULL);
    assert(vector->data != NULL);
    assert(vector->elem_size == sizeof(int));
    assert(vector->size == 0);
    assert(vector->capacity == MIN_SIZE);
    
    vectorFree(vector);
    
    // Corner tests
    Vector *vector2 = createVector(0);
    assert(vector2 != NULL);
    assert(vector2->elem_size == 0);
    vectorFree(vector2);
    
    printf("Тесты createVector пройдены!\n");
    return 0;
}

int test_append_vector_item() {
    printf("Тестируем appendVectorItem...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int value = 42;
    int result = appendVectorItem(vector, &value);
    assert(result == 0);
    assert(vector->size == 1);
    assert(*(int*)getVectorItem(vector, 0) == 42);
    
    int value2 = 24;
    result = appendVectorItem(vector, &value2);
    assert(result == 0);
    assert(vector->size == 2);
    assert(*(int*)getVectorItem(vector, 1) == 24);
    
    // Тест автоматического resize
    for (int i = 0; i < 20; i++) {
        appendVectorItem(vector, &i);
    }
    assert(vector->size == 22);
    
    vectorFree(vector);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(appendVectorItem(NULL, &value) == -1);
    // Тестирование с пустым элементом
    Vector *vector2 = createVector(sizeof(int));
    assert(appendVectorItem(vector2, NULL) == -1);
    vectorFree(vector2);
    
    printf("Тесты appendVectorItem пройдены!\n");
    return 0;
}

int test_get_vector_item() {
    printf("Тестируем getVectorItem...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int values[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        appendVectorItem(vector, &values[i]);
    }
    
    assert(*(int*)getVectorItem(vector, 0) == 10);
    assert(*(int*)getVectorItem(vector, 1) == 20);
    assert(*(int*)getVectorItem(vector, 2) == 30);
    assert(*(int*)getVectorItem(vector, 3) == 40);
    
    vectorFree(vector);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(getVectorItem(NULL, 0) == NULL);
    // Тестирование с неправильным индексом
    Vector *vector2 = createVector(sizeof(int));
    assert(getVectorItem(vector2, 0) == NULL);
    appendVectorItem(vector2, &values[0]);
    assert(getVectorItem(vector2, 1) == NULL);
    vectorFree(vector2);
    
    printf("Тесты getVectorItem пройдены!\n");
    return 0;
}

int test_set_vector_item() {
    printf("Тестируем setVectorItem...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        appendVectorItem(vector, &values[i]);
    }
    
    int new_value = 99;
    int result = setVectorItem(vector, 1, &new_value);
    assert(result == 0);
    assert(*(int*)getVectorItem(vector, 1) == 99);
    
    vectorFree(vector);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(setVectorItem(NULL, 0, &new_value) == -1);
    // Тестирование с пустым значением
    Vector *vector2 = createVector(sizeof(int));
    appendVectorItem(vector2, &values[0]);
    assert(setVectorItem(vector2, 0, NULL) == -1);
    // Тестирование с неправильным индексом
    assert(setVectorItem(vector2, 1, &new_value) == -1);
    vectorFree(vector2);
    
    printf("Тесты setVectorItem пройдены!\n");
    return 0;
}

int test_pop_vector_item() {
    printf("Тестируем popVectorItem...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        appendVectorItem(vector, &values[i]);
    }
    
    // Pop middle element
    int *popped = (int*)popVectorItem(vector, 2);
    assert(*popped == 30);
    free(popped);
    assert(vector->size == 4);
    assert(*(int*)getVectorItem(vector, 2) == 40);
    
    // Pop first element
    popped = (int*)popVectorItem(vector, 0);
    assert(*popped == 10);
    free(popped);
    assert(vector->size == 3);
    assert(*(int*)getVectorItem(vector, 0) == 20);
    
    // Pop last element
    popped = (int*)popVectorItem(vector, 2);
    assert(*popped == 50);
    free(popped);
    assert(vector->size == 2);
    assert(*(int*)getVectorItem(vector, 1) == 40);
    
    vectorFree(vector);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(popVectorItem(NULL, 0) == NULL);
    // Тестирование с неправильным индексом
    Vector *vector2 = createVector(sizeof(int));
    assert(popVectorItem(vector2, 0) == NULL);
    appendVectorItem(vector2, &values[0]);
    assert(popVectorItem(vector2, 1) == NULL);
    vectorFree(vector2);
    
    printf("Тесты popVectorItem пройдены!\n");
    return 0;
}

int test_find_vector_item() {
    printf("Тестируем findVectorItem...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int values[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        appendVectorItem(vector, &values[i]);
    }
    
    int searchValue = 30;
    long int index = findVectorItem(vector, &searchValue, intEquals);
    assert(index == 2);
    
    searchValue = 50;
    index = findVectorItem(vector, &searchValue, intEquals);
    assert(index == -1);
    
    vectorFree(vector);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(findVectorItem(NULL, &searchValue, intEquals) == -1);
    // Тестирование с пустым значением
    Vector *vector2 = createVector(sizeof(int));
    assert(findVectorItem(vector2, NULL, intEquals) == -1);
    // Тестирование с пустым компаратором
    appendVectorItem(vector2, &searchValue);
    assert(findVectorItem(vector2, &searchValue, NULL) == -1);
    vectorFree(vector2);
    
    printf("Тесты findVectorItem пройдены!\n");
    return 0;
}

int test_vector_free() {
    printf("Тестируем vectorFree...\n");
    
    // Functional tests
    Vector *vector = createVector(sizeof(int));
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        appendVectorItem(vector, &values[i]);
    }
    
    int result = vectorFree(vector);
    assert(result == 0);
    
    // Corner tests
    // Тестирование с пустым вектором
    assert(vectorFree(NULL) == -1);
    
    printf("Тесты vectorFree пройдены!\n");
    return 0;
}

int main() {
    printf("Запуск тестов Vector...\n\n");
    
    test_create_vector();
    test_append_vector_item();
    test_get_vector_item();
    test_set_vector_item();
    test_pop_vector_item();
    test_find_vector_item();
    test_vector_free();
    
    printf("\nВсе тесты Vector пройдены!\n");
    return 0;
}