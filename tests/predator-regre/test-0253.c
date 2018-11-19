#include <stdlib.h>     // exit(), malloc(), free()

int *f(int *p) {
    int i;
    *p = 1;
    if(i)
      exit(1);          // end of program
    return p;
}

int main(int argc, char *argv[]) {
    int *p = malloc(100);
    p = f(p);
    free(p);
    return p==0;
}

/**
 * @file test-0253.c
 *
 * @brief test for reporting correct variables (name vs level)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
