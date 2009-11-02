/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

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
     * @attention This function should never return (TODO: attribute?).
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

/**
 * type's unique ID
 */
typedef int cl_type_uid_t;

/**
 * basic type enumeration
 */
enum cl_type_e {
    CL_TYPE_VOID,
    CL_TYPE_UNKNOWN,
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

/**
 * used for nesting types into another (composite) type
 */
struct cl_type_item {
    /**
     * nested type
     */
    const struct cl_type                *type;

    /**
     * name of the nested item, mainly used by struct/union
     */
    const char                          *name;

    /**
     * offset of the nested item [in bytes], valid only for struct/union
     */
    int                                 offset;
};

/**
 * type definition available for each operand.  It can be also returned
 * by cl_get_type_fnc_t function (useful to traverse type definition
 * recursively).
 */
struct cl_type {
    /**
     * type's unique ID
     */
    cl_type_uid_t                       uid;

    /**
     * type of type (enumeration)
     */
    enum cl_type_e                      code;

    /**
     * location of type's declaration (may or may not be valid)
     */
    struct cl_location                  loc;

    /**
     * scope of type's declaration
     */
    enum cl_scope_e                     scope;

    /**
     * type's name, or NULL for anonymous type
     */
    const char                          *name;

    /**
     * type's sizeof
     */
    int                                 size;

    /**
     * count of nested types
     *
     * 0 for elementary types
     * 1 for e.g. CL_TYPE_PTR and CL_TYPE_ARRAY
     * n for e.g. CL_TYPE_STRUCT and CL_TYPE_UNION
     * 0 < n for CL_TYPE_FNC
     */
    int                                 item_cnt;

    /**
     * array of nested types. Its size is exactly item_cnt.
     */
    struct cl_type_item                 *items;
};

/**
 * basic accessor enumeration
 */
enum cl_accessor_e {
    /**
     * reference, in the C language: &
     */
    CL_ACCESSOR_REF,

    /**
     * dereference, in the C language: *
     * @note operator -> is decomposed as (*ptr).item
     * (CL_ACCESSOR_DEREF followed by CL_ACCESSOR_ITEM)
     */
    CL_ACCESSOR_DEREF,

    /**
     * array accessor, in the C language: []
     */
    CL_ACCESSOR_DEREF_ARRAY,

    /**
     * record accessor, in the C language: .
     */
    CL_ACCESSOR_ITEM
};

/**
 * accessor definition, available for @b some operands.  Accessors can be
 * chained as needed.
 */
struct cl_accessor {
    /**
     * type of accessor (enumeration)
     */
    enum cl_accessor_e                  code;

    /**
     * reference to type which the accessor is used for
     */
    struct cl_type                      *type;

    /**
     * next accessor, or NULL if this is the last one (singly-linked list)
     */
    struct cl_accessor                  *next;

    /**
     * some accessors have extra data
     */
    union {
        /* CL_ACCESSOR_DEREF_ARRAY */
        struct {
            /**
             * operand used as index to access array's item,
             * in the C language: array[index]
             */
            struct cl_operand           *index;
        } array;

        /* CL_ACCESSOR_ITEM */
        struct {
            /**
             * record's item ought to be accessed, enumeration starts with zero
             */
            int                         id;
        } item;
    } data;
};

struct cl_cst {
    enum cl_type_e code;
    union {
        /* CL_TYPE_FNC */
        struct {
            /**
             * name of the function
             */
            const char                  *name;

            /**
             * true if the function is external for the analysed module
             */
            bool                        is_extern;
        } cst_fnc;

        /* CL_TYPE_INT */
        struct {
            int                         value;
        } cst_int;

        /* CL_TYPE_STRING */
        struct {
            const char                  *value;
        } cst_string;
    } data;
};

/**
 * generic operand type enumeration
 */
enum cl_operand_e {
    /**
     * there is NO operand
     */
    CL_OPERAND_VOID,

    /**
     * constant operand, in the C language: literal
     * @note this has nothing to do with the C/C++ keyword 'const'
     */
    CL_OPERAND_CST,

