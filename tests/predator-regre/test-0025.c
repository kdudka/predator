int main() {
    void *p;
    void **p1 = &p;
    void **p2;

    if (p1 == p2) {
        if (p1 != p2) {
            *p2 = (void *)0;
        }
    }

    return 0;
}

/**
 * @file test-0025.c
 *
 * @brief a regression test for inconsistency checking (trivial case)
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
