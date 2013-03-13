/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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
#include "symdiscover.hh"

#include <cl/cl_msg.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "prototype.hh"
#include "symcmp.hh"
#include "symjoin.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <set>

#include <boost/foreach.hpp>

// costs are now hard-wired in the paper, so they were removed from config.h
#define SE_PROTO_COST_SYM           0
#define SE_PROTO_COST_ASYM          1
#define SE_PROTO_COST_THREEWAY      2

int minLengthByCost(int cost)
{
    // abstraction length thresholds are now configurable in config.h
    static const int thrTable[] = {
        (SE_COST0_LEN_THR),
        (SE_COST1_LEN_THR),
        (SE_COST2_LEN_THR)
    };

    static const int maxCost = sizeof(thrTable)/sizeof(thrTable[0]) - 1;
    if (maxCost < cost)
        cost = maxCost;

    // Predator counts elementar merges whereas the paper counts objects on path
    const int minLength = thrTable[cost] - 1;
    CL_BREAK_IF(minLength < 1);
    return minLength;
}

bool matchSegBinding(
        const SymHeap              &sh,
        const TObjId                seg,
        const ShapeProps           &props)
{
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_REGION:
            // nothing to match actually
            return true;

        case OK_OBJ_OR_NULL:
            // OK_OBJ_OR_NULL can be the last node of a NULL-terminated list
            return true;

        default:
            break;
    }

    const BindingOff offObj = sh.segBinding(seg);
    const BindingOff offPath = props.bOff;

    if (offObj.head != offPath.head)
        // head mismatch
        return false;

    if (OK_SLS == props.kind) {
        // OK_SLS
        switch (kind) {
            case OK_SEE_THROUGH:
            case OK_SLS:
                return (offObj.next == offPath.next);

            default:
                return false;
        }
    }

    // OK_DLS
    switch (kind) {
        case OK_SEE_THROUGH_2N:
            if ((offObj.next == offPath.prev) && (offObj.prev == offPath.next))
                // both fields are equal
                return true;

            // fall through!

        case OK_DLS:
            return (offObj.next == offPath.next)
                && (offObj.prev == offPath.prev);

        default:
            return false;
    }
}

/// (VAL_INVALID == prev && VAL_INVALID == next) denotes prototype validation
bool validatePointingObjects(
        SymHeap                    &sh,
        const ShapeProps           &props,
        const TObjId                obj,
        const TObjId                prev,
        const TObjId                next,
        TObjSet                     allowedReferers,
        const ETargetSpecifier      tsEntry = TS_INVALID)
{
    // allow pointers to self
    allowedReferers.insert(obj);

    // collect all objects pointing at/inside the object
    FldList refs;
    sh.pointedBy(refs, obj);

    const BindingOff &off = props.bOff;

    // unless this is a prototype, disallow self loops from _binding_ pointers
    TFldSet blackList;
    if (OBJ_INVALID != prev || OBJ_INVALID != next)
        buildIgnoreList(blackList, sh, obj, off);

    TFldSet whiteList;
    if (OBJ_INVALID != prev) {
        const PtrHandle prevNext(sh, prev, off.next);
        whiteList.insert(prevNext);
    }
    if (OBJ_INVALID != next && OK_DLS == props.kind) {
        const PtrHandle nextPrev(sh, next, off.prev);
        whiteList.insert(nextPrev);
    }

    BOOST_FOREACH(const FldHandle &fld, refs) {
        if (hasKey(blackList, fld))
            return false;

        if (hasKey(whiteList, fld))
            continue;

        if (TS_INVALID != tsEntry) {
            const TValId val = fld.value();
            if (VT_OBJECT == sh.valTarget(val) && sh.valOffset(val) == off.head) {
                const ETargetSpecifier ts = sh.targetSpec(val);
                if (TS_REGION == ts || ts == tsEntry)
                    continue;
            }
        }

        if (hasKey(allowedReferers, fld.obj()))
            continue;

        // someone points at/inside who should not
        return false;
    }

    // no problems encountered
    return true;
}

bool validatePrototypes(
        SymHeap                    &sh,
        const ShapeProps           &props,
        const TObjId                obj,
        const TObjSet              &protos)
{
    TObjSet allowedReferers(protos);
    allowedReferers.insert(obj);

    BOOST_FOREACH(const TObjId proto, protos) {
        if (!validatePointingObjects(sh, props, proto, OBJ_INVALID, OBJ_INVALID,
                                     allowedReferers))
            return false;
    }

    // all OK!
    return true;
}

