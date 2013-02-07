#include "include/vk.h"

int foo(int i)
{
    VK_ASSERT(VK_LIVE, &i);

    int dead = 0;

    VK_ASSERT(VK_DEAD, &dead);

    if (i)
        dead = 1;
    else
        dead = 2;

    VK_ASSERT(VK_DEAD, &dead, &i);

    return ((dead = 3));
}
