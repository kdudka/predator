
// ListSorted
// contains memory leak: no list deleting

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct list_item {
    struct list_item *prev;
    struct list_item *next;
    int data;
};

struct list {
    struct list_item *head;
    struct list_item *tail;
};

void ERROR()
{
    puts("ERROR");
    exit(1);
}

void list_add(struct list *l, int data)
{
    //printf("LIST%p ADD %d\n",l,data);
    struct list_item *p = malloc(sizeof(struct list_item));
    if (!p)
        ERROR();
    p->next = NULL;             // --|
    p->data = data;
    if (l->tail) {
        p->prev = l->tail;      // <--
        l->tail->next = p;      // -->
        l->tail = p;
    } else {
        p->prev = NULL;         // |--
        l->head = p;
        l->tail = p;
    }
}

int main()
{
    struct list l1 = { NULL, NULL };

    // list fill -- WARNING: should be sorted
    list_add(&l1, 10);
    list_add(&l1, 20);
    list_add(&l1, 30);


    int ins = 15;
    // add ins to sorted list
    //printf("LIST%p INSERT %d\n",&l1,ins);
    // SEARCH:
    struct list_item *it = l1.head;
    while (it != NULL) {
        int d = it->data;
        if (d > ins)
            break;
        it = it->next;
    }
    // INSERT:
    if (it == NULL) {
        // push_back
        list_add(&l1, ins);
    } else {
        // insert before it
        struct list_item *p = malloc(sizeof(struct list_item));
        if (!p)
            ERROR();
        p->next = it;           // -->
        p->data = ins;
        p->prev = it->prev;     // <--
        if (it->prev == NULL)   // first item
            l1.head = p;        // adjust head
        else
            it->prev->next = p; // -->
        it->prev = p;           //     <--
    }

    // CHECK_IF_SORTED_LIST:
    it = l1.head;
    while (it != NULL) {
        struct list_item *it2 = it->next;
        if (it2 == NULL)
            break;
        int element1 = it->data;
        int element2 = it2->data;

        if ( ! (element1 <= element2) )
            ERROR();            // Verify.assertTrue("element1 > element2", false);

        it = it2;
    }

    return 0;
}

