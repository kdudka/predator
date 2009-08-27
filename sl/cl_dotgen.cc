#include "cl_dotgen.hh"
#include "cl_private.hh"

#include <libgen.h>         // for basename(3)

#include <fstream>
#include <map>
#include <set>
#include <sstream>

class ClDotGenerator: public ICodeListener {
    public:
        ClDotGenerator(const char *glDotFile);
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
        bool                    hasGlDotFile_;
        std::ofstream           glOut_;
        std::ofstream           perFileOut_;
        std::ofstream           perFncOut_;

        Location                loc_;
        std::string             fnc_;
        std::string             bb_;
        int                     bbPos_;

    private:
        enum NodeType {
            NT_PLAIN,
            NT_ENTRY,
            NT_RET,
            NT_ABORT
        };
        enum {
            CNT_NT = NT_ABORT + 1
        };
        static const char       *NtColors[];

        enum EdgeType {
            ET_JMP,
            ET_COND_THEN,
            ET_COND_ELSE,
            ET_SWITCH_CASE,
            ET_LC_CALL,
            ET_LC_CALL_INDIR,
            ET_GL_CALL,
            ET_GL_CALL_INDIR
        };
        enum {
            CNT_ET = ET_GL_CALL_INDIR + 1
        };
        static const char       *EtColors[];

        typedef std::set<std::string>                   TCallSet;
        typedef std::map<std::string, std::string>      TCallMap;
        typedef std::map<std::string, TCallSet>         TCallMultiMap;
        typedef std::map<std::string, EdgeType>         TEdgeMap;

        TCallMap                perFncCalls_;
        TCallMultiMap           perBbCalls_;
        TEdgeMap                perFncEdgeMap_;
        TEdgeMap                perBbEdgeMap_;
        NodeType                nodeType_;
        enum cl_insn_e          lastInsn_;

    private:
        static void createDotFile(std::ofstream &str, std::string fileName,
                                  bool appendSuffix);
        static void closeSub(std::ofstream &str);
        static void closeDot(std::ofstream &str);
        void gobbleEdge(std::string dst, EdgeType type);
        void emitEdge(std::string dst, EdgeType type);
        void emitBb();
        void emitPendingCalls();
        void emitFncEntry(const char *label);
        void emitInsnJmp(const char *label);
        void emitInsnCond(const char *then_label, const char * else_label);
        void emitOpIfNeeded();
        void emitInsnCall();
        void checkForFncRef(const struct cl_operand *op);
};

// TODO: chain streams using boost::iostreams instead of the ugly macro
#define FILE_FNC_STREAM(what) do { \
    perFileOut_ << what; \
    perFncOut_ << what; \
} while (0)

#define SL_QUOTE(what) \
    "\"" << what << "\""

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

#define SL_GRAPH(name) \
    "digraph " << SL_QUOTE(name) << " {" << std::endl \
    << "\tlabel=<<FONT POINT-SIZE=\"18\">" << name << "</FONT>>;" << std::endl \
    << "\tlabelloc=t;" << std::endl

#define SL_SUBGRAPH(name, label) \
    "subgraph \"cluster" << name << "\" {" << std::endl \
    << "\tlabel=" << SL_QUOTE(label) << ";" << std::endl

using std::ios;
using std::string;

const char *ClDotGenerator::NtColors[ClDotGenerator::CNT_NT] = {
    "black",                        // NT_PLAIN
    "blue",                         // NT_ENTRY
    "blue",                         // NT_RET
    "red"                           // NT_ABORT
};

const char *ClDotGenerator::EtColors[ClDotGenerator::CNT_ET] = {
    "black",                        // ET_JMP
    "green",                        // ET_COND_THEN
    "green",                        // ET_COND_ELSE,
    "yellow",                       // ET_SWITCH_CASE
    "blue",                         // ET_LC_CALL
    "red",                          // ET_LC_CALL_INDIR
    "blue",                         // ET_GL_CALL
    "red"                           // ET_GL_CALL_INDIR
};

