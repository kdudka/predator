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
