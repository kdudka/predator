#include <stdlib.h>

int __nondet();

struct cell {
    struct cell* next;
};

struct cell *S = NULL;

void push()
{
    static struct cell *t1 = NULL;
    static struct cell *x1 = NULL;

    static int pc1 = 1;
    switch (pc1++) {
        case 1:
            x1 = malloc(sizeof(*x1));
            return; // A thread local move: no need to return.

        case 2:
            t1 = S;
            return;

        case 3:
            x1->next = t1;
            return; // A thread local move: no need to return.

        case 4:
            if (S == t1) {
                S = x1;
                pc1 = 1;
                x1 = NULL;
                t1 = NULL;
            } else {
                pc1 = 2;
                t1 = NULL;
            }
            return;
    }
}

struct cell* garbage = NULL;

void pop()
{
    static struct cell *t4 = NULL;
    static struct cell *x4 = NULL;

    static int pc4 = 1;
    switch (pc4++) {
        case 1:
            t4 = S;
            return;

        case 2:
            if(t4 == NULL)
                pc4 = 1;
            return; // A thread local move: no need to return.

        case 3:
            x4 = t4->next;
            return; // A thread local move: no need to return.

        case 4:
            if (S == t4) {
                S = x4;
                t4->next = garbage;
                garbage = t4;
                pc4 = 1;
                x4 = NULL;
                t4 = NULL;
            } else {
                pc4 = 1;
                x4 = NULL;
                t4 = NULL;
            }
            return;
    }
}

int main()
{
    for (;;) {
        if (__nondet())
            push();
        else
            pop();
    }
}