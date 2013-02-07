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
#include "cl_pp.hh"

#include <cl/cl_msg.hh>

#include "cl.hh"
#include "cl_private.hh"
#include "ssd.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

class ClPrettyPrint: public ICodeListener {
    public:
        ClPrettyPrint(bool showTypes);
        ClPrettyPrint(const char *fileName, bool showTypes);
        virtual ~ClPrettyPrint();

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

        virtual void acknowledge() { }

    private:
        const char              *fname_;
        std::fstream            fstr_;
        std::ostream            &out_;
        struct cl_loc           loc_;
        std::string             fnc_;
        bool                    showTypes_;
        bool                    printingArgDecls_;

    private:
        void printIntegralCst   (const struct cl_operand *);
        void printCst           (const struct cl_operand *);
        void printBareType      (const struct cl_type *, bool expandFnc);
        void printVarType       (const struct cl_operand *);
        void printNestedVar     (const struct cl_operand *);
        void printOffsetAccessor(const int off);
        void printRecordAccessor(const struct cl_accessor **);
        void printOperandVar    (const struct cl_operand *);
        void printOperand       (const struct cl_operand *);
        void printAssignmentLhs (const struct cl_operand *);
        void printInsnNop       (const struct cl_insn *);
        void printInsnJmp       (const struct cl_insn *);
        void printInsnCond      (const struct cl_insn *);
        void printInsnRet       (const struct cl_insn *);
        void printInsnAbort     (const struct cl_insn *);
        void printInsnUnop      (const struct cl_insn *);
        void printInsnBinop     (const struct cl_insn *);
        void printInsnLabel     (const struct cl_insn *);
};

using namespace ssd;
using std::string;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(bool showTypes):
    fname_(0),
    out_(std::cout),
    showTypes_(showTypes),
    printingArgDecls_(false)
{
    // FIXME: static variable
    ColorConsole::enableForTerm(STDOUT_FILENO);
}

ClPrettyPrint::ClPrettyPrint(const char *fileName, bool showTypes):
    fname_(fileName),
    fstr_(fileName, std::fstream::out),
    out_(fstr_),
    showTypes_(showTypes),
    printingArgDecls_(false)
{
    if (!fstr_)
        CL_ERROR("unable to create file '" << fileName << "'");
}

ClPrettyPrint::~ClPrettyPrint()
{
    if (fname_)
        fstr_.close();
}

void ClPrettyPrint::file_open(
            const char              *file_name)
{
    CL_LOC_SET_FILE(loc_, file_name);
}

void ClPrettyPrint::file_close()
{
    loc_ = cl_loc_unknown;
    out_ << std::endl;
}

void ClPrettyPrint::fnc_open(
            const struct cl_operand *fnc)
{
    const struct cl_cst &cst = fnc->data.cst;
    fnc_ = cst.data.cst_fnc.name;
    loc_ = cst.data.cst_fnc.loc;
    switch (fnc->scope) {
        case CL_SCOPE_GLOBAL:
            break;

        case CL_SCOPE_STATIC:
            out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "static") << " ";
            break;

        default:
            CL_ERROR_MSG(&loc_, "invalid scope for function: " << fnc->scope);
    }
    this->printVarType(fnc);
    SSD_COLORIZE(out_, C_LIGHT_BLUE) << fnc_;
    SSD_COLORIZE(out_, C_LIGHT_RED) << "(";
    printingArgDecls_ = true;
}

void ClPrettyPrint::fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    // TODO: sort arguments if not already
    if (1 < arg_id)
        out_ << ", ";
    out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "%arg" << arg_id) << ": ";
    this->printOperandVar(arg_src);
}

void ClPrettyPrint::fnc_close()
{
    out_ << std::endl;
}

void ClPrettyPrint::bb_open(
            const char              *bb_name)
{
    out_ << std::endl;
    out_ << "\t"
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, bb_name)
        << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << std::endl;
}

namespace {
    string prettyEscaped(const char *raw_str) {
        std::string ret;
        unsigned char c;
        while ((c = *raw_str++)) {
            if (isprint(c) && '\'' != c && '\"' != c && '\\' != c) {
                // preserve printable chars
                ret += static_cast<char>(c);
                continue;
            }

            // escape non-printable chars
            ret += '\\';

            switch (c) {
                // simple escaping
                case '\'':  ret += '\'';    continue;
                case '\"':  ret += '\"';    continue;
                case '\\':  ret += '\\';    continue;
                case '\n':  ret += 'n';     continue;
                case '\r':  ret += 'r';     continue;
                case '\t':  ret += 't';     continue;

                // octal escaping
                default:
                    ret += ('0' + ( c        >> 6 ));
                    ret += ('0' + ((c & 070) >> 3 ));
                    ret += ('0' +  (c & 007)       );
            }
        }

        return ret;
    }

