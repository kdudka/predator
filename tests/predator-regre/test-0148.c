#include <verifier-builtins.h>
#include <stdlib.h>

static void *ptrs[8] = {
    NULL,
    ptrs + 3,
    NULL,
    ptrs
};

int main() {
    __VERIFIER_plot(NULL, ptrs);
    return !!ptrs[2];
}
