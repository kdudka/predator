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

namespace ContShape {

class AparentShapeDetector {
    public:
        AparentShapeDetector(SymHeap &sh, TShapeList &dstArray):
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
};

bool AparentShapeDetector::probeEntry(const TObjId obj, const ShapeProps &props)
{
    // TODO
    CL_BREAK_IF("please implement");
    (void) obj;
    (void) props;
    return false;
}

void detectApparentShapes(TShapeList &dst, SymHeap &sh)
{
    CL_BREAK_IF(!dst.empty());
    AparentShapeDetector shapeDetector(sh, dst);

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
