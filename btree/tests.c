#include "btree.h"

#include <stdio.h>
#include <string.h>

static int tests_failed = 0;

static void check(int condition, const char* message) {
    if (condition) {
        printf("[OK] %s\n", message);
    } else {
        printf("[FAIL] %s\n", message);
        tests_failed++;
    }
}

static void printKey(const char* key, Vector* postings, void* ctx) {
    (void)ctx;
    printf("%-12s : %zu works\n", key, postings->size);
}

static int hasTitle(Vector* postings, const char* title) {
    if (postings == NULL) {
        return 0;
    }

    for (size_t i = 0; i < postings->size; i++) {
        PostingEntry* entry = getVectorItem(postings, i);

        if (strcmp(entry->title, title) == 0) {
            return 1;
        }
    }

    return 0;
}

int main(void) {
    BTree* tree = createBTree();

    check(tree != NULL, "tree created");

    /*
        Ключ = автор.
        PostingEntry = одно произведение автора.
    */

    btreeInsert(tree, "pushkin", 1, "Евгений Онегин");
    btreeInsert(tree, "pushkin", 2, "Капитанская дочка");
    btreeInsert(tree, "pushkin", 3, "Пиковая дама");

    btreeInsert(tree, "gogol", 4, "Мёртвые души");
    btreeInsert(tree, "gogol", 5, "Ревизор");

    btreeInsert(tree, "tolstoy", 6, "Война и мир");
    btreeInsert(tree, "tolstoy", 7, "Анна Каренина");

    btreeInsert(tree, "dostoevsky", 8, "Преступление и наказание");
    btreeInsert(tree, "dostoevsky", 9, "Братья Карамазовы");

    btreeInsert(tree, "chekhov", 10, "Вишнёвый сад");
    btreeInsert(tree, "chekhov", 11, "Чайка");

/*
    После первых 5 уникальных авторов корень заполнен.
    Следующий новый автор вызывает split корня.
*/
    btreeInsert(tree, "lermontov", 12, "Герой нашего времени");

/*
    Остальные авторы проверяют, что после split дерево
    продолжает корректно вставлять и искать ключи.
*/
    btreeInsert(tree, "turgenev", 13, "Отцы и дети");
    btreeInsert(tree, "bulgakov", 14, "Мастер и Маргарита");
    btreeInsert(tree, "nekrasov", 15, "Кому на Руси жить хорошо");

    check(tree->size == 9, "unique authors count is 9");

    Vector* pushkin = btreeSearch(tree, "pushkin");
    check(pushkin != NULL, "pushkin found");
    check(pushkin != NULL && pushkin->size == 3, "pushkin has 3 works");
    check(hasTitle(pushkin, "Евгений Онегин"), "pushkin has Eugene Onegin");
    check(hasTitle(pushkin, "Пиковая дама"), "pushkin has Queen of Spades");

    Vector* gogol = btreeSearch(tree, "gogol");
    check(gogol != NULL, "gogol found");
    check(gogol != NULL && gogol->size == 2, "gogol has 2 works");
    check(hasTitle(gogol, "Ревизор"), "gogol has The Government Inspector");

    Vector* dostoevsky = btreeSearch(tree, "dostoevsky");
    check(dostoevsky != NULL, "dostoevsky found");
    check(dostoevsky != NULL && dostoevsky->size == 2, "dostoevsky has 2 works");

    Vector* mayakovsky = btreeSearch(tree, "mayakovsky");
    check(mayakovsky == NULL, "mayakovsky is not in tree");

    Vector* bulgakov = btreeSearch(tree, "bulgakov");
    check(bulgakov != NULL, "bulgakov found after split");
    check(hasTitle(bulgakov, "Мастер и Маргарита"), "bulgakov title is correct");

    printf("\nTraverse:\n");
    btreeTraverse(tree, printKey, NULL);

    printf("\nTotal unique authors: %d\n", tree->size);

    freeBTree(tree);

    if (tests_failed == 0) {
        printf("\nAll B-tree tests passed.\n");
        return 0;
    }

    printf("\nFailed tests: %d\n", tests_failed);
    return 1;
}