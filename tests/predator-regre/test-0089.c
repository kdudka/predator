#include <verifier-builtins.h>
/* #include <linux/stddef.h> */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

struct inner {
    void *p0;
    void *p1;
};

struct middle {
    struct inner i0;
    struct inner i1;
};

struct outer {
    struct middle m0;
    struct middle m1;
};

static struct outer* create(void)
{
    struct outer *data = malloc(sizeof *data);
    if (!data)
        abort();

#define SAVE_SELF_ADDR(what) what = &what
    SAVE_SELF_ADDR(data->m0.i0.p0);
    SAVE_SELF_ADDR(data->m0.i0.p1);
    SAVE_SELF_ADDR(data->m0.i1.p0);
    SAVE_SELF_ADDR(data->m0.i1.p1);
    SAVE_SELF_ADDR(data->m1.i0.p0);
    SAVE_SELF_ADDR(data->m1.i0.p1);
    SAVE_SELF_ADDR(data->m1.i1.p0);
    SAVE_SELF_ADDR(data->m1.i1.p1);

    return data;
}

#define ROOT(type, field, addr) \
    (type *) ((char *)addr - offsetof(type, field))

#define CHK_CORE(neg, e1, e2) do {                      \
    if (__VERIFIER_nondet_int())                         \
        break;                                          \
                                                        \
    struct outer *_data = create();                     \
    free(_data);                                        \
    const bool eq = ((void *)(e1) == (void *)(e2));     \
    if (neg == eq)                                      \
        /* double free */                               \
        free(_data);                                    \
} while (0)

#define CHK_EQ(e1, e2) CHK_CORE(false, e1, e2)
#define CHK_NEQ(e1, e2) CHK_CORE(true, e1, e2)

#define TRY_FREE_BY(item) do {                          \
    if (__VERIFIER_nondet_int())                         \
        break;                                          \
                                                        \
    struct outer *_data = create();                     \
    void *_ptr = &_data->item;                          \
    free(_ptr);                                         \
} while (0)

void chk_offval_handling()
{
    struct middle *ptr = malloc(sizeof *ptr);
    if (!ptr)
        abort();

    struct middle *same_ptr = ROOT(struct middle, i1, &ptr->i1);

    struct outer *d0 = ROOT(struct outer, m0, ptr);
    struct outer *d1 = ROOT(struct outer, m1, ptr);
    struct outer *same_d1 = ROOT(struct outer, m1, same_ptr);
    __VERIFIER_plot(NULL);

    // we are able to deduce this
    CHK_EQ(ptr, d0);
    // FIXME: CHK_NEQ(d0, same_d1);
    // FIXME: CHK_EQ(ptr, &d1->m1);

    // this should be detected as invalid free()
    free(d1);

    // this should be OK
    free(d0);

    // this should be detected as double free()
    free(ptr);
}

int main()
{
    struct outer *data = create();
    __VERIFIER_plot(NULL);

    // ==
    CHK_EQ(data,        &data->m0);
    CHK_EQ(data,        &data->m0.i0);
    CHK_EQ(data,        &data->m0.i0.p0);
    CHK_EQ(&data->m1,   data->m1.i0.p0);

    // !=
    CHK_NEQ(data,       &data->m1);
    CHK_NEQ(data,       &data->m0.i1);
    CHK_NEQ(data,       data->m1.i0.p0);
    CHK_NEQ(&data->m1,  &data->m0.i0.p0);

    // avoid junk
    free(data);

    // this should be OK
    TRY_FREE_BY(m0);
    TRY_FREE_BY(m0.i0);
    TRY_FREE_BY(m0.i0.p0);

    // this shoould be detected as invalid free
    TRY_FREE_BY(m1);
    TRY_FREE_BY(m0.i1);
    TRY_FREE_BY(m0.i0.p1);

    // now some off-values play
    chk_offval_handling();

    return EXIT_SUCCESS;
}

/**
 * @file test-0089.c
 *
 * @brief regression test focused on off-values, address aliasing, etc.
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