    inline string prettyEscaped(const char raw_char) {
        const char raw_str[] = { raw_char, '\0' };
        return prettyEscaped(raw_str);
    }
}

void ClPrettyPrint::printIntegralCst(const struct cl_operand *op)
{
    const struct cl_type *type = op->type;
    const long value = op->data.cst.data.cst_int.value;

    enum cl_type_e code = type->code;
    switch (code) {
        case CL_TYPE_PTR: {
            struct cl_accessor *ac = op->accessor;
            if (ac && ac->code == CL_ACCESSOR_DEREF) {
                if (ac->code == CL_ACCESSOR_DEREF)
                    SSD_COLORIZE(out_, C_LIGHT_RED) << "*";
                else
                    CL_DEBUG("unexpected accessor by CL_OPERAND_CST pointer");
                if (ac->next)
                    CL_DEBUG("2+ accessors by CL_OPERAND_CST pointer");
            }

            if (value) {
                const std::ios_base::fmtflags oldFlags = out_.flags();
                SSD_COLORIZE(out_, C_LIGHT_RED) << "0x" << std::hex << value;
                out_.flags(oldFlags);
            }
            else
                SSD_COLORIZE(out_, C_WHITE) << "NULL";

            }
            break;

        case CL_TYPE_UNKNOWN:
        case CL_TYPE_ENUM:
            this->printVarType(op);
            // fall through!

        case CL_TYPE_INT:
            if (value < 0)
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");

            SSD_COLORIZE(out_, C_WHITE) << value << ((op->type->is_unsigned)
                    ? "U"
                    : "");

            if (value < 0)
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")");

            break;

        case CL_TYPE_BOOL:
            if (value)
                SSD_COLORIZE(out_, C_WHITE) << "true";
            else
                SSD_COLORIZE(out_, C_WHITE) << "false";
            break;

        case CL_TYPE_CHAR:
            SSD_COLORIZE(out_, C_WHITE)
                << "\'" << prettyEscaped(static_cast<char>(value)) << "\'";
            break;

        default:
            CL_BREAK_IF("printIntegralCst() got something special");
    }
}

void ClPrettyPrint::printCst(const struct cl_operand *op)
{
    enum cl_type_e code = op->data.cst.code;
    switch (code) {
        case CL_TYPE_INT:
            this->printIntegralCst(op);
            break;

        case CL_TYPE_FNC:
            this->printVarType(op);
            if (!op->data.cst.data.cst_fnc.name) {
                CL_ERROR_MSG(&loc_, "anonymous function");
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN,
                    op->data.cst.data.cst_fnc.name);
            break;

        case CL_TYPE_STRING: {
                const char *text = op->data.cst.data.cst_string.value;
                if (!text) {
                    CL_ERROR_MSG(&loc_, "CL_TYPE_STRING with no string");
                    break;
                }
                SSD_COLORIZE(out_, C_LIGHT_PURPLE)
                    << "\"" << prettyEscaped(text) << "\"";
            }
            break;

        case CL_TYPE_REAL:
            SSD_COLORIZE(out_, C_WHITE) << op->data.cst.data.cst_real.value;
            break;

        default:
            CL_TRAP;
    }
}

namespace {
    const char* typeName(const struct cl_type *clt) {
        CL_BREAK_IF(!clt);

        const char *name = clt->name;
        return (name)
            ? name
            : "<anon_type>";
    }

    const char* strUnsig(const struct cl_type *clt) {
        return (clt->is_unsigned)
            ? "unsigned "
            : "";
    }
}

