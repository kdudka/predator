#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// sparse headers
#include <sparse/expression.h>
#include <sparse/lib.h>
#include <sparse/parse.h>
#include <sparse/symbol.h>
#include <sparse/token.h>

#define WARN_UNHANDLED(what) \
    fprintf(stderr, "--- %s: %d: warning: '%s' not handled\n", \
            __FUNCTION__, __LINE__, (what))

#define WARN_UNHANDLED_SYM(sym) \
    WARN_UNHANDLED(show_ident(sym->ident))

#define CASE_UNHANDLED(what) \
    case what: WARN_UNHANDLED(#what); break;

// FIXME: global variable
static int nest_level = 0;

static void handle_expr(struct expression *expr);
static void handle_stmt(struct statement *stmt);

// input: global variable nest_level
static void print_indent(void)
{
    int i;
    for (i = 0; i < nest_level; ++i)
        printf("    ");
}

// input: global variable nest_level
static void print_nl_indent(void)
{
    printf("\n");
    print_indent();
}

static void handle_fnc_def_begin(struct symbol *sym)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct symbol *arg;
    int argc = 0;

    // write function name
    printf("%s(", show_ident(sym->ident));

    // dump argument list
    FOR_EACH_PTR(base_type->arguments, arg) {
        if (argc)
            printf(", ");
        printf("%s", show_ident(arg->ident));
        argc++;
    } END_FOR_EACH_PTR(arg);

    printf(")\n");
}

static void handle_fnc_def_end(struct symbol *sym)
{
    // function definition complete
    printf("\n\n");
}

static void handle_expr_call (struct expression *expr)
{
    struct expression *fn = expr->fn;
    struct symbol *direct = NULL;
    struct expression *arg;
    int i = 0;

    // dereference (look for function name)
    if (fn->type == EXPR_PREOP
            && fn->unop->type == EXPR_SYMBOL) {

        struct symbol *sym = fn->unop->symbol;
        if (sym->ctype.base_type->type == SYM_FN)
            direct = sym;
    }

    // dereference failed
    if (!direct) {
        WARN_UNHANDLED_SYM(fn->ctype);
        return;
    }

    // dump function call using usual C syntax
    printf("%s(", show_ident(direct->ident));
    FOR_EACH_PTR(expr->args, arg) {
        if (i++)
            printf(", ");
        handle_expr(arg);
    } END_FOR_EACH_PTR(arg);
    printf(")");
}

static void handle_expr_assignment (struct expression *expr)
{
    handle_expr(expr->left);
    printf(" := ");
    handle_expr(expr->right);
}

static void handle_expr_preop (struct expression *expr)
{
    struct expression *unop = expr->unop;
    bool last = (unop->type == EXPR_SYMBOL);

    if (expr->op != '*') {
        // operator handling not implemented yet

        char *msg;
        if (-1 == asprintf(&msg, "op = '%c'", expr->op))
            die("asprintf failed");

        WARN_UNHANDLED(msg);
        free(msg);
        return;
    }

    // use [E] notation for dereferencing E
    if (!last)
        printf("[");
    handle_expr(unop);
    if (!last)
        printf("]");
}

static void handle_expr_symbol (struct expression *expr)
{
    // print symbol name
    printf("%s", show_ident(expr->symbol->ident));
}

static void handle_expr_cast (struct expression *expr)
{
    // ignore cast
    handle_expr(expr->cast_expression);
}

