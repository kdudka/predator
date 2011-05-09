#include <stdlib.h>

struct pair {
    void *p0;
};

static struct pair alloc_one(void)
{
    struct pair data;
    data.p0 = malloc(13);
    if (!data.p0)
        abort();

    return data;
}

int main()
{
    struct pair data = alloc_one();
    data = alloc_one();
    free(data.p0);
    return EXIT_SUCCESS;
}
