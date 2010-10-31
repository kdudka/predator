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
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "symdiscover.hh"
#include "symgc.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <iomanip>
#include <set>
#include <sstream>

#include <boost/foreach.hpp>

#if DEBUG_SYMABSTRACT
#   include "symdump.hh"
#   define FIXW(w) std::fixed << std::setfill('0') << std::setw(w)
namespace {
    static int cntAbstraction = -1;
    static int cntAbstractionStep;
    static std::string abstractionName;

    void debugPlotInit(std::string name) {
        ++::cntAbstraction;
        ::cntAbstractionStep = 0;
        ::abstractionName = name;
    }

    std::string debugPlotName() {
        std::ostringstream str;

        str << "symabstract-" << FIXW(4) << ::cntAbstraction
            << "-" << ::abstractionName
            << "-" << FIXW(4) << (::cntAbstractionStep++);

        return str.str();
    }

    void debugPlot(const SymHeap &sh) {
        std::string name = debugPlotName();
        dump_plot(sh, name.c_str());
    }
} // namespace

#else // DEBUG_SYMABSTRACT
namespace {
    void debugPlotInit(std::string) { }
    void debugPlot(const SymHeap &) { }
}
#endif // DEBUG_SYMABSTRACT

/// common configuration template for abstraction triggering
struct AbstractionThreshold {
    unsigned sparePrefix;
    unsigned innerSegLen;
    unsigned spareSuffix;
};

/// abstraction trigger threshold for SLS
static struct AbstractionThreshold slsThreshold = {
    /* sparePrefix */ 0,
    /* innerSegLen */ 1,
    /* spareSuffix */ 0
};

/// abstraction trigger threshold for DLS
static struct AbstractionThreshold dlsThreshold = {
    /* sparePrefix */ 0,
    /* innerSegLen */ 1,
    /* spareSuffix */ 0
};

void segHandleNeq(SymHeap &sh, TObjId seg, TObjId peer, SymHeap::ENeqOp op) {
    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);

    const TValueId headAddr = segHeadAddr(sh, seg);
    sh.neqOp(op, headAddr, valNext);
}

void dlSegSetMinLength(SymHeap &sh, TObjId dls, unsigned len) {
    const TObjId peer = dlSegPeer(sh, dls);
    switch (len) {
        case 0:
            segHandleNeq(sh, dls, peer, SymHeap::NEQ_DEL);
            return;

        case 1:
            segHandleNeq(sh, dls, peer, SymHeap::NEQ_ADD);
            return;

        case 2:
        default:
            break;
    }

    // let it be DLS 2+
    const TValueId a1 = segHeadAddr(sh, dls);
    const TValueId a2 = segHeadAddr(sh, peer);
    sh.neqOp(SymHeap::NEQ_ADD, a1, a2);
}

void segSetMinLength(SymHeap &sh, TObjId seg, unsigned len) {
    const EObjKind kind = sh.objKind(seg);
    switch (kind) {
        case OK_SLS:
            segHandleNeq(sh, seg, seg, (len)
                    ? SymHeap::NEQ_ADD
                    : SymHeap::NEQ_DEL);
            break;

        case OK_HEAD:
            seg = objRoot(sh, seg);
            // fall through!

        case OK_DLS:
            dlSegSetMinLength(sh, seg, len);
            break;

        default:
#if SE_SELF_TEST
            SE_TRAP;
#endif
            break;
    }
}

TObjId segClone(SymHeap &sh, const TObjId seg) {
    const TObjId dupSeg = sh.objDup(seg);

    if (OK_DLS == sh.objKind(seg)) {
        // we need to clone the peer as well
        const TObjId peer = dlSegPeer(sh, seg);
        const TObjId dupPeer = sh.objDup(peer);

        // dig the 'peer' selectors of the cloned objects
        const TFieldIdxChain icpSeg  = sh.objBinding(dupSeg).peer;
        const TFieldIdxChain icpPeer = sh.objBinding(dupPeer).peer;

        // resolve selectors -> sub-objects
        const TObjId ppSeg  = subObjByChain(sh, dupSeg , icpSeg);
        const TObjId ppPeer = subObjByChain(sh, dupPeer, icpPeer);

        // now cross the 'peer' pointers
        sh.objSetValue(ppSeg, segHeadAddr(sh, dupPeer));
        sh.objSetValue(ppPeer, segHeadAddr(sh, dupSeg));
    }

    return dupSeg;
}