static void handle_expr (struct expression *expr)
{
    if (!expr)
        return;

    switch (expr->type) {
        CASE_UNHANDLED(EXPR_VALUE)
        CASE_UNHANDLED(EXPR_STRING)
        CASE_UNHANDLED(EXPR_TYPE)
        CASE_UNHANDLED(EXPR_BINOP)
        CASE_UNHANDLED(EXPR_LOGICAL)
        CASE_UNHANDLED(EXPR_DEREF)
        CASE_UNHANDLED(EXPR_POSTOP)
        CASE_UNHANDLED(EXPR_SIZEOF)
        CASE_UNHANDLED(EXPR_ALIGNOF)
        CASE_UNHANDLED(EXPR_PTRSIZEOF)
        CASE_UNHANDLED(EXPR_CONDITIONAL)
        CASE_UNHANDLED(EXPR_SELECT)
        CASE_UNHANDLED(EXPR_STATEMENT)
        CASE_UNHANDLED(EXPR_COMMA)
        CASE_UNHANDLED(EXPR_COMPARE)
        CASE_UNHANDLED(EXPR_LABEL)
        CASE_UNHANDLED(EXPR_INITIALIZER)
        CASE_UNHANDLED(EXPR_IDENTIFIER)
        CASE_UNHANDLED(EXPR_INDEX)
        CASE_UNHANDLED(EXPR_POS)
        CASE_UNHANDLED(EXPR_FVALUE)
        CASE_UNHANDLED(EXPR_SLICE)
        CASE_UNHANDLED(EXPR_OFFSETOF)

        case EXPR_CALL:
            handle_expr_call(expr);
            break;

        case EXPR_ASSIGNMENT:
            handle_expr_assignment(expr);
            break;

        case EXPR_PREOP:
            handle_expr_preop(expr);
            break;

        case EXPR_SYMBOL:
            handle_expr_symbol(expr);
            break;

        case EXPR_CAST:
        case EXPR_FORCE_CAST:
        case EXPR_IMPLIED_CAST:
            handle_expr_cast(expr);
            break;
    }
}

static void handle_sym_decl(struct symbol *sym)
{
    struct expression *expr = sym->initializer;

    printf("/* local variable */ %s", show_ident(sym->ident));
    if (expr) {
        // variable initialization
        printf(" := ");
        handle_expr(expr);
    }
}

static void handle_sym_list_decl(struct symbol_list *syms)
{
    struct symbol *sym;
    int i = 0;

    // go through all declarations
    FOR_EACH_PTR(syms, sym) {
        if (i++)
            printf(", ");
        handle_sym_decl(sym);
    } END_FOR_EACH_PTR(sym);
}

static void print_while(struct expression *expr)
{
    // print "while (E)"
    printf("while (");
    handle_expr(expr);
    printf(")");
}

static void handle_stmt_compound(struct statement *stmt)
{
    struct statement *s;

    // enter nested block
    printf("{\n");
    ++nest_level;

    // handle all nested statements
    FOR_EACH_PTR(stmt->stmts, s) {
        print_indent();
        handle_stmt(s);
        printf("\n");
    } END_FOR_EACH_PTR(s);

    // leave nested block
    --nest_level;
    print_indent();
    printf("}");
}

static void handle_stmt_iterator(struct statement *stmt)
{
    struct symbol_list *decls = stmt->iterator_syms;
    struct statement *pre_statement = stmt->iterator_pre_statement;
    struct expression *pre_condition = stmt->iterator_pre_condition;
    struct statement *iterator_statement = stmt->iterator_statement;
    struct statement *post_statement = stmt->iterator_post_statement;
    struct expression *post_condition = stmt->iterator_post_condition;

    if (decls) {
        // 'for' local variables
        handle_sym_list_decl(decls);
        print_nl_indent();
    }

    if (pre_statement) {
        // 'for' initialization
        handle_stmt(pre_statement);
        print_nl_indent();
    }

    if (pre_condition)
        // 'for/while' condition
        print_while(pre_condition);
    else
        printf("do");

    // loop body
    ++nest_level;
    if (iterator_statement) {
        print_nl_indent();
        handle_stmt(iterator_statement);
    }

    // 'for' increment part
    if (post_statement) {
        handle_stmt(post_statement);
        print_nl_indent();
    }

    --nest_level;
    if (post_condition) {
        // 'do-while' condition
        printf(" ");
        print_while(post_condition);
    }
}

