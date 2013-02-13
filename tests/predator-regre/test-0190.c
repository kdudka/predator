#include <verifier-builtins.h>

static void error(void) 
{
ERROR:
    goto ERROR;
}
extern int __VERIFIER_nondet_int(void);
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

    __VERIFIER_plot((void *) 0);
    gl_eval();
    __VERIFIER_plot((void *) 0);

    gl_eval();

    return 0;
}

/**
 * @file test-0190.c
 *
 * @brief test-0189 narrowed down to a minimal example
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
