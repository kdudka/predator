// TODO: move to config.h
#define UNIFY_LABELS            1
#define UNIFY_LABELS_SCOPE      CL_SCOPE_GLOBAL

// scope for UNIFY_REGS is always CL_SCOPE_FUNCTION
#define UNIFY_REGS              1

#define ARG_SUBST               1

#include "cl_pp.hh"
#include "cl_private.hh"
#include "ssd.hh"

#if UNIFY_LABELS
#   include "cld_unilabel.hh"
#endif

#if UNIFY_REGS
#   include "cld_uniregs.hh"
#endif

#if ARG_SUBST
#   include "cld_argsub.hh"
#endif

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
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn_jmp(
            int                     line,
            const char              *label);

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false);

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src);

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src);

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2);

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc);

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src);

        virtual void insn_call_close();

    private:
        typedef boost::iostreams::file_descriptor_sink  TSink;
        typedef boost::iostreams::stream<TSink>         TStream;

        TSink           sink_;
        TStream         out_;
        std::string     file_;
        int             line_;
        std::string     fnc_;
        bool            printingArgDecls_;

    private:
        bool closeArgDeclsIfNeeded();
        void printNestedVar(struct cl_operand *);
        void printOperand(struct cl_operand *);
        void printAssignmentLhs(struct cl_operand *);
};

using namespace ssd;

// /////////////////////////////////////////////////////////////////////////////
// ClPrettyPrint implementation
ClPrettyPrint::ClPrettyPrint(int fd_out):
    sink_(fd_out),
    out_(sink_),
    line_(-1),
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
    file_ = file_name;
}

void ClPrettyPrint::file_close()
{
    out_ << std::endl;
}

void ClPrettyPrint::fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
{
    fnc_ = fnc_name;
    line_ = line;
    SSD_COLORIZE(out_, C_LIGHT_BLUE) << fnc_name;
    SSD_COLORIZE(out_, C_LIGHT_RED) << "(";
    printingArgDecls_ = true;
}

void ClPrettyPrint::fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
{
    // TODO: sort arguments if not already
    if (1 < arg_pos)
        out_ << ", ";
    SSD_COLORIZE(out_, C_LIGHT_GREEN) << "%arg" << arg_pos;
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
        CL_MSG_STREAM(cl_warn, file_ << ":"
                << line_ << ": warning: "
                << "function '" << fnc_
                << "' has no basic blocks");

    out_ << std::endl;
}

void ClPrettyPrint::bb_open(
            const char              *bb_name)
{
    if (this->closeArgDeclsIfNeeded())
        CL_MSG_STREAM(cl_warn, file_ << ":"
                << line_ << ": warning: "
                << "omitted jump to entry in function '" << fnc_
                << "'");

    out_ << std::endl;
    out_ << "\t"
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, bb_name)
        << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << std::endl;
}

void ClPrettyPrint::insn_jmp(
            int                     line,
            const char              *label)
{
    line_ = line;
    this->closeArgDeclsIfNeeded();
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_YELLOW, "goto") << " "
        << SSD_INLINE_COLOR(C_LIGHT_CYAN, label)
        << std::endl;
}

void ClPrettyPrint::printNestedVar(struct cl_operand *op) {
    switch (op->type) {
        case CL_OPERAND_VAR:
            if (!op->name) {
                CL_MSG_STREAM(cl_error, file_ << ":" << line_ << ": error: "
                        << "anonymous variable");
                break;
            }
            out_ << SSD_INLINE_COLOR(C_LIGHT_BLUE, op->name);
            break;

        case CL_OPERAND_REG:
            SSD_COLORIZE(out_, C_LIGHT_BLUE) << "%r" << op->value.reg_id;
            break;

        case CL_OPERAND_ARG:
            SSD_COLORIZE(out_, C_LIGHT_GREEN) << "%arg" << op->value.arg_pos;
            break;

        default:
            CL_MSG_STREAM(cl_debug, __FILE__ << ":" << __LINE__ << ": "
                    << "internal error in " << __FUNCTION__
                    << " [internal location]");
            break;
    }
}

