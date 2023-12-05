/*
 * Copyright (C) 2013-2022 Kamil Dudka <kdudka@redhat.com>
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

#include "cont_shape.hh"

#include "symdiscover.hh"
#include "symstate.hh"
#include "symutil.hh"

namespace ContShape {

bool debuggingEnabled;

class ApparentShapeDetector {
    public:
        ApparentShapeDetector(SymHeap &sh, TShapeList &dstArray):
            sh_(sh),
            dstArray_(dstArray)
        {
        }

        bool probeEntry(const TObjId obj, const ShapeProps &props);

    private:
        typedef std::map<ShapeProps, TObjSet>       TObjsByProps;

        SymHeap                    &sh_;
        TShapeList                 &dstArray_;
        TObjsByProps                objsByProps_;

        bool isFreeEnt(const TObjId obj, const ShapeProps &props);
};

bool ApparentShapeDetector::isFreeEnt(const TObjId obj, const ShapeProps &props)
{
    const TObjsByProps::const_iterator it = objsByProps_.find(props);
    if (it == objsByProps_.end())
        return true;

    const TObjSet &taken = it->second;
    return !hasKey(taken, obj);
}

bool ApparentShapeDetector::probeEntry(const TObjId obj, const ShapeProps &props)
{
    if (!this->isFreeEnt(obj, props))
        return false;

    TObjSet seen;

    // seek beg
    TObjId beg = obj;
    for (;;) {
        seen.insert(beg);
        TObjId prev = nextObj(sh_, beg, props.bOff.prev);
        if (!canMergeObjWithNextObj(sh_, prev, props))
            break;

        if (hasKey(seen, prev))
            // loop detected (we intentionally do not use insertOnce() here)
            break;

        beg = prev;
    }

    // seek end
    TObjId end = obj;
    for (;;) {
        TObjId next = nextObj(sh_, end, props.bOff.next);
        if (!canMergeObjWithNextObj(sh_, end, props))
            break;

        if (hasKey(seen, next))
            // loop detected (we intentionally do not use insertOnce() here)
            break;

        end = next;
        seen.insert(end);
    }

    // check the end-points
    const TValId valPrev = valOfPtr(sh_, beg, props.bOff.prev);
    const TValId valNext = valOfPtr(sh_, end, props.bOff.next);
    if (valPrev != valNext)
        // terminator mismatch
        return false;

    if (VAL_NULL != valNext)
        CL_WARN("ApparentShapeDetector uses a non-NULL terminator");

    // check the length
    const unsigned len = seen.size();
    CS_DEBUG("ApparentShapeDetector found a new container shape, len = "<< len);

    // mark all used objects as taken
    TObjSet &taken = objsByProps_[props];
    if (taken.empty())
        taken.swap(seen);
    else {
        for (const TObjId seenObj : seen)
            taken.insert(seenObj);
    }

    // insert the shape in the destination array
    const Shape shape(
        /* entry  */ beg,
        /* props  */ props,
        /* length */ len);
    dstArray_.push_back(shape);

    // found a new container shape
    return true;
}

void detectApparentShapes(TShapeList &dst, SymHeap &sh)
{
    CL_BREAK_IF(!dst.empty());
    ApparentShapeDetector shapeDetector(sh, dst);

    // go through all potential shape container entries
    TObjList heapObjs;
    sh.gatherObjects(heapObjs, isOnHeap);
    for (const TObjId obj : heapObjs) {
        if (sh.objProtoLevel(obj))
            // FIXME: we support only L0 data structures for now
            continue;

        const EObjKind kind = sh.objKind(obj);
        if (OK_DLS == kind) {
            const TSizeRange size = sh.objSize(obj);
            CL_BREAK_IF(!IR::isSingular(size));

            // OK_DLS can be seen as list on its own
            // FIXME: we support only OK_DLS for now
            const ShapeProps dlsProps(OK_DLS,
                    sh.segBinding(obj),
                    size.lo);

            shapeDetector.probeEntry(obj, dlsProps);
            continue;
        }

        TShapePropsList propList;
        digShapePropsCandidates(&propList, sh, obj);

        // go through all potential shape properties candidates
        for (const ShapeProps &props : propList) {
            if (OK_DLS != props.kind)
                // FIXME: we support only OK_DLS for now
                continue;

            shapeDetector.probeEntry(obj, props);
        }
    }
}

typedef std::pair<CVar, TOffset>            TPointer;
typedef std::set<TPointer>                  TPointerSet;
typedef std::set<TPointerSet>               TPointerSetLookup;

struct ShapePattern {
    ShapeProps          props;
    TObjType            type;
    TSizeRange          size;
    TPointerSet         headPtrs;
};

