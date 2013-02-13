#include <verifier-builtins.h>
#include <stdlib.h>

struct item {
    struct item *null;
    struct item *next;
    struct item *data;
};

struct item* alloc_or_die(void)
{
    struct item *pi = malloc(sizeof(*pi));
    if (!pi)
        abort();

    pi->null = NULL;
    pi->data = malloc(sizeof(struct item));
#if 1
    if (!pi->data)
        abort();
#endif
    return pi;
}

struct item* create_sll(void)
{
    struct item *sll = alloc_or_die();
    struct item *now = sll;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        now->next = alloc_or_die();
        now->next->next = NULL;
        now = now->next;
    }

    return sll;
}

void destroy_sll(struct item **pSll)
{
    struct item *sll = *pSll;
    while (sll) {
        struct item *next = sll->next;

        // check if the SLS properly holds the 'next' value at its end
        *pSll = sll->null;

        free(sll->data);
        free(sll);
        sll = next;
    }
}

int main()
{
    struct item *sll = create_sll();
    __VERIFIER_plot("01-sll-ready");

    destroy_sll(&sll);
    __VERIFIER_plot("03-sll-gone");

    if (sll) {
        // assertion failed, yell now
        sll = NULL;
        *sll = *sll->next;
    }

    return 0;
}

/**
 * @file test-0051.c
 *
 * @brief SLS, each item owns its own on heap allocated object
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
