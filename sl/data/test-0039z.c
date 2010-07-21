#include <stdlib.h>

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
    int i;

    t = malloc(sizeof(struct topLevel));
    t->next = 0;
    t->low = 0;

    for (i = 0; i < 10; i++) {
        int j;
        for (j = 0; j < 10; j++) {
            l = malloc(sizeof(struct lowLevel));
            if (t->low == 0) {
                l->head = l;
                l->next = 0;
                t->low = l;
            }
            else {
                l->head = t->low;
                l->next = t->low->next;
                t->low->next = l;
            }
        }
        tt = malloc(sizeof(struct topLevel));
        tt->next = t;
        tt->low = 0;
        t = tt;
    }

    return 0;
}
