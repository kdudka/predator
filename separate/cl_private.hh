#ifndef H_GUARD_CL_PRIVATE_H
#define H_GUARD_CL_PRIVATE_H

#include "code_listener.h"

#include <cstdlib>

/**
 * C++ interface for listener objects. It can be wrapped to struct code_listener
 * object when exposing to pure C world. See code_listener for details about
 * methods and their parameters.
 */
class ICodeListener {
    public:
        virtual ~ICodeListener() { }

        virtual void file_open(
            const char              *file_name) = 0;

        virtual void file_close() = 0;

        virtual void fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope) = 0;

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name) = 0;

        virtual void fnc_close() = 0;

        virtual void bb_open(
            const char              *bb_name) = 0;

        virtual void insn_jmp(
            int                     line,
            const char              *label) = 0;

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false) = 0;

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src) = 0;

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src) = 0;

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2) = 0;

        virtual void insn_call(
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name) = 0;

        virtual void insn_call_arg(
            int                     pos,
            struct cl_operand       *src) = 0;
};

/**
 * wrap ICodeListener object so that it can be exposed to pure C world
 */
struct cl_code_listener* cl_create_listener_wrap(ICodeListener *);

/**
 * retrieve wrapped ICodeListener object
 */
ICodeListener* cl_obtain_from_wrap(struct cl_code_listener *);

void cl_debug(const char *msg);
void cl_warn(const char *msg);
void cl_error(const char *msg);
void cl_note(const char *msg);
void cl_die(const char *msg);

// pull in __attribute__ ((__noreturn__))
#define CL_DIE(msg) do { \
    cl_die(msg); \
    abort(); \
} while (0)

#endif /* H_GUARD_CL_PRIVATE_H */
