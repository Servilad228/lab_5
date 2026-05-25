// Тут все на ваше усмотрение, просто переиспользуйте код из предыдущих лабораторных, если он вам подходит. Главное, чтобы интерфейс был таким же, как в avl и btree, чтобы не менять код в main.c
#include "search.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
static int comparePostingById(const void* a, const void* b) {
    const PostingEntry* pa = (const PostingEntry*)a;
    const PostingEntry* pb = (const PostingEntry*)b;
    return pa->doc_id - pb->doc_id;
}
//пересечение n списков AND-семантика
Vector* intersectPostings(Vector** lists, int n) {
    if (!lists || n == 0) return NULL;

    Vector* result = createPostingList();
    if (!result) return NULL;

    if (n == 1) {
        for (size_t i = 0; i < lists[0]->size; i++) {
            PostingEntry* e = getVectorItem(lists[0], i);
            appendPosting(result, e->doc_id, e->title);
        }
        return result;
    }

    //ищем самый короткий
    int shortest = 0;
    for (int i = 1; i < n; i++) {
        if (lists[i]->size < lists[shortest]->size) {
            shortest = i;
        }
    }

    //текущии позиции списка
    size_t* idx = calloc(n, sizeof(size_t));
    if (!idx) {
        vectorFree(result);
        return NULL;
    }

    for (size_t i = 0; i < lists[shortest]->size; i++) {
        PostingEntry* entry = getVectorItem(lists[shortest], i);
        int target = entry->doc_id;
        int found = 1;

        //есть ли вдругих?
        for (int j = 0; j < n; j++) {
            if (j == shortest) continue;

            //идем до нужного дока
            while (idx[j] < lists[j]->size) {
                PostingEntry* e = getVectorItem(lists[j], idx[j]);
                if (e->doc_id >= target) break;
                idx[j]++;
            }

            if (idx[j] >= lists[j]->size) {
                free(idx);
                return result;
            }

            //doc_id не совпал значит есть не во всех
            PostingEntry* e = getVectorItem(lists[j], idx[j]);
            if (e->doc_id != target) {
                found = 0;
                break;
            }
        }

        //есть-доьавляем
        if (found) {
            appendPosting(result, entry->doc_id, entry->title);
        }
    }

    free(idx);
    return result;
}

//поиск по запросу
SearchResults* search(Index* idx, const char* query) {
    SearchResults* sr = malloc(sizeof(SearchResults));
    if (!sr) return NULL;

    sr->results = NULL;
    sr->total = 0;
    sr->time_ms = 0;

    if (!idx || !query) return sr;

    clock_t start = clock();

    char* copy = strdup(query);
    if (!copy) return sr;

    //разбивае  по слова
    char* tokens[MAX_TOKENS];
    int count = 0;

    char* word = strtok(copy, " ");
    while (word && count < MAX_TOKENS) {
        tokens[count] = word;
        count++;
        word = strtok(NULL, " ");
    }

    if (count == 0) {
        free(copy);
        return sr;
    }

    //постин лист для каждого слова
    Vector** lists = malloc(count * sizeof(Vector*));
    if (!lists) {
        free(copy);
        return sr;
    }

    int valid = 0;
    for (int i = 0; i < count; i++) {
        Vector* pl = lookupTerm(idx, tokens[i]);
        if (pl) {
            lists[valid] = pl;
            valid++;
        }
    }

    //пересекаем если все слова найдены
    if (valid == count) {
        Vector* inter = intersectPostings(lists, valid);

        clock_t end = clock();
        sr->time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;

        if (inter && inter->size > 0) {
            qsort(inter->data, inter->size, inter->elem_size, comparePostingById);

            sr->total = (int)inter->size;

            //первые 10
            int top = inter->size > 10 ? 10 : (int)inter->size;
            sr->results = createPostingList();

            for (int i = 0; i < top; i++) {
                PostingEntry* e = getVectorItem(inter, i);
                appendPosting(sr->results, e->doc_id, e->title);
            }

            vectorFree(inter);
        } else {
            clock_t end = clock();
            sr->time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            if (inter) vectorFree(inter);
        }
    } else {
        clock_t end = clock();
        sr->time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    }

    free(lists);
    free(copy);
    return sr;
}

void printResultsText(const SearchResults* sr) {
    if (!sr) return;

    printf("Time: %.2f ms | Found: %d documents\n\n", sr->time_ms, sr->total);

    if (sr->results && sr->results->size > 0) {
        for (size_t i = 0; i < sr->results->size; i++) {
            PostingEntry* e = getVectorItem(sr->results, i);
            printf("%2zu. [id=%d] %s\n", i + 1, e->doc_id, e->title);
        }
    } else {
        printf("Nothing found\n");
    }
}

void printResultsJSON(const SearchResults* sr) {
    if (!sr) return;

    printf("{\n");
    printf("\"time_ms\": %.2f,\n", sr->time_ms);
    printf("\"total\": %d,\n", sr->total);
    printf("\"results\": [\n");

    if (sr->results) {
        for (size_t i = 0; i < sr->results->size; i++) {
            PostingEntry* e = getVectorItem(sr->results, i);
            printf("{\"doc_id\": %d, \"title\": \"%s\"}", e->doc_id, e->title);
            if (i < sr->results->size - 1) printf(",");
            printf("\n");
        }
    }

    printf("]\n");
    printf("}\n");
}

void freeSearchResults(SearchResults* sr) {
    if (!sr) return;
    if (sr->results) vectorFree(sr->results);
    free(sr);
}

// Vector* lookupTerm(const Index* idx, const char* term) {
//     return NULL;
// }

// int main(void) {
//     Vector* list1 = createPostingList();
//     appendPosting(list1, 1, "Doc one");
//     appendPosting(list1, 2, "Doc two");
//     appendPosting(list1, 3, "Doc three");
//     appendPosting(list1, 5, "Doc five");

//     Vector* list2 = createPostingList();
//     appendPosting(list2, 2, "Doc two");
//     appendPosting(list2, 3, "Doc three");
//     appendPosting(list2, 5, "Doc five");

//     Vector* list3 = createPostingList();
//     appendPosting(list3, 1, "Doc one");
//     appendPosting(list3, 3, "Doc three");
//     appendPosting(list3, 5, "Doc five");

//     Vector* lists[] = {list1, list2, list3};
//     Vector* result = intersectPostings(lists, 3);

//     printf("Intersection result:\n");
//     for (size_t i = 0; i < result->size; i++) {
//         PostingEntry* e = getVectorItem(result, i);
//         printf("  [id=%d] %s\n", e->doc_id, e->title);
//     }

//     vectorFree(list1);
//     vectorFree(list2);
//     vectorFree(list3);
//     vectorFree(result);
//     printf("Done.\n");
//     return 0;
// }