bool validateSegEntry(
        SymHeap                    &sh,
        const ShapeProps           &props,
        const TObjId                obj,
        const TObjId                prev,
        const TObjId                next,
        const TObjSet              &protos,
        const ETargetSpecifier      tsEntry)
{
    // first validate 'root' itself
    if (!validatePointingObjects(sh, props, obj, prev, next, protos, tsEntry))
        return false;

    return validatePrototypes(sh, props, obj, protos);
}

bool canMergeObjWithNextObj(
        SymHeap                    &sh,
        const TObjId                obj,
        const ShapeProps           &props,
        TObjId                     *pNextObj)
{
    if (!sh.isValid(obj) || !isOnHeap(sh.objStorClass(obj)))
        // neither the starting point is valid
        return false;

    if (!matchSegBinding(sh, obj, props))
        // binding mismatch
        return false;

    const BindingOff &off = props.bOff;
    const TValId valNext = valOfPtr(sh, obj, off.next);
    if (off.head != sh.valOffset(valNext))
        // head offset mismatch in forward direction
        return false;

    if (!canPointToFront(sh.targetSpec(valNext)))
        // target specifier mismatch in forward direction
        return false;

    const TObjId next = sh.objByAddr(valNext);
    if (!sh.isValid(next) || !isOnHeap(sh.objStorClass(next)))
        // only objects on heap can be abstracted for now
        return false;

    if (!matchSegBinding(sh, next, props))
        // binding mismatch
        return false;

    if (sh.objSize(obj) != sh.objSize(next))
        // mismatch in size of targets
        return false;

    const TObjType clt = sh.objEstimatedType(obj);
    if (clt) {
        const TObjType cltNext = sh.objEstimatedType(next);
        if (cltNext && *cltNext != *clt)
            // both objects have estimated types assigned, but the types differ
            return false;
    }

    if (OK_DLS == props.kind) {
        const TValId valPrev = valOfPtr(sh, next, off.prev);
        if (sh.objByAddr(valPrev) != obj)
            // DLS back-link mismatch
            return false;

        if (off.head != sh.valOffset(valNext))
            // head offset mismatch in backward direction
            return false;

        if (!canPointToBack(sh.targetSpec(valPrev)))
            // target specifier mismatch in backward direction
            return false;
    }

    if (OBJ_INVALID == nextObj(sh, next, off.next))
        // valNext has no target
        return false;

    // all OK
    if (pNextObj)
        *pNextObj = next;

    return true;
}

TObjId jumpToNextObj(
        SymHeap                    &sh,
        const TObjId                obj,
        const ShapeProps           &props)
{
    TObjId next;
    if (canMergeObjWithNextObj(sh, obj, props, &next))
        return next;

    // unable to jump (broken binding, objects incompatible, ...)
    return OBJ_INVALID;
}

bool isPointedByVar(SymHeap &sh, const TObjId obj)
{
    FldList refs;
    sh.pointedBy(refs, obj);
    BOOST_FOREACH(const FldHandle fld, refs) {
        const TObjId refObj = fld.obj();
        const EStorageClass code = sh.objStorClass(refObj);
        if (isProgramVar(code))
            return true;
    }

    // no reference by a program variable
    return false;
}

typedef TObjSet TProtoPairs[2];

bool matchData(
        SymHeap                      sh,
        const ShapeProps            &props,
        const TObjId                 obj1,
        const TObjId                 obj2,
        TProtoPairs                 *protoPairs,
        int                         *pCost)
{
    if (OK_SLS == props.kind && isPointedByVar(sh, obj2))
        // only first node of an SLS can be pointed by a program var, giving up
        return false;

    EJoinStatus status;
    if (!joinData(sh, props, obj1, obj2, /* pDst */ 0, protoPairs, &status)) {
        CL_DEBUG("    joinData() refuses to create a segment!");
        return false;
    }

    int cost = 0;
    switch (status) {
        case JS_USE_ANY:
            cost = (SE_PROTO_COST_SYM);
            break;

        case JS_USE_SH1:
        case JS_USE_SH2:
            cost = (SE_PROTO_COST_ASYM);
            break;

        case JS_THREE_WAY:
            cost = (SE_PROTO_COST_THREEWAY);
            break;
    }

    *pCost = cost;
    return true;
}

typedef std::map<int /* cost */, int /* length */> TRankMap;

