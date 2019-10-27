#include <stdlib.h>

int main() {

    int **p = (int **) malloc(sizeof(int*) * 3);
    if (p == NULL) 
        return 0;

    p[2] = (int *) malloc(sizeof(int) * 2);
    if (p[2] == NULL) {
        free(p);
        return 0;
    }

    int **tmp = (int **) realloc(p, sizeof(int*) * 2);
    if (tmp == NULL) {
        free(p[2]);
        free(p);
        return 0;
    }

    free(tmp);
    return 0;
}

/**
 * @file test-0259.c
 *
 * @brief a regression test for memory leaks in realloc()
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
