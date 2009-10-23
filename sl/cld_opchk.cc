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

#include "cld_opchk.hh"

// /////////////////////////////////////////////////////////////////////////////
// CldOpCheckerBase implementation
CldOpCheckerBase::CldOpCheckerBase(ICodeListener *slave):
    ClDecoratorBase(slave)
{
}

void CldOpCheckerBase::handleArrayIdx(const struct cl_operand *op) {
    if (CL_OPERAND_VOID == op->code)
        return;

    struct cl_accessor *ac = op->accessor;
    for (; ac; ac = ac->next) {
        if (ac->code != CL_ACCESSOR_DEREF_ARRAY)
            continue;

        struct cl_operand *idx = ac->data.array.index;
        if (CL_OPERAND_REG == idx->code)
            this->checkSrcOperand(idx);
    }
}

void CldOpCheckerBase::handleSrc(const struct cl_operand *op) {
    this->checkSrcOperand(op);
    this->handleArrayIdx(op);
}

void CldOpCheckerBase::handleDstSrc(const struct cl_operand *op) {
    if (CL_OPERAND_VOID == op->code)
        return;

    if (op->accessor && op->accessor->code == CL_ACCESSOR_DEREF)
        this->checkSrcOperand(op);
    else
        this->checkDstOperand(op);

    this->handleArrayIdx(op);
}
