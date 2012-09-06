/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config_cl.h"
#include "cl_dotgen.hh"

#include <cl/cl_msg.hh>

#include "cl.hh"
#include "cl_private.hh"
#include "util.hh"

#include <libgen.h>         // for basename(3)

#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include <boost/algorithm/string/replace.hpp>

class ClDotGenerator: public ICodeListener {
    public:
        ClDotGenerator(const char *glDotFile);
        virtual ~ClDotGenerator();

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            const struct cl_operand *fnc);

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn(
            const struct cl_insn    *cli);

        virtual void insn_call_open(
            const struct cl_loc     *loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc);

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void insn_call_close();

        virtual void insn_switch_open(
            const struct cl_loc     *loc,
            const struct cl_operand *src);

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label);

        virtual void insn_switch_close();

        virtual void acknowledge();

    private:
        bool                    hasGlDotFile_;
        std::ofstream           glOut_;
        std::ofstream           perFileOut_;
        std::ofstream           perFncOut_;

        struct cl_loc           loc_;
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
            ET_GL_CALL_INDIR,
            ET_PTR_CALL
        };
        enum {
            CNT_ET = ET_PTR_CALL + 1
        };
        static const char       *EtColors[];

        typedef std::set<std::string>                   TCallSet;
        typedef std::map<std::string, TCallSet>         TCallMultiMap;
        typedef std::map<std::string, EdgeType>         TEdgeMap;

        TCallMultiMap           perFncCalls_;
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
        void emitCallSet(std::ofstream &, TCallSet &cs, const std::string &dst);
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
    SL_QUOTE(basename((char *) loc_.file) << SL_DOT_SUFFIX)

#define SL_QUOTE_URL(fnc) \
    SL_QUOTE(basename((char *) loc_.file) \
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
    "red",                          // ET_GL_CALL_INDIR
    "green"                         // ET_PTR_CALL
};

// /////////////////////////////////////////////////////////////////////////////
// ClDotGenerator implementation
void ClDotGenerator::createDotFile(std::ofstream &str, std::string fileName,
                                   bool appendSuffix)
{
    // do not create dot files in /usr/include and the like
    boost::algorithm::replace_all(fileName, "/", "-");

    if (appendSuffix)
        fileName += ".dot";

    str.open(fileName.c_str(), ios::out);
    if (str)
        CL_DEBUG("ClDotGenerator: created dot file '" << fileName << "'");
    else
        CL_ERROR("unable to create file '" << fileName << "'");
}

void ClDotGenerator::closeSub(std::ofstream &str)
{
    str << "}" << std::endl;
}

void ClDotGenerator::closeDot(std::ofstream &str)
{
    ClDotGenerator::closeSub(str);

    if (!str)
        CL_WARN("error detected while closing a file");

    str.close();
}

ClDotGenerator::ClDotGenerator(const char *glDotFile):
    hasGlDotFile_(glDotFile && *glDotFile),
    loc_(cl_loc_unknown),
    bbPos_(0),
    nodeType_(NT_PLAIN)
{
    if (hasGlDotFile_) {
        ClDotGenerator::createDotFile(glOut_, glDotFile, false);
        glOut_ << SL_GRAPH(glDotFile);
    }
}

ClDotGenerator::~ClDotGenerator()
{
    if (hasGlDotFile_)
        this->closeDot(glOut_);
}

void ClDotGenerator::acknowledge()
{
    // we haven't been waiting for acknowledge anyway, sorry...
}

void ClDotGenerator::gobbleEdge(std::string dst, EdgeType type)
{
    perBbEdgeMap_[dst] = type;
    perFncEdgeMap_[dst] = type;
}