// /////////////////////////////////////////////////////////////////////////////
// ClDotGenerator implementation
void ClDotGenerator::createDotFile(std::ofstream &str, std::string fileName,
                                   bool appendSuffix)
{
    if (appendSuffix)
        fileName += ".dot";

    str.open(fileName.c_str(), ios::out);
    if (!str) {
        CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                               "unable to create file '" << fileName << "'");
    }
}

void ClDotGenerator::closeSub(std::ofstream &str) {
    str << "}" << std::endl;
}

void ClDotGenerator::closeDot(std::ofstream &str) {
    ClDotGenerator::closeSub(str);
    if (!str) {
        CL_MSG_STREAM_INTERNAL(cl_warning, "warning: "
                "error detected while closing a file");
    }
    str.close();
}

ClDotGenerator::ClDotGenerator(const char *glDotFile):
    hasGlDotFile_(glDotFile && *glDotFile),
    nodeType_(NT_PLAIN)
{
    if (hasGlDotFile_) {
        ClDotGenerator::createDotFile(glOut_, glDotFile, false);
        glOut_ << SL_GRAPH(glDotFile);
    }
}

ClDotGenerator::~ClDotGenerator() {
    if (hasGlDotFile_)
        this->closeDot(glOut_);
}

void ClDotGenerator::gobbleEdge(std::string dst, EdgeType type) {
    perBbEdgeMap_[dst] = type;
    perFncEdgeMap_[dst] = type;
}

void ClDotGenerator::emitEdge(std::string dst, EdgeType type) {
    switch (type) {
        case ET_GL_CALL:
        case ET_GL_CALL_INDIR:
            perFncCalls_[dst] = bb_;
            return;

        case ET_LC_CALL:
        case ET_LC_CALL_INDIR:
            if (!hasKey(perFncCalls_, dst)) {
                perFncCalls_[dst] = bb_;
                glOut_ << "\t" << SL_QUOTE(fnc_) << " -> " << SL_QUOTE(dst)
                    << " [color=" << EtColors[type] << "];" << std::endl;
            }
            return;

        default:
            break;
    }

    perFileOut_ << "\t" << SL_QUOTE_BB(bb_) << " -> " << SL_QUOTE_BB(dst)
            << " [color=" << EtColors[type] << "];" << std::endl;
}

void ClDotGenerator::emitBb() {
    // colorize current BB node
    perFileOut_ << "\t" << SL_QUOTE_BB(bb_)
        << " [color=" << NtColors[nodeType_]
        << ", label=" << SL_QUOTE(bb_) << "];"
        << std::endl;

    // emit all BB edges
    TEdgeMap::iterator i;
    for (i = perBbEdgeMap_.begin(); i != perBbEdgeMap_.end(); ++i)
        this->emitEdge(i->first, i->second);

    // clear emitted edges
    perBbEdgeMap_.clear();
}

void ClDotGenerator::emitPendingCalls() {
    TCallMap::iterator i;
    for (i = perFncCalls_.begin(); i != perFncCalls_.end(); ++i) {
        const string &dst = i->first;
        const EdgeType type = perFncEdgeMap_[dst];

        FILE_FNC_STREAM("\t" << SL_QUOTE_BB(dst)
            << " [label=" << SL_QUOTE(dst));
        switch (type) {
            case ET_LC_CALL:
            case ET_LC_CALL_INDIR:
                FILE_FNC_STREAM(", URL=" << SL_QUOTE_URL(i->first)
                    << ", color=" << EtColors[type]
                    << ", fontcolor=" << EtColors[type]);
            default:
                break;
        }
        FILE_FNC_STREAM("];" << std::endl);

        perFileOut_ << "\t" << SL_QUOTE_BB(i->second)
            << " -> " << SL_QUOTE_BB(dst)
            << " [color=" << EtColors[type] << "];"
            << std::endl;

        TCallSet &cs = perBbCalls_[dst];
        TCallSet::iterator j;
        for (j = cs.begin(); j != cs.end(); ++j) {
            perFncOut_ << "\t" << SL_QUOTE_BB(*j)
                << " -> " << SL_QUOTE_BB(dst)
                << " [color=" << EtColors[type] << "];"
                << std::endl;
        }
    }

    perBbCalls_.clear();
    perFncCalls_.clear();
    perFncEdgeMap_.clear();
}

void ClDotGenerator::emitFncEntry(const char *label) {
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
}

