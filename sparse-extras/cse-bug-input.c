static void test(int i)
{
    while (i) {
        if (i)
            test(0);
        i++;
    }
}
