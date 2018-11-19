#include <stdlib.h>     // exit(), malloc(), free()

int *p;

int main(int argc, char *argv[]) {
    p = malloc(100);
    *p = 1;
    if(argc==1)
        exit(0);
    free(p);
    return p==0;
}

/**
 * @file test-0250.c
 *
 * @brief test for reporting leaks at exit() call
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
