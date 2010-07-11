/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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
#include "symabstract.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "symgc.hh"
#include "symutil.hh"
#include "util.hh"

#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef SE_DISABLE_DLS
#   define SE_DISABLE_DLS 0
#endif

#ifndef SE_DISABLE_SLS
#   define SE_DISABLE_SLS 0
#endif

/// common configuration template for abstraction triggering
struct AbstractionThreshold {
    unsigned sparePrefix;
    unsigned innerSegLen;
    unsigned spareSuffix;
};

/// abstraction trigger threshold for SLS
static struct AbstractionThreshold slsThreshold = {
    /* sparePrefix */ 1,
    /* innerSegLen */ 1,
    /* spareSuffix */ 0
};

/// abstraction trigger threshold for DLS
static struct AbstractionThreshold dlsThreshold = {
    /* sparePrefix */ 1,
    /* innerSegLen */ 1,
    /* spareSuffix */ 1
};

typedef std::pair<TObjId, TObjId> TObjPair;

// helper template for traverseSubObjs()
template <class TItem> struct TraverseSubObjsHelper { };

// specialisation for TObjId, which means basic implementation of the traversal
template <> struct TraverseSubObjsHelper<TObjId> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjId obj) {
        return sh.objType(obj);
    }
    static TObjId getNextItem(const SymHeap &sh, TObjId obj, int nth) {
        return sh.subObj(obj, nth);
    }
};

// specialisation suitable for traversing two composite objects simultaneously
template <> struct TraverseSubObjsHelper<TObjPair> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjPair item) {
        const struct cl_type *clt1 = sh.objType(item.first);
        const struct cl_type *clt2 = sh.objType(item.second);
        if (clt1 != clt2)
            TRAP;

        return clt1;
    }
    static TObjPair getNextItem(const SymHeap &sh, TObjPair item, int nth) {
        item.first  = sh.subObj(item.first,  nth);
        item.second = sh.subObj(item.second, nth);
        return item;
    }
};

// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, class TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjs(THeap &sh, TItem item, TVisitor visitor) {
    std::stack<TItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, item);
        if (!clt || clt->code != CL_TYPE_STRUCT)
            TRAP;

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TItem next = THelper::getNextItem(sh, item, i);
            if (!/* continue */visitor(sh, next))
                return false;

            const struct cl_type *subClt = THelper::getItemClt(sh, next);
            if (subClt && subClt->code == CL_TYPE_STRUCT)
                todo.push(next);
        }
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

