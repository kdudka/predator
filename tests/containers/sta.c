#include <stdlib.h>

struct Stack
{
    int data;
    struct Stack* next;
};

struct Stack* S_push(struct Stack* const s, int const val)
{
    struct Stack* l = (struct Stack*)malloc(sizeof(struct Stack));
    l->data = val;
    l->next = s;
    return l;
}

int S_top(struct Stack* const s)
{
    return s->data;
}

struct Stack* S_pop(struct Stack* const s)
{
    struct Stack* p = s->next;
    free((void*)s);
    return p;
}

int S_empty(struct Stack* const s)
{
    return s == 0;
}

void S_destroy(struct Stack* s)
{
    while (!S_empty(s))
        s = S_pop(s);
}

int main()
{
    struct Stack* s = 0;
    s = S_push(s,1);
    s = S_push(s,2);
    s = S_push(s,3);
    s = S_pop(s);
    S_destroy(s);
    return 0;
}
