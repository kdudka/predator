/*
 * Copyright (C) 2009-2012 Kamil Dudka <kdudka@redhat.com>
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
#include "symgc.hh"

#include <cl/cl_msg.hh>

#include "symheap.hh"
#include "symplot.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "worklist.hh"

#include <stack>

#include <boost/foreach.hpp>

void gatherReferredRoots(TObjSet &dst, SymHeap &sh, TObjId obj)
{
    FldList ptrs;
    sh.gatherLiveFields(ptrs, obj);
    BOOST_FOREACH(const FldHandle &fld, ptrs) {
        const TValId val = fld.value();
        if (val <= 0)
            continue;

        const TObjId obj = sh.objByAddr(val);
        if (!sh.isValid(obj))
            continue;

        dst.insert(obj);
    }
}

bool isJunk(SymHeap &sh, TObjId obj)
{
    WorkList<TObjId> wl(obj);

    while (wl.next(obj)) {
        if (!sh.isValid(obj))
            // this object is already freed
            return false;

        const EStorageClass code = sh.objStorClass(obj);
        if (!isOnHeap(code))
            // non-heap objects cannot be JUNK
            return false;

        // go through all referrers
        FldList refs;
        sh.pointedBy(refs, obj);
        BOOST_FOREACH(const FldHandle &fld, refs)
            wl.schedule(fld.obj());
    }

    return true;
}

bool gcCore(SymHeap &sh, TObjId obj, TObjSet *leakObjs, bool sharedOnly)
{
    if (OBJ_INVALID == obj)
        return false;

    bool detected = false;

    std::set<TObjId> whiteList;
    if (sharedOnly)
        whiteList.insert(obj);

    WorkList<TObjId> wl(obj);
    while (wl.next(obj)) {
        if (!isJunk(sh, obj))
            // not a junk, keep going...
            continue;

        // gather all roots pointed by the junk object
        TObjSet refs;
        gatherReferredRoots(refs, sh, obj);

        if (sharedOnly) {
            if (hasKey(whiteList, obj))
                goto skip_root;

            if (0 < sh.objProtoLevel(obj))
                goto skip_root;
        }

        // leak detected
        detected = true;
        sh.objInvalidate(obj);
        if (leakObjs)
            leakObjs->insert(obj);

skip_root:
        // schedule just created junk candidates for next wheel
        BOOST_FOREACH(const TObjId refObj, refs)
            wl.schedule(refObj);
    }

    return detected;
}

bool collectJunk(SymHeap &sh, TObjId obj, TObjSet *leakObjs)
{
    return gcCore(sh, obj, leakObjs, /* sharedOnly */ false);
}

bool collectSharedJunk(SymHeap &sh, TObjId obj, TObjSet *leakObjs)
{
    return gcCore(sh, obj, leakObjs, /* sharedOnly */ true);
}

bool destroyObjectAndCollectJunk(
        SymHeap                 &sh,
        const TObjId             obj,
        TObjSet                 *leakObjs)
{
    CL_BREAK_IF(!sh.isValid(obj));

    // gather potentialy destroyed pointer values
    TObjSet refs;
    gatherReferredRoots(refs, sh, obj);

    // destroy the target
    sh.objInvalidate(obj);

    // now check for memory leakage
    bool leaking = false;
    BOOST_FOREACH(const TObjId obj, refs) {
        if (collectJunk(sh, obj, leakObjs))
            leaking = true;
    }

    return leaking;
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of LeakMonitor
static bool debuggingGarbageCollector = static_cast<bool>(DEBUG_SYMGC);

void debugGarbageCollector(const bool enable)
{
    if (enable == ::debuggingGarbageCollector)
        return;

    CL_DEBUG("symgc: debugGarbageCollector(" << enable << ") takes effect");
    ::debuggingGarbageCollector = enable;
}

void LeakMonitor::enter()
{
    if (::debuggingGarbageCollector)
        snap_ = sh_;
}

void LeakMonitor::leave()
{
    if (!::debuggingGarbageCollector || leakObjs_.empty())
        return;

    plotHeap(snap_, "memleak", /* loc */ 0, leakObjs_);
}

bool /* leaking */ LeakMonitor::importLeakObjs(TObjSet *leakObjs)
{
    CL_BREAK_IF(!leakObjs_.empty());
    leakObjs_ = *leakObjs;

    return /* leaking */ !leakObjs_.empty();
}