namespace {

// we use a controlled recursion of depth 1
void flatScan(SymHeap &sh, EObjKind kind, TObjId obj);

bool doesAnyonePointToInsideVisitor(const SymHeap &sh, TObjId sub) {
    const TValueId subAddr = sh.placedAt(sub);
    return /* continue */ !sh.usedByCount(subAddr);
}

bool doesAnyonePointToInside(const SymHeap &sh, TObjId obj) {
    return !traverseSubObjs(sh, obj, doesAnyonePointToInsideVisitor);
}

TValueId /* addr */ segClone(SymHeap &sh, TValueId atAddr) {
    const TObjId seg = sh.pointsTo(atAddr);
    const EObjKind kind = sh.objKind(seg);
    if (OK_SLS != kind) {
        CL_ERROR("segment cloning implemented only for SLS");
        TRAP;
    }

    const TObjId dup = sh.objDup(seg);
    return sh.placedAt(dup);
}

TValueId /* new addr */ considerFlatScan(SymHeap &sh, TObjId obj) {
    const TValueId addr = sh.placedAt(obj);
    const unsigned cnt = sh.usedByCount(addr);

    TObjId refObj = OBJ_INVALID;
    if (0 < cnt) {
        // keep track of at least one pointing object
        SymHeapCore::TContObj refs;
        sh.usedBy(refs, addr);
        refObj = refs.at(0);
    }

    switch (cnt) {
        case 0:
            CL_WARN("considerFlatScan() encountered an unused root "
                    "object #" << obj);
            return addr;

        case 1:
            // we use a controlled recursion of depth 1
            flatScan(sh, OK_SLS, obj);
            break;

        case 2:
            // we use a controlled recursion of depth 1
            flatScan(sh, OK_DLS, obj);
            break;
    }

    // attempt to search the new address
    const TValueId newAddr = sh.valueOf(refObj);
    if (newAddr <= 0)
        TRAP;

    if (newAddr != addr)
        CL_DEBUG("flatScan(): address of the scanned object has been changed");

    return newAddr;
}

void considerFlatScan(SymHeap &sh, TValueId *val1, TValueId *val2) {
    const EUnknownValue code1 = sh.valGetUnknown(*val1);
    if (UV_KNOWN != code1 && UV_ABSTRACT != code1)
        // too fuzzy for us
        return;

    const EUnknownValue code2 = sh.valGetUnknown(*val2);
    if (UV_KNOWN != code2 && UV_ABSTRACT != code2)
        // too fuzzy for us
        return;

    const TObjId o1 = sh.pointsTo(*val1);
    const TObjId o2 = sh.pointsTo(*val2);
    if (o1 <= 0 || o2 <= 0)
        // there is no valid target --> nothing to abstract actually
        return;

    *val1 = considerFlatScan(sh, o1);
    *val2 = considerFlatScan(sh, o2);
}

TValueId mergeValues(SymHeap &sh, TValueId v1, TValueId v2) {
    if (v1 == v2)
        return v1;

    // attempt to dig some type-info for the new unknown value
    const struct cl_type *clt1 = sh.valType(v1);
    const struct cl_type *clt2 = sh.valType(v2);
    if (clt1 && clt2 && clt1 != clt2)
        // should be safe to ignore
        TRAP;

    // if we know type of at least one of the values, use it
    const struct cl_type *clt = (clt1)
        ? clt1
        : clt2;

    // if the types of _unknown_ values are compatible, it should be safe to
    // pass it through;  UV_UNKNOWN otherwise
    const EUnknownValue code1 = sh.valGetUnknown(v1);
    const EUnknownValue code2 = sh.valGetUnknown(v2);
    EUnknownValue code = (code1 != UV_KNOWN && code1 == code2)
        ? code1
        : UV_UNKNOWN;

    if (UV_ABSTRACT == code) {
        CL_WARN("support for nested segments is not fully implemented yet");
        if (1 != sh.usedByCount(v1))
            CL_NOTE("even worse with DLS abstraction nesting");

        if (segEqual(sh, v1, v2)) {
            // by merging the values, we drop the last reference;  destroy the seg
            const TObjId seg = sh.pointsTo(v1);
            segDestroy(sh, seg);
            return VAL_INVALID;
        }
        else
            // segments are not equal, no chance to merge them
            code = UV_UNKNOWN;
    }

    return sh.valCreateUnknown(code, clt);
}

// visitor
struct ValueAbstractor {
    std::set<TObjId>    ignoreList;
    bool                flatScan;

    bool operator()(SymHeap &sh, TObjPair item) const {
        const TObjId dst = item.second;
        if (hasKey(ignoreList, dst))
            return /* continue */ true;

        TValueId valSrc = sh.valueOf(item.first);
        TValueId valDst = sh.valueOf(dst);
        bool eq;
        if (sh.proveEq(&eq, valSrc, valDst) && eq)
            // values are equal
            return /* continue */ true;

        if (!flatScan)
            // prepare any nested abstractions for collapsing eventually
            considerFlatScan(sh, &valSrc, &valDst);

        // create a new unknown value as a placeholder
        const TValueId valNew = mergeValues(sh, valSrc, valDst);
        if (VAL_INVALID == valNew)
            return /* continue */ true;

        sh.objSetValue(dst, valNew);

        // if the last reference is gone, we have a problem
        if (collectJunk(sh, valDst))
            CL_ERROR("junk detected during abstraction"
                    ", the analysis is no more sound!");

        return /* continue */ true;
    }
};

// visitor
struct UnknownValuesDuplicator {
    std::set<TObjId> ignoreList;

    bool operator()(SymHeap &sh, TObjId obj) const {
        if (hasKey(ignoreList, obj))
            return /* continue */ true;

        const TValueId valOld = sh.valueOf(obj);
        if (valOld <= 0)
            return /* continue */ true;

        // branch by _unknown_ value type
        TValueId valNew = VAL_INVALID;
        const EUnknownValue code = sh.valGetUnknown(valOld);
        switch (code) {
            case UV_KNOWN:
                // we can keep known values as they are (shared data)
                break;

            case UV_ABSTRACT:
                // we need to clone nested list segments
                valNew = segClone(sh, valOld);
                break;

            default:
                valNew = sh.valDuplicateUnknown(valOld);
        }

        // duplicate any unknown value
        if (VAL_INVALID != valNew)
            sh.objSetValue(obj, valNew);

        return /* continue */ true;
    }
};

template <class TIgnoreList>
void buildIgnoreList(const SymHeap &sh, TObjId obj, TIgnoreList &ignoreList) {
    TObjId tmp;

    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of buildIgnoreList()
            TRAP;

        case OK_DLS:
            // preserve 'peer' field
            tmp = subObjByChain(sh, obj, sh.objPeerField(obj));
            ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
            // preserve 'next' field
            tmp = subObjByChain(sh, obj, sh.objNextField(obj));
            ignoreList.insert(tmp);
    }
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst,
                               bool flatScan)
{
    ValueAbstractor visitor;
    visitor.flatScan = flatScan;
    buildIgnoreList(sh, dst, visitor.ignoreList);

    // traverse all sub-objects
    const TObjPair item(src, dst);
    traverseSubObjs(sh, item, visitor);
}