    /**
     * variable. We actually do not distinguish between local/global variables,
     * temporal variables and (substituted) function arguments.
     */
    CL_OPERAND_VAR,

    /**
     * function argument. Addressed by argument ID. Currently used only by
     * slsparse, but it's translated by "arg_subst" cl decorator to
     * CL_OPERAND_VAR afterwards.
     */
    CL_OPERAND_ARG,

    /**
     * register. Addressed by register ID. Usually managed by compiler only.
     */
    CL_OPERAND_REG
};

/**
 * generic operand. There is in fact no operand if code==CL_OPERAND_VOID. Each
 * operand with extra data has it's own member in the union data.
 */
struct cl_operand {
    /**
     * type of operand (enumeration)
     */
    enum cl_operand_e                   code;

    /**
     * location of the operand's occurrence
     */
    struct cl_location                  loc;

    /**
     * scope of the operand's validity
     */
    enum cl_scope_e                     scope;

    /**
     * type of operand, use type->uid to get its UID
     */
    struct cl_type                      *type;

    /**
     * chain of accessors, or NULL if there are no accessors
     */
    struct cl_accessor                  *accessor;

    /**
     * per operand type specific data
     */
    union {

        /* CL_OPERAND_VAR */
        struct {
            int                         id;
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

        /* CL_OPERAND_CST */
        struct cl_cst                   cst;
    } data;
};

/**
 * basic instruction enumeration
 */
enum cl_insn_e {
    /**
     * no instruction, used only internally for now
     */
    CL_INSN_NOP,

    /**
     * 'goto' instruction
     */
    CL_INSN_JMP,

    /**
     * if(...) instruction
     */
    CL_INSN_COND,

    /**
     * 'return' instruction
     */
    CL_INSN_RET,

    /**
     * this follows each call of a function declared with attribute 'noreturn'
     */
    CL_INSN_ABORT,

    /**
     * unary (lhs + 1) operation
     */
    CL_INSN_UNOP,

    /**
     * binary (lhs + 2) operation
     */
    CL_INSN_BINOP

    /* TODO: CL_INSN_CALL? */
    /* TODO: CL_INSN_SWITCH? */
    /* TODO */
};

/**
 * unary operation's enumeration
 */
enum cl_unop_e {
    /**
     * simple assignment
     */
    CL_UNOP_ASSIGN,

    /**
     * truth not, in the C language: !
     */
    CL_UNOP_TRUTH_NOT

    /* TODO */
};

/**
 * binary operation's enumeration
 */
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
    CL_BINOP_MULT,
    CL_BINOP_TRUNC_DIV,
    CL_BINOP_TRUNC_MOD,
    CL_BINOP_RDIV,
    CL_BINOP_MIN,
    CL_BINOP_MAX,

    /* pointer arithmetic */
    CL_BINOP_POINTER_PLUS,

    /* bitwise */
    CL_BINOP_BIT_AND,
    CL_BINOP_BIT_IOR,
    CL_BINOP_BIT_XOR

    /* TODO */
};

/**
 * single instruction definition
 */
struct cl_insn {
    /**
     * type of instruction (enumeration)
     */
    enum cl_insn_e                      code;

    /**
     * location of the instruction's occurrence
     */
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

/**
 * function used to access type database
 * @param uid Unique ID of requested type.
 * @param data User data, previously given to reg_type_db callback.
 * @return Returns a pointer to requested type's definition.
 */
typedef struct cl_type* (*cl_get_type_fnc_t)(cl_type_uid_t uid, void *data);

/**
 * listener object - the core part of this interface
 *
 * @verbatim
 * It accepts a context-free language defined by substitution to regex:
 *
 *     reg_type_db (file_open FILE_CONTENT file_close)* destroy
 *
 *
 * FILE_CONTENT is defined by substitution to regex:
 *
 *     fnc_open (fnc_arg_decl)* FNC_BODY fnc_close
 *
 *
 * FNC_BODY is defined by substitution to regex:
 *
 *     FNC_ENTRY (bb_open (NONTERM_INSN)* TERM_INSN)*
 *
 *
 * FNC_ENTRY is defined as:
 *
 *     insn{CL_INSN_JMP}
 *
 *
 * NON_TERM_INSN is defined as:
 *
 *     INSN_CALL | insn{CL_INSN_UNOP, CL_INSN_BINOP}
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
 * @endverbatim
 */
struct cl_code_listener {
    /**
     * listener's internal data. Do not use this member.
     */
    void *data;