void redirectInboundEdges(
        SymHeap                 &sh,
        const TObjId            pointingFrom,
        const TObjId            pointingTo,
        const TObjId            redirectTo)
{
    // go through all objects pointing at/inside pointingTo
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, pointingTo, /* toInsideOnly */ false);
    BOOST_FOREACH(const TObjId obj, refs) {
        if (pointingFrom != objRoot(sh, obj))
            // pointed from elsewhere, keep going
            continue;

        TObjId parent = sh.pointsTo(sh.valueOf(obj));
        SE_BREAK_IF(parent <= 0);

        // seek obj's root
        int nth;
        TFieldIdxChain invIc;
        while (OBJ_INVALID != (parent = sh.objParent(parent, &nth)))
            invIc.push_back(nth);

        // now take the selector chain reversely
        TObjId target = redirectTo;
        BOOST_REVERSE_FOREACH(int nth, invIc) {
            target = sh.subObj(target, nth);
            SE_BREAK_IF(OBJ_INVALID == target);
        }

        // redirect!
        sh.objSetValue(obj, sh.placedAt(target));
    }
}

void detachClonedPrototype(
        SymHeap                 &sh,
        const TObjId            proto,
        const TObjId            clone,
        const TObjId            rootDst,
        const TObjId            rootSrc)
{
    const bool isRootDls = (OK_DLS == sh.objKind(rootDst));
    SE_BREAK_IF(isRootDls && (OK_DLS != sh.objKind(rootSrc)));

    TObjId rootSrcPeer = OBJ_INVALID;
    if (isRootDls) {
        rootSrcPeer = dlSegPeer(sh, rootSrc);
        SE_BREAK_IF(dlSegPeer(sh, rootDst) != rootSrcPeer);
    }

    redirectInboundEdges(sh, rootDst, proto, clone);
    redirectInboundEdges(sh, proto, rootDst, rootSrc);
    if (isRootDls)
        redirectInboundEdges(sh, clone, rootSrcPeer, rootDst);

    if (OK_DLS == sh.objKind(proto)) {
        const TObjId protoPeer = dlSegPeer(sh, proto);
        const TObjId clonePeer = dlSegPeer(sh, clone);
        redirectInboundEdges(sh, rootDst, protoPeer, clonePeer);
        redirectInboundEdges(sh, protoPeer, rootDst, rootSrc);
        if (isRootDls)
            redirectInboundEdges(sh, clonePeer, rootSrcPeer, rootDst);
    }
}

TValueId /* addr */ segCloneIfNeeded(
        SymHeap                 &sh,
        const TObjId            rootDst,
        const TObjId            rootSrc,
        const TValueId          atAddr)
{
    TObjId seg = sh.pointsTo(atAddr);
    const bool isHead = (OK_HEAD == sh.objKind(seg));
    if (isHead)
        // jump to root
        seg = objRoot(sh, seg);

    if (sh.objShared(seg))
        // object is shared, nothing to clone here
        return VAL_INVALID;

    // read lower bound estimation of seg length
    const unsigned len = segMinLength(sh, seg);
    segSetMinLength(sh, seg, /* LS 0+ */ 0);

    // once the object is cloned, it's no longer a prototype object
    TObjId dup = segClone(sh, seg);
    segSetShared(sh, dup, true);

    // redirect all edges to the right direction
    detachClonedPrototype(sh, seg, dup, rootDst, rootSrc);

    if (len) {
        // restore lower bound estimation of segment length
        segSetMinLength(sh, seg, len);
        segSetMinLength(sh, dup, len);
    }

    if (isHead)
        // jump back to head
        dup = segHead(sh, dup);

    return sh.placedAt(dup);
}

