#include "cl_pp.hh"
#include "cl_private.hh"
#include "ssd.hh"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

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

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc);

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src);

        virtual void insn_call_close();

    private:
        typedef boost::iostreams::file_descriptor_sink  TSink;
        typedef boost::iostreams::stream<TSink>         TStream;

        TSink           sink_;
        TStream         out_;
        std::string     file_;
        int             line_;
        std::string     fnc_;
        bool            printingArgDecls_;

    private:
        bool closeArgDeclsIfNeeded();
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(int fd_out):
    sink_(fd_out),
    out_(sink_),
    printingArgDecls_(false)
{
    ColorConsole::enableForTerm(fd_out);
}

ClPrettyPrint::~ClPrettyPrint() {
}

void ClPrettyPrint::file_open(
            const char              *file_name)
{
    file_ = file_name;
}

void ClPrettyPrint::file_close()
{
    out_ << std::endl;
}

void ClPrettyPrint::fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope)
{
    fnc_ = fnc_name;
    SSD_COLORIZE(out_, C_LIGHT_BLUE) << fnc_name;
    SSD_COLORIZE(out_, C_LIGHT_RED) << "(";
    printingArgDecls_ = true;
}

void ClPrettyPrint::fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
{
    // TODO: reorder arguments if not already
    if (1 < arg_pos)
        out_ << ", ";
    SSD_COLORIZE(out_, C_LIGHT_GREEN) << "%arg" << arg_pos;
    SSD_COLORIZE(out_, C_LIGHT_RED) << ": " << arg_name;
}

bool ClPrettyPrint::closeArgDeclsIfNeeded() {
    if (printingArgDecls_) {
        printingArgDecls_ = false;
        out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")") << std::endl;
        return true;
    }
    return false;
}

void ClPrettyPrint::fnc_close()
{
    if (this->closeArgDeclsIfNeeded())
        CL_MSG_STREAM(cl_warn, file_ << ":"
                << line_ << ": "
                << "function '" << fnc_
                << "' has no basic blocks");

    out_ << std::endl;
}

void ClPrettyPrint::bb_open(
            const char              *bb_name)
{
    this->closeArgDeclsIfNeeded();
    out_ << "\t"
        << SSD_INLINE_COLOR(C_LIGHT_PURPLE, bb_name)
        << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << std::endl;
}

void ClPrettyPrint::insn_jmp(
            int                     line,
            const char              *label)
{
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_PURPLE, label);
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

void ClPrettyPrint::insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
{
}

void ClPrettyPrint::insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
{
}

void ClPrettyPrint::insn_call_close()
{
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.h for more details
ICodeListener* createClPrettyPrint(int fd_out) {
    return new ClPrettyPrint(fd_out);
}
