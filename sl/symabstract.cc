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

#include "symproc.hh"       // for checkForJunk()
#include "util.hh"

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
    /* sparePrefix */ 0,    // any non-zero value will break the algorithm
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
template <class THeap, typename TVisitor, class TItem = TObjId>
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

bool doesAnyonePointToInsideVisitor(const SymHeap &sh, TObjId sub) {
    const TValueId subAddr = sh.placedAt(sub);
    return /* continue */ !sh.usedByCount(subAddr);
}

bool doesAnyonePointToInside(const SymHeap &sh, TObjId obj) {
    return !traverseSubObjs(sh, obj, doesAnyonePointToInsideVisitor);
}

// visitor
struct ValueAbstractor {
    std::set<TObjId> ignoreList;

    bool operator()(SymHeap &sh, TObjPair item) {
        const TObjId dst = item.second;
        if (hasKey(ignoreList, dst))
            return /* continue */ true;

        const TValueId valSrc = sh.valueOf(item.first);
        const TValueId valDst = sh.valueOf(dst);
        bool eq;
        if (sh.proveEq(&eq, valSrc, valDst) && eq)
            // values are equal
            return /* continue */ true;

        // attempt to dig some type-info for the new unknown value
        const struct cl_type *clt = sh.valType(valSrc);
        const struct cl_type *cltDst = sh.valType(valDst);
        if (!clt)
            clt = cltDst;
        else if (cltDst && cltDst != clt)
            // should be safe to ignore
            TRAP;

        // create a new unknown value as a placeholder
        const TValueId valNew = sh.valCreateUnknown(UV_UNKNOWN, clt);
        const TValueId oldVal = sh.valueOf(dst);
        sh.objSetValue(dst, valNew);

        // if the last reference is gone, we have a problem
        if (checkForJunk(sh, oldVal))
            CL_ERROR("junk detected during abstraction"
                    ", the analysis is no more sound!");

        return /* continue */ true;
    }
};

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst) {
    ValueAbstractor visitor;
    TObjId tmp;

    // build the ignore-list
    const EObjKind kind = sh.objKind(dst);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of abstractNonMatchingValues()
            TRAP;

        case OK_DLS:
            // preserve 'peer' field
            tmp = subObjByChain(sh, dst, sh.objPeerField(dst));
            visitor.ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
            // preserve 'bind' field
            tmp = subObjByChain(sh, dst, sh.objBinderField(dst));
            visitor.ignoreList.insert(tmp);
    }

    // traverse all sub-objects
    const TObjPair item(src, dst);
    traverseSubObjs(sh, item, visitor);
}

void abstractNonMatchingValuesBidir(SymHeap &sh, TObjId o1, TObjId o2) {
    abstractNonMatchingValues(sh, o1, o2);
    abstractNonMatchingValues(sh, o2, o1);
}

void objReplace(SymHeap &sh, TObjId oldObj, TObjId newObj) {
    if (OBJ_INVALID != sh.objParent(oldObj)
            || OBJ_INVALID != sh.objParent(newObj))
        // attempt to replace a sub-object
        TRAP;

    // resolve object addresses
    const TValueId oldAddr = sh.placedAt(oldObj);
    const TValueId newAddr = sh.placedAt(newObj);
    if (oldAddr <= 0 || newAddr <= 0)
        TRAP;

    // update all references
    sh.valReplace(oldAddr, newAddr);

    // now destroy the old object
    sh.objDestroy(oldObj);
}

void skipObj(const SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext) {
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TValueId valNext = sh.valueOf(objPtrNext);
    const TObjId objNext = sh.pointsTo(valNext);
    if (OBJ_INVALID == objNext)
        TRAP;

    // move to the next object
    *pObj = objNext;
}

TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg) {
    if (OK_CONCRETE == sh.objKind(seg))
        // invalid call of lSegNext()
        TRAP;

    const TFieldIdxChain icBind = sh.objBinderField(seg);
    return subObjByChain(sh, seg, icBind);
}

TObjId dlSegPeer(const SymHeap &sh, TObjId dls) {
    if (OK_DLS != sh.objKind(dls))
        // invalid call of dlSegPeer()
        TRAP;

    TObjId peer = dls;
    skipObj(sh, &peer, sh.objPeerField(dls));
    return peer;
}

