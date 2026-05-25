// Тут все на ваше усмотрение, просто переиспользуйте код из предыдущих лабораторных, если он вам подходит. Главное, чтобы интерфейс был таким же, как в avl и btree, чтобы не менять код в main.c
#include "search.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define MAX_TOKENS 256

//пересечение n списков AND-семантика
Vector* intersectPostings(Vector** lists, int n) {
    if (!lists || n == 0) return NULL;

    Vector* result = createVector(sizeof(SearchResult));
    if (!result) return NULL;

    if (n == 1) {
        for (size_t i = 0; i < lists[0]->size; i++) {
            PostingEntry* e = getVectorItem(lists[0], i);
            SearchResult sr;
            sr.doc_id = e->doc_id;
            strncpy(sr.title, e->title, MAX_TITLE_LEN - 1);
            sr.title[MAX_TITLE_LEN - 1] = '\0';
            sr.score = 0;
            appendVectorItem(result, &sr);
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

        //есть-добавляем
        if (found) {
            SearchResult sr;
            sr.doc_id = entry->doc_id;
            strncpy(sr.title, entry->title, MAX_TITLE_LEN - 1);
            sr.title[MAX_TITLE_LEN - 1] = '\0';
            sr.score = 0;
            appendVectorItem(result, &sr);
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
            sr->total = (int)inter->size;

            //первые 10
            int top = inter->size > 10 ? 10 : (int)inter->size;
            sr->results = createVector(sizeof(SearchResult));

            for (int i = 0; i < top; i++) {
                SearchResult* e = getVectorItem(inter, i);
                appendVectorItem(sr->results, e);
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
            SearchResult* e = getVectorItem(sr->results, i);
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
            SearchResult* e = getVectorItem(sr->results, i);
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