typedef std::vector<ShapePattern>           TShapePatternList;

class ImpliedShapeDetector {
    public:
        ImpliedShapeDetector(bool useHeadChk):
            useHeadChk_(useHeadChk)
        {
        }

        bool indexShape(SymHeap &sh, const Shape &shape);
        void appendImpliedShapes(TShapeList *pDst, SymHeap &sh);

    private:
        const bool                  useHeadChk_;
        TShapePatternList           plist_;
        TPointerSetLookup           index_;
};

void digHeadPtrs(
        TPointerSet                *pDst,
        SymHeap                    &sh,
        const TObjId                beg,
        const TObjId                end,
        const TOffset               offHead)
{
    // gather all references of the boundary objects
    FldList refs;
    sh.pointedBy(refs, beg);
    if (beg != end)
        sh.pointedBy(refs, end);

    // go through the list of references
    for (const FldHandle &fld : refs) {
        const TObjId obj = fld.obj();
        const EStorageClass code = sh.objStorClass(obj);
        if (!isProgramVar(code))
            // not a program variable
            continue;

        const TValId val = fld.value();
        const TOffset off = sh.valOffset(val);
        if (off != offHead)
            // not a head offset
            continue;

        const CVar var = sh.cVarByObject(obj);
        const TPointer ptr(var, fld.offset());
        pDst->insert(ptr);
    }
}

bool ImpliedShapeDetector::indexShape(SymHeap &sh, const Shape &shape)
{
    const TObjId beg = shape.entry;
    const TObjId end = lastObjOfShape(sh, shape);

    if (OK_DLS != shape.props.kind)
        // FIXME: only OK_DLS supported for now
        return false;

    const TValId valPrev = valOfPtr(sh, beg, shape.props.bOff.prev);
    const TValId valNext = valOfPtr(sh, end, shape.props.bOff.next);
    if (VAL_NULL != valPrev || VAL_NULL != valNext)
        // FIXME: only NULL-terminated lists supported for now
        return false;

    TPointerSet headPtrs;
    if (useHeadChk_) {
        const TOffset offHead = shape.props.bOff.head;
        digHeadPtrs(&headPtrs, sh, beg, end, offHead);

        if (!insertOnce(index_, headPtrs))
            // already indexed
            return false;
    }

    ShapePattern sp = {
        /* props    */ shape.props,
        /* type     */ sh.objEstimatedType(beg),
        /* size     */ sh.objSize(beg),
        /* headPtrs */ std::move(headPtrs)
    };
    plist_.push_back(sp);

    // shape successfully indexed
    return true;
}

bool detectImpliedSingleNode(
        Shape                      *pDst,
        SymHeap                    &sh,
        const ShapePattern         &sp,
        const TValId                valHead)
{
    const TOffset offHead = sh.valOffset(valHead);
    if (offHead != sp.props.bOff.head)
        // head offset mismatch
        return false;

    const TObjId obj = sh.objByAddr(valHead);
    if (!sh.isValid(obj) || !isOnHeap(sh.objStorClass(obj)))
        // not a valid heap object
        return false;

    const TSizeRange size = sh.objSize(obj);
    if (size != sp.size)
        // size mismatch
        return false;

    const TObjType type = sh.objEstimatedType(obj);
    if (type && sp.type && (*type != *sp.type))
        // type mismatch
        return false;

    const TValId valNext = valOfPtr(sh, obj, sp.props.bOff.next);
    if (VAL_NULL != valNext)
        // next pointer mismatch
        return false;

    const TValId valPrev = valOfPtr(sh, obj, sp.props.bOff.prev);
    if (VAL_NULL != valPrev)
        // prev pointer mismatch
        return false;

    pDst->entry  = obj;
    pDst->props  = sp.props;
    pDst->length = 1U;

    CS_DEBUG("ImpliedShapeDetector matches a region as container shape");
    return true;
}

bool detectImpliedShape(Shape *pDst, SymHeap &sh, const ShapePattern &sp)
{
    TValId valHead = VAL_INVALID;

    for (const TPointer &ptr : sp.headPtrs) {
        const CVar &var = ptr.first;
        const TOffset off = ptr.second;

        const TObjId obj = sh.regionByVar(var, /* createIfNeeded */ false);
        if (OBJ_INVALID == obj)
            // missing program variable
            return false;

        const TValId val = valOfPtr(sh, obj, off);
        CL_BREAK_IF(VAL_INVALID == val);

        if (VAL_INVALID == valHead)
            valHead = val;
        else if (val != valHead)
            // head value mismatch
            return false;
    }

    if (VAL_NULL != valHead)
        return detectImpliedSingleNode(pDst, sh, sp, valHead);

#if 0
    // empty list
    pDst->entry  = OBJ_INVALID;
    pDst->props  = sp.props;
    pDst->length = 0U;

    CS_DEBUG("ImpliedShapeDetector matches an empty list as container shape");
    return true;
#else
    return false;
#endif
}

