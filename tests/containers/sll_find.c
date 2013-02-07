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

struct SLL const* SLL_find(struct SLL const* sll, int const x)
{
    struct SLL const* p;
    for (p = sll; p != 0; p = p->next)
        if (p->data == x)
            return p;
    return 0;
}

struct SLL const* SLL_find_rec(struct SLL const* sll, int const x)
{
    if (sll == 0)
        return 0;
    return (sll->data == x) ? sll : SLL_find_rec(sll->next,x);
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

    struct SLL* sll = SLL_create(n,A);

    struct SLL const* it1 = SLL_find(sll,3);
    struct SLL const* it2 = SLL_find_rec(sll,3);

    SLL_destroy(sll);

    return 0;
}
