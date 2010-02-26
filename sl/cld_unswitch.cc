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

#include "config.h"
#include "cld_unswitch.hh"

#include "cl_decorator.hh"
#include "cld_unilabel.hh"

#include <sstream>
#include <string>

class CldUnfoldSwitch: public ClDecoratorBase {
    public:
        CldUnfoldSwitch(ICodeListener *slave):
            ClDecoratorBase(slave),
            casePerSwitchCnt_(0),
            switchCnt_(0)
        {
        }

        virtual void insn_switch_open(
            const struct cl_location *,
            const struct cl_operand *src)
        {
            this->cloneSwitchSrc(src);
        }

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            if (val_lo->code == CL_OPERAND_VOID
                    && val_hi->code == CL_OPERAND_VOID)
            {
                defLabel_ = label;
                defLoc_   = *loc;
                return;
            }

            loc_ = *loc;

            const int lo = CldUnfoldSwitch::getCaseVal(val_lo);
            const int hi = CldUnfoldSwitch::getCaseVal(val_hi);
            for (int i = lo; i <= hi; ++i)
                this->emitCase(i, val_lo->type, label);
        }

        virtual void insn_switch_close() {
            this->emitDefault();
            this->freeClonedSwitchSrc();
            casePerSwitchCnt_ = 0;
            ++switchCnt_;
        }

    private:
        int                 casePerSwitchCnt_;
        int                 switchCnt_;
        struct cl_operand   src_;
        struct cl_location  loc_;
        std::string         defLabel_;
        struct cl_location  defLoc_;

    private:
        static int getCaseVal(const struct cl_operand *);
        void cloneSwitchSrc(const struct cl_operand *);
        void freeClonedSwitchSrc();
        void emitCase(int, struct cl_type *, const char *);
        void emitDefault();
};

using std::string;

int CldUnfoldSwitch::getCaseVal(const struct cl_operand *op) {
    if (!op || !op->type)
        TRAP;

    enum cl_type_e code = op->type->code;
    switch (code) {
        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
            break;

        default:
            TRAP;
    }

    return op->data.cst.data.cst_int.value;
}

// FIXME: duplicated code from cld_uniregs.cc
// TODO: implement shared module providing this
void CldUnfoldSwitch::cloneSwitchSrc(const struct cl_operand *op) {
    if (!op)
        TRAP;

    src_ = *op;

    // Traverse cl_accessor chain recursively and make a deep copy of it.
    // For each CL_ACCESSOR_DEREF_ARRAY clone its index operand as well.
    struct cl_accessor **ac = &src_.accessor;
    for (; *ac; ac = &((*ac)->next)) {
        *ac = new struct cl_accessor(**ac);
        if ((*ac)->code == CL_ACCESSOR_DEREF_ARRAY)
            (*ac)->data.array.index =
                new struct cl_operand(*(*ac)->data.array.index);
    }
}

// FIXME: duplicated code from cld_uniregs.cc
// TODO: implement shared module providing this
void CldUnfoldSwitch::freeClonedSwitchSrc() {
    struct cl_accessor *ac = src_.accessor;
    while (ac) {
        struct cl_accessor *next = ac->next;
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            delete ac->data.array.index;

        delete ac;
        ac = next;
    }
}

void CldUnfoldSwitch::emitCase(int cst, struct cl_type *type, const char *label)
{
    struct cl_type btype;
    btype.uid                       = /* FIXME */ -1;
    btype.code                      = CL_TYPE_BOOL;
    btype.loc.file                  = 0;
    btype.loc.line                  = -1;
    btype.scope                     = CL_SCOPE_GLOBAL;
    btype.name                      = "<builtin_bool>";
    btype.size                      = /* FIXME */ sizeof(bool);

    struct cl_operand reg;
    reg.code                        = CL_OPERAND_REG;
    reg.loc.file                    = 0;
    reg.loc.line                    = -1;
    reg.scope                       = CL_SCOPE_FUNCTION;
    reg.type                        = &btype;
    reg.accessor                    = 0;
    reg.data.reg.id                 = /* XXX */ 0x10000 + switchCnt_;

    struct cl_operand val;
    val.code                        = CL_OPERAND_CST;
    val.loc                         = loc_;
    val.scope                       = CL_SCOPE_BB;
    val.type                        = type;
    val.accessor                    = 0;
    val.data.cst.data.cst_int.value = cst;

    struct cl_insn cli;
    cli.code                        = CL_INSN_BINOP;
    cli.loc                         = loc_;
    cli.data.insn_binop.code        = CL_BINOP_EQ;
    cli.data.insn_binop.dst         = &reg;
    cli.data.insn_binop.src1        = &src_;
    cli.data.insn_binop.src2        = &val;
    ClDecoratorBase::insn(&cli);

    std::ostringstream str;
    str << "switch_" << switchCnt_
        << "__case_" << (++casePerSwitchCnt_)
        << std::flush;

    string auxLabel(str.str());
    const char *aux_label = auxLabel.c_str();

    cli.code                        = CL_INSN_COND;
    cli.data.insn_cond.src          = &reg;
    cli.data.insn_cond.then_label   = label;
    cli.data.insn_cond.else_label   = aux_label;
    ClDecoratorBase::insn(&cli);

    ClDecoratorBase::bb_open(aux_label);
}

void CldUnfoldSwitch::emitDefault() {
    if (defLabel_.empty())
        TRAP;

    struct cl_insn cli;
    cli.code                = CL_INSN_JMP;
    cli.loc                 = defLoc_;
    cli.data.insn_jmp.label = defLabel_.c_str();
    ClDecoratorBase::insn(&cli);

    defLabel_.clear();
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unswitch.hh for more details
ICodeListener* createCldUnfoldSwitch(ICodeListener *slave) {
    return createCldUniLabel(new CldUnfoldSwitch(
            createCldUniLabel(slave, CL_SCOPE_GLOBAL)),
            CL_SCOPE_GLOBAL);
}
