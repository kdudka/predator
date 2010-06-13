#include <stdbool.h>
#include <stdlib.h>

// return VAL_UNKNOWN of type CL_TYPE_BOOL while running symbolic execution
bool synthesize_unknown_bool_value(void)
{
    int a = 1, b = 2;
    return (a < b);
}

// reaching this function indicates we are on a wrong way
void trigger_null_dereference(void)
{
    void **ptr = NULL;
    ptr = (void **) *ptr;
}

void test__cond_reasoning__with_zero_lookback_level() {
    // obtain unknown value
    const bool who_knows = synthesize_unknown_bool_value();

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
