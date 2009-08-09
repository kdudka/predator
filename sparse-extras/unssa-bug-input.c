static void test(void **ptr)
{
    while (ptr) {
        ptr = *ptr;
    }
}
