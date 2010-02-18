#include <stdlib.h>

// return VAL_UNKNOWN of type CL_TYPE_PTR while running symbolic execution
// NOTE: executing this function on bare metal is usually bad idea
void** synthesize_unknown_ptr_value(void)
{
    return synthesize_unknown_ptr_value();
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

    // obtain unknown value
    void **const who_knows = synthesize_unknown_ptr_value();

    // who_knows is UNKNOWN at this point
    if (who_knows == &ptr) {

        // who_knows should be seen as &PTR at this point, fail otherwise
        *who_knows = NULL;
    } else {

        // FIXME: who_knows is still UNKNOWN here
        // TODO: we should be at least aware of (who_knows != &ptr)
    }

    if (ptr != NULL)
        // something went wrong
        trigger_null_dereference();
}

int main() {
    test__cond_reasoning__with_single_lookback_level();
    return 0;
}
