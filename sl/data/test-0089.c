#include "../sl.h"
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

int main()
{
    struct outer *data = create();
    ___sl_plot_by_ptr(&data, "test-0089");
    return EXIT_SUCCESS;
}
