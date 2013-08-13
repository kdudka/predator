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
#include "adt_op.hh"

#include "cont_shape.hh"
#include "symplot.hh"
#include "symstate.hh"
#include "symtrace.hh"

#include <iomanip>
#include <sstream>

#include <boost/foreach.hpp>

namespace AdtOp {

int countObjsInContShapes(const TShapeListByHeapIdx &slistByHeap)
{
    int cnt = 0;
    BOOST_FOREACH(const TShapeList &slist, slistByHeap)
        BOOST_FOREACH(const Shape &shape, slist)
            cnt += shape.length;

    return cnt;
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of OpFootprint
OpFootprint::OpFootprint(const SymHeap &input_, const SymHeap &output_):
    input(input_),
    output(output_)
{
    Trace::waiveCloneOperation(input);
    Trace::waiveCloneOperation(output);
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of OpTemplate
void OpTemplate::updateMetaIfNeeded() const
{
    if (this->dirty_)
        this->dirty_ = false;
    else
        return;

    // wipe out all meta data
    inShapes_.clear();
    outShapes_.clear();

    // get the lists of input/output heaps from all footprints
    SymHeapList inState, outState;
    BOOST_FOREACH(const OpFootprint *fp, fList_) {
        inState.insert(fp->input);
        outState.insert(fp->output);
    }

    // detect container shapes in the input/output heaps
    detectLocalContShapes(&inShapes_, inState);
    detectLocalContShapes(&outShapes_, outState);

    // pick the side with more objects included in container shapes
    const int cntIn  = countObjsInContShapes(inShapes_);
    const int cntOut = countObjsInContShapes(outShapes_);
    searchDirection_ = (cntIn < cntOut)
        ? SD_BACKWARD
        : SD_FORWARD;
}

/// FIXME: copy-pasted from fixed_point_proxy.cc
void contShapeIdsByShapeList(
        TIdSet                     *pDst,
        const SymHeap              &sh,
        const TShapeList           &shapeList)
{
    BOOST_FOREACH(const Shape &shape, shapeList) {
        TObjSet contShapeObjs;
        objSetByShape(&contShapeObjs, sh, shape);
        BOOST_FOREACH(const TObjId obj, contShapeObjs)
            pDst->insert(static_cast<int>(obj));
    }
}

void OpTemplate::plot() const
{
    unsigned idx = 0U;

    BOOST_FOREACH(const OpFootprint *fprint, fList_) {
        // convert the ID to string
        std::ostringstream str;
        str << name_ << "-"
            << std::fixed
            << std::setfill('0')
            << std::setw(/* width of the idx suffix */ 2)
            << idx;

        TIdSet inIds, outIds;
        contShapeIdsByShapeList(&inIds,  fprint->input, this->inShapes()[idx]);
        contShapeIdsByShapeList(&outIds, fprint->output,this->outShapes()[idx]);

        plotHeap(fprint->input, str.str() + "-in", /* loc */ 0, 0, &inIds);
        plotHeap(fprint->output, str.str() + "-out", /* loc */ 0, 0, &outIds);

        ++idx;
    }
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of OpCollection
void OpCollection::plot() const
{
    BOOST_FOREACH(const OpTemplate *tpl, tList_)
        tpl->plot();
}

} // namespace AdtOp