void abstractNonMatchingValuesBidir(SymHeap &sh, TObjId o1, TObjId o2,
                                    bool flatScan)
{
    abstractNonMatchingValues(sh, o1, o2, flatScan);
    abstractNonMatchingValues(sh, o2, o1, flatScan);
}

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TObjId obj) {
    UnknownValuesDuplicator visitor;
    buildIgnoreList(sh, obj, visitor.ignoreList);

    // traverse all sub-objects
    traverseSubObjs(sh, obj, visitor);
}

class ProbeVisitor {
    private:
        TValueId                addr_;
        const struct cl_type    *clt_;
        unsigned                arrity_;
        TFieldIdxChain          icNext_;

        bool dlSegEndCandidate(const SymHeap &sh, TObjId obj) const {
            if (OK_DLS != static_cast<EObjKind>(arrity_) || icNext_.empty())
                // we are not looking for a DLS either
                return false;

            const TObjId root = sh.pointsTo(addr_);
            TObjId nextPtr = subObjByChain(sh, root, icNext_);
            obj = sh.pointsTo(sh.valueOf(nextPtr));
            if (obj <= 0)
                // no valid next object
                return false;

            // if the next object has zero as 'next', give it a chance to go
            nextPtr = subObjByChain(sh, obj, icNext_);
            return (VAL_NULL == sh.valueOf(nextPtr));
        }

    public:
        ProbeVisitor(const SymHeap &sh, TObjId root, EObjKind kind,
                     TFieldIdxChain icNext = TFieldIdxChain()):
            icNext_(icNext)
        {
            addr_ = sh.placedAt(root);
            clt_  = sh.objType(root);
            if (!addr_ || !clt_ || CL_TYPE_STRUCT != clt_->code)
                TRAP;

            arrity_ = static_cast<unsigned>(kind);
            if (!arrity_)
                TRAP;
        }

    bool operator()(const SymHeap &sh, TObjId obj) const {
        const TValueId valNext = sh.valueOf(obj);
        if (valNext <= 0 || valNext == addr_ || sh.valType(valNext) != clt_)
            return /* continue */ true;

        const EUnknownValue code = sh.valGetUnknown(valNext);
        switch (code) {
            case UV_KNOWN:
            case UV_ABSTRACT:
                // only known objects can be chained
                break;

            default:
                return /* continue */ true;
        }

        const TObjId target = sh.pointsTo(valNext);
        const TValueId targetAddr = sh.placedAt(target);
        if (targetAddr <= 0)
            // someone points to an already deleted object
            return /* continue */ true;

        if (sh.cVar(0, obj))
            // a list segment through non-heap objects basically makes no sense
            return /* continue */ true;

        // special quirk for DLS "end"
        const unsigned refs = sh.usedByCount(targetAddr);
        if (refs != arrity_ && (1 != refs || !this->dlSegEndCandidate(sh, obj)))
            return /* continue */ true;

        return doesAnyonePointToInside(sh, target);
    }
};

bool probe(SymHeap &sh, TObjId obj, EObjKind kind) {
    if (doesAnyonePointToInside(sh, obj))
        return false;

    const ProbeVisitor visitor(sh, obj, kind);
    return !traverseSubObjs(sh, obj, visitor);
}

