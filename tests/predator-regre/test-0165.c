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
    struct topLevel *tt = calloc(1, sizeof *tt);
    if (!tt)
        abort();

    while (__VERIFIER_nondet_int())
        add_sub(tt);

    tt->next = t;
    return tt;
}

struct topLevel* create_top(void)
{
    struct topLevel *t = NULL;
    while (__VERIFIER_nondet_int())
        t = add_top(t);

    return t;
}

void inspect(struct topLevel *t) {
    for (; t; t = t->next) {
        struct lowLevel *const head = t->low;
        ___SL_BREAK_IF(head != head->head);
        ___SL_BREAK_IF(head->next);
    }
}

void prune(struct topLevel *t) {
    for (; t; t = t->next) {
        struct lowLevel *cursor = t->low;

        while (cursor) {
            struct lowLevel *next = cursor->next;
            if (cursor == cursor->head) {
                cursor->next = NULL;
            }
            else {
                free(cursor);
            }

            //__VERIFIER_plot(NULL);
            cursor = next;
            //___sl_enable_debugging_of(___SL_SYMABSTRACT, 1);
        }

        //___sl_enable_debugging_of(___SL_SYMABSTRACT, 0);
        ___SL_BREAK_IF(t->low->next);
    }
    //___sl_enable_debugging_of(___SL_SYMABSTRACT, 0);
}

int main()
{
    struct topLevel *top = create_top();
    __VERIFIER_plot(NULL);

    prune(top);
    __VERIFIER_plot(NULL);

    inspect(top);

    while (top) {
        struct topLevel *next = top->next;
        free(top->low);
        free(top);

        top = next;
    }

    return !!top;
}