void ClDotGenerator::emitInsnJmp(const char *label) {
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
        << " [shape=box, color=black, fontcolor=black,"
        << " style=bold, label=goto];" << std::endl;

    ClDotGenerator::closeSub(perFncOut_);

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
        << SL_QUOTE_BB(label<< SL_BB_ENTRY_SUFFIX)
        << " [color=black];" << std::endl;
}

void ClDotGenerator::emitInsnCond(const char *then_label,
                                  const char *else_label)
{
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
        << " [shape=box, color=green, fontcolor=green, style=bold,"
        << " label=if];" << std::endl;
    ClDotGenerator::closeSub(perFncOut_);

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(then_label << SL_BB_ENTRY_SUFFIX)
            << " [color=green];" << std::endl
        << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(else_label << SL_BB_ENTRY_SUFFIX)
            << " [color=green];" << std::endl;
}

void ClDotGenerator::emitOpIfNeeded() {
    switch (lastInsn_) {
        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
            // do not show unop/binop more than once in the sequence
            return;
        default:
            break;
    }

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=black, fontcolor=gray, style=dotted,"
            << " label=\"...\"];" << std::endl
            << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> ";

    ++bbPos_;
    perFncOut_ << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [color=gray, style=dotted, arrowhead=open];"
            << std::endl;
}

void ClDotGenerator::emitInsnCall() {
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=blue, fontcolor=blue, style=dashed,"
            << " label=call];" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> ";
    ++bbPos_;
    perFncOut_ << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [color=gray, style=dotted, arrowhead=open];"
            << std::endl;
}

void ClDotGenerator::checkForFncRef(const struct cl_operand *op) {
    if (CL_OPERAND_FNC != op->type)
        return;

    string name(op->data.fnc.name);
    this->gobbleEdge(name, (op->data.fnc.is_extern)
            ? ET_GL_CALL_INDIR
            : ET_LC_CALL_INDIR);

    std::ostringstream str;
    str << bb_ << "." << (bbPos_ - 1);
    perBbCalls_[name].insert(str.str());
}

void ClDotGenerator::file_open(const char *file_name) {
    loc_.currentFile = file_name;
    ClDotGenerator::createDotFile(perFileOut_, file_name, true);
    perFileOut_ << SL_GRAPH(file_name);

    glOut_ << SL_SUBGRAPH(file_name, file_name)
        << "\tcolor=red;" << std::endl
        << "\tURL=" << SL_QUOTE_PER_FILE_URL << ";" << std::endl;
}

void ClDotGenerator::file_close()
{
    ClDotGenerator::closeDot(perFileOut_);
    ClDotGenerator::closeSub(glOut_);
}

void ClDotGenerator::fnc_open(const struct cl_location *loc,
                              const char *fnc_name, enum cl_scope_e)
{
    loc_ = loc;
    fnc_ = fnc_name;

    ClDotGenerator::createDotFile(perFncOut_,
                                  string(loc_.currentFile) + "-" + fnc_name,
                                  true);
    perFncOut_ << SL_GRAPH(fnc_name << "()"
            << " at " << loc_.locFile << ":" << loc_.locLine);

    glOut_ << "\t" << SL_QUOTE(fnc_name)
            << " [label=" << SL_QUOTE(fnc_name)
            << ", color=" << EtColors[ET_LC_CALL]
            << ", URL=" << SL_QUOTE_URL(fnc_name) << "];"
            << std::endl;
}

void ClDotGenerator::fnc_arg_decl(int, const char *) { }

void ClDotGenerator::fnc_close()
{
    if (!bb_.empty())
        // emit previous bb
        this->emitBb();

    ClDotGenerator::closeSub(perFncOut_);
    ClDotGenerator::closeSub(perFileOut_);

    this->emitPendingCalls();
    this->closeDot(perFncOut_);
    bb_.clear();
}

void ClDotGenerator::bb_open(const char *bb_name) {
    if (!bb_.empty())
        // emit last BB
        this->emitBb();

    nodeType_ = NT_PLAIN;

    bb_ = bb_name;
    bbPos_ = 0;
    perFncOut_ << SL_SUBGRAPH(fnc_ << "::" << bb_, bb_)
        << "\tcolor=black;" << std::endl
        << "\tbgcolor=white;" << std::endl
        << "\tstyle=dashed;" << std::endl
        << "\tURL=\"\";" << std::endl;
}

