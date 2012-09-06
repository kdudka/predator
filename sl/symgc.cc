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

void gatherReferredRoots(TValList &dst, SymHeap &sh, TValId at)
{
    CL_BREAK_IF(sh.valOffset(at));

    ObjList ptrs;
    sh.gatherLivePointers(ptrs, at);
    BOOST_FOREACH(const ObjHandle &obj, ptrs) {
        const TValId val = obj.value();
        if (val <= 0)
            continue;

        const TValId root = sh.valRoot(val);
        dst.push_back(root);
    }
}

bool isJunk(SymHeap &sh, TValId root)
{
    WorkList<TValId> wl(root);

    while (wl.next(root)) {
        const EValueTarget code = sh.valTarget(root);
        if (!isOnHeap(code))
            // non-heap objects cannot be JUNK
            return false;

        // go through all referrers
        ObjList refs;
        sh.pointedBy(refs, root);
        BOOST_FOREACH(const ObjHandle &obj, refs) {
            const TValId refAt = obj.placedAt();
            const TValId refRoot = sh.valRoot(refAt);
            wl.schedule(refRoot);
        }
    }

    return true;
}

bool gcCore(SymHeap &sh, TValId root, TValList *leakList, bool sharedOnly)
{
    CL_BREAK_IF(sh.valOffset(root));
    bool detected = false;

    std::set<TValId> whiteList;
    if (sharedOnly) {
        whiteList.insert(root);
        if (OK_DLS == sh.valTargetKind(root))
            whiteList.insert(dlSegPeer(sh, root));
    }

    WorkList<TValId> wl(root);
    while (wl.next(root)) {
        if (!isJunk(sh, root))
            // not a junk, keep going...
            continue;

        // gather all roots pointed by the junk object
        TValList refs;
        gatherReferredRoots(refs, sh, root);

        if (sharedOnly) {
            if (hasKey(whiteList, root))
                goto skip_root;

            if (0 < sh.valTargetProtoLevel(root))
                goto skip_root;
        }

        // leak detected
        detected = true;
        sh.valDestroyTarget(root);
        if (leakList)
            leakList->push_back(root);

skip_root:
        // schedule just created junk candidates for next wheel
        BOOST_FOREACH(TValId refRoot, refs)
            wl.schedule(refRoot);
    }

    return detected;
}

bool collectJunk(SymHeap &sh, TValId root, TValList *leakList)
{
    return gcCore(sh, root, leakList, /* sharedOnly */ false);
}

bool collectSharedJunk(SymHeap &sh, TValId root, TValList *leakList)
{
    return gcCore(sh, root, leakList, /* sharedOnly */ true);
}

bool destroyRootAndCollectJunk(
        SymHeap                 &sh,
        const TValId             root,
        TValList                *leakList)
{
    CL_BREAK_IF(sh.valOffset(root));
    CL_BREAK_IF(!isPossibleToDeref(sh.valTarget(root)));

    // gather potentialy destroyed pointer values
    TValList killedPtrs;
    gatherReferredRoots(killedPtrs, sh, root);

    // destroy the target
    sh.valDestroyTarget(root);

    // now check for memory leakage
    bool leaking = false;
    BOOST_FOREACH(TValId val, killedPtrs) {
        if (collectJunk(sh, val, leakList))
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
    if (leakList_.empty())
        return;

    if (::debuggingGarbageCollector)
        plotHeap(snap_, "memleak", /* TODO: loc */ 0, leakList_,
                /* digForward */ false);
}

bool /* leaking */ LeakMonitor::importLeakList(TValList *leakList)
{
    CL_BREAK_IF(!leakList_.empty());
    leakList_ = *leakList;

    return /* leaking */ !leakList_.empty();
}
