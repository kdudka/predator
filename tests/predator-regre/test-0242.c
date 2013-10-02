#define NULL ((void *) 0)

void assert(int cond)
{
    if (cond)
        return;
ERROR:
    goto ERROR;
}

extern int __VERIFIER_nondet_int(void);

void *my_malloc(int size)
{
    static void *guard_malloc_counter;

    ++guard_malloc_counter;

    return (__VERIFIER_nondet_int())
        ? guard_malloc_counter
        : NULL;
}

void *elem;

static void list_add(void *new)
{
    assert(new != elem);

    if (__VERIFIER_nondet_int())
        elem = new;
}

static void list_del(void *entry)
{
    if (entry == elem)
        elem = NULL;
}

int main()
{
    void *dev1 = my_malloc(sizeof dev1);
    void *dev2 = my_malloc(sizeof dev2);

    if (dev1 && dev2) {
        list_add(dev2);
        list_add(dev1);
        list_del(dev2);
        list_add(dev2);
    }

    return 0;
}

/**
 * @file test-0242.c
 *
 * @brief a regression test for joining shifted addresses of NULL
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
