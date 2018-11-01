int main(void) {
    int* p;
    if(1) {
        int a[10];
        p = a;
    }
    p[0] = 1;
}

/**
 * @file test-0249.c
 *
 * @brief a regression test for scope of automatic variables
 *
 * - taken from https://sv-comp.sosy-lab.org/2019/results/sv-benchmarks/c/memsafety-ext3/scopes5_false-valid-deref.c
 *
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