TValueId mergeAbstractValues(SymHeap            &sh,
                             const TObjPair     &roots,
                             const TValueId     v1,
                             const TValueId     v2)
{
    if (OBJ_DELETED == sh.pointsTo(v1)
            && OK_DLS == sh.objKind(objRoot(sh, sh.pointsTo(v2))))
        // this is tricky, we've already deleted the peer of nested Linux DLS,
        // as part of a precedent merge, we should keep going...
        return v2;

    SE_BREAK_IF(!segConsiderPrototype(sh, roots, v1, v2));
    (void) roots;

    // read lower bound estimation of seg1 length
    const TObjId seg1 = objRoot(sh, sh.pointsTo(v1));
    const unsigned len1 = segMinLength(sh, seg1);

    // by merging the values, we drop the last reference;  destroy the seg
    segSetMinLength(sh, seg1, /* LS 0+ */ 0);
    segDestroy(sh, seg1);

    // read lower bound estimation of seg2 length
    const TObjId seg2 = objRoot(sh, sh.pointsTo(v2));
    const unsigned len2 = segMinLength(sh, seg2);
    segSetMinLength(sh, seg2, /* LS 0+ */ 0);

    // duplicate the nested abstract object on call of concretizeObj()
    segSetShared(sh, seg2, false);

    // revalidate the lower bound estimation of segment length
    segSetMinLength(sh, seg2, (len1 < len2)
            ? len1
            : len2);

    return v2;
}

TValueId mergeValues(SymHeap            &sh,
                     const TObjPair     &roots,
                     const TValueId     v1,
                     const TValueId     v2)
{
    if (v1 == v2)
        return v1;

    // TODO: some quirk for small lists (of length 0 or 1) at this point;  but
    //       we need to improve symdiscover to detect them first...

    // if the types of _unknown_ values are compatible, it should be safe to
    // pass it through;  UV_UNKNOWN otherwise
    const EUnknownValue code1 = sh.valGetUnknown(v1);
    const EUnknownValue code2 = sh.valGetUnknown(v2);
    EUnknownValue code = (code1 != UV_KNOWN && code1 == code2)
        ? code1
        : UV_UNKNOWN;

    if (UV_ABSTRACT == code)
        // create or update a prototype list segment
        return mergeAbstractValues(sh, roots, v1, v2);

    // attempt to dig some type-info for the new unknown value
    const struct cl_type *clt1 = sh.valType(v1);
    const struct cl_type *clt2 = sh.valType(v2);

    // if both values are of the same type, pass the type into the result
    const struct cl_type *clt = (clt1 && clt2 && *clt1 == *clt2)
        ? clt1
        : /* type-info is either unknown, or incompatible */ 0;

    // introduce a new unknown value, representing the join of v1 and v2
    return sh.valCreateUnknown(code, clt);
}

// visitor
struct ValueAbstractor {
    std::set<TObjId>    ignoreList;
    bool                bidir;
    TObjPair            roots_;

    ValueAbstractor(TObjId o1, TObjId o2):
        roots_(o1, o2)
    {
    }

    bool operator()(SymHeap &sh, TObjPair item) const {
        const TObjId src = item.first;
        const TObjId dst = item.second;
        if (hasKey(ignoreList, dst))
            return /* continue */ true;

        TValueId valSrc = sh.valueOf(item.first);
        TValueId valDst = sh.valueOf(dst);
        bool eq;
        if (sh.proveEq(&eq, valSrc, valDst) && eq)
            // values are equal
            return /* continue */ true;

        // merge values
        const TValueId valNew = mergeValues(sh, roots_, valSrc, valDst);
        sh.objSetValue(dst, valNew);
        if (this->bidir)
            sh.objSetValue(src, valNew);

        // if the last reference is gone, we have a problem
        if (collectJunk(sh, valDst)) {
            CL_ERROR("junk detected during abstraction"
                    ", the analysis is no more sound!");
#if SE_SELF_TEST
            SE_TRAP;
#endif
        }

        return /* continue */ true;
    }
};

