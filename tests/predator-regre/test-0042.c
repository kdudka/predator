static void a(int arg)
{
    (void) arg;
}

static void c(int arg)
{
    (void) arg;
    a(0);
}

int main()
{
    a(0);
    c(0);

    return 0;
}

/**
 * @file test-0042.c
 *
 * @brief regression test focused on call cache
 *
 * - inspired by an already fixed bug there
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