// TODO: hook this somehow on the existing visitor infrastructure in order
//       to avoid code duplicity ... challenge? ;-)
//
// NOTE: we have basically the same code in SymHeapPlotter::Private::digObj()
template <class TDst>
void digAnyListSelectors(TDst &dst, const SymHeap &sh, TObjId obj,
                         EObjKind kind)
{
    const ProbeVisitor visitor(sh, obj, kind);
    TFieldIdxChain ic;

    typedef boost::tuple<TObjId, int /* nth */, bool /* last */> TStackItem;
    std::stack<TStackItem> todo;
    todo.push(TStackItem(obj, -1, false));
    while (!todo.empty()) {
        bool last;
        int nth;
        boost::tie(obj, nth, last) = todo.top();
        todo.pop();

        const struct cl_type *clt = sh.objType(obj);
        if (clt && clt->code == CL_TYPE_STRUCT) {
            if (-1 != nth)
                // nest into structure
                ic.push_back(nth);

            for (int i = 0; i < clt->item_cnt; ++i) {
                const TObjId sub = sh.subObj(obj, i);
                const struct cl_type *subClt = sh.objType(sub);
                const bool backLinkCandidate = (VAL_NULL == sh.valueOf(sub)
                        && subClt && subClt->code == CL_TYPE_PTR
                        && subClt->items[0].type == clt);

                if (backLinkCandidate || !visitor(sh, sub)) {
                    // great, we have a candidate
                    ic.push_back(i);
                    dst.push_back(ic);
                    ic.pop_back();
                }

                if (subClt && subClt->code == CL_TYPE_STRUCT)
                    todo.push(TStackItem(sub, i, /* last */ (0 == i)));
            }
        }

        if (last)
            // leave the structure
            ic.pop_back();
    }
}

// FIXME: this function tends to be crowded
// TODO: split to some reasonable functions
unsigned /* len */ segDiscover(const SymHeap &sh, TObjId entry, EObjKind kind,
                               TFieldIdxChain icNext, TFieldIdxChain icPrev)
{
    int dlSegsOnPath = 0;

    // we use std::set to avoid an infinite loop
    TObjId obj = entry;
    std::set<TObjId> path;
    while (!hasKey(path, obj)) {
        path.insert(obj);
        const TObjId objCurrent = obj;

        const EObjKind kindEncountered = sh.objKind(obj);
        if (OK_DLS == kindEncountered) {
            // we've hit an already existing DLS on path, let's handle it such
            if (OK_DLS != kind)
                // arrity vs. kind mismatch
                break;

            // check selectors
            const TFieldIdxChain icPeerEncountered = sh.objPeerField(obj);
            if (icPeerEncountered != icNext && icPeerEncountered != icPrev)
                // completely incompatible DLS, it gives us no go
                break;

            // jump to peer
            skipObj(sh, &obj, sh.objPeerField(obj));
            if (hasKey(path, obj))
                // we came from the wrong side this time
                break;

            path.insert(obj);
            dlSegsOnPath++;
        }

        const TObjId objPtrNext = subObjByChain(sh, obj, icNext);
        const ProbeVisitor visitor(sh, obj, kind, icNext);
        if (visitor(sh, objPtrNext))
            // we can't go further
            break;

        const TValueId valNext = sh.valueOf(objPtrNext);
        const TObjId objNext = sh.pointsTo(valNext);
        if (objNext <= 0)
            // there is no valid next object
            break;

        if (OK_DLS == kind) {
            // check the back-link
            const TValueId addrSelf = sh.placedAt(obj);
            const TObjId objBackLink = subObjByChain(sh, objNext, icPrev);
            const TValueId valBackLink = sh.valueOf(objBackLink);
            if (objCurrent == entry && VAL_NULL == valBackLink)
                // we allow VAL_NULL as backLink in the first item of DLL
                goto blink_ok;

            if (valBackLink != addrSelf)
                // inappropriate back-link
                break;
        }

blink_ok:
        obj = objNext;
    }

    // path consisting of N nodes has N-1 edges
    const unsigned rawPathLen = path.size() - 1;

    // each DLS consists of two nodes
    return rawPathLen - dlSegsOnPath;
}

