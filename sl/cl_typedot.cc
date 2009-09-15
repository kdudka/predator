#include "cl_typedot.hh"
#include "cl_private.hh"

#include <fstream>
#include <map>
#include <set>
#include <sstream>

class ClTypeDotGenerator: public AbstractCodeListener {
    public:
        ClTypeDotGenerator(const char *glDotFile);
        virtual ~ClTypeDotGenerator();

        virtual void file_open(const char *file_name) {
            loc_.currentFile = file_name;
        }

        virtual void file_close() {
            loc_.currentFile.clear();
        }

        virtual void fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            fnc_ = fnc_name;
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->handleOperand(arg_src);
        }

        virtual void fnc_close() {
            fnc_.clear();
        }

        virtual void bb_open(const char *bb_name) { }

        virtual void insn(const struct cl_insn *cli) {
            switch (cli->code) {
                case CL_INSN_NOP:
                case CL_INSN_JMP:
                case CL_INSN_ABORT:
                    // no operand
                    break;

                case CL_INSN_COND:
                    this->handleOperand(cli->data.insn_cond.src);
                    break;

                case CL_INSN_RET:
                    this->handleOperand(cli->data.insn_ret.src);
                    break;

                case CL_INSN_UNOP:
                    this->handleOperand(cli->data.insn_unop.dst);
                    this->handleOperand(cli->data.insn_unop.src);
                    break;

                case CL_INSN_BINOP:
                    this->handleOperand(cli->data.insn_binop.dst);
                    this->handleOperand(cli->data.insn_binop.src1);
                    this->handleOperand(cli->data.insn_binop.src2);
                    break;
            }
        }

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            this->handleOperand(dst);
            this->handleOperand(fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->handleOperand(arg_src);
        }

        virtual void insn_call_close() { }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            this->handleOperand(src);
        }

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            this->handleOperand(val_lo);
            this->handleOperand(val_hi);
        }

        virtual void insn_switch_close() { }

    private:
        std::ofstream           glOut_;
        Location                loc_;
        std::string             fnc_;

    private:
        void handleOperand(const struct cl_operand *operand);
};

#define SL_QUOTE(what) \
    "\"" << what << "\""

#define SL_GRAPH(name) \
    "digraph " << SL_QUOTE(name) << " {" << std::endl \
    << "\tlabel=<<FONT POINT-SIZE=\"18\">" << name << "</FONT>>;" << std::endl \
    << "\tlabelloc=t;" << std::endl

#if 0
#define SL_QUOTE_BB(bb) \
    SL_QUOTE(fnc_ << "." << bb)

#define SL_BB_POS_SUFFIX \
    "." << bbPos_

#define SL_BB_ENTRY_SUFFIX ".0"

#define SL_DOT_SUFFIX ".svg"

#define SL_QUOTE_PER_FILE_URL \
    SL_QUOTE(basename((char *) loc_.currentFile.c_str()) << SL_DOT_SUFFIX)

#define SL_QUOTE_URL(fnc) \
    SL_QUOTE(basename((char *) loc_.currentFile.c_str()) \
    << "-" << fnc << SL_DOT_SUFFIX)

#define SL_SUBGRAPH(name, label) \
    "subgraph \"cluster" << name << "\" {" << std::endl \
    << "\tlabel=" << SL_QUOTE(label) << ";" << std::endl
#endif

using std::ios;
using std::string;

#if 0
const char *ClTypeDotGenerator::NtColors[ClTypeDotGenerator::CNT_NT] = {
    "black",                        // NT_PLAIN
    "blue",                         // NT_ENTRY
    "blue",                         // NT_RET
    "red"                           // NT_ABORT
};

const char *ClTypeDotGenerator::EtColors[ClTypeDotGenerator::CNT_ET] = {
    "black",                        // ET_JMP
    "green",                        // ET_COND_THEN
    "green",                        // ET_COND_ELSE,
    "yellow",                       // ET_SWITCH_CASE
    "blue",                         // ET_LC_CALL
    "red",                          // ET_LC_CALL_INDIR
    "blue",                         // ET_GL_CALL
    "red",                          // ET_GL_CALL_INDIR
    "green"                         // ET_PTR_CALL
};
#endif