void ClPrettyPrint::printOperand(struct cl_operand *op) {
    if (!op) {
        CL_MSG_STREAM(cl_debug, __FILE__ << ":" << __LINE__ << ": debug: "
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

            if (op->deref) {
                if (op->offset)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ":") << op->offset;

                out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "]");
            }
            break;

        case CL_OPERAND_STRING:
            {
                const char *text = op->value.text;
                if (!text) {
                    CL_MSG_STREAM(cl_error, file_ << ":" << line_ << ": error: "
                            << "CL_OPERAND_STRING with no string");
                    break;
                }
                // FIXME: bad quting!!
                SSD_COLORIZE(out_, C_LIGHT_PURPLE) << text;
            }
            break;

        case CL_OPERAND_INT:
            {
                int num = op->value.num_int;
                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, "(");

                SSD_COLORIZE(out_, C_WHITE) << op->value.num_int;

                if (num < 0)
                    out_ << SSD_INLINE_COLOR(C_LIGHT_RED, ")");
            }
            break;
    }
}

void ClPrettyPrint::insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
{
    line_ = line;
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

void ClPrettyPrint::insn_ret(
            int                     line,
            struct cl_operand       *src)
{
    line_ = line;
    out_ << "\t\t"
        << SSD_INLINE_COLOR(C_LIGHT_GREEN, "ret");

    if (src && src->type != CL_OPERAND_VOID) {
        out_ << " ";
        this->printOperand(src);
    }

    out_ << std::endl;
}

void ClPrettyPrint::printAssignmentLhs(struct cl_operand *lhs) {
    if (!lhs || lhs->type == CL_OPERAND_VOID) {
        CL_MSG_STREAM(cl_debug, __FILE__ << ":" << __LINE__ << ": debug: "
                << "no lhs given to " << __FUNCTION__
                << " [internal location]");
        return;
    }

    this->printOperand(lhs);
    out_ << " "
        << SSD_INLINE_COLOR(C_YELLOW, ":=")
        << " ";
}

void ClPrettyPrint::insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
{
    line_ = line;
    out_ << "\t\t";
    this->printAssignmentLhs(dst);

    switch (type) {
        case CL_UNOP_ASSIGN:
            this->printOperand(src);
            break;
    }

    out_ << std::endl;
}

void ClPrettyPrint::insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
{
    line_ = line;
    out_ << "\t\t";
    this->printAssignmentLhs(dst);

    switch (type) {
        case CL_BINOP_ADD:
            this->printOperand(src1);
            out_ << " "
                << SSD_INLINE_COLOR(C_YELLOW, "+")
                << " ";
            this->printOperand(src2);
            break;
    }

    out_ << std::endl;
}

void ClPrettyPrint::insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
{
    line_ = line;
    out_ << "\t\t";
    if (dst && dst->type != CL_OPERAND_VOID)
        this->printAssignmentLhs(dst);
    this->printOperand(fnc);
    out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, "(");
}

void ClPrettyPrint::insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
{
    // TODO: sort arguments if not already
    if (1 < arg_pos)
        out_ << ", ";
    this->printOperand(arg_src);
}

void ClPrettyPrint::insn_call_close()
{
    out_ << SSD_INLINE_COLOR(C_LIGHT_GREEN, ")")
        << std::endl;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_pp.hh for more details
ICodeListener* createClPrettyPrint(int fd_out) {
    ICodeListener *cl = new ClPrettyPrint(fd_out);

#if UNIFY_LABELS
    cl = createCldUniLabel(cl, UNIFY_LABELS_SCOPE);
#endif

#if UNIFY_REGS
    cl = createCldUniRegs(cl);
#endif

#if ARG_SUBST
    cl = createCldArgSubst(cl);
#endif

    return cl;
}
