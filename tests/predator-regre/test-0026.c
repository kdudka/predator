#include <verifier-builtins.h>

int main() {
    void **p1;
    void **p2;

    if (p1 == p2) {
        __VERIFIER_plot("test26-00");
        if (p1 != p2) {
            *p2 = (void *)0;
        } else {
            __VERIFIER_plot("test26-02");
        }
    } else {
        __VERIFIER_plot("test26-01");
    }

    return 0;
}

/**
 * @file test-0026.c
 *
 * @brief alternation of test-0025.c that plots some heap graphs
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