void ClDotGenerator::insn(const struct cl_insn *cli) {
    switch (cli->type) {
        case CL_INSN_JMP: {
                const char *label = cli->data.insn_jmp.label;
                if (bb_.empty()) {
                    nodeType_ = NT_ENTRY;
                    this->emitFncEntry(label);
                } else {
                    this->emitInsnJmp(label);
                    this->gobbleEdge(label, ET_JMP);
                }
            }
            break;

        case CL_INSN_COND: {
                const char *then_label = cli->data.insn_cond.then_label;
                const char *else_label = cli->data.insn_cond.else_label;
                this->emitInsnCond(then_label, else_label);
                this->gobbleEdge(then_label, ET_COND_THEN);
                this->gobbleEdge(else_label, ET_COND_ELSE);
                this->checkForFncRef(cli->data.insn_cond.src);
            }
            break;

        case CL_INSN_RET:
            nodeType_ = NT_RET;
            perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
                << " [shape=box, color=blue, fontcolor=blue, style=bold,"
                << " label=ret];" << std::endl;
            this->checkForFncRef(cli->data.insn_ret.src);
            ClDotGenerator::closeSub(perFncOut_);
            break;

        case CL_INSN_ABORT:
            nodeType_ = NT_ABORT;
            perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
                << " [shape=box, color=red, fontcolor=red, style=bold,"
                << " label=abort];" << std::endl;
            ClDotGenerator::closeSub(perFncOut_);
            break;

        case CL_INSN_UNOP:
            this->emitOpIfNeeded();
            this->checkForFncRef(cli->data.insn_unop.src);
            break;

        case CL_INSN_BINOP:
            this->emitOpIfNeeded();
            this->checkForFncRef(cli->data.insn_binop.src1);
            this->checkForFncRef(cli->data.insn_binop.src2);
            break;
    }
    lastInsn_ = cli->type;
}

void ClDotGenerator::insn_call_open(const struct cl_location *loc,
                                    const struct cl_operand *,
                                    const struct cl_operand *fnc)
{
    if (fnc->type != CL_OPERAND_FNC || fnc->deref || fnc->offset) {
        CL_MSG_STREAM(cl_warn, LocationWriter(loc, &loc_) << "warning: "
                      "ClDotGenerator: unhandled call");
        return;
    }

    string name(fnc->data.fnc.name);

    std::ostringstream str;
    str << bb_ << SL_BB_POS_SUFFIX;
    perBbCalls_[name].insert(str.str());

    this->emitInsnCall();
    this->gobbleEdge(name, (fnc->data.fnc.is_extern)
            ? ET_GL_CALL
            : ET_LC_CALL);

    lastInsn_ =
        /* FIXME: we have no CL_INSN_CALL in enum cl_insn_e */
        CL_INSN_JMP;
}

void ClDotGenerator::insn_call_arg(int, const struct cl_operand *arg_src) {
    this->checkForFncRef(arg_src);
}

void ClDotGenerator::insn_call_close()
{
}

void ClDotGenerator::insn_switch_open(const struct cl_location *,
                                      const struct cl_operand *src)
{
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=yellow, fontcolor=yellow, style=bold,"
            << " label=switch];" << std::endl
        << "}" << std::endl;
    this->checkForFncRef(src);
}

void ClDotGenerator::insn_switch_case(const struct cl_location *,
                                      const struct cl_operand *,
                                      const struct cl_operand *,
                                      const char              *label)
{
    this->gobbleEdge(label, ET_SWITCH_CASE);
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(label << SL_BB_ENTRY_SUFFIX)
            << " [color=yellow];" << std::endl;
}

void ClDotGenerator::insn_switch_close() {
    lastInsn_ =
        /* FIXME: we have no CL_INSN_SWITCH in enum cl_insn_e */
        CL_INSN_JMP;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_dotgen.hh for more details
ICodeListener* createClDotGenerator(const char *args) {
    return new ClDotGenerator(args);
}
