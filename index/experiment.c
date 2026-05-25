#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

#include "index.h"
#include "search.h"

#define LINE_BUFFER 8192

// Парсер JSONL для экспериментов
static int parseJsonLineExp(char* line, int* doc_id, char* title, const char*** tokens_out, int* token_count_out) {
    char* id_pos = strstr(line, "\"doc_id\"");
    char* title_pos = strstr(line, "\"title\"");
    char* tokens_pos = strstr(line, "\"tokens\"");
    if (!id_pos || !title_pos || !tokens_pos) return -1;

    *doc_id = atoi(strchr(id_pos, ':') + 1);

    char* tstart = strchr(title_pos, ':') + 1;
    while (*tstart == ' ' || *tstart == '\"') tstart++;
    char* tend = strchr(tstart, '\"');
    size_t len = tend - tstart;
    if (len >= MAX_TITLE_LEN) len = MAX_TITLE_LEN - 1;
    strncpy(title, tstart, len);
    title[len] = '\0';

    char* bstart = strchr(tokens_pos, '[') + 1;
    char* bend = strchr(tokens_pos, ']');
    static const char* tokens[256];
    int count = 0;
    char* p = bstart;

    while (p < bend) {
        while (*p == ' ' || *p == ',') p++;
        if (*p == '\"') {
            p++;
            char* ts = p;
            char* te = strchr(ts, '\"');
            size_t tlen = te - ts;
            char* tok = malloc(tlen + 1);
            strncpy(tok, ts, tlen);
            tok[tlen] = '\0';
            tokens[count++] = tok;
            p = te + 1;
        } else break;
    }
    *tokens_out = tokens;
    *token_count_out = count;
    return 0;
}

void runExperiments(TreeType type, const char* data_path, const char* queries_path, int doc_limit) {
    printf("=== Запуск эксперимента (Документов: %d) ===\n", doc_limit);
    
    // --- 1. Индексация и Память ---
    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_SELF, &usage_start);
    clock_t start_idx = clock();

    Index* idx = createIndex(type);
    FILE* f = fopen(data_path, "r");
    if (!f) { perror("open docs"); return; }

    char line[LINE_BUFFER];
    int indexed_count = 0;

    while (fgets(line, sizeof(line), f) && indexed_count < doc_limit) {
        int doc_id;
        char title[MAX_TITLE_LEN];
        const char** tokens;
        int token_count;

        if (parseJsonLineExp(line, &doc_id, title, &tokens, &token_count) == 0) {
            indexDocument(idx, doc_id, title, tokens, token_count);
            for (int i = 0; i < token_count; i++) free((void*)tokens[i]);
            indexed_count++;
        }
    }
    fclose(f);

    clock_t end_idx = clock();
    getrusage(RUSAGE_SELF, &usage_end);

    double time_idx = (double)(end_idx - start_idx) / CLOCKS_PER_SEC;
    long mem_kb = usage_end.ru_maxrss; 
    
    // Для macOS ru_maxrss возвращается в байтах, делим на 1024*1024 чтобы получить МБ
    printf("Время индексации: %.3f сек\n", time_idx);
    printf("Использование памяти: %ld MB\n", mem_kb / (1024 * 1024));

    // --- 2. Замеры поиска ---
    FILE* qf = fopen(queries_path, "r");
    if (!qf) {
        printf("Файл с запросами не найден. Для теста поиска сгенерируйте их: python generate_queries.py\n");
        freeIndex(idx);
        return;
    }

    char query[256];
    int query_count = 0;
    double total_search_time_ms = 0;

    while (fgets(query, sizeof(query), qf) && query_count < 1000) {
        query[strcspn(query, "\r\n")] = 0; // Удаляем перенос строки
        
        clock_t s_start = clock();
        SearchResults* sr = search(idx, query);
        clock_t s_end = clock();
        
        total_search_time_ms += ((double)(s_end - s_start) / CLOCKS_PER_SEC) * 1000.0;
        freeSearchResults(sr);
        query_count++;
    }
    fclose(qf);

    if (query_count > 0) {
        printf("Среднее время на 1 запрос (из %d шт): %.4f мс\n", query_count, total_search_time_ms / query_count);
    }
    printf("==========================================\n\n");

    freeIndex(idx);
}