    /**
     * register type database
     * @param self Pointer to cl_code_listener object.
     * @param fnc Function used to access type database.
     * @param user_data User data later given as arg to the registered function.
     */
    void (*reg_type_db)(
            struct cl_code_listener     *self,
            cl_get_type_fnc_t           fnc,
            void                        *user_data);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param file_name Zero-terminated string with file name being opened.
     */
    void (*file_open)(
            struct cl_code_listener     *self,
            const char                  *file_name);

    /**
     * @param self Pointer to cl_code_listener object.
     */
    void (*file_close)(
            struct cl_code_listener     *self);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param fnc An operand used as function declaration (without args).
     */
    void (*fnc_open)(
            struct cl_code_listener     *self,
            const struct cl_operand     *fcn);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param arg_id Position of the argument being specified.
     * @param arg_src Function argument given as operand.
     */
    void (*fnc_arg_decl)(
            struct cl_code_listener     *self,
            int                         arg_id,
            const struct cl_operand     *arg_src);

    /**
     * @param self Pointer to cl_code_listener object.
     */
    void (*fnc_close)(
            struct cl_code_listener     *self);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param label Zero-terminated string containing label (and thus BB) name
     */
    void (*bb_open)(
            struct cl_code_listener     *self,
            const char                  *label);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param insn Instruction definition.
     */
    void (*insn)(
            struct cl_code_listener     *self,
            const struct cl_insn        *insn);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param loc location of the function definition
     * @param dst An operand taking fnc's return value, may be CL_OPERAND_VOID
     * @param fnc An operand used as function to call (not necessarily constant)
     */
    void (*insn_call_open)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *dst,
            const struct cl_operand     *fnc);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param arg_id Position of the argument being specified.
     * @param arg_src Call argument given as operand.
     */
    void (*insn_call_arg)(
            struct cl_code_listener     *self,
            int                         arg_id,
            const struct cl_operand     *arg_src);

    /**
     * @param self Pointer to cl_code_listener object.
     */
    void (*insn_call_close)(
            struct cl_code_listener     *self);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param loc location of the function definition
     * @param src An operand used as switch source.
     */
    void (*insn_switch_open)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *src);

    /**
     * @param self Pointer to cl_code_listener object.
     * @param loc location of the function definition
     * @param val_lo Begin of the range for given case.
     * @param val_hi End of the range for given case.
     * @param label Zero-terminated string containing label name
     */
    void (*insn_switch_case)(
            struct cl_code_listener     *self,
            const struct cl_location    *loc,
            const struct cl_operand     *val_lo,
            const struct cl_operand     *val_hi,
            const char                  *label);

    /**
     * @param self Pointer to cl_code_listener object.
     */
    void (*insn_switch_close)(
            struct cl_code_listener     *self);

    /**
     * @param self Pointer to cl_code_listener object.
     */
    void (*destroy)(
            struct cl_code_listener     *self);
};

/**
 * create cl_code_listener object
 * @param config_string determines the type and attributes of the listener.
 * @note config_string is currently undocumented. You can look to slplug.c for
 * examples or turn on verbose output to see how it is parsed.
 */
struct cl_code_listener* cl_code_listener_create(const char *config_string);

/**
 * create cl_code_listener object for grouping another cl_code_listener objects
 * @return Returns on heap allocated cl_code_listener object which does nothing.
 */
struct cl_code_listener* cl_chain_create(void);

/**
 * append cl_code_listener object to chain
 * @param chain Object returned by cl_chain_create() function.
 * @param listener Object ought to be added to the chain.
 * @note Listener objects are notified in the same order as they are added.
 */
void cl_chain_append(
        struct cl_code_listener         *chain,
        struct cl_code_listener         *listener);

#ifdef __cplusplus
}
#endif

#endif /* H_GUARD_CODE_LISTENER_H */
