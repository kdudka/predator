/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cl_pp.hh"
#include "cl_private.hh"
#include "ssd.hh"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/system/system_error.hpp>

class ClPrettyPrint: public AbstractCodeListener {
    public:
        ClPrettyPrint(int fd_out, bool close_on_exit, bool showTypes);
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

        TSink                   sink_;
        TStream                 out_;
        Location                loc_;
        std::string             fnc_;
        bool                    showTypes_;
        bool                    printingArgDecls_;

    private:
        void printCst           (const struct cl_operand *);
        void printVarType       (const struct cl_operand *);
        void printNestedVar     (const struct cl_operand *);
        const char* getItemName (const struct cl_accessor *);
        void printRecordAcessor (const struct cl_accessor **);
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
};

using namespace ssd;
using std::string;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(int fd_out, bool close_on_exit, bool showTypes):
    sink_(fd_out, close_on_exit),
    out_(sink_),
    showTypes_(showTypes),
    printingArgDecls_(false)
{
    // FIXME: static variable
    ColorConsole::enableForTerm(fd_out);
}

ClPrettyPrint::~ClPrettyPrint() {
}

void ClPrettyPrint::file_open(
            const char              *file_name)
{
    loc_.currentFile = file_name;
}

void ClPrettyPrint::file_close()
{
    out_ << std::endl;
}

void ClPrettyPrint::fnc_open(
            const struct cl_operand *fnc)
{
    fnc_ = fnc->data.cst_fnc.name;
    loc_ = &fnc->loc;
    switch (fnc->scope) {
        case CL_SCOPE_GLOBAL:
            break;

        case CL_SCOPE_STATIC:
            out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "static") << " ";
            break;

        default:
            CL_MSG_STREAM(cl_error, LocationWriter(&fnc->loc) << "error: "
                    << "invalid scope for function: " << fnc->scope);
    }
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

void ClPrettyPrint::printCst(const struct cl_operand *op) {
    const struct cl_type *type = op->type;
    enum cl_type_e code = type->code;
    switch (code) {
        case CL_TYPE_UNKNOWN:
            this->printVarType(op);
            break;

        case CL_TYPE_PTR:
            if (op->data.cst_int.value)
                TRAP;

            SSD_COLORIZE(out_, C_WHITE) << "NULL";
            break;

        case CL_TYPE_ENUM:
            this->printVarType(op);
            // fall through!

        case CL_TYPE_INT: {
                int num = op->data.cst_int.value;
                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");

                SSD_COLORIZE(out_, C_WHITE) << op->data.cst_int.value;
                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")");
            }
            break;

        case CL_TYPE_FNC:
            if (!op->data.cst_fnc.name) {
                CL_MSG_STREAM(cl_error, LocationWriter(loc_) << "error: "
                        << "anonymous function");
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, op->data.cst_fnc.name);
            break;

        case CL_TYPE_BOOL:
            if (op->data.cst_int.value)
                SSD_COLORIZE(out_, C_WHITE) << "true";
            else
                SSD_COLORIZE(out_, C_WHITE) << "false";
            break;

        case CL_TYPE_STRING: {
                const char *text = op->data.cst_string.value;
                if (!text) {
                    CL_MSG_STREAM(cl_error, LocationWriter(loc_) << "error: "
                            << "CL_TYPE_STRING with no string");
                    break;
                }
                SSD_COLORIZE(out_, C_LIGHT_PURPLE) << "\"" << text << "\"";
            }
            break;

        default:
            TRAP;
    }
}

namespace {
    const char* typeName(const struct cl_type *clt) {
        if (!clt)
            TRAP;

        const char *name = clt->name;
        return (name)
            ? name
            : "<anon_type>";
    }
}

void ClPrettyPrint::printVarType(const struct cl_operand *op) {
    if (op->code == CL_OPERAND_VOID)
        TRAP;

    if (!showTypes_)
        return;

    SSD_COLORIZE(out_, C_DARK_GRAY) << "[";
    struct cl_type *clt = op->type;

    string str;
    for (; clt; clt = this->getType(clt->items[0].type)) {
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
            out_ << SSD_INLINE_COLOR(C_GREEN, "fnc");
            break;

        case CL_TYPE_INT:
            out_ << SSD_INLINE_COLOR(C_GREEN, "int");
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

        default:
            TRAP;
    }

    if (!str.empty())
        str = string(" ") + str;
    SSD_COLORIZE(out_, C_DARK_GRAY) << str << "]";
}

