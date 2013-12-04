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

#include "adt_op_match.hh"          // for selectMappedObjByTs()
#include "fixed_point.hh"
#include "symtrace.hh"

#include <cl/cl_msg.hh>

using FixedPoint::TObjectMapper;

namespace AdtOp {

bool debuggingHeapDiff;

#define MO_DEBUG(msg) do {              \
    if (!AdtOp::debuggingHeapDiff)      \
        break;                          \
    CL_DEBUG(msg);                      \
} while (0)

struct DiffHeapsCtx {
    TMetaOpSet                     &opSet;
    SymHeap                        &sh1;
    SymHeap                        &sh2;
    Trace::TIdMapper                idMap;

    DiffHeapsCtx(TMetaOpSet *pOpSet, SymHeap &sh1_, SymHeap &sh2_):
        opSet(*pOpSet),
        sh1(sh1_),
        sh2(sh2_)
    {
        resolveIdMapping(&idMap, sh1.traceNode(), sh2.traceNode());
        if (!idMap.isTrivial())
            MO_DEBUG("diffHeaps() operates on non-trivial map of object IDs");
    }
};

bool selectTargetObj(
        TObjList                   *pObjList1,
        DiffHeapsCtx               &ctx,
        const TObjId                obj2,
        const ETargetSpecifier      ts2)
{
    const unsigned cnt = pObjList1->size();
    switch (cnt) {
        case 0:
            // got no objects to select from
            return false;

        case 1:
            // unambiguous ID mapping, we are done
            return true;

        default:
            break;
    }

    const EObjKind kind2 = ctx.sh2.objKind(obj2);
    if (kind2 != OK_DLS)
        // we only support OK_DLS for now
        return false;

    const BindingOff bOff = ctx.sh2.segBinding(obj2);
    const TObjId obj1 = selectMappedObjByTs(ctx.sh1, bOff, *pObjList1, ts2);
    if (OBJ_INVALID == obj1) {
        MO_DEBUG("selectMappedObjByTs() failed to resolve ambiguous mapping");
        return false;
    }

    pObjList1->clear();
    pObjList1->push_back(obj1);
    return true;
}

bool diffSetField(DiffHeapsCtx &ctx, const TObjId obj1, const FldHandle &fld2)
{
    // resolve val2
    const TValId val2 = fld2.value();
    const TObjType clt = fld2.type();
    const TOffset off = fld2.offset();

    // check object mapping
    const TObjId obj2 = fld2.obj();
    if (obj1 != obj2) {
        MO_DEBUG("diffSetField() operates on non-trivially mapped objects"
                << ", obj1 = #" << obj1
                << ", obj2 = #" << obj2);
    }

    // resolve target
    const TObjId tgtObj2 = ctx.sh2.objByAddr(val2);
    const TOffset tgtOff2 = ctx.sh2.valOffset(val2);
    const ETargetSpecifier tgtTs2 = ctx.sh2.targetSpec(val2);

    const EValueTarget vt2 = ctx.sh2.valTarget(val2);
    switch (vt2) {
        case VT_OBJECT:
            break;

        case VT_UNKNOWN:
            return true;

        default:
            CL_BREAK_IF("diffSetField() does not support non-pointer fields");
            return false;
    }

    // check target object mapping
    TObjList tgtObjList1;
    ctx.idMap.query<D_RIGHT_TO_LEFT>(&tgtObjList1, tgtObj2);
    if (!selectTargetObj(&tgtObjList1, ctx, tgtObj2, tgtTs2)) {
        MO_DEBUG("selectTargetObj() failed to resolve ambiguous ID mapping");
        return false;
    }

    if (ctx.sh1.isValid(obj1)) {
        // resolve val1
        const FldHandle fld1(ctx.sh1, obj1, clt, off);
        const TValId val1 = fld1.value();
        if (val1 == val2)
            return true;

        const EValueTarget vt1 = ctx.sh1.valTarget(val1);
        switch (vt1) {
            case VT_UNKNOWN:
                break;

            case VT_OBJECT:
                if (ctx.sh1.objByAddr(val1) != /* XXX */ tgtObjList1.front())
                    break;
                if (ctx.sh1.valOffset(val1) != tgtOff2)
                    break;
                if (/* XXX */ TS_REGION != tgtTs2
                        && ctx.sh1.targetSpec(val1) != tgtTs2)
                    break;

                // nothing changed actually
                return true;

            default:
                CL_BREAK_IF("unhandled value target in diffSetField()");
                return false;
        }
    }

    // insert meta-operation
    const MetaOperation moSet(MO_SET, obj1, off, tgtObj2, tgtOff2, tgtTs2);
    ctx.opSet.insert(moSet);
    return true;
}

