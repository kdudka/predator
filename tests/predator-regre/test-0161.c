#include <verifier-builtins.h>
#include <stdlib.h>

struct lowLevel {
    struct lowLevel *head;
    struct lowLevel *next;
};

struct topLevel {
    struct topLevel *next;
    struct lowLevel *low;
};

void add_sub(struct topLevel *t)
{
    struct lowLevel *l = calloc(1, sizeof *l);
    if (!l)
        abort();

    if (t->low == 0) {
        l->head = l;
        t->low = l;
    }
    else {
        l->head = t->low;
        l->next = t->low->next;
        t->low->next = l;
    }
}

struct topLevel* add_top(struct topLevel *t)
{
    while (__VERIFIER_nondet_int())
        add_sub(t);

    struct topLevel *tt = calloc(1, sizeof *tt);
    if (!tt)
        abort();

    tt->next = t;
    return tt;
}

struct topLevel* create_top(void)
{
    struct topLevel *t = calloc(1, sizeof *t);
    if (!t)
        abort();

    t = add_top(t);
    t = add_top(t);

    //___sl_break("mind the abstraction step");
    return t;
}

int main()
{
    struct topLevel *t = create_top();
    __VERIFIER_plot(NULL, &t);

    return 0;
}