namespace {
    char scopeFlag(enum cl_scope_e scope) {
        switch (scope) {
            case CL_SCOPE_GLOBAL:       return 'G';
            case CL_SCOPE_STATIC:       return 'S';
            case CL_SCOPE_FUNCTION:     return 'F';
            case CL_SCOPE_BB:           return 'B';
            default:
                TRAP;
                return 'U';
        }
    }
}

void ClPrettyPrint::printNestedVar(const struct cl_operand *op) {
    switch (op->code) {
        case CL_OPERAND_VAR:
            if (!op->data.var.name) {
                CL_MSG_STREAM(cl_error, LocationWriter(loc_) << "error: "
                        << "anonymous variable");
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, "%m") << scopeFlag(op->scope)
                << op->data.var.id << ":";
            switch (op->scope) {
                case CL_SCOPE_GLOBAL:
                case CL_SCOPE_STATIC:
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, op->data.var.name);
                    break;
                default:
                    out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, op->data.var.name);
            }
            break;

        case CL_OPERAND_REG:
            SSD_COLORIZE(out_, C_LIGHT_BLUE) << "%r" << op->data.reg.id;
            break;

        case CL_OPERAND_ARG:
            SSD_COLORIZE(out_, C_LIGHT_GREEN) << "%arg" << op->data.arg.id;
            break;

        default:
            CL_MSG_STREAM(cl_debug, __FILE__ << ":" << __LINE__ << ": "
                    << "internal error in " << __FUNCTION__
                    << " [internal location]");
            break;
    }
}

const char* ClPrettyPrint::getItemName(const struct cl_accessor *ac) {
    const struct cl_type_item *items = ac->type->items;
    const char *name = items[ac->data.item.id].name;
    return (name)
        ? name
        : "<anon_item>";
}

void ClPrettyPrint::printRecordAcessor(const struct cl_accessor **ac) {
    std::string tag(this->getItemName(*ac));

    while ((*ac)->next && (*ac)->next->code == CL_ACCESSOR_ITEM) {
        *ac = (*ac)->next;
        // FIXME: check concatenation direction
        tag += ".";
        tag += this->getItemName(*ac);
    }

    out_ << tag;
}

void ClPrettyPrint::printOperandVar(const struct cl_operand *op) {
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

            case CL_ACCESSOR_ITEM:
                this->printRecordAcessor(&ac);
                break;

            case CL_ACCESSOR_REF:
                if (!ac->next)
                    // already handled
                    break;

            default:
                TRAP;
        }
    }
}

void ClPrettyPrint::printOperand(const struct cl_operand *op) {
    if (!op) {
        CL_MSG_STREAM(cl_debug, LocationWriter(loc_) << "debug: "
                << "no operand given to " << __FUNCTION__
                << " [internal location]");
        return;
    }

    switch (op->code) {
        case CL_OPERAND_VOID:
            SSD_COLORIZE(out_, C_LIGHT_BLUE) << "CL_OPERAND_VOID";
            break;

        case CL_OPERAND_CST:
            if (op->accessor)
                CL_DEBUG("CL_OPERAND_CST with op->accessor");
            this->printCst(op);
            break;

        case CL_OPERAND_ARG:
        case CL_OPERAND_REG:
        case CL_OPERAND_VAR:
            this->printOperandVar(op);
            break;
    }
}

void ClPrettyPrint::printAssignmentLhs(const struct cl_operand *lhs) {
    if (!lhs || lhs->code == CL_OPERAND_VOID) {
        CL_MSG_STREAM(cl_debug, Location(loc_) << "debug: "
                << "no lhs given to " << __FUNCTION__
                << " [internal location]");
        return;
    }

    this->printOperand(lhs);
    out_ << " "
        << SSD_INLINE_COLOR(C_YELLOW, ":=")
        << " ";
}

