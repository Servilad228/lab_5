#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "index.h"
#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"

#define LINE_BUFFER 8192

/*CORE INDEX*/

Index* createIndex(TreeType type) {
    Index* idx = malloc(sizeof(Index));
    if (!idx) return NULL;

    idx->type = type;

    switch (type) {
        case TREE_AVL:   idx->tree = createAVLTree(); break;
        case TREE_RB:    idx->tree = createRBTree(); break;
        case TREE_BTREE: idx->tree = createBTree(); break;
    }

    return idx;
}

void insertTerm(Index* idx, const char* term, int doc_id, const char* title) {
    if (!idx || !term) return;

    switch (idx->type) {
        case TREE_AVL:
            avlInsert((AVLTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_RB:
            rbInsert((RBTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_BTREE:
            btreeInsert((BTree*)idx->tree, term, doc_id, title);
            break;
    }
}

Vector* lookupTerm(const Index* idx, const char* term) {
    if (!idx || !term) return NULL;

    switch (idx->type) {
        case TREE_AVL:
            return avlSearch((AVLTree*)idx->tree, term);
        case TREE_RB:
            return rbSearch((RBTree*)idx->tree, term);
        case TREE_BTREE:
            return btreeSearch((BTree*)idx->tree, term);
    }
    return NULL;
}

void indexDocument(Index* idx, int doc_id, const char* title,
                   const char** tokens, int n_tokens)
{
    for (int i = 0; i < n_tokens; i++)
        insertTerm(idx, tokens[i], doc_id, title);
}

void traverseIndex(
    const Index* idx,
    void (*visit)(const char*, Vector*, void*),
    void* ctx)
{
    switch (idx->type) {
        case TREE_AVL:
            avlTraverse((AVLTree*)idx->tree, visit, ctx);
            break;
        case TREE_RB:
            rbTraverse((RBTree*)idx->tree, visit, ctx);
            break;
        case TREE_BTREE:
            btreeTraverse((BTree*)idx->tree, visit, ctx);
            break;
    }
}

/*  SAVE / LOAD  */


static void saveCallback(const char* key, Vector* postings, void* ctx) {
    FILE* f = ctx;

    for (size_t i = 0; i < postings->size; i++) {
        PostingEntry* e = getVectorItem(postings, i);
        fprintf(f, "%s|%d|%s\n", key, e->doc_id, e->title);
    }
}

void saveIndex(const Index* idx, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) { perror("saveIndex"); return; }

    traverseIndex(idx, saveCallback, f);

    fclose(f);
}

Index* loadIndex(const char* path, TreeType type) {
    FILE* f = fopen(path, "r");
    if (!f) { perror("loadIndex"); return NULL; }

    Index* idx = createIndex(type);
    char line[LINE_BUFFER];

    while (fgets(line, sizeof(line), f)) {
        char* term = strtok(line, "|");
        char* doc  = strtok(NULL, "|");
        char* title = strtok(NULL, "\n");

        if (term && doc && title)
            insertTerm(idx, term, atoi(doc), title);
    }

    fclose(f);
    return idx;
}

void freeIndex(Index* idx) {
    if (!idx) return;

    switch (idx->type) {
        case TREE_AVL:   freeAVLTree((AVLTree*)idx->tree); break;
        case TREE_RB:    freeRBTree((RBTree*)idx->tree); break;
        case TREE_BTREE: freeBTree((BTree*)idx->tree); break;
    }

    free(idx);
}

/*JSONL PARSER*/


static int parseJsonLine(
    char* line,
    int* doc_id,
    char* title,
    const char*** tokens_out,
    int* token_count_out)
{
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


/*RUN INDEX */

void runIndex(TreeType type,
              const char* data_path,
              const char* idx_path)
{
    printf("Indexing from %s\n", data_path);

    clock_t start = clock();

    Index* idx = createIndex(type);
    FILE* f = fopen(data_path, "r");
    if (!f) { perror("open docs"); exit(1); }

    char line[LINE_BUFFER];

    while (fgets(line, sizeof(line), f)) {

        int doc_id;
        char title[MAX_TITLE_LEN];
        const char** tokens;
        int token_count;

        if (parseJsonLine(line, &doc_id, title,
                          &tokens, &token_count) != 0)
            continue;

        indexDocument(idx, doc_id, title,
                      tokens, token_count);

        for (int i = 0; i < token_count; i++)
            free((void*)tokens[i]);
    }

    fclose(f);

    saveIndex(idx, idx_path);

    double t = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("Saved to %s\nTime: %.2f sec\n", idx_path, t);

    freeIndex(idx);
}