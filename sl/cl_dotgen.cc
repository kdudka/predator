#include "cl_dotgen.hh"
#include "cl_private.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fstream>
#include <map>
#include <sstream>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

class ClDotGenerator: public ICodeListener {
    public:
        ClDotGenerator(int fd_out, bool close_on_exit);
        virtual ~ClDotGenerator();

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

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src);

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label);

        virtual void insn_switch_close();

    private:
        typedef boost::iostreams::file_descriptor_sink  TSink;
        typedef boost::iostreams::stream<TSink>         TStream;

        TSink                   glSink_;
        TStream                 glOut_;
        std::ofstream           perFileOut_;
        std::ofstream           perFncOut_;

// TODO: chain streams using boost::iostreams instead of the ugly macro
#define FILE_FNC_STREAM(what) do { \
    perFileOut_ << what; \
    perFncOut_ << what; \
} while (0)

#define QUOTE_NODE(what) \
    "\"" << what << "\""

#define QUOTE_BB(what) \
    QUOTE_NODE(fnc_ << "." << what)

        Location                loc_;
        std::string             fnc_;
        std::string             bb_;

    private:
        enum NodeType {
            NT_PLAIN,
            NT_ENTRY,
            NT_RET,
            NT_ABORT
        };

        enum EdgeType {
            ET_JMP,
            ET_COND_THEN,
            ET_COND_ELSE,
            ET_SWITCH_CASE,
            ET_CALL
        };

        typedef std::map<std::string, EdgeType>         TEdgeMap;
        TEdgeMap                edgeMap_;
        NodeType                nodeType_;

    private:
        void createDotFile(std::ofstream &str, std::string fileName);
        void gobbleEdge(std::string dst, EdgeType type);
        void emitEdge(std::string dst, EdgeType type);
        void emitBb();
};

using std::ios;
using std::string;

// /////////////////////////////////////////////////////////////////////////////
// ClDotGenerator implementation
ClDotGenerator::ClDotGenerator(int fd_out, bool close_on_exit):
    glSink_(fd_out, close_on_exit),
    glOut_(glSink_),
    nodeType_(NT_PLAIN)
{
    glOut_ << "digraph gl {" << std::endl;
    if (!glOut_)
        CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                               "global dot file not ready for writing");
}

ClDotGenerator::~ClDotGenerator() {
    glOut_ << "}" << std::endl;
}

void ClDotGenerator::createDotFile(std::ofstream &str, std::string fileName) {
    fileName += ".dot";
    str.open(fileName.c_str(), ios::out);
    if (str)
        return;

    CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                           "unable to create file '" << fileName << "'");
}

void ClDotGenerator::gobbleEdge(std::string dst, EdgeType edgeType) {
    if (bb_.empty() && (ET_JMP == edgeType))
        // skip jmp to ENTRY
        return;

    edgeMap_[dst] = edgeType;
}

void ClDotGenerator::emitEdge(std::string dst, EdgeType edgeType) {
    if (ET_CALL == edgeType) {
        perFncOut_ << "\t" << QUOTE_NODE(dst)
                << " [label=" << QUOTE_NODE(dst)
                << ", color=blue];" << std::endl;
        perFncOut_ << "\t" << QUOTE_BB(bb_)
                << " -> " << QUOTE_NODE(dst)
                << " [color=blue];" << std::endl;
        return;
    }

    FILE_FNC_STREAM("\t" << QUOTE_BB(bb_)
            << " -> " << QUOTE_BB(dst)
            << " [color=");
    switch (edgeType) {
        case ET_JMP:            FILE_FNC_STREAM("black");   break;
        case ET_COND_THEN:
        case ET_COND_ELSE:      FILE_FNC_STREAM("green");   break;
        case ET_SWITCH_CASE:    FILE_FNC_STREAM("yellow");  break;

        // just to make compiler happy
        case ET_CALL:
            break;
    }
    FILE_FNC_STREAM("];" << std::endl);
}

void ClDotGenerator::emitBb() {
    // colorize current BB node
    FILE_FNC_STREAM("\t" << QUOTE_BB(bb_) << " [color=");
    switch (nodeType_) {
        case NT_PLAIN:          FILE_FNC_STREAM("black");   break;
        case NT_ENTRY:
        case NT_RET:            FILE_FNC_STREAM("blue");    break;
        case NT_ABORT:          FILE_FNC_STREAM("red");     break;
    }
    FILE_FNC_STREAM(", label=" << QUOTE_NODE(bb_) << "];" << std::endl);

    // emit all BB edges
    TEdgeMap::iterator i;
    for (i = edgeMap_.begin(); i != edgeMap_.end(); ++i)
        this->emitEdge(i->first, i->second);

    // clear emitted edges
    edgeMap_.clear();
}

