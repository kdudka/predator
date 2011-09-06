#include "../sl.h"
#include <stdlib.h>

struct list {
    struct list     *head;
    struct list     *next;
};

int main()
{
    struct list *data = malloc(sizeof *data);
    struct list iter;

    if ((iter.head = data))
        iter.next = data->next;

    free(data);
    if (!iter.head)
        return 42;

    return 0;
}