// /////////////////////////////////////////////////////////////////////////////
// ClTypeDotGenerator implementation
ClTypeDotGenerator::ClTypeDotGenerator(const char *glDotFile)
{
    glOut_.open(glDotFile, ios::out);
    if (glOut_) {
        CL_DEBUG("ClTypeDotGenerator: created dot file '" << glDotFile << "'");
    } else {
        CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                               "unable to create file '" << glDotFile << "'");
    }
    glOut_ << SL_GRAPH("data type graph");
}

ClTypeDotGenerator::~ClTypeDotGenerator() {
    glOut_ << "}" << std::endl;
    if (!glOut_) {
        CL_MSG_STREAM_INTERNAL(cl_warn, "warning: "
                "error detected while closing a file");
    }
    glOut_.close();
}

#if 0
    glOut_ << "\t" << SL_QUOTE(fnc_) << " -> " << SL_QUOTE(dst)
        << " [color=" << EtColors[type] << "];" << std::endl;

    perFileOut_ << "\t" << SL_QUOTE_BB(bb_) << " -> " << SL_QUOTE_BB(dst)
            << " [color=" << EtColors[type] << "];" << std::endl;

    // colorize current BB node
    perFileOut_ << "\t" << SL_QUOTE_BB(bb_)
        << " [color=" << NtColors[nodeType_]
        << ", label=" << SL_QUOTE(bb_) << "];"
        << std::endl;

    perFileOut_ << "\t" << SL_QUOTE_BB(i->second)
        << " -> " << SL_QUOTE_BB(dst)
        << " [color=" << EtColors[type] << "];"
        << std::endl;

    FILE_FNC_STREAM(SL_SUBGRAPH(fnc_ << "." << label, fnc_
                << "() at " << loc_.locFile << ":" << loc_.locLine)
            << "\tcolor=blue;" << std::endl
            << "\tbgcolor=gray99;" << std::endl);

    perFncOut_ << "\tURL=" << SL_QUOTE_PER_FILE_URL << ";" << std::endl
        << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=blue, fontcolor=blue, style=bold,"
            << " label=ENTRY];" << std::endl
        << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
        << SL_QUOTE_BB(label << SL_BB_ENTRY_SUFFIX)
            << " [color=black];" << std::endl;

    perFileOut_ << "\tURL=" << SL_QUOTE_URL(fnc_) << ";" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
        << " [shape=box, color=black, fontcolor=black,"
        << " style=bold, label=goto];" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
        << " [shape=box, color=green, fontcolor=green, style=bold,"
        << " label=if];" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(then_label << SL_BB_ENTRY_SUFFIX)
            << " [color=green];" << std::endl
        << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(else_label << SL_BB_ENTRY_SUFFIX)
            << " [color=green];" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=black, fontcolor=gray, style=dotted,"
            << " label=\"...\"];" << std::endl
            << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> ";

    perFncOut_ << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [color=gray, style=dotted, arrowhead=open];"
            << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=blue, fontcolor=blue, style=dashed,"
            << " label=call];" << std::endl;

    perFncOut_ << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [color=gray, style=dotted, arrowhead=open];"
            << std::endl;

    glOut_ << SL_SUBGRAPH(file_name, file_name)
        << "\tcolor=red;" << std::endl
        << "\tURL=" << SL_QUOTE_PER_FILE_URL << ";" << std::endl;

    perFncOut_ << SL_SUBGRAPH(fnc_ << "::" << bb_, bb_)
        << "\tcolor=black;" << std::endl
        << "\tbgcolor=white;" << std::endl
        << "\tstyle=dashed;" << std::endl
        << "\tURL=\"\";" << std::endl;

            perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
                << " [shape=box, color=blue, fontcolor=blue, style=bold,"
                << " label=ret];" << std::endl;

            perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
                << " [shape=box, color=red, fontcolor=red, style=bold,"
                << " label=abort];" << std::endl;
            ClTypeDotGenerator::closeSub(perFncOut_);
#endif

void ClTypeDotGenerator::handleOperand(const struct cl_operand *operand) {
    // TODO
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_dotgen.hh for more details
ICodeListener* createClTypeDotGenerator(const char *args) {
    return new ClTypeDotGenerator(args);
}