template <class TSelectorList>
unsigned segDiscoverAll(const SymHeap &sh, const TObjId entry, EObjKind kind,
                        const TSelectorList &selectors, TFieldIdxChain *icNext,
                        TFieldIdxChain *icPrev)
{
    // check count of the selectors
    const unsigned cnt = selectors.size();
    CL_DEBUG("    --> found " << cnt << " list selector candidate(s)");
    if (!cnt)
        // why are we called actually?
        TRAP;

    unsigned prevMax = cnt;
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segDiscoverAll()
            TRAP;

        case OK_SLS:
            // just choose one selector
            prevMax = /* dummy value */ 1;
            break;

        case OK_DLS:
            // we need at least 2 selectors for DLS abstraction
            if (cnt < 2)
                return /* found nothing */ 0;
    }

    // if (2 < cnt), try all possible combinations in case of DLS
    // NOTE: This may take some time...
    unsigned bestLen = 0, bestNext, bestPrev;
    for (unsigned next = 0; next < cnt; ++next) {
        for (unsigned prev = 0; prev < prevMax; ++prev) {
            if (OK_DLS == kind && next == prev)
                // we need two _distinct_ selectors for a DLS
                continue;

            const unsigned len = segDiscover(sh, entry, kind,
                                             selectors[next],
                                             selectors[prev]);
            if (!len)
                continue;

            CL_DEBUG("        --> found segment of length " << len);
            if (bestLen < len) {
                bestLen = len;
                bestNext = next;
                bestPrev = prev;
            }
        }
    }

    if (!bestLen) {
        CL_DEBUG("        <-- no segment found");
        return /* not found */ 0;
    }

    // something found
    *icNext = selectors[bestNext];
    *icPrev = selectors[bestPrev];
    return bestLen;
}

void slSegCreateIfNeeded(SymHeap &sh, TObjId obj, TFieldIdxChain icNext) {
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_SLS:
            // already abstract, check the next pointer
            if (sh.objNextField(obj) == icNext)
                // all OK
                return;
            // fall through!

        case OK_DLS:
            TRAP;
            // fall through!

        case OK_CONCRETE:
            break;
    }

    // abstract a concrete object
    sh.objAbstract(obj, OK_SLS, icNext);

    // we're constructing the abstract object from a concrete one --> it
    // implies non-empty LS at this point
    const TValueId addr = sh.placedAt(obj);
    const TObjId objNextPtr = subObjByChain(sh, obj, icNext);
    const TValueId valNext = sh.valueOf(objNextPtr);
    if (addr <= 0 || valNext < /* we allow VAL_NULL here */ 0)
        TRAP;
    sh.addNeq(addr, valNext);
}

void slSegAbstractionStep(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext,
                          bool flatScan)
{
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TValueId valNext = sh.valueOf(objPtrNext);
    if (valNext <= 0 || 1 != sh.usedByCount(valNext))
        // this looks like a failure of segDiscover()
        TRAP;

    // make sure the next object is abstract
    const TObjId objNext = sh.pointsTo(valNext);
    slSegCreateIfNeeded(sh, objNext, icNext);
    if (OK_SLS != sh.objKind(objNext))
        TRAP;

    // replace self by the next object
    abstractNonMatchingValues(sh, *pObj, objNext, flatScan);
    objReplace(sh, *pObj, objNext);

    // move to the next object
    *pObj = objNext;
}

void dlSegCreate(SymHeap &sh, TObjId o1, TObjId o2,
                 TFieldIdxChain icNext, TFieldIdxChain icPrev, bool flatScan)
{
    if (OK_CONCRETE != sh.objKind(o1) || OK_CONCRETE != sh.objKind(o2))
        // invalid call of dlSegCreate()
        TRAP;

    sh.objAbstract(o1, OK_DLS, icPrev, icNext);
    sh.objAbstract(o2, OK_DLS, icNext, icPrev);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValuesBidir(sh, o1, o2, flatScan);

    // a just created DLS is said to be non-empty
    dlSegHandleCrossNeq(sh, o1, &SymHeapCore::addNeq);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId var, bool backward,
                 bool flatScan)
{
    if (OK_DLS != sh.objKind(dls) || OK_CONCRETE != sh.objKind(var))
        // invalid call of dlSegGobble()
        TRAP;

    // kill Neq if any
    dlSegHandleCrossNeq(sh, dls, &SymHeap::delNeq);

    if (!backward)
        // jump to peer
        skipObj(sh, &dls, sh.objPeerField(dls));

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls, flatScan);

    // store the pointer DLS -> VAR
    const TFieldIdxChain icNext = sh.objNextField(dls);
    const TObjId dlsNextPtr = subObjByChain(sh, dls, icNext);
    const TObjId varNextPtr = subObjByChain(sh, var, icNext);
    sh.objSetValue(dlsNextPtr, sh.valueOf(varNextPtr));

    // replace VAR by DLS
    objReplace(sh, var, dls);

    // we've just added an object, the DLS can't be empty
    dlSegHandleCrossNeq(sh, dls, &SymHeap::addNeq);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2, bool flatScan) {
    // the resulting DLS will be non-empty as long as at least one of the given
    // DLS is non-empty
    const bool ne = dlSegNotEmpty(sh, seg1) || dlSegNotEmpty(sh, seg2);
    if (ne) {
        dlSegHandleCrossNeq(sh, seg1, &SymHeap::delNeq);
        dlSegHandleCrossNeq(sh, seg2, &SymHeap::delNeq);
    }

    if (sh.objNextField(seg1) != sh.objNextField(seg2)
            || (sh.objPeerField(seg1) != sh.objPeerField(seg2)))
        // failure of segDiscover()?
        TRAP;

    const TObjId peer1 = dlSegPeer(sh, seg1);
    const TObjId nextPtr = nextPtrFromSeg(sh, peer1);
    const TValueId valNext = sh.valueOf(nextPtr);
    if (valNext != sh.placedAt(seg2))
        TRAP;

    const TObjId peer2 = dlSegPeer(sh, seg2);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValuesBidir(sh,  seg1,  seg2, flatScan);
    abstractNonMatchingValuesBidir(sh, peer1, peer2, flatScan);

    // preserve backLink
    const TValueId valNext2 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    sh.objSetValue(nextPtrFromSeg(sh, seg2), valNext2);

    // replace both parts point-wise
    objReplace(sh,  seg1,  seg2);
    objReplace(sh, peer1, peer2);

    if (ne)
        // non-empty DLS
        dlSegHandleCrossNeq(sh, seg2, &SymHeap::addNeq);
}

