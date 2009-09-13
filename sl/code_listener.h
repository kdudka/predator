#ifndef H_GUARD_CODE_LISTENER_H
#define H_GUARD_CODE_LISTENER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * type of function writing a message
 * @param msg zero-terminated string to write
 */
typedef void (*cl_print_fnc_t)(const char *msg);

/**
 * collection of call-backs for writing various types of messages
 */
struct cl_init_data {
    /**
     * function to write debug messages (perhaps empty function when not
     * debugging?)
     */
    cl_print_fnc_t debug;

    /**
     * function to write a warning message (may or may not be related to
     * location currently being processed)
     */
    cl_print_fnc_t warn;

    /**
     * function to write an error message (may or may not be related to
     * location currently being processed)
     */
    cl_print_fnc_t error;

    /**
     * function to write an information message (which usually gives some extra
     * info related to previous warning/error message)
     */
    cl_print_fnc_t note;

    /**
     * function to display critical error and then terminate the application
     * @attention This function should never return.
     */
    cl_print_fnc_t die;
};

/**
 * global initialization - it sets message printing call-backs
 * @param init_data - collection of call-backs to set (none of them can be NULL)
 * @note You should call cl_global_cleanup() to free resources before exit.
 */
void cl_global_init(struct cl_init_data *init_data);

/**
 * global initialization - it sets built-in functions to print messages
 * @param app_name - name of the application which appears in all messages. If
 * NULL is given, no application name will be printed.
 * @param verbose - if true debug messages are printed as well
 * @note You should call cl_global_cleanup() to free resources before exit.
 */
void cl_global_init_defaults(
        const char                      *app_name,
        bool                            verbose);

/**
 * free resources allocated by cl_global_init() or cl_global_init_defaults()
 */
void cl_global_cleanup(void);

/**
 * generic location info.
 * @note taken from gcc's expanded_location defined in <gcc/input.h>
 */
struct cl_location {
    /**
     * input file as zero-terminated string. If NULL, the file currently being
     * processed is taken as the input file.
     */
    const char                          *file;

    /**
     * line number in the input file (starting at 1). If the value is negative,
     * data in this structure are not considered valid at all.
     */
    int                                 line;

    /**
     * column number in the input file (starting at 1). If such information is
     * not available for the token, set the value to -1.
     */
    int                                 column;

    /**
     * true, if the token is located in a system header.
     * @note not used for now
     */
    bool                                sysp;
};

/**
 * symbol scope enumeration (linearly ordered)
 */
enum cl_scope_e {
    /**
     * scope is unlimited
     */
    CL_SCOPE_GLOBAL,

    /**
     * scope is limited to current source file
     */
    CL_SCOPE_STATIC,

    /**
     * scope is limited to currently processed function
     */
    CL_SCOPE_FUNCTION,

    /**
     * scope is limited to currently processed basic block
     */
    CL_SCOPE_BB
};

typedef int cl_type_uid_t;
enum {
    CL_UID_INVALID = -1
};

enum cl_type_e {
    CL_TYPE_VOID,
    CL_TYPE_PTR,
    CL_TYPE_STRUCT,
    CL_TYPE_UNION,
    CL_TYPE_ARRAY,
    CL_TYPE_FNC,
    CL_TYPE_INT,
    CL_TYPE_CHAR,
    CL_TYPE_BOOL,
    CL_TYPE_ENUM,

    /* cst only */
    CL_TYPE_STRING
};

struct cl_type_item {
    cl_type_uid_t                       type;
    const char                          *name;
};

struct cl_type {
    cl_type_uid_t                       uid;
    enum cl_type_e                      code;
    struct cl_location                  loc;
    enum cl_scope_e                     scope;
    const char                          *name;
    int                                 size;
    int                                 item_cnt;
    struct cl_type_item                 *items;
};

