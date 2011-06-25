#include "../sl.h"
#include <stdlib.h>

struct T {
    struct T *head;
    struct T *next;
};

static void inspect_by_ptr(const struct T *const ptr)
{
    ___sl_plot(NULL);
    ___SL_ASSERT(ptr == ptr->head);
    ___SL_ASSERT(NULL == ptr->next);
}

static void inspect_by_val(const struct T str)
{
    ___sl_plot(NULL);
    ___SL_ASSERT(str.head == str.head->head);
    ___SL_ASSERT(NULL == str.next);
}

int main()
{
    struct T data = {
        &data
    };

    inspect_by_ptr(&data);
    inspect_by_val(data);

    return 0;
}
