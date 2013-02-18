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

#include "cont_shape.hh"

#include "symdiscover.hh"
#include "symstate.hh"
#include "symutil.hh"

#include <cl/cl_msg.hh>

namespace ContShape {

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
    CL_DEBUG("ApparentShapeDetector found a new container shape, len = "<< len);

    // mark all used objects as taken
    TObjSet &taken = objsByProps_[props];
    if (taken.empty())
        taken.swap(seen);
    else {
        CL_BREAK_IF("please check this branch with a debugger");
        BOOST_FOREACH(const TObjId seenObj, seen)
            taken.insert(seenObj);
    }

    // insert the shape in the destination array
    const Shape shape = {
        /* entry  */ beg,
        /* props  */ props,
        /* length */ len
    };
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
    BOOST_FOREACH(const TObjId obj, heapObjs) {
        if (sh.objProtoLevel(obj))
            // FIXME: we support only L0 data structures for now
            continue;

        const EObjKind kind = sh.objKind(obj);
        if (OK_DLS == kind) {
            // OK_DLS can be seen as list on its own
            // FIXME: we support only OK_DLS for now
            const ShapeProps dlsProps = {
                OK_DLS, 
                sh.segBinding(obj)
            };

            shapeDetector.probeEntry(obj, dlsProps);
            continue;
        }

        TShapePropsList propList;
        digShapePropsCandidates(&propList, sh, obj);

        // go through all potential shape properties candidates
        BOOST_FOREACH(const ShapeProps &props, propList) {
            if (OK_DLS != props.kind)
                // FIXME: we support only OK_DLS for now
                continue;

            shapeDetector.probeEntry(obj, props);
        }
    }
}

struct DetectionCtx {
    TShapeListByHeapIdx            &dstArray;
    const SymState                 &srcState;

    DetectionCtx(TShapeListByHeapIdx &dstArray_, const SymState &srcState_):
        dstArray(dstArray_),
        srcState(srcState_)
    {
    }
};

void detectImpliedShapes(DetectionCtx &ctx)
{
    // TODO
    (void) ctx;
    CL_BREAK_IF("please implement");
}

void detectContShapes(TShapeListByHeapIdx *pDst, const SymState &state)
{
    CL_BREAK_IF(!pDst->empty());

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

} // namespace ContShape
