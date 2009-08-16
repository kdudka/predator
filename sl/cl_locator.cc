#include "cl_locator.hh"
#include "cl_private.hh"
#include "ssd.hh"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

class ClLocator: public ICodeListener {
    public:
        ClLocator(int fd_out);
        virtual ~ClLocator();

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
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

    private:
        void printLocation();
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClLocator implementation
// TODO: remove idents of unused args
ClLocator::ClLocator(int fd_out):
    sink_(fd_out),
    out_(sink_),
    line_(-1)
{
    // FIXME: static variable
    ColorConsole::enableForTerm(fd_out);
}

ClLocator::~ClLocator() {
}

void ClLocator::file_open(const char *file_name) {
    file_ = file_name;
}

void ClLocator::file_close() {
}

void ClLocator::printLocation() {
    out_ << file_ << ":" << line_ << ": linearized code:" << std::endl;
}

void ClLocator::fnc_open(int line, const char *, enum cl_scope_e) {
    line_ = line;
}

void ClLocator::fnc_arg_decl(int, const char *) {
}

void ClLocator::fnc_close() {
}

void ClLocator::bb_open(const char *) {
}

void ClLocator::insn_jmp(int line, const char *) {
    line_ = line;
    // this->printLocation();
}

void ClLocator::insn_cond(int line, struct cl_operand *, const char *,
                          const char *)
{
    line_ = line;
    this->printLocation();
}

void ClLocator::insn_ret(int line, struct cl_operand *) {
    line_ = line;
    this->printLocation();
}

void ClLocator::insn_unop(int line, enum cl_unop_e, struct cl_operand *,
                          struct cl_operand *)
{
    line_ = line;
    this->printLocation();
}

void ClLocator::insn_binop(int line, enum cl_binop_e, struct cl_operand *,
                           struct cl_operand *, struct cl_operand *)
{
    line_ = line;
    this->printLocation();
}

void ClLocator::insn_call_open(int line, struct cl_operand *dst,
                               struct cl_operand *fnc)
{
    line_ = line;
    this->printLocation();
}

void ClLocator::insn_call_arg(int, struct cl_operand *) {
}

void ClLocator::insn_call_close() {
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.h for more details
ICodeListener* createClLocator(int fd_out) {
    return new ClLocator(fd_out);
}
