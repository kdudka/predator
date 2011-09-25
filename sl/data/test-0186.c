static void error(void) 
{
ERROR:
    goto ERROR;
}

int gl_mloop;
int gl_num;
int gl_cnt;
int gl_zero;
int gl_time;
char gl_d0;
char gl_d1;

static char gl_read(int i) 
{
    if (!i)
        return gl_d0;

    if (1 != i)
        error();

    return gl_d1;
}

static void gl_write(int i , char c) 
{
    if (!i) {
        gl_d0 = c;
        return;
    }

    if (1 != i)
        error();

    gl_d1 = c;
}

int gl_pc1;
int gl_st1;
int gl_i1;
int gl_ev1;

static void gl_proc1(void) 
{
    if (gl_mloop <= gl_cnt) {
        gl_st1 = 2;
        return;
    }

    gl_write(gl_num, 'A');
    gl_num += 1;
    gl_pc1 = 1;
    gl_st1 = 2;
}

static int gl_trig1(void) 
{
    return (1 == gl_pc1)
        && (1 == gl_ev1);
}

int gl_pc2;
int gl_st2;
int gl_i2;
int gl_ev2;

static void gl_proc2(void) 
{
    if (1 == gl_pc2 || (gl_cnt < gl_mloop && gl_num)) {
        gl_num -= 1;
        if (! (gl_num >= 0))
            error();

        gl_read(gl_num);
        gl_cnt += 1;
        gl_pc2 = 2;
        gl_st2 = 2;
        return;
    }

    if (gl_cnt < gl_mloop && !gl_num) {
        gl_time = 1;
        gl_cnt += 1;
        gl_pc2 = 1;
        gl_st2 = 2;
        return;
    }

    gl_st2 = 2;
}

static int gl_trig2(void) 
{
    return (1 == gl_pc2 && 1 == gl_zero)
        || (2 == gl_pc2 && 1 == gl_ev2);
}

static void gl_init(void) 
{
    if (1 == gl_i1)
        gl_st1 = 0;
    else
        gl_st1 = 2;

    if (1 == gl_i2)
        gl_st2 = 0;
    else
        gl_st2 = 2;
}

static int gl_runnable(void) 
{
    return !gl_st1
        || !gl_st2;
}

static void gl_eval(void) 
{
    while (gl_runnable()) {
        if (!gl_st1 && __VERIFIER_nondet_int()) {
            gl_st1 = 1;
            gl_proc1();
        }

        if (!gl_st2 && __VERIFIER_nondet_int()) {
            gl_st2 = 1;
            gl_proc2();
        }
    }
}

static void gl_fire(void) 
{
    gl_ev2 = 1;
    gl_ev1 = 1;
}

static void gl_reset(void) 
{
    if (1 == gl_ev1)
        gl_ev1 = 2;

    if (1 == gl_ev2)
        gl_ev2 = 2;
}

static void gl_activate(void) 
{
    if (gl_trig1())
        gl_st1 = 0;

    if (gl_trig2())
        gl_st2 = 0;
}

int main(void) 
{
    gl_num      = 0;
    gl_cnt      = 0;
    gl_mloop    = 2;
    gl_time     = 0;
    gl_pc1      = 0;
    gl_pc2      = 0;
    gl_i1       = 1;
    gl_i2       = 1;

    gl_init();
    gl_activate();

    do {
        gl_eval();

        gl_activate();

        if (!gl_runnable()) {
            gl_fire();
            gl_activate();
            gl_reset();
        }

    }
    while (gl_runnable());

    return 0;
}
