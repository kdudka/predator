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
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_id,
            const char              *arg_name);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn(
            const struct cl_insn    *cli);

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc);

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void insn_call_close();

    private:
        typedef boost::iostreams::file_descriptor_sink  TSink;
        typedef boost::iostreams::stream<TSink>         TStream;

        TSink                   sink_;
        TStream                 out_;
        std::string             file_;
        struct cl_location      loc_;

    private:
        void printLocation();
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClLocator implementation
// TODO: remove idents of unused args
ClLocator::ClLocator(int fd_out):
    sink_(fd_out),
    out_(sink_)
{
    cl_set_location(&loc_, -1);
}

ClLocator::~ClLocator() {
}

void ClLocator::file_open(const char *file_name) {
    file_ = file_name;
}

void ClLocator::file_close() {
}

void ClLocator::printLocation() {
    out_ << file_ << ":" << loc_.line << ": linearized code:" << std::endl;
}

void ClLocator::fnc_open(const struct cl_location *loc, const char *,
                         enum cl_scope_e)
{
    loc_ = *loc;
}

void ClLocator::fnc_arg_decl(int, const char *) {
}

void ClLocator::fnc_close() {
}

void ClLocator::bb_open(const char *) {
}

void ClLocator::insn(const struct cl_insn *cli) {
    loc_ = cli->loc;
    if (CL_INSN_JMP != cli->type)
        this->printLocation();
}

void ClLocator::insn_call_open(const struct cl_location *loc,
                               const struct cl_operand *dst,
                               const struct cl_operand *fnc)
{
    loc_ = *loc;
    this->printLocation();
}

void ClLocator::insn_call_arg(int, const struct cl_operand *) {
}

void ClLocator::insn_call_close() {
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.h for more details
ICodeListener* createClLocator(int fd_out) {
    return new ClLocator(fd_out);
}