void segDiscover(
        TRankMap                   &dst,
        SymHeap                    &sh,
        const ShapeProps           &props,
        const TObjId                entry)
{
    CL_BREAK_IF(!dst.empty());

    const BindingOff &off = props.bOff;
    if (OK_DLS == props.kind && (OBJ_INVALID == nextObj(sh, entry, off.prev)))
        // valPrev has no target
        return;

    // we use std::set to detect loops
    TObjSet haveSeen;
    haveSeen.insert(entry);
    TObjId prev = entry;

    // the entry can already have some prototypes we should take into account
    TObjSet initialProtos;
    if (OK_DLS == sh.objKind(entry))
        collectPrototypesOf(initialProtos, sh, entry);

    // jump to the immediate successor
    TObjId obj = jumpToNextObj(sh, entry, props);
    if (!insertOnce(haveSeen, obj))
        // loop detected
        return;

    // we need a way to prefer lossless prototypes
    int maxCostOnPath = 0;

    // main loop of segDiscover()
    TObjList path;
    while (OBJ_INVALID != obj) {
        // compare the data
        TProtoPairs protoPairs;
        int cost = 0;

        // join data of the current pair of objects
        if (!matchData(sh, props, prev, obj, &protoPairs, &cost))
            break;

        if (prev == entry && !validateSegEntry(sh, props, entry, OBJ_INVALID,
                                               obj, protoPairs[0], TS_FIRST))
            // invalid entry
            break;

        if (!insertOnce(haveSeen, nextObj(sh, obj, off.next)))
            // loop detected
            break;

        if (!validatePrototypes(sh, props, obj, protoPairs[1]))
            // someone points to a prototype
            break;

        bool leaving = false;

        // look ahead
        TObjId next = jumpToNextObj(sh, obj, props);
        if (!validatePointingObjects(sh, props, obj, prev, next, protoPairs[1]))
        {
            // someone points at/inside who should not

            leaving = (OK_DLS == props.kind) && validateSegEntry(sh, props, obj,
                    prev, OBJ_INVALID, protoPairs[1], TS_LAST);

            if (!leaving)
                break;

            if (OBJ_INVALID == nextObj(sh, obj, off.next))
                // valNext has no target
                break;
        }

        // enlarge the path by one
        path.push_back(obj);
        if (maxCostOnPath < cost)
            maxCostOnPath = cost;

        // remember the longest path at this cost level
#if !SE_ALLOW_SUBPATH_RANKING
        dst.clear();
#endif
        dst[maxCostOnPath] = path.size();

        if (leaving)
            // we allow others to point at DLS end-point's _head_
            break;

        // jump to the next object on the path
        prev = obj;
        obj = next;
    }
}

class PtrFinder {
    private:
        const TObjId                obj_;
        const TOffset               off_;
        TOffset                     offFound_;

    public:
        // cppcheck-suppress uninitMemberVar
        PtrFinder(const TObjId obj, const TOffset off):
            obj_(obj),
            off_(off)
        {
        }

        TOffset offFound() const {
            return offFound_;
        }

    bool operator()(const FldHandle &fld) {
        const TValId val = fld.value();
        if (val <= 0)
            return /* continue */ true;

        SymHeapCore *sh = fld.sh();
        if (sh->objByAddr(val) != obj_)
            return /* continue */ true;

        if (sh->valTarget(val) != VT_OBJECT)
            return /* continue */ true;

        if (sh->valOffset(val) != off_)
            return /* continue */ true;

        // target found!
        offFound_ = fld.offset();
        return /* break */ false;
    }
};

bool digBackLink(
        bool                       *pSkip,
        BindingOff                 *pOff,
        SymHeap                    &sh,
        const TObjId                obj,
        const TObjId                next)
{
    // set up a visitor
    PtrFinder visitor(obj, pOff->head);

    // guide it through the next object
    if (/* found nothing */ traverseLiveFields(sh, next, visitor))
        return false;

    // got a back-link!
    pOff->prev = visitor.offFound();

    // we require offNext < offPrev (symmetry breaking rule)
    *pSkip = (pOff->prev < pOff->next);
    return true;
}

class ProbeEntryVisitor {
    private:
        TShapePropsList         &dst_;
        const TObjId            obj_;

    public:
        ProbeEntryVisitor(
                TShapePropsList            &dst,
                const TObjId                obj):
            dst_(dst),
            obj_(obj)
        {
        }

