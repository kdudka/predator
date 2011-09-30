#include "../sl.h"

static void error(void) 
{
ERROR:
    goto ERROR;
}

int gl_mloop;
int gl_st2;

static void gl_proc1(void) 
{
    if (gl_mloop != 2)
        error();
}

static void gl_eval(void) 
{
    if (__VERIFIER_nondet_int()) {
        gl_proc1();
    }

    if (!gl_st2)
        gl_st2 = 1;
}

int main(void) 
{
    gl_mloop = 2;

    ___sl_plot((void *) 0);
    gl_eval();
    ___sl_plot((void *) 0);

    gl_eval();

    return 0;
}