bool proveNeq(const SymHeap &sh, TValueId v1, TValueId v2) {
    bool eq;
    if (!sh.proveEq(&eq, v1, v2))
        return /* no idea */ false;

    if (eq)
        // equal ... basically means 'invalid LS'
        TRAP;

    return /* not equal */ true;
}

bool dlSegNotEmpty(const SymHeap &sh, TObjId dls) {
    if (OK_DLS != sh.objKind(dls))
        // invalid call of dlSegNotEmpty()
        TRAP;

    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // red the values (addresses of the surround)
    const TValueId val1 = sh.valueOf(next1);
    const TValueId val2 = sh.valueOf(next2);

    // attempt to prove both
    const bool ne1 = proveNeq(sh, val1, sh.placedAt(peer));
    const bool ne2 = proveNeq(sh, val2, sh.placedAt(dls));
    if (ne1 && ne2)
        return /* not empty */ true;

    if (!ne1 && !ne2)
        return /* possibly empty */ false;

    // the given DLS is guaranteed to be non empty in one direction, but not
    // vice versa --> such a DLS is considered as mutant and should be not
    // passed through
    TRAP;
    return false;
}

bool segNotEmpty(const SymHeap &sh, TObjId seg) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of segNotEmpty()
            TRAP;

        case OK_SLS:
            break;

        case OK_DLS:
            return dlSegNotEmpty(sh, seg);
    }

    const TObjId next = nextPtrFromSeg(sh, seg);
    const TValueId nextVal = sh.valueOf(next);
    const TValueId addr = sh.placedAt(seg);
    return /* not empty */ proveNeq(sh, addr, nextVal);
}

class ProbeVisitor {
    private:
        TValueId                addr_;
        const struct cl_type    *clt_;
        unsigned                arrity_;

    public:
        ProbeVisitor(const SymHeap &sh, TObjId root, EObjKind kind) {
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

        if (sh.usedByCount(targetAddr) != arrity_)
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
                if (!visitor(sh, sub)) {
                    // great, we have a candidate
                    ic.push_back(i);
                    dst.push_back(ic);
                    ic.pop_back();
                }

                const struct cl_type *subClt = sh.objType(sub);
                if (subClt && subClt->code == CL_TYPE_STRUCT)
                    todo.push(TStackItem(sub, i, /* last */ (0 == i)));
            }
        }

        if (last)
            // leave the structure
            ic.pop_back();
    }
}

unsigned /* len */ segDiscover(const SymHeap &sh, TObjId entry, EObjKind kind,
                               TFieldIdxChain icBind,
                               TFieldIdxChain icPeer = TFieldIdxChain())
{
    int dlSegsOnPath = 0;

    // we use std::set to avoid an infinite loop
    TObjId obj = entry;
    std::set<TObjId> path;
    while (!hasKey(path, obj)) {
        path.insert(obj);

        const EObjKind kindEncountered = sh.objKind(obj);
        if (OK_DLS == kindEncountered) {
            // we've hit an already existing DLS on path, let's handle it such
            if (OK_DLS != kind)
                // arrity vs. kind mismatch
                TRAP;

            // check selectors
            const TFieldIdxChain icPeerEncountered = sh.objPeerField(obj);
            if (icPeerEncountered != icBind && icPeerEncountered != icPeer)
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

        const TObjId objPtrNext = subObjByChain(sh, obj, icBind);
        const ProbeVisitor visitor(sh, obj, kind);
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
            const TObjId objBackLink = subObjByChain(sh, objNext, icPeer);
            const TValueId valBackLink = sh.valueOf(objBackLink);
            if (valBackLink != addrSelf)
                // inappropriate back-link
                break;
        }

        // if there is at least one DLS on the path, we demand that the path
        // begins with a DLS;  otherwise we just ignore the path and wait for
        // a better one
        if (dlSegsOnPath && OK_DLS != sh.objKind(entry))
            return /* not found */ 0;

        obj = objNext;
    }

    // path consisting of N nodes has N-1 edges
    const unsigned rawPathLen = path.size() - 1;

    // each DLS consists of two nodes
    return rawPathLen - dlSegsOnPath;
}

