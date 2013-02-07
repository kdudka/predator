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

void SLL_copy(struct SLL const* begin, struct SLL const* const end, struct SLL* out)
{
    for ( ; begin != end; begin = begin->next, out = out->next)
        out->data = begin->data;
}

void SLL_xcopy(struct SLL const* begin, struct SLL const* const end, struct SLL* out)
{
    for ( ; begin != end; begin = begin->next)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data = begin->data;
        p->next = out->next;
        out->next = p;
        out = p;
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

    struct SLL* sll_1 = SLL_create(n,A);
    struct SLL* sll_2 = SLL_create(2,A);

    SLL_copy(sll_1->next->next,sll_1->next->next->next->next,sll_2);
    SLL_xcopy(sll_1,sll_1->next->next->next->next->next,sll_2->next);

    SLL_destroy(sll_2);
    SLL_destroy(sll_1);

    return 0;
}
