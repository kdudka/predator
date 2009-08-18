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
            struct cl_location      *loc,
            const char              *fnc_name,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn_jmp(
            struct cl_location      *loc,
            const char              *label);

        virtual void insn_cond(
            struct cl_location      *loc,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false);

        virtual void insn_ret(
            struct cl_location      *loc,
            struct cl_operand       *src);

        virtual void insn_unop(
            struct cl_location      *loc,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src);

        virtual void insn_binop(
            struct cl_location      *loc,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2);

        virtual void insn_call_open(
            struct cl_location      *loc,
            struct cl_operand       *dst,
            struct cl_operand       *fnc);

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src);

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

void ClLocator::fnc_open(struct cl_location *loc, const char *,
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

void ClLocator::insn_jmp(struct cl_location *loc, const char *) {
    loc_ = *loc;
    // this->printLocation();
}

void ClLocator::insn_cond(struct cl_location *loc, struct cl_operand *,
                          const char *, const char *)
{
    loc_ = *loc;
    this->printLocation();
}

void ClLocator::insn_ret(struct cl_location *loc, struct cl_operand *) {
    loc_ = *loc;
    this->printLocation();
}

void ClLocator::insn_unop(struct cl_location *loc, enum cl_unop_e,
                          struct cl_operand *, struct cl_operand *)
{
    loc_ = *loc;
    this->printLocation();
}

void ClLocator::insn_binop(struct cl_location *loc, enum cl_binop_e,
                           struct cl_operand *, struct cl_operand *,
                           struct cl_operand *)
{
    loc_ = *loc;
    this->printLocation();
}

void ClLocator::insn_call_open(struct cl_location *loc, struct cl_operand *dst,
                               struct cl_operand *fnc)
{
    loc_ = *loc;
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
