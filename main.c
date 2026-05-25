#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "index/index.h"
#include "index/search.h"

void runExperiments(TreeType type, const char* data_path, const char* queries_path, int doc_limit);

static TreeType parseType(const char* str) {
    if (strcmp(str, "rb") == 0) return TREE_RB;
    if (strcmp(str, "btree") == 0) return TREE_BTREE;
    return TREE_AVL;
}

static const char* typeName(TreeType type) {
    switch (type) {
        case TREE_AVL:   return "avl";
        case TREE_RB:    return "rb";
        case TREE_BTREE: return "btree";
        default:         return "unknown";
    }
}

static void runSearch(TreeType type, const char* idx_path,
                      const char* query, int json_out) {
    Index* idx = loadIndex(idx_path, type);
    if (!idx) { 
        fprintf(stderr, "Failed to load index: %s\n", idx_path); 
        exit(1); 
    }

    SearchResults* sr = search(idx, query);
    if (json_out) printResultsJSON(sr);
    else          printResultsText(sr);

    freeSearchResults(sr);
    freeIndex(idx);
}

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s index      --type=<avl|rb|btree> [--data=PATH] [--index=PATH]\n"
        "  %s search     --type=<avl|rb|btree> [--index=PATH] [--json] \"query\"\n"
        "  %s experiment --type=<avl|rb|btree> [--limit=N]\n",
        prog, prog, prog);
}

int main(int argc, char* argv[]) {
    if (argc < 3) { usage(argv[0]); return 1; }

    const char* mode = argv[1];
    TreeType    type = TREE_AVL;
    const char* data_path = "data/processed/docs.jsonl";
    char        idx_path[512] = {0};
    int         json_out = 0;
    const char* query    = NULL;
    int         limit    = 50000;

    for (int i = 2; i < argc; i++) {
        if      (strncmp(argv[i], "--type=",  7) == 0) type = parseType(argv[i] + 7);
        else if (strncmp(argv[i], "--data=",  7) == 0) data_path = argv[i] + 7;
        else if (strncmp(argv[i], "--index=", 8) == 0) strncpy(idx_path, argv[i] + 8, sizeof(idx_path) - 1);
        else if (strncmp(argv[i], "--limit=", 8) == 0) limit = atoi(argv[i] + 8);
        else if (strcmp(argv[i], "--json")    == 0)    json_out = 1;
        else if (argv[i][0] != '-')                    query = argv[i];
    }

    if (idx_path[0] == '\0') {
        snprintf(idx_path, sizeof(idx_path), "data/index_%s.txt", typeName(type));
    }

    if (strcmp(mode, "index") == 0) {
        runIndex(type, data_path, idx_path);
    } else if (strcmp(mode, "search") == 0) {
        if (!query) { fprintf(stderr, "No query provided\n"); return 1; }
        runSearch(type, idx_path, query, json_out);
    } else if (strcmp(mode, "experiment") == 0) {
        runExperiments(type, data_path, "data/queries.txt", limit);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        usage(argv[0]);
        return 1;
    }
    return 0;
}