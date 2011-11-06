#include <stdbool.h>
#include <stdlib.h>

static void **gl_ptr; // automatically initialized to NULL

void chk_fork(int i1, int i2)
{
    bool b1 = false;
    bool b2 = false;

    if (i1 == i2)
        b1 = true;
    else
        b2 = true;

    // at this point we now that either b1 or b2 is true, but not both of them
    if (b1 == b2)
        // something went wrong, shout now
        *gl_ptr = gl_ptr;
}

int main()
{
    // test #1 - both i1, i2 uninitialized
    int i1, i2;
    chk_fork(i1, i2);

    // test #2 - i1 uninitialized, (i2 == 0)
    i2 = 0;
    chk_fork(i1, i2);

    // test #3 - (i1 == 0), i2 got uninitialized value from i1
    i2 = i1;
    i1 = 0;
    chk_fork(i1, i2);

    // test #4 - (i1 == 1), (i2 == 2)
    i1 = 1;
    i2 = 2;
    chk_fork(i1, i2);

    // now utilize the prover
    if (i1 == i2) {
        if (i1 != i2)
            *gl_ptr = gl_ptr;
    } else {
        if (i1 == i2)
            *gl_ptr = gl_ptr;
    }

    // test #5 - (i1 == 0), (i2 == 2)
    i1 = 0;
    if (i1 == i2)
        // something went wrong, shout now
        *gl_ptr = gl_ptr;

    // test #6 - (i1 == 0), (i2 == 0)
    i2 = 0;
    if (i1 != i2)
        // something went wrong, shout now
        *gl_ptr = gl_ptr;

    if (i1 < i2)
        // something went wrong, shout now
        *gl_ptr = gl_ptr;

    if (i1 > i2)
        // something went wrong, shout now
        *gl_ptr = gl_ptr;

    return 0;
}