void ClDotGenerator::file_open(
            const char              *file_name)
{
    loc_.currentFile = file_name;
    this->createDotFile(perFileOut_, file_name);
    perFileOut_ << "digraph " << QUOTE_NODE(file_name) << " {" << std::endl
        << "\tlabel=<<FONT POINT-SIZE=\"36\">"
        << file_name << "</FONT>>;" << std::endl
        << "\tlabelloc=t;" << std::endl;
}

void ClDotGenerator::file_close()
{
    perFileOut_ << "}" << std::endl;
    if (!perFileOut_)
        CL_MSG_STREAM_INTERNAL(cl_warning, "warning: "
                "error detected while closing a file");

    perFileOut_.close();
}

void ClDotGenerator::fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
{
    loc_ = loc;
    fnc_ = fnc_name;
    this->createDotFile(perFncOut_, string(loc_.currentFile) + "-" + fnc_name);
    std::ostringstream label;
    label << fnc_name << "() at " << loc_.locFile << ":" << loc_.locLine;
    perFncOut_ << "digraph " << QUOTE_NODE(label.str()) << " {" << std::endl
        << "\tlabel=<<FONT POINT-SIZE=\"36\">"
        << label.str() << "</FONT>>;" << std::endl
        << "\tlabelloc=t;" << std::endl;
}

void ClDotGenerator::fnc_arg_decl(
            int                     arg_id,
            const char              *arg_name)
{
}

void ClDotGenerator::fnc_close()
{
    if (!bb_.empty())
        // emit previous bb
        this->emitBb();

    FILE_FNC_STREAM("}" << std::endl);
    if (!perFncOut_)
        CL_MSG_STREAM_INTERNAL(cl_warning, "warning: "
                "error detected while closing a file");

    perFncOut_.close();
    bb_.clear();
}

void ClDotGenerator::bb_open(
            const char              *bb_name)
{
    // FIXME: the condition is not going to work well in general
    if (bb_.empty()) {
        nodeType_ = NT_ENTRY;

        perFileOut_ << "subgraph \"cluster" << fnc_
            << "." << bb_name << "\" {" << std::endl
            << "\tcolor=blue;" << std::endl
            << "\tlabel=" << QUOTE_NODE(fnc_ << "() at "
                    << loc_.locFile << ":" << loc_.locLine)
            << std::endl;

    } else {
        // emit last BB
        this->emitBb();
        nodeType_ = NT_PLAIN;
    }

    bb_ = bb_name;
}

void ClDotGenerator::insn(
            const struct cl_insn    *cli)
{
    switch (cli->type) {
        case CL_INSN_JMP:
            this->gobbleEdge(cli->data.insn_jmp.label, ET_JMP);
            break;

        case CL_INSN_COND:
            this->gobbleEdge(cli->data.insn_cond.then_label, ET_COND_THEN);
            this->gobbleEdge(cli->data.insn_cond.else_label, ET_COND_ELSE);
            break;

        case CL_INSN_RET:
            nodeType_ = NT_RET;
            break;

        case CL_INSN_ABORT:
            nodeType_ = NT_ABORT;
            break;

        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
            break;
    }
}

void ClDotGenerator::insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
{
    if (fnc->type != CL_OPERAND_VAR || fnc->deref || fnc->offset) {
        CL_MSG_STREAM(cl_warn, LocationWriter(loc, &loc_) << "warning: "
                      "ClDotGenerator: unhandled call");
        return;
    }

    this->gobbleEdge(fnc->data.var.name, ET_CALL);
}

void ClDotGenerator::insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
{
}

void ClDotGenerator::insn_call_close()
{
}

void ClDotGenerator::insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
{
}

void ClDotGenerator::insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    this->gobbleEdge(label, ET_SWITCH_CASE);
}

void ClDotGenerator::insn_switch_close()
{
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_dotgen.hh for more details
ICodeListener* createClDotGenerator(const char *args) {
    // write to stdout by default
    int fd = STDOUT_FILENO;

    // check whether a file name is given
    bool openFile = args && *args;
    if (openFile)
        // write to file requested
        fd = open(/* file name is the only arg for now */ args,
                  O_WRONLY | O_CREAT);

    // TODO: error msg
    return (fd < 0) ? 0
        : new ClDotGenerator(fd, openFile);
}
