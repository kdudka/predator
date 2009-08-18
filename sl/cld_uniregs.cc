#include "cld_uniregs.hh"
#include "cl_decorator.hh"

#include <map>

class CldUniRegs: public ClDecoratorBase {
    public:
        CldUniRegs(ICodeListener *slave);

        virtual void fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            this->reset();
            ClDecoratorBase::fnc_open(loc, fnc_name, scope);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            struct cl_insn local_cli = *cli;

            switch (cli->type) {
                case CL_INSN_COND: {
                        struct cl_operand src = *(cli->data.insn_cond.src);
                        this->relocReg(&src);
                        local_cli.data.insn_cond.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_RET: {
                        struct cl_operand src = *(cli->data.insn_ret.src);
                        this->relocReg(&src);
                        local_cli.data.insn_ret.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_UNOP: {
                        struct cl_operand dst = *(cli->data.insn_unop.dst);
                        struct cl_operand src = *(cli->data.insn_unop.src);

                        this->relocReg(&dst);
                        this->relocReg(&src);

                        local_cli.data.insn_unop.dst = &dst;
                        local_cli.data.insn_unop.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_BINOP: {
                        struct cl_operand dst = *(cli->data.insn_binop.dst);
                        struct cl_operand src1 = *(cli->data.insn_binop.src1);
                        struct cl_operand src2 = *(cli->data.insn_binop.src2);

                        this->relocReg(&dst);
                        this->relocReg(&src1);
                        this->relocReg(&src2);

                        local_cli.data.insn_binop.dst = &dst;
                        local_cli.data.insn_binop.src1 = &src1;
                        local_cli.data.insn_binop.src2 = &src2;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                default:
                    ClDecoratorBase::insn(cli);
                    break;
            }
        }

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            struct cl_operand local_dst = *dst;
            struct cl_operand local_fnc = *fnc;

            this->relocReg(&local_dst);
            this->relocReg(&local_fnc);

            ClDecoratorBase::insn_call_open(loc, &local_dst, &local_fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            struct cl_operand local_src = *arg_src;
            this->relocReg(&local_src);
            ClDecoratorBase::insn_call_arg(arg_id, &local_src);
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

    op->data.reg.id = this->regLookup(op->data.reg.id);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniRegs(ICodeListener *slave) {
    return new CldUniRegs(slave);
}
