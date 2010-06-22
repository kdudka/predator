#include "../sl.h"

int* max_core(int *pa, int *pb)
{
    if (*pb < *pa)
        return pa;
    else if (*pa < *pb)
        return pb;
    else
        return (int *)0;
}

int max(int **ppa, int **ppb) {
    const int *pmax = max_core(*ppa, *ppb);
    return *pmax;
}

int main()
{
    int a = 0;
    int b = 1;
    int c = 2;

    int *pa = &a;
    int *pb = &b;
    int *pc = &c;

    int **ppa = &pa;
    int **ppb = &pb;
    int **ppc = &pc;

    // this should be OK as long as Neq predicates work for non-zero ints
    const int x = max(ppa, ppb);

    // this should trigger an invalid dereference as long as symstate handles
    // Neq predicates properly
    const int y = max(ppb, ppc);

    // this should be evaluated as zero
    const int result = (&x == &y);

    // dump all possibilities of the resulting state
    ___sl_plot("test-0045");
    return result;
}
