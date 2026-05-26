#include "fuzzy.h"
#include "../levenshtein/levenshtein.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TOP_K 10
#define MAX_QUERY_TOKENS 32

typedef struct {
    const char* term;
    int max_distance;
    Vector* candidates;
} FuzzyCtx;

static void collectCandidate(const char* key, Vector* postings, void* ctx) {
    FuzzyCtx* fctx = ctx;

    // сравниваем слово запроса с очередным ключом индекса
    int dist = levenshteinDistance(fctx->term, key);

    if (dist <= fctx->max_distance) {
        FuzzyCandidate candidate;

        strncpy(candidate.term, key, sizeof(candidate.term) - 1);
        candidate.term[sizeof(candidate.term) - 1] = '\0';

        candidate.distance = dist;
        candidate.postings = postings;

        appendVectorItem(fctx->candidates, &candidate);
    }
}

Vector* fuzzyFindCandidates(Index* idx, const char* term, int max_distance) {
    if (!idx || !term) {
        return NULL;
    }

    Vector* candidates = createVector(sizeof(FuzzyCandidate));

    FuzzyCtx ctx;
    ctx.term = term;
    ctx.max_distance = max_distance;
    ctx.candidates = candidates;

    // обходим всё дерево и собираем похожие ключи
    traverseIndex(idx, collectCandidate, &ctx);

    return candidates;
}

static int findResultByDocId(Vector* results, int doc_id) {
    for (size_t i = 0; i < results->size; i++) {
        SearchResult* r = getVectorItem(results, i);

        if (r->doc_id == doc_id) {
            return (int)i;
        }
    }

    return -1;
}

static void addPostingsToResults(Vector* results, FuzzyCandidate* candidate) {
    // чем меньше расстояние, тем больше вклад кандидата в score
    int score_add = 10 - candidate->distance;

    for (size_t i = 0; i < candidate->postings->size; i++) {
        PostingEntry* entry = getVectorItem(candidate->postings, i);

        int pos = findResultByDocId(results, entry->doc_id);

        if (pos >= 0) {
            // если документ уже найден другим похожим словом, усиливаем его score
            SearchResult* existing = getVectorItem(results, pos);
            existing->score += score_add; // то есть ранжирование
        } else {
            SearchResult result;

            result.doc_id = entry->doc_id;
            strncpy(result.title, entry->title, MAX_TITLE_LEN - 1);
            result.title[MAX_TITLE_LEN - 1] = '\0';
            result.score = score_add;

            appendVectorItem(results, &result);
        }
    }
}

static int compareResults(const void* a, const void* b) {
    const SearchResult* ra = a;
    const SearchResult* rb = b;

    return rb->score - ra->score;
}

SearchResults* fuzzySearch(Index* idx, const char* query, int max_distance) {
    clock_t start = clock();

    SearchResults* sr = malloc(sizeof(SearchResults));
    sr->results = createVector(sizeof(SearchResult));
    sr->total = 0;
    sr->time_ms = 0.0;

    char* copy = malloc(strlen(query) + 1);
    strcpy(copy, query);

    char* token = strtok(copy, " ");
    int token_count = 0;

    while (token && token_count < MAX_QUERY_TOKENS) {
        // для каждого слова запроса ищем похожие термы в индексе
        Vector* candidates = fuzzyFindCandidates(idx, token, max_distance);

        for (size_t i = 0; i < candidates->size; i++) {
            FuzzyCandidate* candidate = getVectorItem(candidates, i);
            addPostingsToResults(sr->results, candidate);
        }

        vectorFree(candidates);

        token_count++;
        token = strtok(NULL, " ");
    }

    sr->total = (int)sr->results->size;

    // сначала показываем документы с наибольшим score
    qsort(
        sr->results->data,
        sr->results->size,
        sizeof(SearchResult),
        compareResults
    );

    if (sr->results->size > TOP_K) {
        sr->results->size = TOP_K;
    }

    sr->time_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;

    free(copy);

    return sr;
}

void printFuzzyCandidates(Vector* candidates) {
    if (!candidates) {
        return;
    }

    for (size_t i = 0; i < candidates->size; i++) {
        FuzzyCandidate* c = getVectorItem(candidates, i);

        printf(
            "%s | distance = %d | docs = %zu\n",
            c->term,
            c->distance,
            c->postings->size
        );
    }
}

void freeFuzzyCandidates(Vector* candidates) {
    vectorFree(candidates);
}