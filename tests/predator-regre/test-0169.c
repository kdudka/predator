#include <verifier-builtins.h>
#include <stdlib.h>

struct T {
    struct T *head;
    struct T *next;
};

static void inspect_by_ptr(const struct T *const ptr)
{
    __VERIFIER_plot(NULL);
    __VERIFIER_assert(ptr == ptr->head);
    __VERIFIER_assert(NULL == ptr->next);
}

static void inspect_by_val(const struct T str)
{
    __VERIFIER_plot(NULL);
    __VERIFIER_assert(str.head == str.head->head);
    __VERIFIER_assert(NULL == str.next);
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
