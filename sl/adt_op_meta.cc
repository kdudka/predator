/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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
#include "adt_op_meta.hh"

#include "fixed_point.hh"
#include "symtrace.hh"

using FixedPoint::TObjectMapper;

namespace AdtOp {

template <EDirection dir>
bool projectMetaOperation(MetaOperation &mo, TObjectMapper idMap)
{
    // TODO
    return false;
}

struct DiffHeapsCtx {
    TMetaOpSet                     &opSet;
    const SymHeap                  &sh1;
    const SymHeap                  &sh2;
    Trace::TIdMapper                idMap;

    DiffHeapsCtx(TMetaOpSet *pOpSet, const SymHeap &sh1_, const SymHeap &sh2_):
        opSet(*pOpSet),
        sh1(sh1_),
        sh2(sh2_)
    {
        resolveIdMapping(&idMap, sh1.traceNode(), sh2.traceNode());
        CL_BREAK_IF(!idMap.isTrivial());
    }
};

bool diffHeaps(TMetaOpSet *pDst, const SymHeap &sh1, const SymHeap &sh2)
{
    DiffHeapsCtx ctx(pDst, sh1, sh2);

    // TODO
    CL_BREAK_IF("please implement");
    return false;
}

} // namespace AdtOp
