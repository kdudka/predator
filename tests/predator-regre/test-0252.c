#include <stdlib.h>     // exit(), malloc(), free()

int *f(int *p) {
    int i;
    int *pp = p;
    *pp = 1;
    if(i)
      exit(1);          // end of program
    return pp;
}

int main(int argc, char *argv[]) {
    int *p = malloc(100);
    p = f(p);
    free(p);
    return p==0;
}

/**
 * @file test-0252.c
 *
 * @brief test for reporting leaks at exit() call
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
