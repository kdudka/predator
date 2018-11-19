#include <stdlib.h>     // abort(), malloc(), free()

int main(int argc, char *argv[]) {
    int *p = malloc(100);
    *p = 1;
    if(argc==1)
        abort();
    free(p);
    return p==0;
}

/**
 * @file test-0251.c
 *
 * @brief test for reporting leaks at abort() call
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
