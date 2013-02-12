// more excessive test-case for SLS discovery; Each node contains 9 pointers of
// the same type, split into 3 sub-structures of the same type.  8 of them are
// always NULL, only one pointer is actually used for the SLL binding; it also
// plots some graphs that should better explain what's going on here

#include <verifier-builtins.h>
#include <stdlib.h>

struct item;
struct triple {
    struct item *field_1st;
    struct item *field_2nd;
    struct item *field_3rd;
};

struct item {
    struct triple triple_1st;
    struct triple triple_2nd;
    struct triple triple_3rd;
};

struct item* alloc_and_zero(void)
{
    struct item *pi = calloc(1, sizeof(*pi));
    if (!pi)
        abort();

    return pi;
}

struct item* create_sll(void)
{
    struct item *sll = alloc_and_zero();
    struct item *pi = sll;

    // NOTE: running this on bare metal may cause the machine to swap a bit
    int i;
    for (i = 1; i; ++i) {
        // just choose an arbitrary selector here, we should be able to
        // find the 'next' ptr anywhere
        struct item **next = &pi->triple_3rd.field_1st;

        struct item *pi_new = alloc_and_zero();

        // now test if the deep copy of composite objects works properly
        *pi_new = *pi;

        // finally bind the item
        *next = pi_new;
    }

    return sll;
}

void destroy_sll(struct item **pSll)
{
    struct item *sll = *pSll;
    while (sll) {
        // you have to choose here the same selector as in create_sll()
        *pSll = sll->triple_3rd.field_1st;
        free(sll);
        sll = *pSll;
    }
}

int main()
{
    struct item *sll = create_sll();
    __VERIFIER_plot("01-sll-ready");

    destroy_sll(&sll);
    __VERIFIER_plot("02-sll-gone");

    if (sll) {
        // assertion failed, yell now
        sll = NULL;
        *sll = *sll->triple_1st.field_2nd;
    }

    return 0;
}

/**
 * @file test-0149.c
 *
 * @brief alternation of test-0052, which uses calloc instead of malloc
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