// visitor
struct UnknownValuesDuplicator {
    std::set<TObjId> ignoreList;
    TObjId rootDst;
    TObjId rootSrc;

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
                valNew = segCloneIfNeeded(sh, rootDst, rootSrc, valOld);
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

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst,
                               bool bidir = false)
{
    ValueAbstractor visitor(src, dst);
    visitor.bidir       = bidir;
    buildIgnoreList(sh, dst, visitor.ignoreList);

    // traverse all sub-objects
    const TObjPair item(src, dst);
    traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

// when concretizing an object, we need to duplicate all _unknown_ values
void duplicateUnknownValues(SymHeap &sh, TObjId obj, TObjId dup) {
    UnknownValuesDuplicator visitor;
    visitor.rootDst = obj;
    visitor.rootSrc = dup;
    buildIgnoreList(sh, obj, visitor.ignoreList);

    // traverse all sub-objects
    traverseSubObjs(sh, obj, visitor, /* leavesOnly */ true);
}

void slSegAbstractionStep(SymHeap &sh, TObjId *pObj, const SegBindingFields &bf)
{
    const TObjId obj = *pObj;
    const TObjId objPtrNext = subObjByChain(sh, obj, bf.next);
    const TValueId valNext = sh.valueOf(objPtrNext);

    // check for a failure of segDiscover() -- FIXME: too strict
    SE_BREAK_IF(valNext <= 0 || 1 != sh.usedByCount(valNext));

    // jump to the next object
    const TObjId objNext = subObjByInvChain(sh, sh.pointsTo(valNext), bf.head);
    const EObjKind kindNext = sh.objKind(objNext);
    SE_BREAK_IF(OK_SLS == kindNext && bf != sh.objBinding(objNext));

    // accumulate resulting segment's minimal length
    unsigned len = 0;
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            len = 1;
            break;

        case OK_SLS:
            len = segMinLength(sh, obj);
            break;

        default:
#if SE_SELF_TEST
            SE_TRAP;
#endif
            break;
    }

    if (OK_CONCRETE == kindNext) {
        // abstract the _next_ object
        sh.objSetAbstract(objNext, OK_SLS, bf);

        // accumulate resulting segment's minimal length
        ++len;
    }
    else
        // accumulate resulting segment's minimal length
        len += segMinLength(sh, objNext);

    // merge data
    SE_BREAK_IF(OK_SLS != sh.objKind(objNext));
    abstractNonMatchingValues(sh, obj, objNext);

    // replace all references to 'head'
    const TFieldIdxChain icHead = sh.objBinding(objNext).head;
    const TObjId head = subObjByChain(sh, obj, icHead);
    sh.valReplace(sh.placedAt(head), segHeadAddr(sh, objNext));

    // replace self by the next object
    objReplace(sh, obj, objNext);

    if (len)
        // initialize resulting segment's minimal length
        segSetMinLength(sh, objNext, len);

    // move to the next object
    *pObj = objNext;
}

void dlSegCreate(SymHeap &sh, TObjId o1, TObjId o2, SegBindingFields bf) {
    // validate call of dlSegCreate()
    SE_BREAK_IF(OK_CONCRETE != sh.objKind(o1) || OK_CONCRETE != sh.objKind(o2));

    swapValues(bf.next, bf.peer);
    sh.objSetAbstract(o1, OK_DLS, bf);

    swapValues(bf.next, bf.peer);
    sh.objSetAbstract(o2, OK_DLS, bf);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, o1, o2, /* bidir */ true);

    // a just created DLS is said to be 2+
    dlSegSetMinLength(sh, o1, /* DLS 2+ */ 2);
}

void dlSegGobble(SymHeap &sh, TObjId dls, TObjId var, bool backward) {
    SE_BREAK_IF(OK_DLS != sh.objKind(dls) || OK_CONCRETE != sh.objKind(var));

    // handle DLS Neq predicates
    const unsigned len = dlSegMinLength(sh, dls) + /* OK_CONCRETE */ 1;
    dlSegSetMinLength(sh, dls, /* DLS 0+ */ 0);

    if (!backward)
        // jump to peer
        dls = dlSegPeer(sh, dls);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh, var, dls);

    // store the pointer DLS -> VAR
    const SegBindingFields &bf = sh.objBinding(dls);
    const TObjId dlsNextPtr = subObjByChain(sh, dls, bf.next);
    const TObjId varNextPtr = subObjByChain(sh, var, bf.next);
    sh.objSetValue(dlsNextPtr, sh.valueOf(varNextPtr));

    // replace VAR by DLS
    const TObjId varHead = subObjByChain(sh, var, bf.head);
    sh.valReplace(sh.placedAt(varHead), segHeadAddr(sh, dls));
    objReplace(sh, var, dls);

    // handle DLS Neq predicates
    dlSegSetMinLength(sh, dls, len);
}

