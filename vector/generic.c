#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generic.h"

// Вспомогательная функция для изменения размера
static bool needToResize(Vector *vector, bool *increase)
{
    // Проверяем потенциальные ошибки
    if (!vector) {
        return false;
    }
    
    // Проверяем, нужно ли увеличивать размер (при добавлении)
    if (vector->size == vector->capacity) {
        if (increase) *increase = true;
        return true;
    }
    
    // Проверяем, нужно ли уменьшать размер (при удалении)
    // Уменьшаем только если размер меньше четверти вместимости и вместимость больше MIN_SIZE
    if (vector->size < vector->capacity / 4 && vector->capacity > MIN_SIZE) {
        if (increase) *increase = false;
        return true;
    }
    
    return false;
}

// Определяем увеличивать размер или уменьшать
static int resize(Vector *vector, bool increase)
{
    // Проверяем потенциальные ошибки
    if (!vector || !vector->data) {
        return -1;
    }
    
    size_t new_capacity;
    if (increase) {
        // Увеличиваем вместимость в 2 раза
        new_capacity = vector->capacity * 2;
    } else {
        // Уменьшаем вместимость в 2 раза, но не меньше MIN_SIZE
        new_capacity = vector->capacity / 2;
        if (new_capacity < MIN_SIZE) {
            new_capacity = MIN_SIZE;
        }
    }
    
    // Изменяем размер памяти
    void *new_data = realloc(vector->data, new_capacity * vector->elem_size);
    if (!new_data) {
        return -1; // Ошибка выделения памяти
    }
    
    vector->data = new_data;
    vector->capacity = new_capacity;
    
    return 0; // Успешное выполнение
}

Vector *createVector(size_t elem_size)
{
    // Выделяем память для структуры Vector
    Vector *vector = malloc(sizeof(Vector));
    if (!vector) {
        return NULL;
    }
    
    // Инициализируем методы структуры
    vector->elem_size = elem_size;
    vector->size = 0;
    vector->capacity = MIN_SIZE;
    
    // Выделяем память для данных
    vector->data = malloc(elem_size * MIN_SIZE);
    if (!vector->data) {
        free(vector); // Освобождаем память структуры при ошибке
        return NULL;
    }
    
    return vector;
}

int appendVectorItem(Vector *vector, void *el)
{
    // Проверяем потенциальные ошибки
    if (!vector || !el) {
        return -1;
    }
    
    
    // Проверяем нужно ли увеличить размер
    if (vector->size == vector->capacity) {
        // Увеличиваем вместимость в 2 раза
        size_t new_capacity = vector->capacity * 2;
        void *new_data = realloc(vector->data, new_capacity * vector->elem_size);
        
        if (!new_data) {
            return -1;
        }
        
        vector->data = new_data;
        vector->capacity = new_capacity;
    }
    
    // Добавляем элемент
    char *target_position = (char*)vector->data + vector->size * vector->elem_size;
    memcpy(target_position, el, vector->elem_size);
    vector->size++;
    
    return 0; // Успешное выполнение
}

void *getVectorItem(Vector *vector, size_t index)
{
    // Проверяем потенциальные ошибки
    if (!vector || !vector->data) {
        return NULL;
    }
    
    if (index >= vector->size) {
        return NULL; // Индекс вне диапазона
    }

    char *elem_position = (char*)vector->data + index * vector->elem_size;
    return (void*)elem_position;
}

int setVectorItem(Vector *vector, size_t index, void *value)
{
    // Проверяем потенциальные ошибки
    if (!vector || !vector->data || !value) {
        return -1;
    }
    
    // Проверяем границы индекса
    if (index >= vector->size) {
        return -1; // Индекс вне диапазона
    }

    char *elem_position = (char*)vector->data + index * vector->elem_size;
    memcpy(elem_position, value, vector->elem_size);

    return 0; // Успешное выполнение
}

void *popVectorItem(Vector *vector, size_t index)
{
    // Проверяем потенциальные ошибки
    if (!vector || !vector->data) {
        return NULL;
    }
    
    // Проверяем границы индекса
    if (index >= vector->size) {
        return NULL; // Индекс вне диапазона
    }
    
    // Выделяем память для копии данных
    void *data = malloc(vector->elem_size);
    if (!data) {
        return NULL; // Ошибка выделения памяти
    }
    
    // Копируем данные удаляемого элемента
    char *elem_position = (char*)vector->data + index * vector->elem_size;
    memcpy(data, elem_position, vector->elem_size);
    
    // Сдвигаем оставшиеся элементы влево
    if (index < vector->size - 1) {
        char *next_elem_position = elem_position + vector->elem_size;
        memmove(elem_position, next_elem_position, 
                (vector->size - index - 1) * vector->elem_size);
    }
    
    // Уменьшаем размер
    vector->size--;
    
    // Проверяем, нужно ли уменьшить вместимость
    bool increase;
    if (needToResize(vector, &increase) && !increase) {
        resize(vector, false);
    }
    
    return data;
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp)
{
    // Проверяем потенциальные ошибки
    if (!vector || !vector->data || !value || !cmp) {
        return -1;
    }
    
    // Проходим по всем элементам вектора
    for (size_t i = 0; i < vector->size; i++) {
        char *elem_position = (char*)vector->data + i * vector->elem_size;
        if (cmp(elem_position, value) == 1) {
            return (long int)i; // Найден элемент
        }
    }
    
    return -1; // Элемент не найден
}

int vectorFree(Vector *vector)
{
    // Проверяем потенциальные ошибки
    if (!vector) {
        return -1;
    }
    
    // Освобождаем данные, если они есть
    if (vector->data) {
        free(vector->data);
    }
    
    // Освобождаем структуру
    free(vector);
    
    return 0; // Успешное выполнение
}
