#include <verifier-builtins.h>
#include <stdlib.h>

// return VAL_UNKNOWN of type CL_TYPE_PTR while running symbolic execution
void** synthesize_unknown_ptr_value(void)
{
    int a = 1, b = 2;
    void *pa = &a, *pb = &b;

    return (__VERIFIER_nondet_int())
        ? &pa
        : &pb;
}

// reaching this function indicates we are on a wrong way
void trigger_null_dereference(void)
{
    void **ptr = NULL;
    ptr = (void **) *ptr;
}

void test__cond_reasoning__with_single_lookback_level() {
    // value of PTR is not initialized for now
    void *ptr;
    void **pptr = &ptr;

    // obtain unknown value
    void **const who_knows = synthesize_unknown_ptr_value();

    // who_knows is UNKNOWN at this point
    if (who_knows == pptr) {

        // who_knows should be seen as &PTR at this point, fail otherwise
        *who_knows = NULL;
    } else {

        // now we have explicit info about who_knows != &ptr, fail otherwise
        if (who_knows == &ptr)
            trigger_null_dereference();
        else
            ptr = NULL;
    }

    if (ptr != NULL)
        // something went wrong
        trigger_null_dereference();
}

int main() {
    test__cond_reasoning__with_single_lookback_level();
    return 0;
}

/**
 * @file test-0021.c
 *
 * @brief regression test of reasoning about unknown pointer values
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