void dlSegMerge(SymHeap &sh, TObjId seg1, TObjId seg2) {
    // handle DLS Neq predicates
    const unsigned len = dlSegMinLength(sh, seg1) + dlSegMinLength(sh, seg2);
    dlSegSetMinLength(sh, seg1, /* DLS 0+ */ 0);
    dlSegSetMinLength(sh, seg2, /* DLS 0+ */ 0);

    // check for a failure of segDiscover()
    SE_BREAK_IF(sh.objBinding(seg1) != sh.objBinding(seg2));

    const TObjId peer1 = dlSegPeer(sh, seg1);
#if SE_SELF_TEST
    const TObjId nextPtr = nextPtrFromSeg(sh, peer1);
    const TValueId valNext = sh.valueOf(nextPtr);
    SE_BREAK_IF(valNext != sh.placedAt(seg2));
#endif

    const TObjId peer2 = dlSegPeer(sh, seg2);

    // introduce some UV_UNKNOWN values if necessary
    abstractNonMatchingValues(sh,  seg1,  seg2, /* bidir */ true);
    abstractNonMatchingValues(sh, peer1, peer2, /* bidir */ true);

    // preserve backLink
    const TValueId valNext2 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    sh.objSetValue(nextPtrFromSeg(sh, seg2), valNext2);

    // update all references to 'head'
    sh.valReplace(segHeadAddr(sh,  seg1), segHeadAddr(sh,  seg2));
    sh.valReplace(segHeadAddr(sh, peer1), segHeadAddr(sh, peer2));

    // replace both parts point-wise
    objReplace(sh,  seg1,  seg2);
    objReplace(sh, peer1, peer2);

    if (len)
        // handle DLS Neq predicates
        dlSegSetMinLength(sh, seg2, len);
}

void dlSegAbstractionStep(SymHeap &sh, TObjId *pObj, const SegBindingFields &bf)
{
    // the first object is clear
    const TObjId o1 = *pObj;

    // we'll find the next one later on
    TObjId o2 = o1;

    EObjKind kind = sh.objKind(o1);
    switch (kind) {
        case OK_SLS:
        case OK_HEAD:
        case OK_PART:
            // *** segDiscover() failure detected ***
            SE_TRAP;

        case OK_DLS:
            // jump to peer
            o2 = dlSegPeer(sh, o2);

            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, sh.objBinding(o2).head, sh.objBinding(o2).next);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // DLS + VAR
                dlSegGobble(sh, o1, o2, /* backward */ false);
                return;
            }

            // DLS + DLS
            dlSegMerge(sh, o1, o2);
            break;

        case OK_CONCRETE:
            // jump to the next object (as we know such an object exists)
            skipObj(sh, &o2, bf.head, bf.next);
            if (OK_CONCRETE == sh.objKind(o2)) {
                // VAR + VAR
                dlSegCreate(sh, o1, o2, bf);
                return;
            }

            // VAR + DLS
            dlSegGobble(sh, o2, o1, /* backward */ true);
            break;
    }

    // the current object has been just consumed, move to the next one
    *pObj = o2;

#if SE_SELF_TEST
    // just check if the Neq predicates work well so far
    dlSegMinLength(sh, o2);
#endif
}

void segAbstractionStep(SymHeap                     &sh,
                        const SegBindingFields      &bf,
                        TObjId                      *pObj)
{
    if (bf.peer.empty())
        slSegAbstractionStep(sh, pObj, bf);
    else
        dlSegAbstractionStep(sh, pObj, bf);
}

