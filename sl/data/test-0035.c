#include <stdlib.h>
#include <stdbool.h>

#define NEW(type) \
    (type *) malloc(sizeof(type))

static struct stack_item {
    void                *data;
    struct stack_item   *next;
} *gl_stack;

static void gl_push(void *data)
{
    // allocate stack item
    struct stack_item *item = NEW(struct stack_item);
    if (!item)
        abort();

    // initialize stack item
    item->data = data;
    item->next = gl_stack;

    // replace the top of stack
    gl_stack = item;
}

static bool gl_pop(void **pData)
{
    if (!gl_stack)
        // empty stack
        return false;

    // read the top of stack
    struct stack_item *item = gl_stack;
    *pData = item->data;

    // remove the item
    gl_stack = item->next;
    free(item);
    return true;
}

static void gl_destroy_until(void *what)
{
    void *data;
    while(gl_pop(&data) && data != what);
}

int main()
{
    gl_push(NULL);
    gl_destroy_until(NULL);
    gl_push(NULL);
    return 0;
}
