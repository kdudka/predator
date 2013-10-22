#include <verifier-builtins.h>

int main()
{
    unsigned int plus_one = 1;
    int minus_one = -1;
    __VERIFIER_assert(plus_one < minus_one);
}