bool considerAbstraction(SymHeap                    &sh,
                         const SegBindingFields     &bf,
                         const TObjId               entry,
                         const unsigned             lenTotal)
{
    const bool isSls = bf.peer.empty();
    const AbstractionThreshold &at = (isSls)
        ? slsThreshold
        : dlsThreshold;

    // check whether the threshold is satisfied or not
    const unsigned threshold = at.sparePrefix + at.innerSegLen + at.spareSuffix;
    if (lenTotal < threshold) {
        CL_DEBUG("<-- length (" << lenTotal
                << ") of the longest segment is under the threshold ("
                << threshold << ")");
        return false;
    }

    CL_DEBUG("    --- length of the longest segment is " << lenTotal
            << ", prefix=" << at.sparePrefix
            << ", suffix=" << at.spareSuffix);

    // cursor
    TObjId obj = entry;

    // handle sparePrefix/spareSuffix
    int len = lenTotal - at.sparePrefix - at.spareSuffix;
    for (unsigned i = 0; i < at.sparePrefix; ++i)
        skipObj(sh, &obj, bf.head, bf.next);

    const char *name = (isSls)
        ? "SLS"
        : "DLS";
    CL_DEBUG("    AAA initiating " << name
             << " abstraction of length " << len);

    debugPlotInit(name);
    debugPlot(sh);

    for (int i = 0; i < len; ++i) {
        segAbstractionStep(sh, bf, &obj);
        debugPlot(sh);
    }

    CL_DEBUG("<-- successfully abstracted " << name);
    return true;
}

void segReplaceRefs(SymHeap &sh, TValueId valOld, TValueId valNew) {
    SE_BREAK_IF(UV_ABSTRACT != sh.valGetUnknown(valOld));

    TObjId objOld = sh.pointsTo(valOld);
    TObjId headOld = objOld;
    sh.valReplace(valOld, valNew);

    const EObjKind kind = sh.objKind(objOld);
    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            headOld = segHead(sh, objOld);
            if (headOld == objOld)
                // no Linux lists involved
                return;

        case OK_HEAD:
            break;

        default:
            SE_TRAP;
    }

    TObjId objNew = sh.pointsTo(valNew);
    if (objNew < 0)
        return;

    const TFieldIdxChain icHead = sh.objBinding(objOld).head;
    if (icHead.empty())
        return;

    if (OK_HEAD == kind) {
        objOld = subObjByInvChain(sh, objOld, icHead);
        SE_BREAK_IF(objOld < 0);

        const TValueId addrOld = sh.placedAt(objOld);
        if (0 == sh.usedByCount(addrOld))
            // root not used anyway
            return;

        objNew = subObjByInvChain(sh, objNew, icHead);
        if (0 < objNew)
            valNew = sh.placedAt(objNew);

        else {
            // attempt to create a virtual object
            const int off = subOffsetIn(sh, objOld, headOld);
            const SymHeapCore::TOffVal ov(valNew, -off);
            valNew = sh.valCreateByOffset(ov);
        }

        sh.valReplace(sh.placedAt(objOld), valNew);
    }
    else {
        // TODO: check this with a debugger at least once
#if SE_SELF_TEST
        SE_TRAP;
#endif
        const TObjId headNew = subObjByChain(sh, objNew, icHead);
        sh.valReplace(sh.placedAt(headOld), sh.placedAt(headNew));
    }
}

bool dlSegReplaceByConcrete(SymHeap &sh, TObjId obj, TObjId peer) {
    debugPlotInit("dlSegReplaceByConcrete");
    debugPlot(sh);

    // first kill any related Neq predicates, we're going to concretize anyway
    dlSegSetMinLength(sh, obj, /* DLS 0+ */ 0);

    // take the value of 'next' pointer from peer
    const TFieldIdxChain icPeer = sh.objBinding(obj).peer;
    const TObjId peerPtr = subObjByChain(sh, obj, icPeer);
    const TValueId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
    sh.objSetValue(peerPtr, valNext);

    // redirect all references originally pointing to peer to the current object
    const TValueId addrSelf = sh.placedAt(obj);
    const TValueId addrPeer = sh.placedAt(peer);
    segReplaceRefs(sh, addrPeer, addrSelf);

    // destroy the peer object and concretize self
    sh.objDestroy(peer);
    sh.objSetConcrete(obj);

    // this can't fail (at least I hope so...)
    debugPlot(sh);
    return true;
}

void spliceOutListSegmentCore(SymHeap &sh, TObjId obj, TObjId peer) {
    debugPlotInit("spliceOutListSegmentCore");
    debugPlot(sh);

    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);

    if (obj != peer) {
        // OK_DLS --> destroy peer
        const TFieldIdxChain icPrev = sh.objBinding(obj).next;
        const TValueId valPrev = sh.valueOf(subObjByChain(sh, obj, icPrev));
        segReplaceRefs(sh, segHeadAddr(sh, peer), valPrev);
        sh.objDestroy(peer);
    }

    // destroy self
    segReplaceRefs(sh, segHeadAddr(sh, obj), valNext);
    sh.objDestroy(obj);

    debugPlot(sh);
}