void ClDotGenerator::emitEdge(std::string dst, EdgeType type)
{
    switch (type) {
        case ET_LC_CALL:
        case ET_LC_CALL_INDIR:
            if (!hasKey(perFncCalls_, dst)) {
                glOut_ << "\t" << SL_QUOTE(fnc_) << " -> " << SL_QUOTE(dst)
                    << " [color=" << EtColors[type] << "];" << std::endl;
            }
            // fall through!

        case ET_PTR_CALL:
        case ET_GL_CALL:
        case ET_GL_CALL_INDIR:
            perFncCalls_[dst].insert(bb_);
            return;

        default:
            break;
    }

    perFileOut_ << "\t" << SL_QUOTE_BB(bb_) << " -> " << SL_QUOTE_BB(dst)
            << " [color=" << EtColors[type] << "];" << std::endl;
}

void ClDotGenerator::emitBb()
{
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

void ClDotGenerator::emitCallSet(std::ofstream &str, TCallSet &cs,
                                 const std::string &dst)
{
    const EdgeType type = perFncEdgeMap_[dst];

    TCallSet::iterator j;
    for (j = cs.begin(); j != cs.end(); ++j) {
    str << "\t" << SL_QUOTE_BB(*j)
        << " -> " << SL_QUOTE_BB(dst)
        << " [color=" << EtColors[type] << "];"
        << std::endl;
    }
}

void ClDotGenerator::emitPendingCalls()
{
    TCallMultiMap::iterator i;
    for (i = perFncCalls_.begin(); i != perFncCalls_.end(); ++i) {
        const string &dst = i->first;
        const EdgeType type = perFncEdgeMap_[dst];

        FILE_FNC_STREAM("\t" << SL_QUOTE_BB(dst)
            << " [label=" << SL_QUOTE(dst));
        switch (type) {
            case ET_LC_CALL:
            case ET_LC_CALL_INDIR:
                FILE_FNC_STREAM(", URL=" << SL_QUOTE_URL(i->first));
                // fall through!!
            case ET_PTR_CALL:
                FILE_FNC_STREAM(", color=" << EtColors[type]
                    << ", fontcolor=" << EtColors[type]);
            default:
                break;
        }
        FILE_FNC_STREAM("];" << std::endl);

        this->emitCallSet(perFncOut_, perBbCalls_[dst], dst);
        this->emitCallSet(perFileOut_, perFncCalls_[dst], dst);
    }

    perBbCalls_.clear();
    perFncCalls_.clear();
    perFncEdgeMap_.clear();
}

void ClDotGenerator::emitFncEntry(const char *label)
{
    FILE_FNC_STREAM(SL_SUBGRAPH(fnc_ << "." << label, fnc_
                << "() at " << loc_.file << ":" << loc_.line)
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

void ClDotGenerator::emitInsnJmp(const char *label)
{
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

void ClDotGenerator::emitOpIfNeeded()
{
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

void ClDotGenerator::emitInsnCall()
{
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=blue, fontcolor=blue, style=dashed,"
            << " label=call];" << std::endl;

    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> ";
    ++bbPos_;
    perFncOut_ << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [color=gray, style=dotted, arrowhead=open];"
            << std::endl;
}

void ClDotGenerator::checkForFncRef(const struct cl_operand *op)
{
    if (CL_OPERAND_CST != op->code)
        return;

    const struct cl_cst &cst = op->data.cst;
    if (CL_TYPE_FNC != cst.code)
        return;

    string name(cst.data.cst_fnc.name);
    this->gobbleEdge(name, (cst.data.cst_fnc.is_extern)
            ? ET_GL_CALL_INDIR
            : ET_LC_CALL_INDIR);

    std::ostringstream str;
    str << bb_ << "." << (bbPos_ - 1);
    perBbCalls_[name].insert(str.str());
}

void ClDotGenerator::file_open(const char *file_name)
{
    CL_LOC_SET_FILE(loc_, file_name);
    ClDotGenerator::createDotFile(perFileOut_, file_name, true);
    perFileOut_ << SL_GRAPH(file_name);

    glOut_ << SL_SUBGRAPH(file_name, file_name)
        << "\tcolor=red;" << std::endl
        << "\tURL=" << SL_QUOTE_PER_FILE_URL << ";" << std::endl;
}

void ClDotGenerator::file_close()
{
    loc_ = cl_loc_unknown;
    ClDotGenerator::closeDot(perFileOut_);
    ClDotGenerator::closeSub(glOut_);
}

void ClDotGenerator::fnc_open(const struct cl_operand *fnc)
{
    const struct cl_cst &cst = fnc->data.cst;
    loc_ = cst.data.cst_fnc.loc;
    fnc_ = cst.data.cst_fnc.name;

    ClDotGenerator::createDotFile(perFncOut_,
                                  string(loc_.file) + "-" + fnc_,
                                  true);
    perFncOut_ << SL_GRAPH(fnc_ << "()"
            << " at " << loc_.file << ":" << loc_.line);

    glOut_ << "\t" << SL_QUOTE(fnc_)
            << " [label=" << SL_QUOTE(fnc_)
            << ", color=" << EtColors[ET_LC_CALL]
            << ", URL=" << SL_QUOTE_URL(fnc_) << "];"
            << std::endl;
}

void ClDotGenerator::fnc_arg_decl(int, const struct cl_operand *)
{
}

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

void ClDotGenerator::bb_open(const char *bb_name)
{
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

void ClDotGenerator::insn(const struct cl_insn *cli)
{
    switch (cli->code) {
        case CL_INSN_NOP:
        case CL_INSN_LABEL:
            // ignore
            return;

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

        case CL_INSN_CALL:
        case CL_INSN_SWITCH:
            CL_TRAP;
    }
    lastInsn_ = cli->code;
}

void ClDotGenerator::insn_call_open(const struct cl_loc     *loc,
                                    const struct cl_operand *,
                                    const struct cl_operand *fnc)
{
    EdgeType callType;
    std::ostringstream name;
    switch (fnc->code) {
        case CL_OPERAND_VAR:
            callType = ET_PTR_CALL;

            if (fnc->data.var->name)
                name << fnc->data.var->name;
            else
                name << "%r" << fnc->data.var->uid;

            // TODO: handle accessor somehow
            break;

        case CL_OPERAND_CST:
            if (CL_TYPE_FNC == fnc->type->code) {
                callType = (fnc->data.cst.data.cst_fnc.is_extern)
                    ? ET_GL_CALL
                    : ET_LC_CALL;
                name << fnc->data.cst.data.cst_fnc.name;
                break;
            }
            // fall through!!

        default:
            CL_WARN_MSG(cl_loc_fallback(loc, &loc_),
                    "ClDotGenerator: unhandled call");
            return;
    }

    std::ostringstream str;
    str << bb_ << SL_BB_POS_SUFFIX;
    perBbCalls_[name.str()].insert(str.str());

    this->emitInsnCall();
    this->gobbleEdge(name.str(), callType);

    lastInsn_ =
        /* FIXME: we have no CL_INSN_CALL in enum cl_insn_e */
        CL_INSN_JMP;
}

void ClDotGenerator::insn_call_arg(int, const struct cl_operand *arg_src)
{
    this->checkForFncRef(arg_src);
}

void ClDotGenerator::insn_call_close()
{
}

void ClDotGenerator::insn_switch_open(const struct cl_loc *,
                                      const struct cl_operand *src)
{
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX)
            << " [shape=box, color=yellow, fontcolor=yellow, style=bold,"
            << " label=switch];" << std::endl
        << "}" << std::endl;
    this->checkForFncRef(src);
}

void ClDotGenerator::insn_switch_case(const struct cl_loc *,
                                      const struct cl_operand *,
                                      const struct cl_operand *,
                                      const char              *label)
{
    this->gobbleEdge(label, ET_SWITCH_CASE);
    perFncOut_ << "\t" << SL_QUOTE_BB(bb_ << SL_BB_POS_SUFFIX) << " -> "
            << SL_QUOTE_BB(label << SL_BB_ENTRY_SUFFIX)
            << " [color=yellow];" << std::endl;
}

void ClDotGenerator::insn_switch_close()
{
    lastInsn_ =
        /* FIXME: we have no CL_INSN_SWITCH in enum cl_insn_e */
        CL_INSN_JMP;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_dotgen.hh for more details
ICodeListener* createClDotGenerator(const char *args)
{
    return new ClDotGenerator(args);
}
