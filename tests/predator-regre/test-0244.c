#include <verifier-builtins.h>

extern void *y;
void *x = &y;
void *y = &x;

int main()
{
    __VERIFIER_assert(x == &y);
    __VERIFIER_assert(y == &x);
}