enum cl_accessor_e {
    CL_ACCESSOR_REF,
    CL_ACCESSOR_DEREF,
    CL_ACCESSOR_DEREF_ARRAY,
    CL_ACCESSOR_ITEM
};

struct cl_accessor {
    enum cl_accessor_e                  code;
    struct cl_type                      *type;
    int                                 item;
    struct cl_accessor                  *next;
};

/**
 * generic operand type enumeration
 */
enum cl_operand_e {
    /**
     * there is NO operand
     */
    CL_OPERAND_VOID,

    CL_OPERAND_CST,

    /**
     * variable. We actually do not distinguish between local/global variables,
     * temporal variables and (substituted) function arguments.
     */
    CL_OPERAND_VAR,

    /**
     * function argument. Addressed by argument ID. Currently used only by
     * slsparse, but it's atranslated by "arg_subst" cl decorator to
     * CL_OPERAND_VAR afterwards.
     */
    CL_OPERAND_ARG,

    /**
     * register. Addressed by register ID. Usually managed by compiler only.
     * @todo We should stop ignoring register's size at some point.
     * @note slsparse now ignores all operands with different size than 32bit
     */
    CL_OPERAND_REG
};

/**
 * generic operand. There is in fact no operand if type==CL_OPERAND_VOID. Each
 * type of operand (with extra data) has it's own member in the union data.
 * Members deref and offset are valid for *most* of operand types.
 */
struct cl_operand {
    /**
     * type of operand. See enum cl_operand_e for documentation.
     */
    enum cl_operand_e                   code;
    struct cl_location                  loc;
    enum cl_scope_e                     scope;

    struct cl_type                      *type;
    struct cl_accessor                  *accessor;

    /**
     * per operand type specific data
     */
    union {

        /* CL_OPERAND_VAR */
        struct {
            const char                  *name;
            /* TODO: is_extern? */
        } var;

        /* CL_OPERAND_ARG */
        struct {
            int                         id;
        } arg;

        /* CL_OPERAND_REG */
        struct {
            int                         id;
        } reg;

        /* CL_OPERAND_CST / CL_TYPE_FNC */
        struct {
            const char                  *name;
            bool                        is_extern;
        } cst_fnc;

        /* CL_OPERAND_CST / CL_TYPE_INT */
        struct {
            int                         value;
        } cst_int;

        /* CL_OPERAND_CST / CL_TYPE_STRING */
        struct {
            const char                  *value;
        } cst_string;
    } data;
};

enum cl_insn_e {
    CL_INSN_NOP,
    CL_INSN_JMP,
    CL_INSN_COND,
    CL_INSN_RET,
    CL_INSN_ABORT,
    CL_INSN_UNOP,
    CL_INSN_BINOP
    /* TODO */
};

enum cl_unop_e {
    CL_UNOP_ASSIGN,
    CL_UNOP_TRUTH_NOT
    /* TODO */
};

enum cl_binop_e {
    /* comparison */
    CL_BINOP_EQ,
    CL_BINOP_NE,
    CL_BINOP_LT,
    CL_BINOP_GT,
    CL_BINOP_LE,
    CL_BINOP_GE,

    /* logical */
    CL_BINOP_TRUTH_AND,
    CL_BINOP_TRUTH_OR,
    CL_BINOP_TRUTH_XOR,

    /* arithmetic */
    CL_BINOP_PLUS,
    CL_BINOP_MINUS,
    CL_BINOP_TRUNC_DIV,
    CL_BINOP_TRUNC_MOD,
    CL_BINOP_RDIV,
    CL_BINOP_MIN,
    CL_BINOP_MAX
    /* TODO */
};

struct cl_insn {
    enum cl_insn_e                      code;
    struct cl_location                  loc;

    /* instruction specific data */
    union {

        /* CL_INSN_JMP */
        struct {
            const char                  *label;
        } insn_jmp;

        /* CL_INSN_COND */
        struct {
            const struct cl_operand     *src;
            const char                  *then_label;
            const char                  *else_label;
        } insn_cond;

