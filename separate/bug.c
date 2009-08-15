#if 0
#include <stdio.h>

#include <sparse/linearize.h>
#include <sparse/symbol.h>
#endif

static void test(int i)
{
                        // 2.
    while (i) {
        //i++;
        if (i - 1)      // 1.
            test(0);
        i++;            // 0.
    }
}

#if 0
static void handle_bb_list(struct basic_block_list *list, unsigned long generation)
{
    struct basic_block *bb;

    FOR_EACH_PTR(list, bb) {
        if (bb->generation == generation)
            continue;
    } END_FOR_EACH_PTR(bb);
}

static void handle_fnc_def_begin(struct symbol *sym)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct symbol *arg;
    int argc = 0;

    // dump argument list
    FOR_EACH_PTR(base_type->arguments, arg) {
        if (argc++)
            printf(", ");
        printf("%%arg%d: %s", argc, show_ident(arg->ident));
    } END_FOR_EACH_PTR(arg);
}
#endif
