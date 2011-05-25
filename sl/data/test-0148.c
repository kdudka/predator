#include <stdlib.h>

static void *ptrs[8] = {
    NULL,
    ptrs + 3,
    NULL,
    ptrs
};

int main() {
    return !!ptrs[2];
}