void ClPrettyPrint::printInsnNop(const struct cl_insn *) {
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "nop")
        << std::endl;
}

void ClPrettyPrint::printInsnJmp(const struct cl_insn *cli) {
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

void ClPrettyPrint::printInsnCond(const struct cl_insn *cli) {
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

void ClPrettyPrint::printInsnRet(const struct cl_insn *cli) {
    const struct cl_operand *src = cli->data.insn_ret.src;

    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "ret");

    if (src && src->code != CL_OPERAND_VOID) {
        out_ << " ";
        this->printOperand(src);
    }

    out_ << std::endl;
}

void ClPrettyPrint::printInsnAbort(const struct cl_insn *cli) {
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_RED, "abort")
        << std::endl;
}

void ClPrettyPrint::printInsnUnop(const struct cl_insn *cli) {
    const enum cl_unop_e code       = cli->data.insn_unop.code;
    const struct cl_operand *dst    = cli->data.insn_unop.dst;
    const struct cl_operand *src    = cli->data.insn_unop.src;

    out_ << "\t\t";
    this->printAssignmentLhs(dst);

    switch (code) {
        case CL_UNOP_TRUTH_NOT:
            out_ << SSD_INLINE_COLOR(C_YELLOW, "!");
            // fall through!

        case CL_UNOP_ASSIGN:
            this->printOperand(src);
            break;
    }

    out_ << std::endl;
}

void ClPrettyPrint::printInsnBinop(const struct cl_insn *cli) {
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
    }

    out_ << " ";
    this->printOperand(src2);
    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")") << std::endl;
}

void ClPrettyPrint::insn(
            const struct cl_insn    *cli)
{
    loc_ = &cli->loc;
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
    }
}

void ClPrettyPrint::insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
{
    loc_ = loc;
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
            const struct cl_location*loc,
            const struct cl_operand *src)
{
    loc_ = loc;
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "switch (");

    this->printOperand(src);

    out_ << SSD_INLINE_COLOR(C_YELLOW, ")") << " {"
        << std::endl;
}

// TODO: simplify
void ClPrettyPrint::insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    loc_ = loc;
    if (CL_OPERAND_VOID == val_lo->code
            && CL_OPERAND_VOID == val_hi->code)
    {
        out_ << "\t\t\t"
            << SSD_INLINE_COLOR(C_YELLOW, "default") << ":";
    } else if (CL_OPERAND_CST == val_lo->code
            && CL_OPERAND_CST == val_hi->code
            && (CL_TYPE_INT == val_lo->type->code
                || CL_TYPE_ENUM == val_lo->type->code)
            && (CL_TYPE_INT == val_hi->type->code
                || CL_TYPE_ENUM == val_hi->type->code))
    {
        const int lo = val_lo->data.cst_int.value;
        const int hi = val_hi->data.cst_int.value;
        for (int i = lo; i <= hi; ++i) {
            out_ << "\t\t\t"
                << SSD_INLINE_COLOR(C_YELLOW, "case")
                << " " << i << ":";
            if (i != hi)
                out_ << " /* fall through */" << std::endl;
        }
    } else {
        CL_MSG_STREAM(cl_error, Location(loc_) << "error: invalid case");
        return;
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
ICodeListener* createClPrettyPrint(const char *args, bool showTypes) {
    // write to stdout by default
    int fd = STDOUT_FILENO;

    // check whether a file name is given
    const bool openFile = args && *args;
    if (openFile) {
        // write to file requested
        fd = open(/* file name is the only arg for now */ args,
                  O_WRONLY | O_CREAT,
                  /* mode */ 0644);
        if (fd < 0) {
            using namespace boost::system;
            CL_MSG_STREAM_INTERNAL(cl_error, "error: "
                    "unable to create file '" << args << "'");
            errc::errc_t ec = static_cast<errc::errc_t>(errno);
            system_error err(errc::make_error_code(ec));
            CL_MSG_STREAM_INTERNAL(cl_note, "note: "
                    "got system error '" << err.what() << "'");
            return 0;
        }
    }

    return new ClPrettyPrint(fd, openFile, showTypes);
}
