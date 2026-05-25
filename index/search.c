#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "search.h"

#define MAX_TOKENS 32
#define TOP_K 10

static int cmpDoc(const void* a, const void* b) {
    return ((PostingEntry*)a)->doc_id -
           ((PostingEntry*)b)->doc_id;
}

static int cmpRes(const void* a, const void* b) {
    return ((SearchResult*)b)->score -
           ((SearchResult*)a)->score;
}

Vector* intersectPostings(Vector** lists, int n) {
    if (n == 0) return NULL;
    if (n == 1) return clonePostingList(lists[0]);

    for (int i = 0; i < n; i++)
        qsort(lists[i]->data, lists[i]->size,
              sizeof(PostingEntry), cmpDoc);

    Vector* result = createPostingList();
    Vector* base = lists[0];

    for (size_t i = 0; i < base->size; i++) {
        PostingEntry* e = getVectorItem(base, i);
        int ok = 1;

        for (int j = 1; j < n; j++) {
            int found = 0;
            for (size_t k = 0; k < lists[j]->size; k++) {
                PostingEntry* x = getVectorItem(lists[j], k);
                if (x->doc_id == e->doc_id) { found = 1; break; }
            }
            if (!found) { ok = 0; break; }
        }

        if (ok) appendPosting(result, e->doc_id, e->title);
    }

    return result;
}

SearchResults* search(Index* idx, const char* query) {

    clock_t start = clock();

    char* copy = strdup(query);
    char* tok = strtok(copy, " ");

    Vector* lists[MAX_TOKENS];
    int count = 0;

    while (tok && count < MAX_TOKENS) {
        Vector* v = lookupTerm(idx, tok);
        if (!v) {
            free(copy);
            SearchResults* sr = malloc(sizeof(SearchResults));
            sr->results = createVector(sizeof(SearchResult));
            sr->total = 0;
            sr->time_ms = 0;
            return sr;
        }
        lists[count++] = v;
        tok = strtok(NULL, " ");
    }

    Vector* inter = intersectPostings(lists, count);

    SearchResults* sr = malloc(sizeof(SearchResults));
    sr->results = createVector(sizeof(SearchResult));
    sr->total = inter ? inter->size : 0;

    if (inter) {
        for (size_t i = 0; i < inter->size; i++) {
            PostingEntry* e = getVectorItem(inter, i);
            SearchResult r;
            r.doc_id = e->doc_id;
            strncpy(r.title, e->title, MAX_TITLE_LEN);
            r.score = count * 10;
            appendVectorItem(sr->results, &r);
        }

        qsort(sr->results->data,
              sr->results->size,
              sizeof(SearchResult),
              cmpRes);

        if (sr->results->size > TOP_K)
            sr->results->size = TOP_K;

        vectorFree(inter);
    }

    sr->time_ms =
        ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;

    free(copy);
    return sr;
}

void printResultsText(const SearchResults* sr) {
    printf("Time: %.2f ms | Found: %d\n\n",
           sr->time_ms, sr->total);

    for (size_t i = 0; i < sr->results->size; i++) {
        SearchResult* r = getVectorItem(sr->results, i);
        printf("%zu. [%d] %s\n",
               i+1, r->doc_id, r->title);
    }
}

void printResultsJSON(const SearchResults* sr) {
    printf("{\"time_ms\":%.2f,\"total\":%d}\n",
           sr->time_ms, sr->total);
}

void freeSearchResults(SearchResults* sr) {
    if (!sr) return;
    vectorFree(sr->results);
    free(sr);
}