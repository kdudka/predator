#include <verifier-builtins.h>
#include <stdlib.h>

int main() {
    struct item {
        struct item *next;
    } *ptr = NULL;

    for(;;) {
        void *data = ptr;
        ptr = malloc(sizeof *ptr);
        if (!ptr)
            // OOM
            return -1;

        ptr->next = data;
        __VERIFIER_plot("test-0001-snapshot");
    }

    return 0;
}

/**
 * @file test-0001.c
 *
 * @brief infinite loop creating a SLL
 *
 * - it plots a heap graph per each iteration
 *
 * - thanks to the SLS abstraction, the program reaches its end
 *   during the symbolic execution (though it can't happen in real
 *   world as long as there is some memory available)
 *
 * - junk is properly detected since there is some memory
 *   allocated, but nothing is free'd afterwards
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
