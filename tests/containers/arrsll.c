#include <stdlib.h>


struct SLL
{
    int data;
    struct SLL* next;
};

struct SLL* SLL_create(int const n, int const* const A)
{
    struct SLL* sll = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data = A[i];
        p->next = sll;
        sll = p;
    }
    return sll;
}

void SLL_destroy(struct SLL const* sll)
{
    while (sll != 0)
    {
        struct SLL* p = sll->next;
        free((void*)sll);
        sll = p;
    }
}

int main()
{
    int const A[] = { 1, 2, 3, 4, 5 };
    int const n = sizeof(A)/sizeof(A[0]);

    struct SLL** arrsll = (struct SLL**)malloc(3 * sizeof(struct SLL*));
    int i;
    for (i = 0; i < 3; ++i)
        arrsll[i] = SLL_create(n,A);

    for (i = 0; i < 3; ++i)
        SLL_destroy(arrsll[i]);
    free((void*)arrsll);

    return 0;
}
