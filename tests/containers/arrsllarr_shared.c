#include <stdlib.h>
#include <memory.h>

struct Inner
{
    char c;
    int* arr;
    int n;
};

struct SLL
{
    struct Inner data;
    struct SLL* next;
};

struct Outer
{
    float f;
    struct SLL* sll_1;
    int m;
    struct SLL sll_2;
};

struct SLL* SLL_create(int const n, struct Inner* const data)
{
    struct SLL* sll = 0;
    int i;
    for (i = 0; i < n; ++i)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data = *data;
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
    struct Outer* outer = (struct Outer*)malloc(5 * sizeof(struct Outer));
    struct Inner inner;
    inner.c = 'A';
    inner.n = 3;
    inner.arr = (int*)malloc(inner.n * sizeof(int));
    int i, j;
    for (j = 0; j < inner.n; ++j)
        inner.arr[j] = j;
    for (i = 0; i < 5; ++i)
    {
        outer[i].f = (float)i + 0.5f;
        outer[i].m = i;
        outer[i].sll_1 = SLL_create(i,&inner);
        if (i)
            outer[i].sll_2 = *outer[i].sll_1;
        else
            memset(&outer[i].sll_2, 0, sizeof(struct SLL));
    }

    for (i = 0; i < 5; ++i)
        SLL_destroy(outer[i].sll_1);
    free((void*)inner.arr);
    free((void*)outer);

    return 0;
}
