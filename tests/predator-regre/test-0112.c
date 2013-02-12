#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *next;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (!pi)
        abort();

    pi->next = NULL;
    return pi;
}

struct item* create_sll(void)
{
    struct item *sll = alloc_or_die();
    struct item *now = sll;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; 0 < i; ++i) {
        now->next = alloc_or_die();
        now = now->next;
    }

    return sll;
}

int main()
{
    struct item *sll = create_sll();
    __VERIFIER_plot(NULL);
    (void) sll;

    return 0;
}

/**
 * @file test-0112.c
 *
 * @brief hello-world, once used as the most trivial example for symjoin
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