void dlSegAbstractionStep(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext,
                          TFieldIdxChain icPrev, bool flatScan)
{
    // the first object is clear
    const TObjId o1 = *pObj;

    // we'll find the next one later on
    TObjId o2 = o1;

    EObjKind kind = sh.objKind(o1);
    switch (kind) {
        case OK_SLS:
            // *** segDiscover() failure detected ***
            TRAP;

        case OK_DLS:
            // jump to peer
            o2 = dlSegPeer(sh, o2);

            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, sh.objNextField(o2));
            if (OK_CONCRETE == sh.objKind(o2)) {
                // DLS + VAR
                dlSegGobble(sh, o1, o2, /* backward */ false, flatScan);
                return;
            }

            // DLS + DLS
            dlSegMerge(sh, o1, o2, flatScan);
            break;

        case OK_CONCRETE:
            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, icNext);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // VAR + VAR
                dlSegCreate(sh, o1, o2, icNext, icPrev, flatScan);
                dlSegNotEmpty(sh, o1);
                return;
            }

            // VAR + DLS
            dlSegGobble(sh, o2, o1, /* backward */ true, flatScan);
            break;
    }

    // the current object has been just consumed, move to the next one
    *pObj = o2;

    // just check if the Neq predicates work well so far (safe to remove when
    // not debugging)
    (void) dlSegNotEmpty(sh, o2);
}

bool considerSegAbstraction(SymHeap &sh, TObjId obj, EObjKind kind,
                            TFieldIdxChain icNext, TFieldIdxChain icPrev,
                            unsigned lenTotal, bool flatScan)
{
    // select the appropriate threshold for the given type of abstraction
    AbstractionThreshold at;
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of considerSegAbstraction()
            TRAP;

        case OK_SLS:
            at = slsThreshold;
            break;

        case OK_DLS:
            at = dlsThreshold;
            break;
    }

    // check whether the threshold is satisfied or not
    unsigned threshold = at.sparePrefix + at.innerSegLen + at.spareSuffix;
    if (flatScan) {
        CL_DEBUG("    FFF ignoring threshold cfg during flat scan!");
        threshold = 1;
    }

    if (lenTotal < threshold) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << threshold << ")");
        return false;
    }

    int len = lenTotal;
    if (!flatScan) {
        CL_DEBUG("    --- length of the longest segment is " << lenTotal
                << ", prefix=" << at.sparePrefix
                << ", suffix=" << at.spareSuffix);

        // handle sparePrefix/spareSuffix
        len = lenTotal - at.sparePrefix - at.spareSuffix;
        for (unsigned i = 0; i < at.sparePrefix; ++i)
            skipObj(sh, &obj, icNext);
    }

    CL_DEBUG("    AAA initiating abstraction of length " << len);

    if (OK_SLS == kind) {
        // perform SLS abstraction!
        for (int i = 0; i < len; ++i)
            slSegAbstractionStep(sh, &obj, icNext, flatScan);

        CL_DEBUG("<-- successfully abstracted SLS");
        return true;
    }
    else {
        // perform DLS abstraction!
        for (int i = 0; i < len; ++i)
            dlSegAbstractionStep(sh, &obj, icNext, icPrev, flatScan);

        CL_DEBUG("<-- successfully abstracted DLS");
        return true;
    }
}

