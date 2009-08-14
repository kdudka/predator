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
 * global initialization - it sets call-backs for printing messages
 * @param init_data - collection of call-backs to set (none of them can be NULL)
 * @note You should call cl_global_cleanup() to free resources before exit.
 */
void cl_global_init(
        struct cl_init_data         *init_data);

/**
 * global initialization - it selects built-in functions for printing messages
 * @param app_name - name of the application which appears in all messages
 * @param verbose - if true debug messages are printed as well
 * @note You should call cl_global_cleanup() to free resources before exit.
 */
void cl_global_init_defaults(
        const char                  *app_name,
        bool                        verbose);

/**
 * free resources allocated by cl_global_init() or cl_global_init_defaults()
 */
void cl_global_cleanup(void);

enum cl_scope_e {
    CL_SCOPE_GLOBAL,
    CL_SCOPE_STATIC
    /* TODO */
};

enum cl_operand_e {
    CL_OPERAND_VOID,
    CL_OPERAND_VAR,
    CL_OPERAND_DEREF,
    CL_OPERAND_STRING,
    CL_OPERAND_INT
    /* TODO */
};

enum cl_unop_e {
    CL_UNOP_ASSIGN
    /* TODO */
};

enum cl_binop_e {
    CL_UNOP_ADD
    /* TODO */
};

union cl_value {
    const char                      *text;          /* CL_OPERAND_STRING */
    int                             num_int;        /* CL_OPERAND_INT */
    /* TODO */
};

struct cl_operand {
    enum cl_operand_e               type;
    const char                      *name;
    const char                      *offset;
    union cl_value                  value;
    /* TODO */
};

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
 *     fnc_open (fnc_arg_decl)* FNC_CONTENT fnc_close
 *
 *
 * FNC_CONTENT is defined by substitution to regex:
 *
 *     bb_open (NONTERM_INSN)* TERM_INSN
 *
 *
 * NON_TERM_INSN is defined as:
 *
 *     INSN_CALL | insn_unop | insn_binop
 *
 *
 * TERM_INSN is defined as:
 *
 *     insn_jmp | insn_cond | insn_ret
 *
 * INSN_CALL is defined by regex:
 *
 *     insn_call_open (insn_call_arg)* insn_call_close
 *
 * @todo avoid (re)formating in dox output
 */
struct cl_code_listener {
    /**
     * listener's internal data. Do not use this member.
     */
    void *data;

    void (*file_open)(
            struct cl_code_listener *self,
            const char              *file_name);

    void (*file_close)(
            struct cl_code_listener *self);

    void (*fnc_open)(
            struct cl_code_listener *self,
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope);

    void (*fnc_arg_decl)(
            struct cl_code_listener *self,
            int                     arg_pos,
            const char              *arg_name);

    void (*fnc_close)(
            struct cl_code_listener *self);

    void (*bb_open)(
            struct cl_code_listener *self,
            const char              *bb_name);

    void (*insn_jmp)(
            struct cl_code_listener *self,
            int                     line,
            const char              *label);

    void (*insn_cond)(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false);

    void (*insn_ret)(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *src);

    void (*insn_unop)(
            struct cl_code_listener *self,
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src);

    void (*insn_binop)(
            struct cl_code_listener *self,
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2);

    void (*insn_call_open)(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name);

    void (*insn_call_arg)(
            struct cl_code_listener *self,
            int                     pos,
            struct cl_operand       *src);

    void (*insn_call_close)(
            struct cl_code_listener *self);

    void (*destroy)(
            struct cl_code_listener *self);
};

struct cl_code_listener* cl_code_listener_create(
        const char                  *fmt,
        int                         fd_out,
        bool                        close_fd_on_destroy);

/**
 * @attention not tested yet
 */
struct cl_code_listener* cl_chain_create(void);

/**
 * @attention not tested yet
 */
void cl_chain_append(
        struct cl_code_listener     *chain,
        struct cl_code_listener     *listener);

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_CODE_LISTENER_H */
