#include <stdlib.h>

struct DLL {
    int data;
    struct DLL* next;
    struct DLL* prev;
};

#define NEW(type) (type *) malloc(sizeof(type))

int main()
{
    struct DLL* head = NULL;
    struct DLL* tail = NULL;
    int i = 0;

    while (i < 10) {
        if (NULL == head) {
            tail = NEW(struct DLL);
            head = tail;
            head->data = i;
            head->prev = NULL;
            head->next = head->prev;
        }
        else {
            struct DLL *p = NEW(struct DLL);
            p->data = i;
            p->next = tail->next;
            p->prev = tail;
            tail->next = p;
            tail = p;
        }

        ++i;
    }

    return 0;
}
