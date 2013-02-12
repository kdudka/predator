#include <verifier-builtins.h>
#include <stdlib.h>

#define BREAK_IMMATURE_VERSION_OF_PREDATOR 1

struct lowLevel {
    struct lowLevel *head;
    struct lowLevel *next;
};

struct topLevel {
    struct topLevel *next;
    struct lowLevel *low;
};

int main()
{
    struct topLevel *t, *tt;
    struct lowLevel *l;

    t = malloc(sizeof(struct topLevel));
    t->next = 0;
    t->low = 0;

    while (__VERIFIER_nondet_int()) {
        do {
            struct lowLevel *l = calloc(1, sizeof *l);
            if (!l)
                abort();

            if (t->low == 0) {
                l->head = l;
#if BREAK_IMMATURE_VERSION_OF_PREDATOR
                l->next = 0;
#endif
                t->low = l;
            }
            else {
                l->head = t->low;
                l->next = t->low->next;
                t->low->next = l;
            }
        }
        while (__VERIFIER_nondet_int());

        tt = malloc(sizeof(struct topLevel));
        tt->next = t;
        tt->low = 0;
        t = tt;
    }

    __VERIFIER_plot(NULL, &t);

    return 0;
}
