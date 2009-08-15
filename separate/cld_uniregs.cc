#include "cld_uniregs.hh"
#include "cl_decorator.hh"

#include <map>

class CldUniRegs: public ClDecoratorBase {
    public:
        CldUniRegs(ICodeListener *slave);

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            this->reset();
            ClDecoratorBase::fnc_open(line, fnc_name, scope);
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            this->relocReg(src);
            ClDecoratorBase::insn_cond(line, src, label_true, label_false);
        }

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src)
        {
            this->relocReg(src);
            ClDecoratorBase::insn_ret(line, src);
        }

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            this->relocReg(dst);
            this->relocReg(src);
            ClDecoratorBase::insn_unop(line, type, dst, src);
        }

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            this->relocReg(dst);
            this->relocReg(src1);
            this->relocReg(src2);
            ClDecoratorBase::insn_binop(line, type, dst, src1, src2);
        }

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            this->relocReg(dst);
            this->relocReg(fnc);
            ClDecoratorBase::insn_call_open(line, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
        {
            this->relocReg(arg_src);
            ClDecoratorBase::insn_call_arg(arg_pos, arg_src);
        }

    private:
        typedef std::map<int, int> TMap;

        TMap            map_;
        int             last_;

    private:
        void reset();
        int regLookup(int);
        void relocReg(struct cl_operand *op);
};

CldUniRegs::CldUniRegs(ICodeListener *slave):
    ClDecoratorBase(slave),
    last_(0)
{
}

void CldUniRegs::reset() {
    map_.clear();
    last_ = 0;
}

int CldUniRegs::regLookup(int reg) {
    TMap::iterator i = map_.find(reg);
    if (map_.end() != i)
        return i->second;

    map_[reg] = ++last_;
    return last_;
}

void CldUniRegs::relocReg(struct cl_operand *op) {
    if (CL_OPERAND_REG != op->type)
        return;

    op->value.reg_id = this->regLookup(op->value.reg_id);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniRegs(ICodeListener *slave) {
    return new CldUniRegs(slave);
}
