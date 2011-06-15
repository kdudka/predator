#include "../sl.h"
#include <stdbool.h>
#include <stdlib.h>

struct data {
    void *p0;
    void *p1;
};

struct node {
    struct data data;
    struct node *next;
};

struct stack {
    bool empty;
    bool direct;
    union {
        struct data data;
        struct node *head;
    };
};

#define STACK_HEAD(name) struct stack name = { true, true }

static struct node* node_create(const struct data *data, struct node *next)
{
    struct node *node = malloc(sizeof *node);
    if (!node)
        abort();

    node->data = *data;
    node->next = next;
    return node;
}

void push(struct stack *stack, const struct data *src)
{
    if (stack->empty) {
        stack->empty = false;
        stack->direct = true;
        stack->data = *src;
        return;
    }

    if (!stack->direct) {
        stack->head = node_create(src, stack->head);
        return;
    }

    stack->direct = false;
    stack->head = node_create(&stack->data, NULL);
    stack->head = node_create(src, stack->head);
}

bool pop(struct data *dst, struct stack *stack)
{
    if (stack->empty)
        return false;

    if (stack->direct) {
        stack->empty = true;
        *dst = stack->data;
        return true;
    }

    struct node *head = stack->head;
    if (!head->next)
        stack->empty = true;
    else
        stack->head = head->next;

    *dst = head->data;
    free(head);
    return true;
}

STACK_HEAD(gl_stack);

void test(void)
{
    ___sl_plot(NULL);

    struct data data;
    push(&gl_stack, &data);

    data.p0 = NULL;
    push(&gl_stack, &data);

    data.p1 = NULL;
    push(&gl_stack, &data);

    do {
        ___sl_plot(NULL);
    }
    while(pop(&data, &gl_stack));
}

void misuse_of_union(void)
{
    // should be OK
    gl_stack.data.p1 = &gl_stack.data.p0;
    ___sl_plot(NULL);

    // ugly, but we should be silent
    gl_stack.data.p0 = gl_stack.head;
    ___sl_plot(NULL);

    // ugly, but we should be silent
    gl_stack.data.p0 = malloc(80);
    free(gl_stack.head);
    ___sl_plot(NULL);
}

int main()
{
    test();
    test();
    test();

    misuse_of_union();

    return EXIT_SUCCESS;
}
