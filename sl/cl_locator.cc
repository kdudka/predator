#include "cl_locator.hh"
#include "cl_private.hh"
#include "ssd.hh"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

class ClLocator: public ICodeListener {
    public:
        ClLocator(int fd_out);

        virtual void file_open(const char *file_name) {
            lastLoc_.currentFile = file_name;
        }

        virtual void file_close() {
            lastLoc_.currentFile.clear();
        }

        virtual void fnc_open(const struct cl_location *loc, const char *,
                              enum cl_scope_e)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void fnc_arg_decl(int, const char *) { }
        virtual void fnc_close() { }
        virtual void bb_open(const char *) { }

        virtual void insn(const struct cl_insn *cli) {
            if (CL_INSN_JMP != cli->type)
                this->printLocation(&cli->loc);
            lastLoc_ = &cli->loc;
        }

        virtual void insn_call_open(const struct cl_location *loc,
                                    const struct cl_operand *,
                                    const struct cl_operand *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_call_arg(int arg_id, const struct cl_operand *) { }
        virtual void insn_call_close() { }

        virtual void insn_switch_open(const struct cl_location *loc,
                                      const struct cl_operand *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_switch_case(const struct cl_location *loc,
                                      const struct cl_operand *,
                                      const struct cl_operand *, const char *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_switch_close() { }

    private:
        typedef boost::iostreams::file_descriptor_sink  TSink;
        typedef boost::iostreams::stream<TSink>         TStream;

        TSink                   sink_;
        TStream                 out_;
        Location                lastLoc_;

    private:
        void printLocation(const struct cl_location *);
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClLocator implementation
// TODO: remove idents of unused args
ClLocator::ClLocator(int fd_out):
    sink_(fd_out),
    out_(sink_)
{
}

void ClLocator::printLocation(const struct cl_location *loc) {
    out_ << LocationWriter(loc, &lastLoc_)
        << "linearized code follows..."
        << std::endl;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.h for more details
ICodeListener* createClLocator(const char *args) {
    // TODO: open a file with open(2) if a file name is given in ARGS
    return new ClLocator(STDOUT_FILENO);
}
