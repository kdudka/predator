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
#include "cl_typedot.hh"

#include <cl/cl_msg.hh>

#include "cl.hh"
#include "util.hh"

#include <fstream>
#include <set>
#include <stack>
#include <vector>

typedef int cl_type_uid_t;

/// @todo rewrite to share code using the template method design pattern
class ClTypeDotGenerator: public ICodeListener {
    public:
        ClTypeDotGenerator(const char *glDotFile);
        virtual ~ClTypeDotGenerator();

        virtual void file_open(const char *) { }
        virtual void file_close() { }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            fnc_ = fnc->data.cst.data.cst_fnc.name;
        }

        virtual void fnc_arg_decl(int, const struct cl_operand *arg_src)
        {
            this->handleOperand(arg_src);
        }

        virtual void fnc_close() {
            fnc_.clear();
        }

        virtual void bb_open(const char *) { }

        virtual void insn(const struct cl_insn *cli) {
            switch (cli->code) {
                case CL_INSN_NOP:
                case CL_INSN_JMP:
                case CL_INSN_ABORT:
                case CL_INSN_LABEL:
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

                case CL_INSN_CALL:
                case CL_INSN_SWITCH:
                    CL_TRAP;
            }
        }

        virtual void insn_call_open(const struct cl_loc *,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            this->handleOperand(dst);
            this->handleOperand(fnc);
        }

        virtual void insn_call_arg(int, const struct cl_operand *arg_src)
        {
            this->handleOperand(arg_src);
        }

        virtual void insn_call_close() { }

        virtual void insn_switch_open(
            const struct cl_loc     *,
            const struct cl_operand *src)
        {
            this->handleOperand(src);
        }

        virtual void insn_switch_case(
            const struct cl_loc     *,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char *)
        {
            this->handleOperand(val_lo);
            this->handleOperand(val_hi);
        }

        virtual void insn_switch_close() { }

        virtual void acknowledge();

    private:
        typedef std::stack<const struct cl_type *> TStack;

        std::ofstream           glOut_;
        std::string             fnc_;

        std::set<cl_type_uid_t> typeSet_;

    private:
        struct Edge {
            cl_type_uid_t   src;
            cl_type_uid_t   dst;
            enum cl_type_e  code;
            std::string     label;

            Edge(cl_type_uid_t src_, cl_type_uid_t dst_, enum cl_type_e code_,
                    const std::string &label_):
                src(src_),
                dst(dst_),
                code(code_),
                label(label_)
            {
            }
        };
        typedef std::vector<Edge>                       TEdgeList;

        TEdgeList               pendingEdges_;
        static const char       *CltColors[];

    private:
        void printType(const struct cl_type *clt);
        void gobbleEdge(cl_type_uid_t src, cl_type_uid_t dst,
                        enum cl_type_e code, const char *label);
        void emitPendingEdges();
        void digOneType(const struct cl_type *, TStack &);
        void handleType(const struct cl_type *);
        void handleOperand(const struct cl_operand *operand);
};

using std::ios;
using std::string;

enum {
    CNT_CL_TYPES = /* FIXME */ CL_TYPE_STRING + 1
};
const char *ClTypeDotGenerator::CltColors[CNT_CL_TYPES] = {
    "green",                        // CL_TYPE_VOID
    "red",                          // CL_TYPE_PTR
    "blue",                         // CL_TYPE_STRUCT
    "cyan",                         // CL_TYPE_UNION
    "yellow",                       // CL_TYPE_ARRAY
    "black",                        // CL_TYPE_FNC
    "green",                        // CL_TYPE_INT
    "green",                        // CL_TYPE_CHAR
    "green",                        // CL_TYPE_BOOL
    "green",                        // CL_TYPE_ENUM
    "purple",                       // CL_TYPE_STRING
};

// /////////////////////////////////////////////////////////////////////////////
// ClTypeDotGenerator implementation
ClTypeDotGenerator::ClTypeDotGenerator(const char *glDotFile)
{
    glOut_.open(glDotFile, ios::out);
    if (glOut_) {
        CL_DEBUG("ClTypeDotGenerator: created dot file '" << glDotFile << "'");
    } else {
        CL_ERROR("unable to create file '" << glDotFile << "'");
    }
    glOut_ << "digraph types" << " {" << std::endl
        << "\tlabel=<<FONT POINT-SIZE=\"18\">"
        << "\"data type graph\"" << "</FONT>>;" << std::endl
        << "\tlabelloc=t;" << std::endl;
}

ClTypeDotGenerator::~ClTypeDotGenerator()
{
    glOut_ << "}" << std::endl;
    if (!glOut_) {
        CL_WARN("error detected while closing a file");
    }
    glOut_.close();
}

void ClTypeDotGenerator::acknowledge()
{
    // we haven't been waiting for acknowledge anyway, sorry...
}