void ClPrettyPrint::printBareType(const struct cl_type *clt, bool expandFnc)
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
        out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "<invalid type>");
        return;
    }

    enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_VOID:
            out_ << SSD_INLINE_COLOR(C_GREEN, "void");
            break;

        case CL_TYPE_UNKNOWN:
            out_ << SSD_INLINE_COLOR(C_LIGHT_CYAN, typeName(clt));
            break;

        case CL_TYPE_STRUCT:
            out_ << SSD_INLINE_COLOR(C_GREEN, "struct") << " "
                << SSD_INLINE_COLOR(C_DARK_GRAY, typeName(clt));
            break;

        case CL_TYPE_UNION:
            out_ << SSD_INLINE_COLOR(C_GREEN, "union") << " "
                << SSD_INLINE_COLOR(C_DARK_GRAY, typeName(clt));
            break;

        case CL_TYPE_FNC:
            if (expandFnc) {
                // recursion limited to depth 1
                this->printBareType(clt->items[0].type, false);
                str = string("(") + str + string(")");
            } else {
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "fnc");
            }
            break;

        case CL_TYPE_INT:
            out_ << SSD_INLINE_COLOR(C_GREEN, strUnsig(clt) << "int");
            break;

        case CL_TYPE_CHAR:
            out_ << SSD_INLINE_COLOR(C_GREEN, "char");
            break;

        case CL_TYPE_BOOL:
            out_ << SSD_INLINE_COLOR(C_GREEN, "bool");
            break;

        case CL_TYPE_ENUM:
            out_ << SSD_INLINE_COLOR(C_GREEN, "enum") << " "
                << SSD_INLINE_COLOR(C_DARK_GRAY, typeName(clt));
            break;

        case CL_TYPE_REAL:
            out_ << SSD_INLINE_COLOR(C_GREEN, "real");
            break;

        default:
            CL_TRAP;
    }

    if (!str.empty())
        str = string(" ") + str;
    SSD_COLORIZE(out_, C_DARK_GRAY) << str;

    if (expandFnc && CL_TYPE_FNC == code) {
        SSD_COLORIZE(out_, C_DARK_GRAY) << "(";
        for (int i = 1; i < clt->item_cnt; ++i) {
            if (1 < i)
                SSD_COLORIZE(out_, C_DARK_GRAY) << ", ";

            this->printBareType(clt->items[i].type, false);
        }
        SSD_COLORIZE(out_, C_DARK_GRAY) << ")";
    }
}

void ClPrettyPrint::printVarType(const struct cl_operand *op)
{
    if (op->code == CL_OPERAND_VOID)
        CL_TRAP;

    if (!showTypes_)
        return;

    const cl_type *clt = op->type;

    SSD_COLORIZE(out_, C_DARK_GRAY) << "[";
    this->printBareType(clt, true);
    SSD_COLORIZE(out_, C_CYAN) << ":" << clt->size;
    SSD_COLORIZE(out_, C_DARK_GRAY) << "]";
}

namespace {
    char scopeFlag(enum cl_scope_e scope) {
        switch (scope) {
            case CL_SCOPE_GLOBAL:       return 'G';
            case CL_SCOPE_STATIC:       return 'S';
            case CL_SCOPE_FUNCTION:     return 'F';
            default:
                CL_TRAP;
                return 'U';
        }
    }
}

void ClPrettyPrint::printNestedVar(const struct cl_operand *op)
{
    switch (op->code) {
        case CL_OPERAND_VAR:
            if (!op->data.var->name) {
                SSD_COLORIZE(out_, C_LIGHT_BLUE) << "%r" << op->data.var->uid;
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, "%m" << scopeFlag(op->scope))
                << op->data.var->uid << ":";
            switch (op->scope) {
                case CL_SCOPE_GLOBAL:
                case CL_SCOPE_STATIC:
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, op->data.var->name);
                    break;
                default:
                    out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, op->data.var->name);
            }
            break;

        default:
            CL_ERROR("internal error in " << __FUNCTION__);
            break;
    }
}

namespace {
    void readItemAccessInfo(const struct cl_accessor *ac, std::string *pName,
                            int *pOffset)
    {
        int id = ac->data.item.id;
        const struct cl_type_item *item = ac->type->items + id;
        if (item->name)
            *pName += item->name;
        else
            *pName += "<anon_item>";

        *pOffset += item->offset;
    }
}

void ClPrettyPrint::printOffsetAccessor(const int off)
{
    out_ << ssd::Color(C_LIGHT_RED) << "<";

    if (0 <= off)
        out_ << "+";

    out_ << off << ">" << ssd::Color(C_NO_COLOR);
}

void ClPrettyPrint::printRecordAccessor(const struct cl_accessor **ac)
{
    std::string tag;
    int offset = 0;
    readItemAccessInfo(*ac, &tag, &offset);

    while ((*ac)->next && (*ac)->next->code == CL_ACCESSOR_ITEM) {
        *ac = (*ac)->next;
        tag += ".";
        //out_ << SSD_INLINE_COLOR(C_CYAN, "[+" << offset << "]");
        readItemAccessInfo(*ac, &tag, &offset);
    }

    out_ << SSD_INLINE_COLOR(C_CYAN, "[+" << offset << "]") << tag;
}

