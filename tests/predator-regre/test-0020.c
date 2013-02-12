#include <verifier-builtins.h>
#include <stdlib.h>

// reaching this function indicates we are on a wrong way
void trigger_null_dereference(void)
{
    void **ptr = NULL;
    ptr = (void **) *ptr;
}

void test__cond_reasoning__with_zero_lookback_level() {
    // obtain an unknown value
    void *who_knows = &who_knows;
    if (__VERIFIER_nondet_int())
        who_knows = NULL;

    // who_knows is UNKNOWN at this point
    if (who_knows) {

        // who_knows should be seen as TRUE at this point, fail otherwise
        if (!who_knows)
            trigger_null_dereference();
    } else {

        // who_knows should be seen as FALSE at this point, fail otherwise
        if (who_knows)
            trigger_null_dereference();
    }
}

int main() {
    test__cond_reasoning__with_zero_lookback_level();
    return 0;
}

/**
 * @file test-0020.c
 *
 * @brief regression test of reasoning about unknown bool values
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
