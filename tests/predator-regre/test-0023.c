#include <verifier-builtins.h>
#include <stdbool.h>

// return VAL_UNKNOWN of type CL_TYPE_BOOL while running symbolic execution
bool synthesize_unknown_bool_value(void)
{
    return !!__VERIFIER_nondet_int();
}

int main() {
    // obtain unknown value
    const bool who_knows = synthesize_unknown_bool_value();

    // who_knows is UNKNOWN at this point
    __VERIFIER_plot("before-cond");
    if (who_knows)
        __VERIFIER_plot("then-branch");
    else
        __VERIFIER_plot("else-branch");

    return 0;
}

/**
 * @file test-0023.c
 *
 * @brief show-case of non-deterministic condition handling
 *
 * - plots 3 heap graphs:
 *     - before the condition
 *     - "then" branch
 *     - "else" branch
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
