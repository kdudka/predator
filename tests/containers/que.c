#include <stdlib.h>

struct QLink
{
    int data;
    struct QLink* next;
};

struct Queue
{
    struct QLink* head;
    struct QLink* tail;
};

struct Queue* Q_make()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->head = q->tail = 0;
    return q;
}

void Q_put(struct Queue* const q, int const val)
{
    struct QLink* l = (struct QLink*)malloc(sizeof(struct QLink));
    l->data = val;
    l->next = q->tail;
    q->tail = l;
    if (q->head == 0)
        q->head = l;
}

int Q_head(struct Queue* const q)
{
    return q->head->data;
}

void Q_get(struct Queue* const q)
{
    if (q->head != 0)
    {
        struct QLink* p = q->head;
        q->head = p->next;
        if (q->head == 0)
            q->tail = 0;
        free((void*)p);
    }
}

int Q_empty(struct Queue* const q)
{
    return q->head == 0;
}

void Q_destroy(struct Queue* q)
{
    while (!Q_empty(q))
        Q_get(q);
}

int main()
{
    struct Queue* q = Q_make();
    Q_put(q,1);
    Q_put(q,2);
    Q_put(q,3);
    Q_get(q);
    Q_destroy(q);
    return 0;
}
