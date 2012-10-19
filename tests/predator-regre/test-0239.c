#include <stdlib.h>

void foo(int n_alloc, int n_free)
{
    void *a[n_alloc];

    int i;
    for (i = 0; i < n_alloc; ++i)
        a[i] = malloc(i);

    for (i = 0; i < n_free; ++i)
        free(a[i]);
}

int main()
{
    foo(0, 0);
    foo(1, 1);
    foo(2, 2);
    foo(3, 3);
    foo(3, 2);
    foo(3, 1);
    foo(3, 0);
    foo(3, 4);
    return 0;
}
