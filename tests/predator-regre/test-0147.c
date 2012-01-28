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

/**
 * @file test-0147.c
 *
 * @brief test-0090.c reduced to a minimal example while debugging a bug
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
