#include "../sl.h"
#include <stdlib.h>

static void *ptrs[8] = {
    NULL,
    ptrs + 3,
    NULL,
    ptrs
};

int main() {
    ___sl_plot(NULL, ptrs);
    return !!ptrs[2];
}
