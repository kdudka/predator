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
#include "relocator.hh"

class CldUniVars: public CldOpTransBase {
    public:
        CldUniVars(ICodeListener *slave);

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            lcReloc_.reset();
            CldOpTransBase::fnc_open(fnc);
        }

    private:
        Relocator<int, int> glReloc_;
        Relocator<int, int> lcReloc_;

    protected:
        virtual void modifyOperand(struct cl_operand *);
};

CldUniVars::CldUniVars(ICodeListener *slave):
    CldOpTransBase(slave),
    glReloc_(1),
    lcReloc_(1)
{
}

void CldUniVars::modifyOperand(struct cl_operand *op) {
    if (CL_OPERAND_VAR != op->code)
        return;

    int &id = op->data.var.id;

    enum cl_scope_e scope = op->scope;
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            id = glReloc_.lookup(id);
            break;

        case CL_SCOPE_FUNCTION:
        case CL_SCOPE_BB:
            id = lcReloc_.lookup(id);
            break;
    }
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldUniVars(ICodeListener *slave) {
    return new CldUniVars(slave);
}
