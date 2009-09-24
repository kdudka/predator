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

            switch (cli->code) {
                case CL_INSN_COND: {
                        struct cl_operand src = *(cli->data.insn_cond.src);
                        CldUniRegs::cloneAccessor(&src);

                        this->relocReg(&src);
                        local_cli.data.insn_cond.src = &src;

                        ClDecoratorBase::insn(&local_cli);
                        CldUniRegs::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_RET: {
                        struct cl_operand src = *(cli->data.insn_ret.src);
                        CldUniRegs::cloneAccessor(&src);

                        this->relocReg(&src);
                        local_cli.data.insn_ret.src = &src;

                        ClDecoratorBase::insn(&local_cli);
                        CldUniRegs::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_UNOP: {
                        struct cl_operand dst = *(cli->data.insn_unop.dst);
                        struct cl_operand src = *(cli->data.insn_unop.src);

                        CldUniRegs::cloneAccessor(&dst);
                        CldUniRegs::cloneAccessor(&src);

                        this->relocReg(&dst);
                        this->relocReg(&src);

                        local_cli.data.insn_unop.dst = &dst;
                        local_cli.data.insn_unop.src = &src;
                        ClDecoratorBase::insn(&local_cli);

                        CldUniRegs::freeClonedAccessor(&dst);
                        CldUniRegs::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_BINOP: {
                        struct cl_operand dst = *(cli->data.insn_binop.dst);
                        struct cl_operand src1 = *(cli->data.insn_binop.src1);
                        struct cl_operand src2 = *(cli->data.insn_binop.src2);

                        CldUniRegs::cloneAccessor(&dst);
                        CldUniRegs::cloneAccessor(&src1);
                        CldUniRegs::cloneAccessor(&src2);

                        this->relocReg(&dst);
                        this->relocReg(&src1);
                        this->relocReg(&src2);

                        local_cli.data.insn_binop.dst = &dst;
                        local_cli.data.insn_binop.src1 = &src1;
                        local_cli.data.insn_binop.src2 = &src2;
                        ClDecoratorBase::insn(&local_cli);

                        CldUniRegs::freeClonedAccessor(&dst);
                        CldUniRegs::freeClonedAccessor(&src1);
                        CldUniRegs::freeClonedAccessor(&src2);
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

            CldUniRegs::cloneAccessor(&local_dst);
            CldUniRegs::cloneAccessor(&local_fnc);

            this->relocReg(&local_dst);
            this->relocReg(&local_fnc);

            ClDecoratorBase::insn_call_open(loc, &local_dst, &local_fnc);

            CldUniRegs::freeClonedAccessor(&local_dst);
            CldUniRegs::freeClonedAccessor(&local_fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            struct cl_operand local_src = *arg_src;
            CldUniRegs::cloneAccessor(&local_src);

            this->relocReg(&local_src);

            ClDecoratorBase::insn_call_arg(arg_id, &local_src);
            CldUniRegs::freeClonedAccessor(&local_src);
        }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            struct cl_operand local_src = *src;
            CldUniRegs::cloneAccessor(&local_src);

            this->relocReg(&local_src);

            ClDecoratorBase::insn_switch_open(loc, &local_src);
            CldUniRegs::freeClonedAccessor(&local_src);
        }

    private:
        typedef std::map<int, int> TMap;

        TMap            map_;
        int             last_;

    private:
        static void cloneAccessor(struct cl_operand *);
        static void freeClonedAccessor(struct cl_operand *);

    private:
        void reset();
        int regLookup(int);
        void relocReg(struct cl_operand *op);
};

void CldUniRegs::cloneAccessor(struct cl_operand *op) {
    if (!op)
        TRAP;

    // Traverse cl_accessor chain recursively and make a deep copy of it.
    // For each CL_ACCESSOR_DEREF_ARRAY clone its index operand as well.
    struct cl_accessor **ac = &op->accessor;
    for (; *ac; ac = &((*ac)->next)) {
        *ac = new struct cl_accessor(**ac);
        if ((*ac)->code == CL_ACCESSOR_DEREF_ARRAY)
            (*ac)->data.array.index =
                new struct cl_operand(*(*ac)->data.array.index);
    }
}

// free all memory allocated by CldUniRegs::cloneAccessor
// it is not inverse operation to cloneAccessor() since the cl_accessor chain is
// deleted completely without any chance to restore it afterwards
void CldUniRegs::freeClonedAccessor(struct cl_operand *op) {
    if (!op)
        TRAP;

    struct cl_accessor *ac = op->accessor;
    while (ac) {
        struct cl_accessor *next = ac->next;
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            delete ac->data.array.index;

        delete ac;
        ac = next;
    }

    op->accessor = 0;
}

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
    if (CL_OPERAND_VOID == op->code)
        return;

    struct cl_accessor *ac = op->accessor;
    for (; ac; ac = ac->next) {
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            // FIXME: unguarded recursion
            this->relocReg(ac->data.array.index);
    }

    if (CL_OPERAND_REG != op->code)
        return;

    op->data.reg.id = this->regLookup(op->data.reg.id);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniRegs(ICodeListener *slave) {
    return new CldUniRegs(slave);
}