template <class TSelectorList>
unsigned segDiscoverAll(const SymHeap &sh, const TObjId obj, EObjKind kind,
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
            const unsigned len = segDiscover(sh, obj, kind,
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

void slSegCreateIfNeeded(SymHeap &sh, TObjId obj, TFieldIdxChain icBind) {
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_SLS:
            // already abstract, check binder
            if (sh.objBinderField(obj) == icBind)
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
    sh.objAbstract(obj, OK_SLS, icBind);

    // we're constructing the abstract object from a concrete one --> it
    // implies non-empty LS at this point
    const TValueId addr = sh.placedAt(obj);
    const TObjId objNextPtr = subObjByChain(sh, obj, icBind);
    const TValueId valNext = sh.valueOf(objNextPtr);
    if (addr <= 0 || valNext < /* we allow VAL_NULL here */ 0)
        TRAP;
    sh.addNeq(addr, valNext);
}

void slSegAbstractionStep(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext) {
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
    abstractNonMatchingValues(sh, *pObj, objNext);
    objReplace(sh, *pObj, objNext);

    // move to the next object
    *pObj = objNext;
}

template <typename TFun>
void dlSegHandleCrossNeq(SymHeap &sh, TObjId dls, TFun fun) {
    const TObjId peer = dlSegPeer(sh, dls);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(sh, dls);
    const TObjId next2 = nextPtrFromSeg(sh, peer);

    // read the values (addresses of the surround)
    const TValueId val1 = sh.valueOf(next1);
    const TValueId val2 = sh.valueOf(next2);

    // add/del Neq as requested
    (sh.*fun)(val1, sh.placedAt(peer));
    (sh.*fun)(val2, sh.placedAt(dls));
}

void dlSegCreate(SymHeap &sh, TObjId o1, TObjId o2,
                 TFieldIdxChain icNext, TFieldIdxChain icPrev)
{
    if (OK_CONCRETE != sh.objKind(o1) || OK_CONCRETE != sh.objKind(o2))
        // invalid call of dlSegCreate()
        TRAP;

    sh.objAbstract(o1, OK_DLS, icPrev, icNext);
    sh.objAbstract(o2, OK_DLS, icNext, icPrev);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValuesBidir(sh, o1, o2);

    // a just created DLS is said to be non-empty
    dlSegHandleCrossNeq(sh, o1, &SymHeapCore::addNeq);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId var, bool backward) {
    if (OK_DLS != sh.objKind(dls) || OK_CONCRETE != sh.objKind(var))
        // invalid call of dlSegGobble()
        TRAP;

    // kill Neq if any
    dlSegHandleCrossNeq(sh, dls, &SymHeap::delNeq);

    if (!backward)
        // jump to peer
        skipObj(sh, &dls, sh.objPeerField(dls));

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls);

    if (backward)
        // not implemented yet
        TRAP;

    // store the pointer DLS -> VAR
    const TFieldIdxChain icBind = sh.objBinderField(dls);
    const TObjId dlsNextPtr = subObjByChain(sh, dls, icBind);
    const TObjId varNextPtr = subObjByChain(sh, var, icBind);
    sh.objSetValue(dlsNextPtr, sh.valueOf(varNextPtr));

    // replace VAR by DLS
    objReplace(sh, var, dls);

    // we've just added an object, the DLS can't be empty
    dlSegHandleCrossNeq(sh, dls, &SymHeap::addNeq);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2) {
    // the resulting DLS will be non-empty as long as at least one of the given
    // DLS is non-empty
    const bool ne = dlSegNotEmpty(sh, seg1) || dlSegNotEmpty(sh, seg2);
    if (ne) {
        // one of the following calls might be probably optimized out, but
        // premature optimization is the root of all evil...
        dlSegHandleCrossNeq(sh, seg1, &SymHeap::delNeq);
        dlSegHandleCrossNeq(sh, seg2, &SymHeap::delNeq);
    }

    // dig peer objects
    const TObjId peer1 = dlSegPeer(sh, seg1);
    const TObjId peer2 = dlSegPeer(sh, seg2);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValuesBidir(sh, seg1, seg2);
    abstractNonMatchingValuesBidir(sh, peer1, peer2);

    // replace both parts point-wise
    objReplace(sh, seg1, seg2);
    objReplace(sh, peer1, peer2);

    if (ne)
        // non-empty DLS
        dlSegHandleCrossNeq(sh, seg2, &SymHeap::addNeq);
}

void dlSegAbstractionStep(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext,
                          TFieldIdxChain icPrev)
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
            skipObj(sh, &o2, sh.objBinderField(o2));
            if (OK_CONCRETE == sh.objKind(o2)) {
                // DLS + VAR
                dlSegGobble(sh, o1, o2, /* backward */ false);
                return;
            }

            // DLS + DLS
            dlSegMerge(sh, o1, o2);
            return;

        case OK_CONCRETE:
            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, icNext);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // VAR + VAR
                dlSegCreate(sh, o1, o2, icNext, icPrev);
                return;
            }

            // VAR + DLS
            dlSegGobble(sh, o2, o1, /* backward */ true);
            *pObj = o2;
            return;
    }
}

