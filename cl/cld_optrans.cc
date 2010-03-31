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

#include "config.h"
#include "cld_optrans.hh"

void CldOpTransBase::cloneAccessor(struct cl_operand *op) {
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

// free all memory allocated by CldOpTransBase::cloneAccessor
// it is not inverse operation to cloneAccessor() since the cl_accessor chain is
// deleted completely without any chance to restore it afterwards
void CldOpTransBase::freeClonedAccessor(struct cl_operand *op) {
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

CldOpTransBase::CldOpTransBase(ICodeListener *slave):
    ClDecoratorBase(slave)
{
}

void CldOpTransBase::traverseOperand(struct cl_operand *op) {
    if (CL_OPERAND_VOID == op->code)
        return;

    struct cl_accessor *ac = op->accessor;
    for (; ac; ac = ac->next) {
        if (ac->code == CL_ACCESSOR_DEREF_ARRAY)
            // FIXME: unguarded recursion
            this->traverseOperand(ac->data.array.index);
    }

    this->modifyOperand(op);
}