bool diffUnsetField(DiffHeapsCtx &ctx, const FldHandle &fld1, const TObjId obj2)
{
    // resolve val1
    const TValId val1 = fld1.value();
    const EValueTarget vt1 = ctx.sh1.valTarget(val1);
    switch (vt1) {
        case VT_OBJECT:
            break;

        case VT_UNKNOWN:
            return true;

        default:
            CL_BREAK_IF("diffUnsetField() does not support non-pointer fields");
            return false;
    }

    // resolve val2
    const TObjType clt = fld1.type();
    const TOffset off = fld1.offset();
    const FldHandle fld2(ctx.sh2, obj2, clt, off);
    const TValId val2 = fld2.value();
    const EValueTarget vt2 = ctx.sh2.valTarget(val2);
    if (VT_UNKNOWN != vt2)
        // this is NOT an "unset" operation
        return true;

    // check object mapping
    const TObjId obj1 = fld1.obj();
    if (obj1 != obj2) {
        CL_BREAK_IF("diffUnsetField() does not support non-trivial object map");
        return false;
    }

    // insert meta-operation
    const MetaOperation moUnset(MO_UNSET, obj1, off);
    ctx.opSet.insert(moUnset);
    return true;
}

bool diffFields(DiffHeapsCtx &ctx, const TObjId obj1, const TObjId obj2)
{
    if (ctx.sh1.isValid(obj1)) {
        const EObjKind kind1 = ctx.sh1.objKind(obj1);
        const EObjKind kind2 = ctx.sh2.objKind(obj2);
        if (kind1 != kind2) {
            MO_DEBUG("diffFields() detected mismatch of kind of objects"
                    << ", obj1 = #" << obj1
                    << ", obj2 = #" << obj2);
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

bool findSingleDls(BindingOff *pDst, const SymHeap &sh, const TObjList &objList)
{
    unsigned dlsCount = 0U;

    BOOST_FOREACH(const TObjId obj, objList) {
        const EObjKind kind = sh.objKind(obj);
        switch (kind) {
            case OK_REGION:
                continue;

            case OK_DLS:
                break;

            default:
                CL_BREAK_IF("unexpected kind of object in findSingleDls()");
                return false;
        }

        ++dlsCount;
        *pDst = sh.segBinding(obj);
    }

    return (1U == dlsCount);
}

bool isConcretizationOp(DiffHeapsCtx &ctx, const MetaOperation &mo)
{
    if (MO_SET != mo.code)
        // only MO_SET ops are handled for now
        return false;

    TObjList objList1;
    ctx.idMap.query<D_RIGHT_TO_LEFT>(&objList1, mo.obj);
    if (1U != objList1.size())
        // not a concretization (at least not the simple case)
        return false;

    TObjList objList2;
    const TObjId obj1 = objList1.front();
    ctx.idMap.query<D_LEFT_TO_RIGHT>(&objList2, obj1);
    if (2U != objList2.size())
        // not a concretization (at least not the simple case)
        return false;

    const TObjList::const_iterator beg2 = objList2.begin();
    const TObjList::const_iterator end2 = objList2.end();

    if (end2 == std::find(beg2, end2, mo.obj))
        // mo.obj not on the list of original objects
        return false;

    if (end2 == std::find(beg2, end2, mo.tgtObj))
        // mo.tgtObj not on the list of original objects
        return false;

    BindingOff bOff;
    if (!findSingleDls(&bOff, ctx.sh2, objList2))
        // not a DLS + REG pair of objects
        return false;

    if (mo.tgtOff != bOff.head)
        // target offset mismatch
        return false;

    switch (mo.tgtTs) {
        case TS_FIRST:
            return mo.off == bOff.next;

        case TS_LAST:
            return mo.off == bOff.prev;

        case TS_REGION:
            return mo.off == bOff.next
                || mo.off == bOff.prev;

        default:
            CL_BREAK_IF("invalid target specifier in isConcretizationOp()");
            return false;
    }
}

void dropConcretizationOps(DiffHeapsCtx &ctx)
{
    TMetaOpSet result;

    BOOST_FOREACH(const MetaOperation &mo, ctx.opSet) {
        if (isConcretizationOp(ctx, mo))
            continue;

        result.insert(mo);
    }

    ctx.opSet.swap(result);
}

bool diffHeaps(TMetaOpSet *pDst, const SymHeap &sh1, const SymHeap &sh2)
{
    DiffHeapsCtx ctx(pDst,
            const_cast<SymHeap &>(sh1),
            const_cast<SymHeap &>(sh2));

    TObjList objList2;
    ctx.sh2.gatherObjects(objList2);
    BOOST_FOREACH(const TObjId obj2, objList2) {
        TObjList objList1;
        ctx.idMap.query<D_RIGHT_TO_LEFT>(&objList1, obj2);
        if (1U < objList1.size()) {
            MO_DEBUG("diffHeaps() does not support ambiguous ID mapping");
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
        if (objList2.empty())
            objList2.push_back(OBJ_INVALID);

        BOOST_FOREACH(const TObjId obj2, objList2) {
            if (!ctx.sh2.isValid(obj2)) {
                const MetaOperation moFree(MO_FREE, obj2);
                ctx.opSet.insert(moFree);
            }
        }
    }

    if (!ctx.idMap.isTrivial())
        dropConcretizationOps(ctx);

    // heaps diffed successfully!
    return true;
}

} // namespace AdtOp
