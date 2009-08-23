#include "cl_pp.hh"
#include "cl_private.hh"
#include "cld_intchk.hh"
#include "ssd.hh"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>


class ClPrettyPrint: public ICodeListener {
    public:
        ClPrettyPrint(int fd_out);
        virtual ~ClPrettyPrint();

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

        TSink                   sink_;
        TStream                 out_;
        Location                loc_;
        std::string             fnc_;
        bool                    printingArgDecls_;

    private:
        bool closeArgDeclsIfNeeded();
        void printNestedVar     (const struct cl_operand *);
        void printOperand       (const struct cl_operand *);
        void printAssignmentLhs (const struct cl_operand *);
        void printInsnJmp       (const struct cl_insn *);
        void printInsnCond      (const struct cl_insn *);
        void printInsnRet       (const struct cl_insn *);
        void printInsnAbort     (const struct cl_insn *);
        void printInsnUnop      (const struct cl_insn *);
        void printInsnBinop     (const struct cl_insn *);
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(int fd_out):
    sink_(fd_out),
    out_(sink_),
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
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
{
    fnc_ = fnc_name;
    loc_ = loc;
    switch (scope) {
        case CL_SCOPE_GLOBAL:
            break;

        case CL_SCOPE_STATIC:
            out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "static") << " ";
            break;

        default:
            CL_MSG_STREAM(cl_error, LocationWriter(loc) << "error: "
                    << "invalid scope for function: " << scope);
    }
    SSD_COLORIZE(out_, C_LIGHT_BLUE) << fnc_name;
    SSD_COLORIZE(out_, C_LIGHT_RED) << "(";
    printingArgDecls_ = true;
}

void ClPrettyPrint::fnc_arg_decl(
            int                     arg_id,
            const char              *arg_name)
{
    // TODO: sort arguments if not already
    if (1 < arg_id)
        out_ << ", ";
    SSD_COLORIZE(out_, C_LIGHT_GREEN) << "%arg" << arg_id;
    SSD_COLORIZE(out_, C_LIGHT_BLUE) << ": " << arg_name;
}

bool ClPrettyPrint::closeArgDeclsIfNeeded() {
    if (printingArgDecls_) {
        printingArgDecls_ = false;
        out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "):")
            << std::endl;
        return true;
    }
    return false;
}

void ClPrettyPrint::fnc_close()
{
    if (this->closeArgDeclsIfNeeded())
        CL_MSG_STREAM(cl_warn, LocationWriter(loc_) << "warning: "
                << "function '" << fnc_
                << "' has no basic blocks");

    out_ << std::endl;
}

void ClPrettyPrint::bb_open(
            const char              *bb_name)
{
    if (this->closeArgDeclsIfNeeded())
        CL_MSG_STREAM(cl_warn, LocationWriter(loc_) << "warning: "
                << "omitted jump to entry in function '" << fnc_
                << "'");

    out_ << std::endl;
    out_ << "\t"
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, bb_name)
        << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << std::endl;
}

void ClPrettyPrint::printNestedVar(const struct cl_operand *op) {
    switch (op->type) {
        case CL_OPERAND_VAR:
            if (!op->data.var.name) {
                CL_MSG_STREAM(cl_error, LocationWriter(loc_) << "error: "
                        << "anonymous variable");
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, op->data.var.name);
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

void ClPrettyPrint::printOperand(const struct cl_operand *op) {
    if (!op) {
        CL_MSG_STREAM(cl_debug, LocationWriter(loc_) << "debug: "
                << "no operand given to " << __FUNCTION__
                << " [internal location]");
        return;
    }

    switch (op->type) {
        case CL_OPERAND_VOID:
            SSD_COLORIZE(out_, C_LIGHT_BLUE) << "CL_OPERAND_VOID";
            break;

        case CL_OPERAND_ARG:
        case CL_OPERAND_REG:
        case CL_OPERAND_VAR:
            if (op->deref)
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "[");

            this->printNestedVar(op);

            if (op->offset) {
                if (op->deref)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ":");
                else
                    out_ << ".";

                out_ << op->offset;
            }

            if (op->deref)
                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "]");

            break;

        case CL_OPERAND_STRING:
            {
                const char *text = op->data.lit_string.value;
                if (!text) {
                    CL_MSG_STREAM(cl_error, LocationWriter(loc_) << "error: "
                            << "CL_OPERAND_STRING with no string");
                    break;
                }
                SSD_COLORIZE(out_, C_LIGHT_PURPLE) << "\"" << text << "\"";
            }
            break;

        case CL_OPERAND_INT:
            {
                int num = op->data.lit_int.value;
                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");

                SSD_COLORIZE(out_, C_WHITE) << op->data.lit_int.value;

                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")");
            }
            break;
    }
}

void ClPrettyPrint::printAssignmentLhs(const struct cl_operand *lhs) {
    if (!lhs || lhs->type == CL_OPERAND_VOID) {
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

void ClPrettyPrint::printInsnJmp(const struct cl_insn *cli) {
    const char *label = cli->data.insn_jmp.label;
    this->closeArgDeclsIfNeeded();
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
        << SSD_INLINE_COLOR(C_LIGHT_GREEN, "ret");

    if (src && src->type != CL_OPERAND_VOID) {
        out_ << " ";
        this->printOperand(src);
    }

    out_ << std::endl;
}

void ClPrettyPrint::printInsnAbort(const struct cl_insn *cli) {
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_GREEN, "abort")
        << std::endl;
}

void ClPrettyPrint::printInsnUnop(const struct cl_insn *cli) {
    const enum cl_unop_e type       = cli->data.insn_unop.type;
    const struct cl_operand *dst    = cli->data.insn_unop.dst;
    const struct cl_operand *src    = cli->data.insn_unop.src;

    out_ << "\t\t";
    this->printAssignmentLhs(dst);

    switch (type) {
        case CL_UNOP_ASSIGN:
            this->printOperand(src);
            break;
    }

    out_ << std::endl;
}

void ClPrettyPrint::printInsnBinop(const struct cl_insn *cli) {
    const enum cl_binop_e type      = cli->data.insn_binop.type;
    const struct cl_operand *dst    = cli->data.insn_binop.dst;
    const struct cl_operand *src1   = cli->data.insn_binop.src1;
    const struct cl_operand *src2   = cli->data.insn_binop.src2;

    out_ << "\t\t";
    this->printAssignmentLhs(dst);
    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");
    this->printOperand(src1);
    out_ << " ";

    switch (type) {
        case CL_BINOP_ADD:
            SSD_COLORIZE(out_, C_YELLOW) << "+";
            break;

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
    }

    out_ << " ";
    this->printOperand(src2);
    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")") << std::endl;
}

void ClPrettyPrint::insn(
            const struct cl_insn    *cli)
{
    loc_ = &cli->loc;
    switch (cli->type) {
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
    if (dst && dst->type != CL_OPERAND_VOID)
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

void ClPrettyPrint::insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    loc_ = loc;
    if (CL_OPERAND_VOID == val_lo->type
            && CL_OPERAND_VOID == val_hi->type)
    {
        out_ << "\t\t\t"
            << SSD_INLINE_COLOR(C_YELLOW, "default") << ":";
    } else if (CL_OPERAND_INT == val_lo->type
            && CL_OPERAND_INT == val_hi->type)
    {
        const int lo = val_lo->data.lit_int.value;
        const int hi = val_hi->data.lit_int.value;
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
ICodeListener* createClPrettyPrint() {
    return new ClPrettyPrint(STDOUT_FILENO);
}
