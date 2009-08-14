#include "cl_pp.hh"
#include "cl_private.hh"
#include "ssd.hh"

#include <unistd.h>

#include <boost/iostreams/device/file_descriptor.hpp>

class ClPrettyPrint: public ICodeListener {
    public:
        ClPrettyPrint(int fd_out);
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

    private:
        boost::iostreams::file_descriptor_sink out_;
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(int fd_out):
    out_(fd_out)
{
    ColorConsole::enable(isatty(fd_out));
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
ICodeListener* createClPrettyPrint(int fd_out) {
    return new ClPrettyPrint(fd_out);
}