bool detectImpliedShapeBlindly(
        Shape                      *pDst,
        SymHeap                    &sh,
        const ShapePattern         &sp,
        const TObjId                obj)
{
    if (!sh.isValid(obj) || !isOnHeap(sh.objStorClass(obj)))
        // not a valid heap object
        return false;

    const TSizeRange size = sh.objSize(obj);
    if (size != sp.size)
        // size mismatch
        return false;

    const TObjType type = sh.objEstimatedType(obj);
    if (type && sp.type && (*type != *sp.type))
        // type mismatch
        return false;

    const TValId valNext = valOfPtr(sh, obj, sp.props.bOff.next);
    if (VAL_NULL != valNext)
        // next pointer mismatch
        return false;

    const TValId valPrev = valOfPtr(sh, obj, sp.props.bOff.prev);
    if (VAL_NULL != valPrev)
        // prev pointer mismatch
        return false;

    CS_DEBUG("ImpliedShapeDetector matches a region as container shape");
    pDst->entry  = obj;
    pDst->props  = sp.props;
    pDst->length = 1U;
    return true;
}

void ImpliedShapeDetector::appendImpliedShapes(TShapeList *pDst, SymHeap &sh)
{
    // eliminate duplicates (one container shape can be implied by many)
    TShapeSet found;
    for (const Shape &shape : *pDst)
        found.insert(shape);

    for (const ShapePattern &sp : plist_) {
        Shape shape;

        if (useHeadChk_) {
            if (detectImpliedShape(&shape, sh, sp) && insertOnce(found, shape))
                pDst->push_back(shape);

            continue;
        }

        TObjList allObjs;
        sh.gatherObjects(allObjs);
        for (const TObjId obj : allObjs) {
            if (detectImpliedShapeBlindly(&shape, sh, sp, obj)
                    && insertOnce(found, shape))
                pDst->push_back(shape);
        }
    }
}

struct DetectionCtx {
    TShapeListByHeapIdx            &dstArray;
    const SymState                 &srcState;
    const unsigned                  cntHeaps;

    DetectionCtx(TShapeListByHeapIdx &dstArray_, const SymState &srcState_):
        dstArray(dstArray_),
        srcState(srcState_),
        cntHeaps(srcState_.size())
    {
    }
};

bool hasAnyProgramVar(const SymState &state)
{
    for (const SymHeap *pSh : state) {
        TObjList vars;
        pSh->gatherObjects(vars, isProgramVar);
        if (!vars.empty())
            return true;
    }

    // found nothing
    return false;
}

void detectImpliedShapes(DetectionCtx &ctx)
{
#if 0
    // if there are no variables, we are likely dealing with templates
    const bool anyVars = hasAnyProgramVar(ctx.srcState);
#else
    // FIXME: temporarily disabled in order to find more container shapes
    const bool anyVars = false;
#endif
    ImpliedShapeDetector shapeDetector(/* useHeadChk */ anyVars);

    for (unsigned i = 0U; i < ctx.cntHeaps; ++i) {
        SymHeap &sh = const_cast<SymHeap &>(ctx.srcState[i]);
        const TShapeList &apparentShapes = ctx.dstArray[i];

        for (const Shape &shape : apparentShapes)
            shapeDetector.indexShape(sh, shape);
    }

    for (unsigned i = 0U; i < ctx.cntHeaps; ++i) {
        SymHeap &sh = const_cast<SymHeap &>(ctx.srcState[i]);
        shapeDetector.appendImpliedShapes(&ctx.dstArray[i], sh);
    }
}

} // namespace ContShape

void detectLocalContShapes(TShapeListByHeapIdx *pDst, const SymState &state)
{
    CL_BREAK_IF(!pDst->empty());

    using namespace ContShape;

    const unsigned cnt = state.size();
    pDst->resize(cnt);

    DetectionCtx ctx(*pDst, state);

    // first detect container shapes we can detect on their own
    bool foundApparentShape = false;
    for (unsigned i = 0U; i < cnt; ++i) {
        TShapeList &dst = ctx.dstArray[i];
        SymHeap &src = const_cast<SymHeap &>(state[i]);

        detectApparentShapes(dst, src);
        if (dst.empty())
            continue;

        foundApparentShape = true;
    }

    if (!foundApparentShape)
        // no apparent shape found, so we cannot look for implied shapes
        return;

    detectImpliedShapes(ctx);
}
