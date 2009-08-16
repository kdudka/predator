#include "cld_argsub.hh"
#include "cl_decorator.hh"

#include <map>
#include <string>

class CldArgSubst: public ClDecoratorBase {
    public:
        CldArgSubst(ICodeListener *slave);

        virtual void file_open(const char *file_name) {
            file_ = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            line_ = line;
            fnc_ = fnc_name;
            this->reset();
            ClDecoratorBase::fnc_open(line, fnc_name, scope);
        }

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
        {
            this->regArg(arg_pos, arg_name);
            ClDecoratorBase::fnc_arg_decl(arg_pos, arg_name);
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            struct cl_operand local_src = *src;
            this->substArg(&local_src);
            ClDecoratorBase::insn_cond(line, &local_src, label_true,
                                       label_false);
        }

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src)
        {
            struct cl_operand local_src = *src;
            this->substArg(&local_src);
            ClDecoratorBase::insn_ret(line, &local_src);
        }

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            struct cl_operand local_dst = *dst;
            struct cl_operand local_src = *src;
            this->substArg(&local_dst);
            this->substArg(&local_src);
            ClDecoratorBase::insn_unop(line, type, &local_dst, &local_src);
        }

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            struct cl_operand local_dst = *dst;
            struct cl_operand local_src1 = *src1;
            struct cl_operand local_src2 = *src2;
            this->substArg(&local_dst);
            this->substArg(&local_src1);
            this->substArg(&local_src2);
            ClDecoratorBase::insn_binop(line, type, &local_dst, &local_src1,
                                        &local_src2);
        }

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            struct cl_operand local_dst = *dst;
            struct cl_operand local_fnc = *fnc;
            this->substArg(&local_dst);
            this->substArg(&local_fnc);
            ClDecoratorBase::insn_call_open(line, &local_dst, &local_fnc);
        }

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
        {
            struct cl_operand local_arg_src = *arg_src;
            this->substArg(&local_arg_src);
            ClDecoratorBase::insn_call_arg(arg_pos, &local_arg_src);
        }

    private:
        // we use map because some arg_pos positions may be omitted
        typedef std::map<int, std::string> TMap;

        std::string     file_;
        int             line_;
        std::string     fnc_;

        TMap            map_;
        int             last_;

    private:
        void reset();
        void regArg(int arg_pos, const char *arg_name);

        // we do not return reference to string because we return NULL
        // when arg position is not found
        const char* argLookup(int);

        void substArg(struct cl_operand *op);
};

CldArgSubst::CldArgSubst(ICodeListener *slave):
    ClDecoratorBase(slave),
    line_(-1),
    last_(0)
{
}

void CldArgSubst::reset() {
    map_.clear();
    last_ = 0;
}

void CldArgSubst::regArg(int arg_pos, const char *arg_name) {
    TMap::iterator i = map_.find(arg_pos);
    if (map_.end() != i) {
        CL_MSG_STREAM(cl_error, file_ << ":" << line_ << ": error: "
                << "argument #" << arg_pos
                << " of function '" << fnc_ << "'"
                << " is already declared as '"
                << i->second << "'");
        return;
    }

    map_[arg_pos] = arg_name;
}

const char* CldArgSubst::argLookup(int arg) {
    TMap::iterator i = map_.find(arg);
    if (map_.end() == i) {
        CL_MSG_STREAM(cl_error, file_ << ":" << line_ << ": error: "
                << "argument #" << arg
                << " of function '" << fnc_ << "'"
                << " was not declared");
        return NULL;
    }

    return i->second.c_str();
}

void CldArgSubst::substArg(struct cl_operand *op) {
    if (CL_OPERAND_ARG != op->type)
        return;

    op->type = CL_OPERAND_VAR;
    op->name = this->argLookup(op->value.arg_pos);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldArgSubst(ICodeListener *slave) {
    return new CldArgSubst(slave);
}
