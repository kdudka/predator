#include <stdlib.h>

struct SLL
{
    int data;
    struct SLL* next;
};

struct SLL* SLL_create(int const n, int const* const A)
{
    struct SLL* sll;

    if (n == 0)
        return 0;

    sll = (struct SLL*)malloc(sizeof(struct SLL));
    sll->data = A[0];
    sll->next = SLL_create(n-1,A+1);

    return sll;
}

void SLL_destroy(struct SLL const* sll)
{
    if (sll != 0)
    {
        SLL_destroy(sll->next);
        free((void*)sll);
    }
}

int main()
{
    int const A[] = { 1, 2, 3, 4, 5 };
    int const n = sizeof(A)/sizeof(A[0]);

    struct SLL* sll = SLL_create(n,A);
    SLL_destroy(sll);

    return 0;
}
