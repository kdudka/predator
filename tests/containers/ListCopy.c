
// ListCopy
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

void ERROR() { puts("ERROR"); exit(1); }

void list_add(struct list *l, int data)
{
    //printf("LIST%p ADD %d\n",l,data);
    struct list_item *p = malloc(sizeof(struct list_item));
    if(!p) ERROR();
    p->next = NULL;             // --|
    p->data = data;
    if(l->tail) {
        p->prev = l->tail;      // <--
        l->tail->next = p;      // -->
        l->tail = p;
    }else{
        p->prev = NULL;         // |--
        l->head = p;
        l->tail = p;
    }
}

int main()
{
    struct list l1 = { NULL, NULL };    // ArrayList<Integer> l1 = new ArrayList<Integer>();

    list_add(&l1, 4);           // l1.add(4);
    list_add(&l1, 67);          // l1.add(67);

    struct list l2 = { NULL, NULL };    // ArrayList<Integer> l2 = new ArrayList<Integer>();

    struct list_item *it = l1.head;     // Iterator<Integer> it = l1.iterator();

    int cur = 0;

    bool b1 = (it != NULL);     // it.hasNext(); /// ==stupid naming?

    while (b1) {
        cur = it->data;
        it = it->next;          // it.next();

        list_add(&l2, cur);     // l2.add(cur);

        b1 = (it != NULL);      // it.hasNext();
    }


    it = l1.head;               // l1.iterator();

    struct list_item *it2 = l2.head;    // Iterator<Integer> it2 = l2.iterator();

    int l1_elem = 0;
    int l2_elem = 0;

    b1 = (it != NULL);          // it.hasNext();
    while (b1) {
        l1_elem = it->data;
        it = it->next;          // it.next();

        l2_elem = it2->data;
        it2 = it2->next;        // it2.next();

        if (l1_elem != l2_elem)
            ERROR();            // Verify.assertTrue("l1_elem != l2_elem", false);

        b1 = (it != NULL);      // it.hasNext();
    }

    return 0;
} // main

