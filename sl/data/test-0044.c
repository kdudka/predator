int main()
{
    // a few dereferences of a scalar type
    const void *******x;
    const void ********px = &x;

    // a few dereferences followed by some field accessors
    struct {
        void *foo;
        struct {
            void *bar;
            void *ptr;
        } data;
    }
        inst,
        *pinst = &inst,
        **ppinst = &pinst,
        ***pppinst = &ppinst,
        ****ppppinst = &pppinst,
        *****pppppinst = &ppppinst;

    // this should be OK
    (****pppppinst)->data.ptr = (void *)0;

    // we are going to read the uninitialized value of 'x' here
    const long val = (long) *******px;
    return val;
}
