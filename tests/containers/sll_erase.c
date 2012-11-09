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

struct SLL* SLL_erase(struct SLL* sll, struct SLL* const it)
{
    if (sll == it)
    {
        sll = it->next;
        free((void*)it);
        return sll;
    }
    else
    {
        struct SLL* p = sll;
        for ( ; p->next != it; p = p->next)
            ;
        p->next = it->next;
        free((void*)it);
        return sll;
    }
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

    sll = SLL_erase(sll,sll->next->next);

    SLL_destroy(sll);

    return 0;
}
