#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"
#include "fuzzy.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s <query> <max_distance>\n", argv[0]);
        return 1;
    }

    const char* query = argv[1];
    int max_distance = atoi(argv[2]);

    const char* index_path = "data/kasik_index_btree.txt";

    Index* idx = loadIndex(index_path, TREE_BTREE);

    if (!idx) {
        printf("Cannot load index: %s\n", index_path);
        return 1;
    }

    SearchResults* result = fuzzySearch(idx, query, max_distance);

    printResultsText(result);

    freeSearchResults(result);
    freeIndex(idx);

    return 0;
}