        bool operator()(const FldHandle &fld) const
        {
            SymHeap &sh = *static_cast<SymHeap *>(fld.sh());
            const TValId nextVal = fld.value();
            const TObjId nextObj = sh.objByAddr(nextVal);
            if (!canWriteDataPtrAt(sh, nextVal))
                return /* continue */ true;

            // read head offset
            ShapeProps props;
            BindingOff &off = props.bOff;
            off.head = sh.valOffset(nextVal);

            // entry candidate found, check the back-link in case of DLL
            off.next = fld.offset();
            off.prev = off.next;
#if !SE_DISABLE_DLS
            bool skip;
            if (digBackLink(&skip, &off, sh, obj_, nextObj) && skip)
                // looks like an oppositely oriented OK_DLS, keep going
                return /* continue */ true;
#endif

            props.kind = (off.next == off.prev)
                ? OK_SLS
                : OK_DLS;

#if SE_DISABLE_SLS
            // allow only DLS abstraction
            if (OK_SLS == props.kind)
                return /* continue */ true;
#endif
            // append a candidate
            dst_.push_back(props);
            return /* continue */ true;
        }
};

bool segOnPath(
        SymHeap                     &sh,
        const BindingOff            &off,
        const TObjId                entry,
        const unsigned              len)
{
    TObjId cursor = entry;

    for (unsigned pos = 0; pos <= len; ++pos) {
        if (OK_REGION != sh.objKind(cursor))
            return true;

        cursor = nextObj(sh, cursor, off.next);
    }

    return false;
}

void digShapePropsCandidates(
        TShapePropsList            *pDst,
        SymHeap                    &sh,
        const TObjId                obj)
{
    CL_BREAK_IF(!pDst->empty());
    const ProbeEntryVisitor visitor(*pDst, obj);
    traverseLiveFields(sh, obj, visitor);
}

struct SegCandidate {
    TObjId                      entry;
    TShapePropsList             propsList;
};

typedef std::vector<SegCandidate> TSegCandidateList;

bool selectBestAbstraction(
        Shape                      *pDst,
        SymHeap                    &sh,
        const TSegCandidateList    &candidates)
{
    const unsigned cnt = candidates.size();
    if (!cnt)
        // no candidates given
        return false;

    CL_DEBUG("--> initiating segment discovery, "
            << cnt << " entry candidate(s) given");

    // go through entry candidates
    int                 bestLen     = 0;
    int                 bestCost    = INT_MAX;
    unsigned            bestIdx     = 0;
    ShapeProps          bestProps;

    for (unsigned idx = 0; idx < cnt; ++idx) {

        // go through binding candidates
        const SegCandidate &segc = candidates[idx];
        BOOST_FOREACH(const ShapeProps &props, segc.propsList) {
            TRankMap rMap;
            segDiscover(rMap, sh, props, segc.entry);

            // go through all cost/length pairs
            BOOST_FOREACH(TRankMap::const_reference rank, rMap) {
                const int len = rank.second;
                if (len <= 0)
                    continue;

                int cost = rank.first;
#if SE_COST_OF_SEG_INTRODUCTION
                if (!segOnPath(sh, props.bOff, segc.entry, len))
                    cost += (SE_COST_OF_SEG_INTRODUCTION);
#endif

                if (len < minLengthByCost(cost))
                    // too short path at this cost level
                    continue;

                if (bestCost < cost)
                    // we already got something cheaper
                    continue;

                if (len <= bestLen)
                    // we already got something longer
                    continue;

                // update best candidate
                bestIdx = idx;
                bestLen = len;
                bestCost = cost;
                bestProps = props;
            }
        }
    }

    if (!bestLen) {
        CL_DEBUG("<-- no new segment found");
        return false;
    }

    // pick up the best candidate
    pDst->entry = candidates[bestIdx].entry;
    pDst->props = bestProps;
    pDst->length = bestLen;
    return true;
}

bool discoverBestAbstraction(Shape *pDst, SymHeap &sh)
{
    TSegCandidateList candidates;

    // go through all potential segment entries
    TObjList heapObjs;
    sh.gatherObjects(heapObjs, isOnHeap);
    BOOST_FOREACH(const TObjId obj, heapObjs) {
        /// probe neighbouring objects
        SegCandidate segc;
        digShapePropsCandidates(&segc.propsList, sh, obj);
        if (segc.propsList.empty())
            // found nothing
            continue;

        // append a segment candidate
        segc.entry = obj;
        candidates.push_back(segc);
    }

    return selectBestAbstraction(pDst, sh, candidates);
}