namespace {
    // FIXME: copy pasted from cl_pp.cc
    const char* typeName(const struct cl_type *clt) {
        CL_BREAK_IF(!clt);

        const char *name = clt->name;
        return (name)
            ? name
            : "<anon_type>";
    }
}

// FIXME: copy pasted from ClPrettyPrint::printVarType
void ClTypeDotGenerator::printType(const struct cl_type *clt)
{
    string str;
    for (; clt; clt = clt->items[0].type) {
        enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_PTR:
                str = string("*") + str;
                break;

            case CL_TYPE_ARRAY:
                str = string("[]") + str;
                break;

            default:
                goto deref_done;
        }
    }
deref_done:

    if (!clt) {
        glOut_ << "<invalid type>";
        return;
    }

    enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_VOID:
            glOut_ << "void";
            break;

        case CL_TYPE_UNKNOWN:
            glOut_ << "unknown";
            break;

        case CL_TYPE_STRUCT:
            glOut_ << "struct" << " " << typeName(clt);
            break;

        case CL_TYPE_UNION:
            glOut_ << "union" << " " << typeName(clt);
            break;

        case CL_TYPE_FNC:
            glOut_ << "fnc";
            break;

        case CL_TYPE_INT:
            if (clt->is_unsigned)
                glOut_ << "unsigned ";

            glOut_ << "int";
            break;

        case CL_TYPE_CHAR:
            glOut_ << "char";
            break;

        case CL_TYPE_BOOL:
            glOut_ << "bool";
            break;

        case CL_TYPE_REAL:
            glOut_ << "real";
            break;

        case CL_TYPE_ENUM:
            glOut_ << "enum" << " " << typeName(clt);
            break;

        default:
            CL_BREAK_IF("base type not supported by ClTypeDotGenerator");
    }

    if (!str.empty())
        str = string(" ") + str;
    glOut_ << str;
}

void ClTypeDotGenerator::gobbleEdge(cl_type_uid_t src, cl_type_uid_t dst,
                                    enum cl_type_e code, const char *label)
{
    string strLabel;
    if (label)
        strLabel = label;

    pendingEdges_.push_back(Edge(src, dst, code, strLabel));
}

void ClTypeDotGenerator::emitPendingEdges()
{
    TEdgeList::iterator i;
    for (i = pendingEdges_.begin(); i != pendingEdges_.end(); ++i) {
        const Edge &e = *i;
        glOut_ << e.src << " -> " << e.dst
            << "[color=" << CltColors[e.code];
        if (!e.label.empty())
            glOut_ << ", label=\"" << e.label << "\"";
        glOut_ << "];" << std::endl;
    }
    pendingEdges_.clear();
}

void ClTypeDotGenerator::digOneType(const struct cl_type *type, TStack &st)
{
    if (!type)
        // TRAP;
        return;

    cl_type_uid_t uid = type->uid;
    enum cl_type_e code = type->code;

    glOut_ << uid << " [label=\"#" << uid << ": ";
    this->printType(type);
    glOut_ << "\", color=" << CltColors[code] << "];" << std::endl;

    switch (code) {
        case CL_TYPE_VOID:
        case CL_TYPE_UNKNOWN:
            break;

        case CL_TYPE_PTR:
        case CL_TYPE_ARRAY:
        case CL_TYPE_STRUCT:
        case CL_TYPE_UNION: {
                int cnt = type->item_cnt;
                if (CL_TYPE_ARRAY == code)
                    cnt = 1;

                for (int i = 0; i < cnt; ++i) {
                    struct cl_type_item &item = type->items[i];
                    const struct cl_type *clt = item.type;
                    cl_type_uid_t dst = clt->uid;
                    st.push(clt);
                    this->gobbleEdge(uid, dst, code, item.name);
                }
            }
            break;

        case CL_TYPE_FNC:
        case CL_TYPE_INT:
        case CL_TYPE_CHAR:
        case CL_TYPE_BOOL:
        case CL_TYPE_ENUM:
        case CL_TYPE_STRING:
        case CL_TYPE_REAL:
            break;
    }
}

void ClTypeDotGenerator::handleType(const struct cl_type *clt)
{
    TStack st;
    st.push(clt);

    while (!st.empty()) {
        clt = st.top();
        st.pop();

        cl_type_uid_t uid = clt->uid;
        if (hasKey(typeSet_, uid))
            continue;

        typeSet_.insert(uid);
        this->digOneType(clt, st);
    }

    this->emitPendingEdges();
}

void ClTypeDotGenerator::handleOperand(const struct cl_operand *op)
{
    if (!op || op->code == CL_OPERAND_VOID)
        return;

    const struct cl_type *type = op->type;
    if (!type)
        return;

    this->handleType(type);
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_typedot.hh for more details
ICodeListener* createClTypeDotGenerator(const char *args)
{
    return new ClTypeDotGenerator(args);
}