template <class TCont>
bool considerAbstraction(SymHeap &sh, EObjKind kind, TCont entries,
                         bool flatScan = false)
{
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of considerAbstraction()
            TRAP;

        case OK_SLS:
            CL_DEBUG("--> considering SLS abstraction...");
            break;

        case OK_DLS:
            CL_DEBUG("--> considering DLS abstraction...");
            break;
    }

    // go through all candidates and find the best possible abstraction
    std::vector<TFieldIdxChain> bestSelectors;
    TFieldIdxChain bestNext, bestPrev;
    TObjId bestEntry;
    unsigned bestLen = 0;
    BOOST_FOREACH(const TObjId obj, entries) {
        // gather suitable selectors
        std::vector<TFieldIdxChain> selectors;
        digAnyListSelectors(selectors, sh, obj, kind);

        // run the LS discovering process
        TFieldIdxChain icNext, icPrev;
        const unsigned len = segDiscoverAll(sh, obj, kind, selectors,
                                            &icNext, &icPrev);

        if (len <= bestLen)
            continue;

        bestLen         = len;
        bestEntry       = obj;
        bestSelectors   = selectors;
        bestNext        = icNext;
        bestPrev        = icPrev;
    }
    if (!bestLen) {
        CL_DEBUG("<-- nothing useful found");
        return false;
    }

    // consider abstraction threshold and trigger the abstraction eventually
    return considerSegAbstraction(sh, bestEntry, kind, bestNext, bestPrev,
                                  bestLen, flatScan);
}

void flatScan(SymHeap &sh, EObjKind kind, TObjId obj) {
    if (!probe(sh, obj, kind))
        return;

    SymHeapCore::TContObj cont;
    cont.push_back(obj);
    considerAbstraction(sh, kind, cont, /* flatScan */ true);
}

bool abstractIfNeededCore(SymHeap &sh) {
    SymHeapCore::TContObj slSegEntries;
    SymHeapCore::TContObj dlSegEntries;

    // collect all possible SLS/DLS entries
    SymHeapCore::TContObj roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        if (sh.cVar(0, obj))
            // skip static/automatic objects
            continue;

        const TValueId addr = sh.placedAt(obj);
        if (VAL_INVALID ==addr)
            continue;

        const unsigned uses = sh.usedByCount(addr);
        switch (uses) {
            case 0:
                CL_WARN("abstractIfNeededLoop() encountered an unused root "
                        "object #" << obj);
                // fall through!

            default:
                continue;

            case 1:
#if !SE_DISABLE_SLS
                if (probe(sh, obj, OK_SLS))
                    // a candidate for SLS entry
                    slSegEntries.push_back(obj);
#endif
                break;

            case 2:
#if !SE_DISABLE_DLS
                if (probe(sh, obj, OK_DLS))
                    // a candidate for DLS entry
                    dlSegEntries.push_back(obj);
#endif
                break;
        }
    }

    // TODO: check if the order of following two steps is anyhow important
    if (!slSegEntries.empty() && considerAbstraction(sh, OK_SLS, slSegEntries))
        return true;

    if (!dlSegEntries.empty() && considerAbstraction(sh, OK_DLS, dlSegEntries))
        return true;

    // no hit
    return false;
}

void spliceOutListSegmentCore(SymHeap &sh, TObjId obj, TObjId peer)
{
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);

    if (obj != peer) {
        // OK_DLS --> destroy peer
        const TFieldIdxChain icPrev = sh.objNextField(obj);
        const TValueId valPrev = sh.valueOf(subObjByChain(sh, obj, icPrev));
        sh.valReplace(sh.placedAt(peer), valPrev);
        sh.objDestroy(peer);
    }

    // destroy self
    const TValueId addr = sh.placedAt(obj);
    sh.valReplace(addr, valNext);
    sh.objDestroy(obj);
}

