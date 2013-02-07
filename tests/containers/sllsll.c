#include <stdlib.h>

struct SLLinner
{
    int data;
    struct SLLinner* next;
};

struct SLL
{
    struct SLLinner* data;
    struct SLL* next;
};

struct SLLinner* SLLinner_create(int const n, int* const A)
{
    struct SLLinner* sll = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        struct SLLinner* p = (struct SLLinner*)malloc(sizeof(struct SLLinner));
        p->data = A[i];
        p->next = sll;
        sll = p;
    }
    return sll;
}

struct SLL* SLL_create(int const n, int** const A, int const* m)
{
    struct SLL* sll = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data = SLLinner_create(m[i],A[i]);
        p->next = sll;
        sll = p;
    }
    return sll;
}

void SLLinner_destroy(struct SLLinner const* sll)
{
    while (sll != 0)
    {
        struct SLLinner* p = sll->next;
        free((void*)sll);
        sll = p;
    }
}

void SLL_destroy(struct SLL const* sll)
{
    while (sll != 0)
    {
        struct SLL* p = sll->next;
        SLLinner_destroy(sll->data);
        free((void*)sll);
        sll = p;
    }
}

int main()
{
    int A0[] = { 1, 2, 3};
    int A1[] = {4, 5};
    int A2[] = {6, 7, 8};
    int m[3] = { 3, 2, 3 };
    int* A[3] = { A0, A1, A2 };

    struct SLL* sll = SLL_create(3,A,m);
    SLL_destroy(sll);

    return 0;
}