        /* CL_INSN_RET */
        struct {
            const struct cl_operand     *src;
        } insn_ret;

        /* CL_INSN_UNOP */
        struct {
            enum cl_unop_e              code;
            const struct cl_operand     *dst;
            const struct cl_operand     *src;
        } insn_unop;

        /* CL_INSN_BINOP */
        struct {
            enum cl_binop_e             code;
            const struct cl_operand     *dst;
            const struct cl_operand     *src1;
            const struct cl_operand     *src2;
        } insn_binop;

    } data;
};

typedef struct cl_type* (*cl_get_type_fnc_t)(cl_type_uid_t, void *);

/**
 * listener object - the core part of this interface
 *
 * It accepts a context-free language defined by substitution to regex:
 *
 *     (file_open FILE_CONTENT file_close)* destroy
 *
 *
 * FILE_CONTENT is defined by substitution to regex:
 *
 *     fnc_open (decl_type | fnc_arg_decl)* FNC_BODY fnc_close
 *
 *
 * FNC_BODY is defined by substitution to regex:
 *
 *     FNC_ENTRY (bb_open (decl_type | NONTERM_INSN)* TERM_INSN)*
 *
 *
 * FNC_ENTRY is defined as:
 *
 *     insn{CL_INSN_JMP}
 *
 *
 * NON_TERM_INSN is defined as:
 *
 *     INSN_CALL | insn{CL_INSN_UNOP, CL_INSN_UNOP}
 *
 *
 * TERM_INSN is defined as:
 *
 *     insn{CL_INSN_JMP, CL_INSN_COND, CL_INSN_RET, CL_INSN_ABORT} | INSN_SWITCH
 *
 *
 * INSN_CALL is defined by regex:
 *
 *     insn_call_open (insn_call_arg)* insn_call_close
 *
 *
 * INSN_SWITCH is defined by regex:
 *
 *     insn_switch_open (insn_switch_case)* insn_switch_close
 *
 *
 * @todo avoid (re)formating in dox output
 */
struct cl_code_listener {
    /**
     * listener's internal data. Do not use this member.
     */
    void *data;

    void (*reg_type_db)(
            struct cl_code_listener     *self,
            cl_get_type_fnc_t           fnc,
            void                        *user_data);

    void (*file_open)(
            struct cl_code_listener     *self,
            const char                  *file_name);

    void (*file_close)(
            struct cl_code_listener     *self);

    void (*fnc_open)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const char                  *fnc_name,
            enum cl_scope_e             scope);

    void (*fnc_arg_decl)(
            struct cl_code_listener     *self,
            int                         arg_id,
            const struct cl_operand     *arg_src);

    void (*fnc_close)(
            struct cl_code_listener     *self);

    void (*bb_open)(
            struct cl_code_listener     *self,
            const char                  *label);

    void (*insn)(
            struct cl_code_listener     *self,
            const struct cl_insn        *insn);

    void (*insn_call_open)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *dst,
            const struct cl_operand     *fnc);

    void (*insn_call_arg)(
            struct cl_code_listener     *self,
            int                         arg_id,
            const struct cl_operand     *arg_src);

    void (*insn_call_close)(
            struct cl_code_listener     *self);

    void (*insn_switch_open)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *src);

    void (*insn_switch_case)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *val_lo,
            const struct cl_operand     *val_hi,
            const char                  *label);

    void (*insn_switch_close)(
            struct cl_code_listener     *self);

    void (*destroy)(
            struct cl_code_listener     *self);
};

struct cl_code_listener* cl_code_listener_create(const char *config_string);

/**
 * @todo document
 */
struct cl_code_listener* cl_chain_create(void);

/**
 * @todo document
 */
void cl_chain_append(
        struct cl_code_listener         *chain,
        struct cl_code_listener         *listener);

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_CODE_LISTENER_H */
