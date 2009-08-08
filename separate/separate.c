#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// sparse headers
#include <sparse/expression.h>
#include <sparse/flow.h>
#include <sparse/linearize.h>
#include <sparse/parse.h>
#include <sparse/storage.h>
#include <sparse/symbol.h>
#include <sparse/token.h>

// safe to remove (it's here for debugging purposes only)
#include <signal.h>
#define TRAP raise(SIGTRAP)

#define DO_EXPAND_SYMBOL            1
#define DO_PER_EP_UNSAA             1
#define DO_PER_EP_SET_UP_STORAGE    1
#define SHOW_PSEUDO_INSNS           0

#define WARN_UNHANDLED(what) \
    fprintf(stderr, "--- %s: %d: warning: '%s' not handled\n", \
            __FUNCTION__, __LINE__, (what))

#define WARN_UNHANDLED_SYM(sym) \
    WARN_UNHANDLED(show_ident(sym->ident))

#define WARN_VA(...) do {\
    fprintf(stderr, "--- %s: %d: warning: ", \
            __FUNCTION__, __LINE__); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

#define CASE_UNHANDLED(what) \
    case what: WARN_UNHANDLED(#what); break;

// FIXME: hard-coded for now
static const unsigned KNOWN_PTR_SIZE = 32;

static void handle_bb(struct basic_block *bb, unsigned long generation);

#if 0
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
#endif

static bool is_pseudo(pseudo_t pseudo)
{
    return pseudo
        && pseudo != VOID;
}

// TODO: simplify
static bool print_pseudo(pseudo_t pseudo)
{
    if (!is_pseudo(pseudo))
        return false;

    switch(pseudo->type) {
        case PSEUDO_SYM: {
            struct symbol *sym = pseudo->sym;
            struct expression *expr;

            if (sym->bb_target) {
                printf("@%p", sym->bb_target);
                break;
            }
            if (sym->ident) {
                printf("%s", show_ident(sym->ident));
                break;
            }
            expr = sym->initializer;
            if (expr) {
                switch (expr->type) {
#if 0
                    case EXPR_VALUE:
                        printf("<symbol value: %lld>", expr->value);
                        break;
#endif
                    case EXPR_STRING:
                        printf("%s", show_string(expr->string));
                        break;

                    default:
                        TRAP;
                        break;
                }
            }
             break;
        }

        case PSEUDO_REG:
#if 0
            if (pseudo->ident)
                printf("/* %s */ ", show_ident(pseudo->ident));
#endif
            printf("r%d", pseudo->nr);
            break;

        case PSEUDO_VAL: {
            long long value = pseudo->value;
            if (value > 1000 || value < -1000)
                printf("%#llx", value);
            else
                printf("%lld", value);
            break;
        }

        case PSEUDO_ARG:
            printf("%%arg%d", pseudo->nr);
            break;

        case PSEUDO_PHI:
            printf("%%phi%d", pseudo->nr);
            if (pseudo->ident)
                printf("\t(%s)", show_ident(pseudo->ident));
            break;

        default:
            TRAP;
    }

    return true;
}

static void print_pseudo_symbol(struct instruction *insn)
{
    const struct ident *id = insn->type->ident;

    printf("[");
    print_pseudo(insn->symbol);
    if (id)
        printf(":%s", show_ident(id));
    printf("]");
}

static void print_insn_call(struct instruction *insn)
{
    struct pseudo *arg;
    int cnt = 0;

    if (print_pseudo(insn->target))
        printf(" := ");

    printf("%s(", show_ident(insn->func->ident));

    FOR_EACH_PTR(insn->arguments, arg) {
        if (cnt++)
            printf(", ");
        print_pseudo(arg);
    } END_FOR_EACH_PTR(arg);

    printf(")");
}

static void print_insn_br(struct instruction *insn)
{
    if (!is_pseudo(insn->cond)) {
        printf("goto @%p", insn->bb_true);
        return;
    }

    printf("if (");
    print_pseudo(insn->cond);
    printf(")\n\t\t\tgoto @%p\n\t\telse\n\t\t\tgoto @%p",
           insn->bb_true,
           insn->bb_false);
}

static void print_insn_ret(struct instruction *insn)
{
    printf("ret");
    if (!is_pseudo(insn->/*src*/symbol))
        return;

    printf(" ");
    print_pseudo_symbol(insn);
}

static void print_insn_store(struct instruction *insn)
{
    print_pseudo_symbol(insn);
    printf(" := ");
    print_pseudo(insn->target);
}

static void print_insn_load(struct instruction *insn)
{
    print_pseudo(insn->target);
    printf(" := ");
    print_pseudo_symbol(insn);
}

static void print_insn_copy(struct instruction *insn)
{
    // TODO: check direction (store/load)
    print_insn_load(insn);
}

static void print_insn_phisource(struct instruction *insn)
{
#if 1
    printf("%s", show_instruction(insn));
#else
    // TODO: check direction (store/load)
    print_insn_load(insn);
#endif
}

static void print_insn(struct instruction *insn)
{
    switch (insn->opcode) {
        CASE_UNHANDLED(OP_BADOP)

        /* Entry */
        case OP_ENTRY:
            // ignore for now
            break;

        /* Terminator */
        case OP_RET /*= OP_TERMINATOR*/:
            print_insn_ret(insn);
            break;

        case OP_BR:
            print_insn_br(insn);
            break;

        CASE_UNHANDLED(OP_SWITCH)
        CASE_UNHANDLED(OP_INVOKE)
        CASE_UNHANDLED(OP_COMPUTEDGOTO)
        CASE_UNHANDLED(OP_TERMINATOR_END /*= OP_UNWIND*/)

        /* Binary */
        CASE_UNHANDLED(OP_ADD /*= OP_BINARY*/)
        CASE_UNHANDLED(OP_SUB)
        CASE_UNHANDLED(OP_MULU)
        CASE_UNHANDLED(OP_MULS)
        CASE_UNHANDLED(OP_DIVU)
        CASE_UNHANDLED(OP_DIVS)
        CASE_UNHANDLED(OP_MODU)
        CASE_UNHANDLED(OP_MODS)
        CASE_UNHANDLED(OP_SHL)
        CASE_UNHANDLED(OP_LSR)
        CASE_UNHANDLED(OP_ASR)

        /* Logical */
        CASE_UNHANDLED(OP_AND)
        CASE_UNHANDLED(OP_OR)
        CASE_UNHANDLED(OP_XOR)
        CASE_UNHANDLED(OP_AND_BOOL)
        CASE_UNHANDLED(OP_BINARY_END /*= OP_OR_BOOL*/)

        /* Binary comparison */
        CASE_UNHANDLED(OP_SET_EQ /*= OP_BINCMP*/)
        CASE_UNHANDLED(OP_SET_NE)
        CASE_UNHANDLED(OP_SET_LE)
        CASE_UNHANDLED(OP_SET_GE)
        CASE_UNHANDLED(OP_SET_LT)
        CASE_UNHANDLED(OP_SET_GT)
        CASE_UNHANDLED(OP_SET_B)
        CASE_UNHANDLED(OP_SET_A)
        CASE_UNHANDLED(OP_SET_BE)
        CASE_UNHANDLED(OP_BINCMP_END /*= OP_SET_AE*/)

        /* Uni */
        CASE_UNHANDLED(OP_NOT)
        CASE_UNHANDLED(OP_NEG)

        /* Select - three input values */
        CASE_UNHANDLED(OP_SEL)

        /* Memory */
        CASE_UNHANDLED(OP_MALLOC)
        CASE_UNHANDLED(OP_FREE)
        CASE_UNHANDLED(OP_ALLOCA)
        case OP_LOAD:
            print_insn_load(insn);
            break;

        case OP_STORE:
            print_insn_store(insn);
            break;

        CASE_UNHANDLED(OP_SETVAL)
        CASE_UNHANDLED(OP_SYMADDR)
        CASE_UNHANDLED(OP_GET_ELEMENT_PTR)

        /* Other */
        CASE_UNHANDLED(OP_PHI)
        case OP_PHISOURCE:
            print_insn_phisource(insn);
            break;

        case OP_CAST:
        case OP_SCAST:
        case OP_FPCAST:
        case OP_PTRCAST:
            // TODO: separate handler?
            print_insn_copy(insn);
            break;

        CASE_UNHANDLED(OP_INLINED_CALL)
        case OP_CALL:
            print_insn_call(insn);
            break;

        CASE_UNHANDLED(OP_VANEXT)
        CASE_UNHANDLED(OP_VAARG)
        CASE_UNHANDLED(OP_SLICE)
        case OP_SNOP:
            //print_insn_store(insn);
            printf("%s", show_instruction(insn));
            break;

        case OP_LNOP:
            //print_insn_load(insn);
            printf("%s", show_instruction(insn));
            break;

        CASE_UNHANDLED(OP_NOP)
        CASE_UNHANDLED(OP_DEATHNOTE)
        CASE_UNHANDLED(OP_ASM)

        /* Sparse tagging (line numbers, context, whatever) */
        CASE_UNHANDLED(OP_CONTEXT)
        CASE_UNHANDLED(OP_RANGE)

        /* Needed to translate SSA back to normal form */
        case OP_COPY:
            print_insn_copy(insn);
            break;
    }
}

static bool is_insn_interesting(struct instruction *insn)
{
    unsigned size = insn->size;
    if (size && KNOWN_PTR_SIZE != size) {
        WARN_VA("ignored instruction with operand size %d", insn->size);
        return false;
    }

    switch (insn->opcode) {
        case OP_ENTRY:
            return false;

        default:
            return true;
    }
}

static void handle_bb_insn(struct instruction *insn)
{
    if (!insn)
        return;

    if (!insn->bb) {
#if SHOW_PSEUDO_INSNS
        WARN_VA("ignoring pseudo: %s", show_instruction(insn));
#endif
        return;
    }

    if (!is_insn_interesting(insn))
        return;

    printf("\t\t");
    print_insn(insn);
    printf("\n");
}

static void handle_bb_content(struct basic_block *bb)
{
    struct instruction *insn;

    if (!bb)
        return;

    printf("\t@%p:\n", bb);
    FOR_EACH_PTR(bb->insns, insn) {
        handle_bb_insn(insn);
    } END_FOR_EACH_PTR(insn);
    printf("\n");
}

static void handle_bb_list(struct basic_block_list *list, unsigned long generation)
{
    struct basic_block *bb;

    FOR_EACH_PTR(list, bb) {
        if (bb->generation == generation)
            continue;
        handle_bb(bb, generation);
    } END_FOR_EACH_PTR(bb);
}

static void handle_bb(struct basic_block *bb, unsigned long generation)
{
    bb->generation = generation;

    handle_bb_list(bb->parents, generation);
    handle_bb_content(bb);
    handle_bb_list(bb->children, generation);
}

static void handle_fnc_body(struct symbol *sym)
{
    struct entrypoint *ep = linearize_symbol(sym);
    if (!ep)
        TRAP;

#if DO_PER_EP_UNSAA
    unssa(ep);
#endif

#if DO_PER_EP_SET_UP_STORAGE
    set_up_storage(ep);
#endif

    handle_bb(ep->entry->bb, ++bb_generation);

#if DO_PER_EP_SET_UP_STORAGE
    free_storage();
#endif
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
        if (argc++)
            printf(", ");
        printf("/* arg%d */ %s", argc, show_ident(arg->ident));
    } END_FOR_EACH_PTR(arg);

    printf("):\n");
}

static void handle_fnc_def_end(struct symbol *sym)
{
    // function definition complete
    printf("\n");
}

static void handle_sym_fn(struct symbol *sym)
{
    struct symbol *base_type = sym->ctype.base_type;
    struct statement *stmt = base_type->stmt;

    if (stmt) {
        // function definition
        handle_fnc_def_begin(sym);
        handle_fnc_body(sym);
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
#if DO_EXPAND_SYMBOL
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
        printf("%s: processing '%s'...\n", argv[0], file);
        clean_up_symbols(sparse(file));
    } END_FOR_EACH_PTR_NOTAG(file);

    return 0;
}
