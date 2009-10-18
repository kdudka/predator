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

#include "cld_uniregs.hh"
#include "cld_optrans.hh"

#include <map>

class CldUniRegs: public CldOpTransBase {
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

    private:
        typedef std::map<int, int> TMap;

        TMap            map_;
        int             last_;

    private:
        void reset();
        int regLookup(int);
        void relocReg(struct cl_operand *op);

    protected:
        virtual void modifyOperand(struct cl_operand *);
};

CldUniRegs::CldUniRegs(ICodeListener *slave):
    CldOpTransBase(slave),
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

void CldUniRegs::modifyOperand(struct cl_operand *op) {
    if (CL_OPERAND_REG != op->code)
        return;

    op->data.reg.id = this->regLookup(op->data.reg.id);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniRegs(ICodeListener *slave) {
    return new CldUniRegs(slave);
}