bool considerSegAbstraction(SymHeap &sh, TObjId obj, EObjKind kind,
                            TFieldIdxChain icNext, TFieldIdxChain icPrev,
                            unsigned lenTotal)
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
    static const unsigned threshold = at.sparePrefix + at.innerSegLen
                                    + at.spareSuffix;

    if (lenTotal < threshold) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << threshold << ")");
        return false;
    }

    CL_DEBUG("    --- length of the longest segment is " << lenTotal
            << ", prefix=" << at.sparePrefix
            << ", suffix=" << at.spareSuffix);

    // handle sparePrefix/spareSuffix
    const int len = lenTotal - at.sparePrefix - at.spareSuffix;
    for (int i = 0; i < static_cast<int>(at.sparePrefix); ++i)
        skipObj(sh, &obj, icNext);

    CL_DEBUG("    AAA initiating abstraction of length " << len);

    if (OK_SLS == kind) {
        // perform SLS abstraction!
        for (int i = 0; i < len; ++i)
            slSegAbstractionStep(sh, &obj, icNext);

        CL_DEBUG("<-- successfully abstracted SLS");
        return true;
    }
    else {
        // perform DLS abstraction!
        for (int i = 0; i < len; ++i)
            dlSegAbstractionStep(sh, &obj, icNext, icPrev);

        CL_DEBUG("<-- successfully abstracted DLS");
        return true;
    }
}

template <class TCont>
bool considerAbstraction(SymHeap &sh, EObjKind kind, TCont entries) {
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
                                  bestLen);
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
    if (ao != peer) {
        // OK_DLS --> destroy peer
        const TFieldIdxChain icPrev = sh0.objBinderField(ao);
        const TValueId valPrev = sh0.valueOf(subObjByChain(sh0, ao, icPrev));
        sh0.valReplace(sh0.placedAt(peer), valPrev);
        sh0.objDestroy(peer);
    }

    // destroy self
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);
    sh0.valReplace(addrSelf, valNext);
    sh0.objDestroy(ao);

    // schedule the empty variant for processing
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

void concretizeObj(SymHeap &sh, TObjId obj, TSymHeapList &todo) {
    TObjId ao = obj;

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
            skipObj(sh, &ao, sh.objPeerField(obj));
            break;
    }

    // handle possibly empty variant (if exists)
    spliceOutSegmentIfNeeded(sh, obj, ao, todo);

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(obj);
    const TValueId aoDupAddr = sh.placedAt(aoDup);
    if (OK_DLS == kind) {
        // DLS relink
        const TObjId peerField = subObjByChain(sh, ao, sh.objPeerField(ao));
        sh.objSetValue(peerField, aoDupAddr);
    }

    // concretize self and recover the list
    const TObjId ptrNext = subObjByChain(sh, obj, (OK_SLS == kind)
            ? sh.objBinderField(obj)
            : sh.objPeerField(obj));
    sh.objConcretize(obj);
    sh.objSetValue(ptrNext, aoDupAddr);

    if (OK_DLS == kind) {
        // update DLS back-link
        const TFieldIdxChain icPrev = sh.objBinderField(aoDup);
        const TObjId backLink = subObjByChain(sh, aoDup, icPrev);
        sh.objSetValue(backLink, sh.placedAt(obj));
    }
}
