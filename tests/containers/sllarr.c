#include <stdlib.h>

struct SLL
{
    int* data;
    struct SLL* next;
};

struct SLL* SLL_create(int const n, int** const A, int const* m)
{
    struct SLL* sll = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data = (int*)malloc(m[i] * sizeof(int));
        int j;
        for (j = 0; j < m[i]; ++j)
             p->data[j] = A[i][j];
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
        free((void*)sll->data);
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
