#include <stdlib.h>
#include <memory.h>

int main()
{
    int *A = 0;
    int n = 0;
    int r = 0;

    int i;
    for (i = 0; i < 10; ++i)
    {
        if (n == r)
        {
            int* p = (int*)malloc((r+2) * sizeof(int));
            memcpy(p,A,n * sizeof(int));
            free((void*)A);
            A = p;
            r += 2;
        }
        A[n++] = i;
    }

    for (i = 0; i < n; ++i)
        if (A[i] = 3)
            break;

    free((void*)A);
    n = r = 0;

    return 0;
}
