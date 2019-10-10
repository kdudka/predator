static void error(void) 
{
ERROR:
    goto ERROR;
}

int gl_num;
int gl_st1;
int gl_ev1;

static void gl_write(int i) 
{
    switch (i) {
        case 0: i = 1;
        case 1:
            break;

        default:
            error();
    }
}

static void gl_proc1(void) 
{
    gl_write(gl_num++);
    gl_st1 = 2;
}

static void gl_eval(void) 
{
    if (!gl_st1)
        gl_proc1();
}

static void gl_activate(void) 
{
    if (1 == gl_ev1)
        gl_st1 = 0;
}

int main(void) 
{
    gl_ev1 = 1;

    gl_eval();
    gl_activate();

    gl_eval();
    gl_activate();

    gl_eval();

    return 0;
}

/**
 * @file test-0187.c
 *
 * @brief test-0186 narrowed down to a minimal example
 *
 * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */
