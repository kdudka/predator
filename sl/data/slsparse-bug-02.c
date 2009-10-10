static const int C = 8;

static void set(int **ptr)
{
    *ptr = &C;
}

static int get(int *ptr)
{
    return *ptr;
}

static int test(int *ptr)
{
    get(ptr);
    set(&ptr);
    return get(ptr);
}
