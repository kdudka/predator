static void error(void) 
{
ERROR:
    goto ERROR;
}

int gl_mloop;
int gl_num;
int gl_st1;
int gl_ev1;
int gl_cnt;

static void gl_write(int i) 
{
    switch (i) {
        case 0:
        case 1:
            break;

        default:
            error();
    }
}

static void gl_proc1(void) 
{
    if (gl_mloop <= gl_cnt)
        return;

    gl_write(gl_num);
    gl_num += 1;
}

int gl_st2;

static void gl_eval(void) 
{
    while (!gl_st1 || !gl_st2) {
        if (!gl_st1 && __VERIFIER_nondet_int()) {
            gl_st1 = 1;
            gl_proc1();
        }

        if (!gl_st2 && __VERIFIER_nondet_int()) {
            gl_st2 = 1;
            gl_num -= 1;
            gl_cnt += 1;
        }
    }
}

static void gl_activate(void) 
{
    if (1 == gl_ev1)
        gl_st1 = 0;
}

int main(void) 
{
    gl_ev1      = 1;
    gl_mloop    = 2;

    gl_eval();
    gl_activate();

    gl_eval();
    gl_activate();

    gl_eval();

    return 0;
}
