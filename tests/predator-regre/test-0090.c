#include <stdbool.h>
#include <stdlib.h>

struct pair {
    void *p0;
    void *p1;
};

static struct pair alloc_pair(void)
{
    struct pair data;
    data.p0 = malloc(13);
    data.p1 = malloc(7);

    const bool have_p0 = data.p0;
    const bool have_p1 = data.p1;
    if (have_p0 != have_p1)
        // exactly one malloc() has failed
        abort();

    return data;
}

static void should_be_ok(void)
{
    struct pair data = alloc_pair();
    free(data.p0);
    free(data.p1);
}

static void leak0(void)
{
    free(alloc_pair().p0);
}

static void leak1(void)
{
    free(alloc_pair().p1);
}

static void leak2(void)
{
    alloc_pair();
}

static void leak3(void)
{
    struct pair data = alloc_pair();
    data = alloc_pair();
    free(data.p0);
    free(data.p1);
}

int main()
{
    should_be_ok();

    leak0();
    leak1();
    leak2();
    leak3();

    return EXIT_SUCCESS;
}