void spliceOutSegmentIfNeeded(SymHeap &sh, TObjId ao, TObjId peer,
                              TSymHeapList &todo)
{
    // check if the LS may be empty
    const TValueId addrSelf = sh.placedAt(ao);
    if (segNotEmpty(sh, ao)) {
        // the segment was _guaranteed_ to be non-empty now, but
        // the concretization makes it _possibly_ empty
        // --> remove the Neq predicate 
        if (ao == peer) {
            // SLS
            const TObjId next = nextPtrFromSeg(sh, ao);
            const TValueId nextVal = sh.valueOf(next);
            sh.delNeq(addrSelf, nextVal); 
        }
        else
            // DLS
            dlSegHandleCrossNeq(sh, ao, &SymHeapCore::delNeq);

        return;
    }

    // possibly empty LS
    SymHeap sh0(sh);
    spliceOutListSegmentCore(sh0, ao, peer);
    todo.push_back(sh0);
}

} // namespace

void abstractIfNeeded(SymHeap &sh) {
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    (void) sh;
#else
    while (abstractIfNeededCore(sh))
        ;
#endif
}

void concretizeObj(SymHeap &sh, TValueId addr, TSymHeapList &todo) {
    const TObjId obj = sh.pointsTo(addr);
    TObjId peer = obj;

    // branch by SLS/DLS
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of concretizeObj()
            TRAP;

        case OK_SLS:
            break;

        case OK_DLS:
            // jump to peer
            skipObj(sh, &peer, sh.objPeerField(obj));
            break;
    }

    // handle the possibly empty variant (if exists)
    spliceOutSegmentIfNeeded(sh, obj, peer, todo);

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(obj);
    const TValueId aoDupAddr = sh.placedAt(aoDup);
    if (OK_DLS == kind) {
        // DLS relink
        const TObjId peerField = subObjByChain(sh, peer, sh.objPeerField(peer));
        sh.objSetValue(peerField, aoDupAddr);
    }

    // duplicate all unknown values, to keep the prover working
    duplicateUnknownValues(sh, obj);

    // concretize self and recover the list
    const TObjId ptrNext = subObjByChain(sh, obj, (OK_SLS == kind)
            ? sh.objNextField(obj)
            : sh.objPeerField(obj));
    sh.objConcretize(obj);
    sh.objSetValue(ptrNext, aoDupAddr);

    if (OK_DLS == kind) {
        // update DLS back-link
        const TFieldIdxChain icPrev = sh.objNextField(aoDup);
        const TObjId backLink = subObjByChain(sh, aoDup, icPrev);
        sh.objSetValue(backLink, sh.placedAt(obj));
    }
}

bool spliceOutListSegment(SymHeap &sh, TValueId atAddr, TValueId pointingTo)
{
    const TObjId obj = sh.pointsTo(atAddr);
    const EObjKind kind = sh.objKind(obj);
    const TObjId peer = (OK_DLS == kind)
        ? dlSegPeer(sh, obj)
        : obj;

    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);
    if (valNext != pointingTo)
        return false;

    spliceOutListSegmentCore(sh, obj, peer);
    return true;
}

bool haveDlSeg(SymHeap &sh, TValueId atAddr, TValueId pointingTo) {
    if (UV_ABSTRACT != sh.valGetUnknown(atAddr))
        // not an abstract object
        return false;

    const TObjId seg = sh.pointsTo(atAddr);
    if (OK_DLS != sh.objKind(seg))
        // not a DLS
        return false;

    const TObjId peer = dlSegPeer(sh, seg);
    if (OK_DLS != sh.objKind(peer))
        // invalid peer
        return false;

    // compare the end-points
    const TObjId nextPtr = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(nextPtr);
    return (valNext == pointingTo);
}

// FIXME: we use the following nonsense to avoid an infinite recursion through
// a call of virtual method;  we should get rid of this ASAP
struct SymHeapQuirk: public SymHeap {
    SymHeapQuirk(const SymHeap &sh):
        SymHeap(sh)
    {
    }

    virtual void addNeq(TValueId valA, TValueId valB) {
        SymHeapCore::addNeq(valA, valB);
    }

    virtual void delNeq(TValueId valA, TValueId valB) {
        SymHeapCore::delNeq(valA, valB);
    }
};

void dlSegHandleCrossNeq(SymHeap &sh, TObjId dls, TNeqOp op) {
    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // read the values (addresses of the surround)
    const TValueId val1 = sh.valueOf(next1);
    const TValueId val2 = sh.valueOf(next2);

    // FIXME: suboptimal API, we need two nonsense clone operations, in order
    // to avoid an infinite recursion through a virtual call of addNeq/delNeq,
    // ridiculous...
    SymHeapQuirk q(sh);
    (q.*op)(val1, sh.placedAt(peer));
    (q.*op)(val2, sh.placedAt(dls));
    sh = q;
}