void ClPrettyPrint::printOperandVar(const struct cl_operand *op)
{
    const struct cl_accessor *ac = op->accessor;
    this->printVarType(op);

    // check whether the operand is referenced
    const struct cl_accessor *is_ref = ac;
    while (is_ref && (is_ref->next || is_ref->code != CL_ACCESSOR_REF))
        is_ref = is_ref->next;
    if (is_ref)
        SSD_COLORIZE(out_, C_LIGHT_RED) << "&";

    if (ac && ac->code == CL_ACCESSOR_DEREF &&
            (!ac->next || ac->next->code != CL_ACCESSOR_ITEM))
    {
        SSD_COLORIZE(out_, C_LIGHT_RED) << "*";
        ac = ac->next;
    }

    // print reg/var/arg
    this->printNestedVar(op);

    if (ac && ac->code == CL_ACCESSOR_DEREF &&
            ac->next && ac->next->code == CL_ACCESSOR_ITEM)
    {
        SSD_COLORIZE(out_, C_LIGHT_RED) << "->";
        ac = ac->next;

    } else if (ac && ac->code == CL_ACCESSOR_ITEM) {
        SSD_COLORIZE(out_, C_LIGHT_RED) << ".";
    }

    for (; ac; ac = ac->next) {
        enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_DEREF_ARRAY:
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "[");

                // FIXME: possible recursion (not intentional)
                this->printOperand(ac->data.array.index);

                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "]");
                break;

            case CL_ACCESSOR_OFFSET:
                this->printOffsetAccessor(ac->data.offset.off);
                break;

            case CL_ACCESSOR_ITEM:
                this->printRecordAccessor(&ac);
                break;

            case CL_ACCESSOR_REF:
                if (!ac->next)
                    // already handled
                    break;

            default:
                CL_BREAK_IF("printOperandVar() got invalid accessor");
        }
    }
}

void ClPrettyPrint::printOperand(const struct cl_operand *op)
{
    if (!op) {
        CL_DEBUG_MSG(&loc_, "no operand given to " << __FUNCTION__);
        return;
    }

    switch (op->code) {
        case CL_OPERAND_VOID:
            SSD_COLORIZE(out_, C_LIGHT_BLUE) << "CL_OPERAND_VOID";
            break;

        case CL_OPERAND_CST:
            if (op->accessor && op->type->code != CL_TYPE_PTR)
                CL_DEBUG("accessor by non-pointer CL_OPERAND_CST");
            this->printCst(op);
            break;

        case CL_OPERAND_VAR:
            this->printOperandVar(op);
            break;
    }
}

void ClPrettyPrint::printAssignmentLhs(const struct cl_operand *lhs)
{
    if (!lhs || lhs->code == CL_OPERAND_VOID) {
        CL_DEBUG_MSG(&loc_, "no lhs given to " << __FUNCTION__);
        return;
    }

    this->printOperand(lhs);
    out_ << " "
        << SSD_INLINE_COLOR(C_YELLOW, ":=")
        << " ";
}

void ClPrettyPrint::printInsnNop(const struct cl_insn *)
{
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "nop")
        << std::endl;
}

void ClPrettyPrint::printInsnJmp(const struct cl_insn *cli)
{
    if (printingArgDecls_) {
        printingArgDecls_ = false;
        out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")") << ":"
            << std::endl;
    }

    const char *label = cli->data.insn_jmp.label;
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, label)
        << std::endl;
}

void ClPrettyPrint::printInsnCond(const struct cl_insn *cli)
{
    const struct cl_operand *src = cli->data.insn_cond.src;
    const char *label_true  = cli->data.insn_cond.then_label;
    const char *label_false = cli->data.insn_cond.else_label;

    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "if (");

    this->printOperand(src);

    out_ << SSD_INLINE_COLOR(C_YELLOW, ")")
        << std::endl

        << "\t\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, label_true)
        << std::endl

        << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "else")
        << std::endl

        << "\t\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, label_false)
        << std::endl;
}

void ClPrettyPrint::printInsnRet(const struct cl_insn *cli)
{
    const struct cl_operand *src = cli->data.insn_ret.src;

    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "ret");

    if (src && src->code != CL_OPERAND_VOID) {
        out_ << " ";
        this->printOperand(src);
    }

    out_ << std::endl;
}

void ClPrettyPrint::printInsnAbort(const struct cl_insn *)
{
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "abort")
        << std::endl;
}

