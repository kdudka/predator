#include "include/vk.h"

int main(void)
{
    int x;

    VK_ASSERT(VK_DEAD, &x);

    x = 1;

    VK_ASSERT(VK_DEAD, &x);

    x = 2;

    VK_ASSERT(VK_DEAD, &x);

    x = 3;

    VK_ASSERT(VK_DEAD, &x);
}
