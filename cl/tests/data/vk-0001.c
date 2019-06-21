#include "include/vk.h"

void use_value(int);

int main() {
    int i = 0;
    int untouched = 1;

    VK_ASSERT(VK_LIVE, &i, &untouched);

    // block L2 -- kill variable 'untouched' per target block 'L4'
    while (i < 10) {
        // block L3
        VK_ASSERT(VK_LIVE, &i, &untouched);
        use_value(untouched);
        i++;
        VK_ASSERT(VK_LIVE, &i, &untouched);
    }

    // block L4
    VK_ASSERT(VK_DEAD, &i, &untouched);
    return 0;
}
