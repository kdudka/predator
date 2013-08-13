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

bool diffSetField(DiffHeapsCtx &ctx, const TObjId obj1, const FldHandle &fld2)
{
    // TODO
    (void) ctx;
    (void) obj1;
    (void) fld2;
    CL_BREAK_IF("please implement");
    return false;
}

bool diffUnsetField(DiffHeapsCtx &ctx, const FldHandle &fld1, const TObjId obj2)
{
    // TODO
    (void) ctx;
    (void) fld1;
    (void) obj2;
    CL_BREAK_IF("please implement");
    return false;
}

bool diffFields(DiffHeapsCtx &ctx, const TObjId obj1, const TObjId obj2)
{
    if (ctx.sh1.isValid(obj1)) {
        const EObjKind kind1 = ctx.sh1.objKind(obj1);
        const EObjKind kind2 = ctx.sh2.objKind(obj2);
        if (kind1 != kind2) {
            CL_BREAK_IF("object kind mismatch in diffFields()");
            return false;
        }

        const TSizeRange size1 = ctx.sh1.objSize(obj1);
        const TSizeRange size2 = ctx.sh2.objSize(obj2);
        if (size1 != size2) {
            CL_BREAK_IF("object size mismatch in diffFields()");
            return false;
        }

        FldList fldList1;
        ctx.sh1.gatherLiveFields(fldList1, obj1);
        BOOST_FOREACH(const FldHandle &fld1, fldList1)
            if (!diffUnsetField(ctx, fld1, obj2))
                return false;
    }

    FldList fldList2;
    ctx.sh2.gatherLiveFields(fldList2, obj2);
    BOOST_FOREACH(const FldHandle &fld2, fldList2)
        if (!diffSetField(ctx, obj1, fld2))
            return false;

    // fields diffed successfully!
    return true;
}

bool diffHeaps(TMetaOpSet *pDst, const SymHeap &sh1, const SymHeap &sh2)
{
    DiffHeapsCtx ctx(pDst, sh1, sh2);

    TObjList objList2;
    ctx.sh2.gatherObjects(objList2);
    BOOST_FOREACH(const TObjId obj2, objList2) {
        TObjList objList1;
        ctx.idMap.query<D_RIGHT_TO_LEFT>(&objList1, obj2);
        if (1U < objList1.size()) {
            CL_BREAK_IF("diffHeaps() does not support ambiguous ID mapping");
            return false;
        }

        if (objList1.empty())
            objList1.push_back(OBJ_INVALID);

        const TObjId obj1 = objList1.front();
        if (!ctx.sh1.isValid(obj1)) {
            const MetaOperation moAlloc(MO_ALLOC, obj1);
            ctx.opSet.insert(moAlloc);
        }

        if (!diffFields(ctx, obj1, obj2))
            return false;
    }

    TObjList objList1;
    ctx.sh1.gatherObjects(objList1);
    BOOST_FOREACH(const TObjId obj1, objList1) {
        TObjList objList2;
        ctx.idMap.query<D_LEFT_TO_RIGHT>(&objList2, obj1);
        if (1U < objList2.size()) {
            CL_BREAK_IF("diffHeaps() does not support ambiguous ID mapping");
            return false;
        }

        if (objList2.empty())
            objList2.push_back(OBJ_INVALID);

        const TObjId obj2 = objList2.front();
        if (!ctx.sh2.isValid(obj2)) {
            const MetaOperation moFree(MO_FREE, obj2);
            ctx.opSet.insert(moFree);
        }
    }

    // heaps diffed successfully!
    return true;
}

} // namespace AdtOp
