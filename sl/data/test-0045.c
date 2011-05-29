#include "../sl.h"

int* max_core(int *pa, int *pb)
{
    if (*pa == *pb)
        // utilize the prover at this point
        return (int *)0;
    else
        return (*pa < *pb)
            ? pb
            : pa;
}

int max(int **ppa, int **ppb) {
    const int *pmax = max_core(*ppa, *ppb);
    ___sl_plot("test-0045-fnc");
    return *pmax;
}

int main()
{
    int a = 0;
    int b;
    int c;

    int *pa = &a;
    int *pb = &b;
    int *pc = &c;

    int **ppa = &pa;
    int **ppb = &pb;
    int **ppc = &pc;

    // this should trigger an invalid dereference ('b' and 'c' not initialized)
    const int x = max(ppa, ppb);

    // this should trigger an invalid dereference as long as symstate handles
    // Neq predicates properly
    const int y = max(ppb, ppc);

    // this should be evaluated as zero
    const int result = (&x == &y);

    // dump all possibilities of the resulting state
    ___sl_plot("test-0045-end");
    return result;
}
