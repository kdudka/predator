#include "cl_pp.hh"
#include "cl_private.hh"

class ClPrettyPrint: public ICodeListener {
    public:
        ClPrettyPrint(FILE *output);
        virtual ~ClPrettyPrint();

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn_jmp(
            int                     line,
            const char              *label);

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false);

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src);

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src);

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2);

        virtual void insn_call(
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name);

        virtual void insn_call_arg(
            int                     pos,
            struct cl_operand       *src);
};

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(FILE *output) {
}

ClPrettyPrint::~ClPrettyPrint() {
}

void ClPrettyPrint::file_open(
            const char              *file_name)
{
}

void ClPrettyPrint::file_close()
{
}

void ClPrettyPrint::fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope)
{
}

void ClPrettyPrint::fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
{
}

void ClPrettyPrint::fnc_close()
{
}

void ClPrettyPrint::bb_open(
            const char              *bb_name)
{
}

void ClPrettyPrint::insn_jmp(
            int                     line,
            const char              *label)
{
}

void ClPrettyPrint::insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
{
}

void ClPrettyPrint::insn_ret(
            int                     line,
            struct cl_operand       *src)
{
}

void ClPrettyPrint::insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
{
}

void ClPrettyPrint::insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
{
}

void ClPrettyPrint::insn_call(
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name)
{
}

void ClPrettyPrint::insn_call_arg(
            int                     pos,
            struct cl_operand       *src)
{
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.h for more details
ICodeListener* createClPrettyPrint(FILE *output) {
    return new ClPrettyPrint(output);
}
