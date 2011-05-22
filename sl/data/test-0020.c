#include "../sl.h"
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
    if (___sl_get_nondet_int())
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
