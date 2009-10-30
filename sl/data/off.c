struct s {
    char a;
    char *b;
    int c[7];
    void (*f)(void);
};

void foo(void) {
    struct s s;
    s.a = 'a';
    s.b = "b";
    s.c[3] = 3;
    s.f = foo;
}