static void handle_stmt_if(struct statement *stmt)
{
    // if
    printf("if (");
    handle_expr(stmt->if_conditional);
    printf(")\n");

    // then
    ++nest_level;
    print_indent();
    handle_stmt(stmt->if_true);
    --nest_level;

    // else
    if (stmt->if_false) {
        print_nl_indent();
        printf("else\n");

        ++nest_level;
        handle_stmt(stmt->if_false);
        --nest_level;
    }
}

static void handle_stmt_declaration(struct statement *stmt)
{
    handle_sym_list_decl(stmt->declaration);
}

static void handle_stmt_expression(struct statement *stmt)
{
    handle_expr(stmt->expression);
}

static void handle_stmt(struct statement *stmt)
{
    if (!stmt)
        return;

    switch (stmt->type) {
        CASE_UNHANDLED(STMT_NONE)
        CASE_UNHANDLED(STMT_RETURN)
        CASE_UNHANDLED(STMT_CASE)
        CASE_UNHANDLED(STMT_SWITCH)
        CASE_UNHANDLED(STMT_LABEL)
        CASE_UNHANDLED(STMT_GOTO)
        CASE_UNHANDLED(STMT_ASM)
        CASE_UNHANDLED(STMT_CONTEXT)
        CASE_UNHANDLED(STMT_RANGE)

        case STMT_COMPOUND:
            handle_stmt_compound(stmt);
            break;

        case STMT_ITERATOR:
            handle_stmt_iterator(stmt);
            break;

        case STMT_IF:
            handle_stmt_if(stmt);
            break;

        case STMT_DECLARATION:
            handle_stmt_declaration(stmt);
            break;

        case STMT_EXPRESSION:
            handle_stmt_expression(stmt);
            break;
    }
}

static void handle_sym_fn(struct symbol *sym)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct statement *stmt = base_type->stmt;

    if (stmt) {
        // function definition
        handle_fnc_def_begin(sym);
        handle_stmt(stmt);
        handle_fnc_def_end(sym);
        return;
    }

    WARN_UNHANDLED_SYM(sym);
}

static void handle_top_level_sym(struct symbol *sym)
{
    struct symbol *base_type;

    if (!sym)
        return;

    base_type = sym->ctype.base_type;
    if (!base_type)
        return;

    switch (base_type->type) {
        CASE_UNHANDLED(SYM_UNINITIALIZED)
        CASE_UNHANDLED(SYM_PREPROCESSOR)
        CASE_UNHANDLED(SYM_BASETYPE)
        CASE_UNHANDLED(SYM_NODE)
        CASE_UNHANDLED(SYM_PTR)
        CASE_UNHANDLED(SYM_ARRAY)
        CASE_UNHANDLED(SYM_STRUCT)
        CASE_UNHANDLED(SYM_UNION)
        CASE_UNHANDLED(SYM_ENUM)
        CASE_UNHANDLED(SYM_TYPEDEF)
        CASE_UNHANDLED(SYM_TYPEOF)
        CASE_UNHANDLED(SYM_MEMBER)
        CASE_UNHANDLED(SYM_BITFIELD)
        CASE_UNHANDLED(SYM_LABEL)
        CASE_UNHANDLED(SYM_RESTRICT)
        CASE_UNHANDLED(SYM_FOULED)
        CASE_UNHANDLED(SYM_KEYWORD)
        CASE_UNHANDLED(SYM_BAD)

        case SYM_FN:
            handle_sym_fn(sym);
            break;
    }

    if (sym->initializer)
        WARN_UNHANDLED("sym->initializer");
}

static void clean_up_symbols(struct symbol_list *list)
{
    struct symbol *sym;

    FOR_EACH_PTR(list, sym) {
#if 1
        expand_symbol(sym);
#endif
        handle_top_level_sym(sym);
    } END_FOR_EACH_PTR(sym);
}

int main(int argc, char **argv)
{
    char *file;
    struct string_list *filelist = NULL;

#if 1
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    clean_up_symbols(sparse_initialize(argc, argv, &filelist));

    FOR_EACH_PTR_NOTAG(filelist, file) {
        clean_up_symbols(sparse(file));
    } END_FOR_EACH_PTR_NOTAG(file);

    return 0;
}