unsigned /* len */ spliceOutSegmentIfNeeded(SymHeap &sh, TObjId ao, TObjId peer,
                                            TSymHeapList &todo)
{
    const unsigned len = segMinLength(sh, ao);
    if (len) {
        debugPlotInit("spliceOutSegmentIfNeeded");
        debugPlot(sh);

        // drop any existing Neq predicates
        segSetMinLength(sh, ao, 0);

        debugPlot(sh);
        return len - 1;
    }

    // possibly empty LS
    SymHeap sh0(sh);
    spliceOutListSegmentCore(sh0, ao, peer);
    todo.push_back(sh0);
    return /* LS 0+ */ 0;
}

void abstractIfNeeded(SymHeap &sh) {
#if SE_DISABLE_SLS && SE_DISABLE_DLS
    return;
#endif
    SegBindingFields    bf;
    TObjId              entry;
    unsigned            len;

    while ((len = discoverBestAbstraction(sh, &bf, &entry))) {
        if (!considerAbstraction(sh, bf, entry, len))
            // the best abstraction given is unfortunately not good enough
            break;

        // some part of the symbolic heap has just been successfully abstracted,
        // let's look if there remains anything else suitable for abstraction
    }
}

void concretizeObj(SymHeap &sh, TValueId addr, TSymHeapList &todo) {
    TObjId obj = sh.pointsTo(addr);
    if (OK_HEAD == sh.objKind(obj))
        obj = objRoot(sh, obj);

    TObjId peer = obj;

    // branch by SLS/DLS
    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
        case OK_PART:
            // invalid call of concretizeObj()
            SE_TRAP;

        case OK_SLS:
            break;

        case OK_DLS:
            // jump to peer
            peer = dlSegPeer(sh, obj);
            break;
    }

    // handle the possibly empty variant (if exists)
    const unsigned lenRemains = spliceOutSegmentIfNeeded(sh, obj, peer, todo);

    debugPlotInit("concretizeObj");
    debugPlot(sh);

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(obj);
    const TValueId aoDupHeadAddr = segHeadAddr(sh, aoDup);
    if (OK_DLS == kind) {
        // DLS relink
        const TFieldIdxChain icPeer = sh.objBinding(peer).peer;
        const TObjId peerField = subObjByChain(sh, peer, icPeer);
        sh.objSetValue(peerField, aoDupHeadAddr);
    }

    // duplicate all unknown values, to keep the prover working
    duplicateUnknownValues(sh, obj, aoDup);

    // concretize self and recover the list
    const TObjId ptrNext = subObjByChain(sh, obj, (OK_SLS == kind)
            ? sh.objBinding(obj).next
            : sh.objBinding(obj).peer);
    sh.objSetConcrete(obj);
    sh.objSetValue(ptrNext, aoDupHeadAddr);

    if (OK_DLS == kind) {
        // update DLS back-link
        const SegBindingFields &bf = sh.objBinding(aoDup);
        const TObjId backLink = subObjByChain(sh, aoDup, bf.next);
        const TValueId headAddr = sh.placedAt(subObjByChain(sh, obj, bf.head));
        sh.objSetValue(backLink, headAddr);
    }

    segSetMinLength(sh, aoDup, lenRemains);

    debugPlot(sh);
}

bool spliceOutListSegment(SymHeap &sh, TValueId atAddr, TValueId pointingTo)
{
    const TObjId obj = sh.pointsTo(atAddr);
    const EObjKind kind = sh.objKind(obj);
    const TObjId peer = (OK_DLS == kind)
        ? dlSegPeer(sh, obj)
        : obj;

    if (OK_DLS == sh.objKind(obj)) {
        const TObjId peer = dlSegPeer(sh, obj);
        if (sh.placedAt(peer) == pointingTo)
            // assume identity over the two parts of DLS
            return dlSegReplaceByConcrete(sh, obj, peer);
    }

    const TObjId next = nextPtrFromSeg(sh, peer);
    const TValueId valNext = sh.valueOf(next);
    if (valNext != pointingTo)
        return false;

    spliceOutListSegmentCore(sh, obj, peer);
    return true;
}
