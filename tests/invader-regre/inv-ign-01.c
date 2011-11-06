#include <stdlib.h>

#define TRIGGER_INV_BUG 1

int main()
{
    typedef struct dll_item dll_item_t;
    struct dll_item {
        dll_item_t *prev;
        dll_item_t *next;
    };

    /*
     * This is a common mistake [Drepper09] not caught by Invader, because it
     * just completely ignores the amount of allocated memory.
     */
    dll_item_t *item = (dll_item_t *) malloc(sizeof(/***/item));

    item->prev = NULL;

#if TRIGGER_INV_BUG
    /*
     * This triggers an invalid write of size sizeof(void *) which can be easily
     * caught e.g. by valgrind.
     */
    item->next = NULL; 
#endif

    free(item);

    return 0;
}
