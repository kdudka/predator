#include <stdlib.h>     // exit(), malloc(), free()

int *f(void) {
    int i;
    int *p = malloc(100);
    *p = 1;
    if(i)
      exit(1);          // end program, memory leak 2 reported
    return p;
}

int main(int argc, char *argv[]) {
    int *p = malloc(100);
    *p = 1;             // memory leak 1 reported here (p killed)
    p = f();
    free(p);
    return p==0;
}

/**
 * @file test-0255.c
 *
 * @brief test for reporting leaks at exit() call
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