void ClPrettyPrint::printInsnUnop(const struct cl_insn *cli)
{
    const enum cl_unop_e code       = cli->data.insn_unop.code;
    const struct cl_operand *dst    = cli->data.insn_unop.dst;
    const struct cl_operand *src    = cli->data.insn_unop.src;

    out_ << "\t\t";
    this->printAssignmentLhs(dst);

    switch (code) {
        case CL_UNOP_TRUTH_NOT:
            out_ << SSD_INLINE_COLOR(C_YELLOW, "!");
            break;

        case CL_UNOP_BIT_NOT:
            out_ << SSD_INLINE_COLOR(C_RED, "~");
            break;

        case CL_UNOP_MINUS:
            out_ << SSD_INLINE_COLOR(C_RED, "-");
            break;

        case CL_UNOP_ABS:
            out_ << SSD_INLINE_COLOR(C_LIGHT_PURPLE, "abs") << "(";
            this->printOperand(src);
            out_ << ")" << std::endl;
            return;

        case CL_UNOP_FLOAT:
            out_ << SSD_INLINE_COLOR(C_LIGHT_PURPLE, "(float)");
            break;

        case CL_UNOP_ASSIGN:
            break;
    }

    this->printOperand(src);
    out_ << std::endl;
}

void ClPrettyPrint::printInsnBinop(const struct cl_insn *cli)
{
    const enum cl_binop_e code      = cli->data.insn_binop.code;
    const struct cl_operand *dst    = cli->data.insn_binop.dst;
    const struct cl_operand *src1   = cli->data.insn_binop.src1;
    const struct cl_operand *src2   = cli->data.insn_binop.src2;

    out_ << "\t\t";
    this->printAssignmentLhs(dst);
    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");
    this->printOperand(src1);
    out_ << " ";

    switch (code) {
        case CL_BINOP_EQ:
            SSD_COLORIZE(out_, C_YELLOW) << "==";
            break;

        case CL_BINOP_NE:
            SSD_COLORIZE(out_, C_YELLOW) << "!=";
            break;

        case CL_BINOP_LT:
            SSD_COLORIZE(out_, C_YELLOW) << "<";
            break;

        case CL_BINOP_GT:
            SSD_COLORIZE(out_, C_YELLOW) << ">";
            break;

        case CL_BINOP_LE:
            SSD_COLORIZE(out_, C_YELLOW) << "<=";
            break;

        case CL_BINOP_GE:
            SSD_COLORIZE(out_, C_YELLOW) << ">=";
            break;

        case CL_BINOP_TRUTH_AND:
            SSD_COLORIZE(out_, C_YELLOW) << "and";
            break;

        case CL_BINOP_TRUTH_OR:
            SSD_COLORIZE(out_, C_YELLOW) << "or";
            break;

        case CL_BINOP_TRUTH_XOR:
            SSD_COLORIZE(out_, C_YELLOW) << "xor";
            break;

        case CL_BINOP_PLUS:
            SSD_COLORIZE(out_, C_YELLOW) << "+";
            break;

        case CL_BINOP_MINUS:
            SSD_COLORIZE(out_, C_YELLOW) << "-";
            break;

        case CL_BINOP_MULT:
            SSD_COLORIZE(out_, C_YELLOW) << "*";
            break;

        case CL_BINOP_EXACT_DIV:
        case CL_BINOP_TRUNC_DIV:
        case CL_BINOP_RDIV:
            SSD_COLORIZE(out_, C_YELLOW) << "/";
            break;

        case CL_BINOP_TRUNC_MOD:
            SSD_COLORIZE(out_, C_YELLOW) << "%";
            break;

        case CL_BINOP_MIN:
            SSD_COLORIZE(out_, C_YELLOW) << "min";
            break;

        case CL_BINOP_MAX:
            SSD_COLORIZE(out_, C_YELLOW) << "max";
            break;

        case CL_BINOP_POINTER_PLUS:
            out_ << "[ptr]" << SSD_INLINE_COLOR(C_LIGHT_RED, "+");
            break;

        case CL_BINOP_BIT_AND:
            SSD_COLORIZE(out_, C_YELLOW) << "&";
            break;

        case CL_BINOP_BIT_IOR:
            SSD_COLORIZE(out_, C_YELLOW) << "|";
            break;

        case CL_BINOP_BIT_XOR:
            SSD_COLORIZE(out_, C_YELLOW) << "^";
            break;

        case CL_BINOP_LSHIFT:
            SSD_COLORIZE(out_, C_YELLOW) << "<<";
            break;

        case CL_BINOP_RSHIFT:
            SSD_COLORIZE(out_, C_YELLOW) << ">>";
            break;

        case CL_BINOP_LROTATE:
            SSD_COLORIZE(out_, C_RED) << "L-ROTATE-BY";
            break;

        case CL_BINOP_RROTATE:
            SSD_COLORIZE(out_, C_RED) << "R-ROTATE-BY";
            break;
    }

    out_ << " ";
    this->printOperand(src2);
    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")") << std::endl;
}

