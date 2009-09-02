static void foo(void) {
    enum ENUM_TYPE_A { VALUE_A } var_a;
    enum ENUM_TYPE_B { VALUE_B } var_b;
    enum /* anon. */ { VALUE_C } anon_enum_var;
    int i;

    // always OK
    var_a = VALUE_A;
    var_a = (enum ENUM_TYPE_A) VALUE_B;
    var_b = (enum ENUM_TYPE_B) i;
    i = (int) VALUE_A;
    anon_enum_var = VALUE_C;
    i = VALUE_C;
    i = anon_enum_var;
    i = 7;

    // caught by -Wenum-mismatch (default) even without the patch
    var_a = var_b;
    var_b = anon_enum_var;
    anon_enum_var = var_a;

    // caught by -Wenum-mismatch (default) only with the patch applied
    var_a = VALUE_B;
    var_b = VALUE_C;
    anon_enum_var = VALUE_A;

    // caught by -Wint-to-enum (default)
    var_a = 0;
    var_b = i;
    anon_enum_var = 0;
    anon_enum_var = i;
    var_a = (int) VALUE_A;
    var_a = (int) VALUE_B;

    // caught only with -Wenum-to-int
    i = var_a;
    i = VALUE_B;
}
