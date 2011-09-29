#include "../sl.h"
#include <stdbool.h>

// return VAL_UNKNOWN of type CL_TYPE_BOOL while running symbolic execution
bool synthesize_unknown_bool_value(void)
{
    return !!___sl_get_nondet_int();
}

int main() {
    // obtain unknown value
    const bool who_knows = synthesize_unknown_bool_value();

    // who_knows is UNKNOWN at this point
    ___sl_plot("before-cond");
    if (who_knows)
        ___sl_plot("then-branch");
    else
        ___sl_plot("else-branch");
}
