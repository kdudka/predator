#include "../sl.h"
#include <stdlib.h>

struct TNode {
    int value;
    struct TNode *lptr;
    struct TNode *rptr;
};

struct TList {
    struct TNode *First;
    struct TNode *Act;
    struct TNode *Last;
};

void DLInitList (struct TList *L) {
    L->First = ((void *)0);
    L->Act = ((void *)0);
    L->Last = ((void *)0);
}

void DLDisposeList (struct TList *L) {
    ___sl_plot(NULL);

    struct TNode * ptr = L->First;
    struct TNode * next;

    while (((void *)0)!= ptr) {
        next = ptr->rptr;
        free (ptr);
        ptr = next;
    }

    ___sl_plot(NULL);
    free(L->Act);
}

void DLInsertFirst (struct TList *L) {
    struct TNode * ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    ptr->lptr = ((void *)0);
    ptr->rptr = L->First;

    if (((void *)0)== L->First)
        L->Last = ptr;
    else
        L->First->lptr = ptr;
    L->First = ptr;
}

void DLFirst (struct TList *L) {
    L->Act = L->First;
}

void DLSucc (struct TList *L) {
    struct TNode * act = L->Act;
    if (((void *)0)==act)
        return;

    L->Act = act->rptr;
}

int main () {
    struct TList list;
    DLInitList(&list);

    DLInsertFirst(&list);
    DLInsertFirst(&list);

    DLFirst(&list);
    DLSucc(&list);
    DLSucc(&list);

    DLDisposeList(&list);
    return 0;
}
