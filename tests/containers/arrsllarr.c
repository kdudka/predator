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

struct SLL* SLL_create(int const n, Inner const* const data)
{
    struct SLL* sll = 0;
    for (int i = 0; i < n; ++i)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data.c = data->c;
        p->data.n = data->n;
        p->data.arr = (int*)malloc(data->n * sizeof(int));
        memcpy(p->data.arr,data->arr,data->n * sizeof(int));
        p->next = sll;
        sll = p;
    }
    return sll;
}

void SLL_xcopy(struct SLL const* begin, struct SLL const* const end, struct SLL* out)
{
    for ( ; begin != end; ++begin)
    {
        struct SLL* p = (struct SLL*)malloc(sizeof(struct SLL));
        p->data.c = begin->data.c;
        p->data.n = begin->data.n;
        p->data.arr = (int*)malloc(begin->data.n * sizeof(int));
        memcpy(p->data.arr,begin->data.arr,begin->data.n * sizeof(int));
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
        free((void*)sll->data.arr);
        free((void*)sll);
        sll = p;
    }
}

int main()
{
    struct Outer* outer = (struct Outer*)malloc(5 * sizeof(struct Outer));
    for (int i = 0; i < 5; ++i)
    {
        struct Inner inner;
        outer[i].f = (float)i + 0.5f;
        outer[i].m = i;
        inner.c = 'A';
        inner.n = 4;
        inner.arr = (int*)malloc(inner.n * sizeof(int));
        for (int j = 0; j < inner.n; ++j)
            inner.arr[j] = i+j;
        outer[i].sll_1 = SLL_create(i,&inner);
        outer[i].sll_2.data = inner;
        outer[i].sll_2.next = 0;
        SLL_xcopy(outer[i].sll_1,outer[i].sll_1+i,&outer[i].sll_2);
    }

    for (int i = 0; i < 5; ++i)
    {
        SLL_destroy(outer[i].sll_1);
        SLL_destroy(outer[i].sll_2.next);
    }
    free((void*)outer);

    return 0;
}
