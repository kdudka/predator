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
#include "clf_unswitch.hh"

#include "cl_filter.hh"
#include "clf_unilabel.hh"

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <boost/foreach.hpp>

#define NULLIFY(what) \
    memset(&(what), 0, sizeof (what))

int getCaseVal(const struct cl_operand *op)
{
    CL_BREAK_IF(!op || !op->type);

    enum cl_type_e code = op->type->code;
    switch (code) {
        case CL_TYPE_ENUM:
        case CL_TYPE_INT:
            break;

        default:
            CL_TRAP;
    }

    return op->data.cst.data.cst_int.value;
}

class ClfUnfoldSwitch: public ClFilterBase {
    public:
        ClfUnfoldSwitch(ICodeListener *slave):
            ClFilterBase(slave),
            casePerSwitchCnt_(0),
            switchCnt_(0)
        {
        }

        virtual ~ClfUnfoldSwitch();

        virtual void insn_switch_open(
            const struct cl_loc      *,
            const struct cl_operand *src)
        {
            this->cloneSwitchSrc(src);
        }

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
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

            const int lo = getCaseVal(val_lo);
            const int hi = getCaseVal(val_hi);
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
        struct cl_loc       loc_;
        std::string         defLabel_;
        struct cl_loc       defLoc_;

        std::vector<struct cl_var *>    ptrs_;

    private:
        void cloneSwitchSrc(const struct cl_operand *);
        void freeClonedSwitchSrc();
        struct cl_var* acquireClVar();
        void emitCase(int, struct cl_type *, const char *);
        void emitDefault();
};

using std::string;

ClfUnfoldSwitch::~ClfUnfoldSwitch()
{
    BOOST_FOREACH(struct cl_var *clv, ptrs_) {
        delete clv;
    }
}

// FIXME: duplicated code from clf_uniregs.cc
// TODO: implement shared module providing this
void ClfUnfoldSwitch::cloneSwitchSrc(const struct cl_operand *op)
{
    CL_BREAK_IF(!op);

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

// FIXME: duplicated code from clf_uniregs.cc
// TODO: implement shared module providing this
void ClfUnfoldSwitch::freeClonedSwitchSrc()
{
    struct cl_accessor *ac = src_.accessor;
    while (ac) {
        struct cl_accessor *next = ac->next;
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            delete ac->data.array.index;

        delete ac;
        ac = next;
    }
}

struct cl_var* ClfUnfoldSwitch::acquireClVar()
{
    struct cl_var *clv = new struct cl_var;
    memset(clv, 0, sizeof *clv);
    clv->uid = /* XXX */ 0x400000 + switchCnt_;
    clv->loc.line = -1;

    ptrs_.push_back(clv);
    return clv;
}

void ClfUnfoldSwitch::emitCase(int cst, struct cl_type *type, const char *label)
{
    static struct cl_type btype;
    btype.uid                       = /* FIXME */ 0x200000;
    btype.code                      = CL_TYPE_BOOL;
    btype.loc.file                  = 0;
    btype.loc.line                  = -1;
    btype.scope                     = CL_SCOPE_GLOBAL;
    btype.name                      = "<builtin_bool>";
    btype.size                      = /* FIXME */ sizeof(bool);

    struct cl_operand reg;
    NULLIFY(reg);
    reg.code                        = CL_OPERAND_VAR;
    reg.scope                       = CL_SCOPE_FUNCTION;
    reg.type                        = &btype;
    reg.data.var                    = this->acquireClVar();

    struct cl_operand val;
    NULLIFY(val);
    val.code                        = CL_OPERAND_CST;
    val.scope                       = CL_SCOPE_FUNCTION;
    val.type                        = type;
    val.data.cst.code               = CL_TYPE_INT;
    val.data.cst.data.cst_int.value = cst;

    struct cl_insn cli;
    NULLIFY(cli);
    cli.code                        = CL_INSN_BINOP;
    cli.loc                         = loc_;
    cli.data.insn_binop.code        = CL_BINOP_EQ;
    cli.data.insn_binop.dst         = &reg;
    cli.data.insn_binop.src1        = &src_;
    cli.data.insn_binop.src2        = &val;
    ClFilterBase::insn(&cli);

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
    ClFilterBase::insn(&cli);

    ClFilterBase::bb_open(aux_label);
}

void ClfUnfoldSwitch::emitDefault()
{
    if (defLabel_.empty())
        CL_TRAP;

    struct cl_insn cli;
    cli.code                = CL_INSN_JMP;
    cli.loc                 = defLoc_;
    cli.data.insn_jmp.label = defLabel_.c_str();
    ClFilterBase::insn(&cli);

    defLabel_.clear();
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see clf_unswitch.hh for more details
ICodeListener* createClfUnfoldSwitch(ICodeListener *slave)
{
    return createClfUniLabel(new ClfUnfoldSwitch(
            createClfUniLabel(slave, CL_SCOPE_GLOBAL)),
            CL_SCOPE_GLOBAL);
}