void ClPrettyPrint::printInsnLabel(const struct cl_insn *cli)
{
    const char *name = cli->data.insn_label.name;
    if (!name)
        return;

    out_ << "\t"
        << SSD_INLINE_COLOR(C_LIGHT_GREEN, name)
        << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << std::endl;
}

void ClPrettyPrint::insn(
            const struct cl_insn    *cli)
{
    loc_ = cli->loc;
    switch (cli->code) {
        case CL_INSN_NOP:
            this->printInsnNop(cli);
            break;

        case CL_INSN_JMP:
            this->printInsnJmp(cli);
            break;

        case CL_INSN_COND:
            this->printInsnCond(cli);
            break;

        case CL_INSN_RET:
            this->printInsnRet(cli);
            break;

        case CL_INSN_ABORT:
            this->printInsnAbort(cli);
            break;

        case CL_INSN_UNOP:
            this->printInsnUnop(cli);
            break;

        case CL_INSN_BINOP:
            this->printInsnBinop(cli);
            break;

        case CL_INSN_CALL:
            CL_BREAK_IF("ClPrettyPrint::insn() got CL_INSN_CALL, why?");
            break;

        case CL_INSN_SWITCH:
            CL_BREAK_IF("ClPrettyPrint::insn() got CL_INSN_SWITCH, why?");
            break;

        case CL_INSN_LABEL:
            this->printInsnLabel(cli);
            break;
    }
}

void ClPrettyPrint::insn_call_open(
            const struct cl_loc     *loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
{
    CL_LOC_SETIF(loc_, loc);
    out_ << "\t\t";
    if (dst && dst->code != CL_OPERAND_VOID)
        this->printAssignmentLhs(dst);
    this->printOperand(fnc);
    out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "(");
}

void ClPrettyPrint::insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    // TODO: sort arguments if not already
    if (1 < arg_id)
        out_ << ", ";
    this->printOperand(arg_src);
}

void ClPrettyPrint::insn_call_close()
{
    out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, ")")
        << std::endl;
}

void ClPrettyPrint::insn_switch_open(
            const struct cl_loc     *loc,
            const struct cl_operand *src)
{
    CL_LOC_SETIF(loc_, loc);
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "switch (");

    this->printOperand(src);

    out_ << SSD_INLINE_COLOR(C_YELLOW, ")") << " {"
        << std::endl;
}

// TODO: simplify
void ClPrettyPrint::insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    CL_LOC_SETIF(loc_, loc);
    if (CL_OPERAND_VOID == val_lo->code && CL_OPERAND_VOID == val_hi->code) {
        out_ << "\t\t\t"
            << SSD_INLINE_COLOR(C_YELLOW, "default") << ":";
    } else if (CL_OPERAND_CST != val_lo->code
               || CL_OPERAND_CST != val_hi->code) {
        CL_TRAP;
    } else {
        const struct cl_cst &cst_lo = val_lo->data.cst;
        const struct cl_cst &cst_hi = val_hi->data.cst;
        if (CL_TYPE_INT != cst_lo.code || CL_TYPE_INT != cst_hi.code)
            CL_TRAP;

        const int lo = cst_lo.data.cst_int.value;
        const int hi = cst_hi.data.cst_int.value;
        for (int i = lo; i <= hi; ++i) {
            out_ << "\t\t\t"
                << SSD_INLINE_COLOR(C_YELLOW, "case")
                << " " << i << ":";
            if (i != hi)
                out_ << " /* fall through */" << std::endl;
        }
    }

    out_ << " "
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, label)
        << std::endl;
}

void ClPrettyPrint::insn_switch_close()
{
    out_ << "\t\t}" << std::endl;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.hh for more details
ICodeListener* createClPrettyPrint(const char *args, bool showTypes)
{
    // check whether a file name is given
    return (args && *args)
        ? new ClPrettyPrint(/* file name */ args, showTypes)
        : new ClPrettyPrint(showTypes);
}
