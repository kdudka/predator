#include <stdlib.h>

struct DLL
{
    int data;
    struct DLL* next;
    struct DLL* prev;
};

int main()
{
    struct DLL* dll_head = 0;
    struct DLL* dll_tail = 0;

    int k;
    for (k = 0; k < 3; ++k)
    {
        int i;
        for (i = 0; i < 10; ++i)
        {
            if (dll_head == 0)
            {
                dll_head = dll_tail = (struct DLL*)malloc(sizeof(struct DLL));
                dll_head->data = i;
                dll_head->next = dll_head->prev = 0;
            }
            else
            {
                struct DLL* p = (struct DLL*)malloc(sizeof(struct DLL));
                p->data = i;
                p->next = dll_tail->next; // i.e. NULL
                p->prev = dll_tail;
                dll_tail->next = p;
                dll_tail = p;
            }
        }

        for (i = 0; i < 10; ++i)
        {
            if (dll_head == 0)
            {
                dll_head = dll_tail = (struct DLL*)malloc(sizeof(struct DLL));
                dll_head->data = i;
                dll_head->next = dll_head->prev = 0;
            }
            else
            {
                struct DLL* p = (struct DLL*)malloc(sizeof(struct DLL));
                p->data = 10+i;
                p->next = dll_head;
                p->prev = dll_head->prev; // i.e. NULL
                dll_head->prev = p;
                dll_head = p;
            }
        }

        if (k == 1)
        {
            while (dll_head != 0)
            {
                struct DLL* p = dll_head->next;
                free((void*)dll_head);
                dll_head = p;
            }
        }
    }

    while (dll_tail != 0)
    {
        struct DLL* p = dll_tail->prev;
        free((void*)dll_tail);
        dll_tail = p;
    }

    return 0;
}
