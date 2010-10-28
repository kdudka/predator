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

#include "symgc.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"

#include <algorithm>                // for std::copy()
#include <iomanip>
#include <set>
#include <sstream>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

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

    // read lower bound estimation of seg length
    const unsigned len = segMinLength(sh, seg);

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
        sh.objSetValue(ppSeg, sh.placedAt(dupPeer));
        sh.objSetValue(ppPeer, sh.placedAt(dupSeg));
    }

    if (len)
        // restore lower bound estimation of segment length
        segSetMinLength(sh, dupSeg, len);

    return dupSeg;
}

TValueId /* addr */ segCloneIfNeeded(SymHeap &sh, TValueId atAddr) {
    const TObjId seg = sh.pointsTo(atAddr);
    if (sh.objShared(seg))
        // object is shared, nothing to clone here
        return VAL_INVALID;

    // once the object is cloned, it's no longer a prototype object
    const TObjId dup = segClone(sh, seg);
    segSetShared(sh, dup, true);
    return sh.placedAt(dup);
}

template <class TIgnoreList>
void buildIgnoreList(const SymHeap &sh, TObjId obj, TIgnoreList &ignoreList) {
    TObjId tmp;

    const EObjKind kind = sh.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
        case OK_HEAD:
        case OK_PART:
            // invalid call of buildIgnoreList()
            SE_TRAP;

        case OK_DLS:
            // preserve 'peer' field
            tmp = subObjByChain(sh, obj, sh.objBinding(obj).peer);
            ignoreList.insert(tmp);
            // fall through!

        case OK_SLS:
            // preserve 'next' field
            tmp = subObjByChain(sh, obj, sh.objBinding(obj).next);
            ignoreList.insert(tmp);
    }
}

bool segConsiderPrototype(const SymHeap     &sh,
                          const TObjPair    &roots,
                          const TValueId    v1,
                          const TValueId    v2);

typedef SymHeap::TContValue TProtoAddrs[2];

struct DataMatchVisitor {
    std::set<TObjId>    ignoreList;
    TProtoAddrs         *protoAddrs;
    TObjPair            roots_;

    DataMatchVisitor(TObjId o1, TObjId o2):
        protoAddrs(0),
        roots_(o1, o2)
    {
    }

    bool operator()(const SymHeap &sh, TObjPair item) const {
        const TObjId o1 = item.first;
        if (hasKey(ignoreList, o1))
            return /* continue */ true;

        // first compare value IDs
        const TValueId v1 = sh.valueOf(o1);
        const TValueId v2 = sh.valueOf(item.second);

        bool eq;
        if (sh.proveEq(&eq, v1, v2) && eq)
            return /* continue */ true;

        // special values have to match
        if (v1 <= 0 || v2 <= 0)
            return /* mismatch */ false;

        // compare _unknown_ value codes
        const EUnknownValue code = sh.valGetUnknown(v1);
        if (code != sh.valGetUnknown(v2))
            return /* mismatch */ false;

        switch (code) {
            case UV_KNOWN:
                // known values have to match
                return false;

            case UV_UNINITIALIZED:
            case UV_UNKNOWN:
                // safe to keep UV_UNKNOWN values as they are
                return true;

            case UV_ABSTRACT:
                // FIXME: unguarded recursion!
                if (!segConsiderPrototype(sh, roots_, v1, v2))
                    break;

                if (protoAddrs) {
                    // FIXME: what about shared prototypes at this point?
                    (*protoAddrs)[0].push_back(v1);
                    (*protoAddrs)[1].push_back(v2);
                }

                return true;
        }
        return /* mismatch */ false;
    }
};

bool validateUpLink(const SymHeap       &sh,
                    const TObjPair      &roots,
                    const TValueId      valNext1,
                    const TValueId      valNext2)
{
    const TObjId root1 = roots.first;
    const TObjId root2 = roots.second;
    SE_BREAK_IF(root1 <= 0 || root2 <= 0);

    TObjId o1 = sh.pointsTo(valNext1);
    TObjId o2 = sh.pointsTo(valNext2);
    if (o1 <= 0 || o2 <= 0)
        // non starter
        return false;

    for (;;) {
        int nth1, nth2;
        o1 = sh.objParent(o1, &nth1);
        o2 = sh.objParent(o2, &nth2);

        if (OBJ_INVALID == o1 || OBJ_INVALID == o2)
            // root mismatch
            return false;

        if (nth1 != nth2)
            // selector mismatch
            return false;

        if (root1 == o1 && root2 == o2)
            // uplink validated!
            break;
    }

    CL_WARN("validateUpLink() matched an up-link, "
            "but concretization of up-links is not yet implemented!");
    return true;
}

bool segMatchNextValues(const SymHeap     &sh,
                        const TObjPair    &roots,
                        const TObjId      seg1,
                        const TObjId      seg2)
{
    const TValueId v1 = sh.valueOf(nextPtrFromSeg(sh, seg1));
    const TValueId v2 = sh.valueOf(nextPtrFromSeg(sh, seg2));
    if (VAL_NULL == v1 && VAL_NULL == v2)
        // both values are NULL
        return true;

    if (validateUpLink(sh, roots, v1, v2))
        // valid uplink found
        return true;

    // TODO
    return false;
}

bool segEqual(const SymHeap     &sh,
              const TObjPair    &roots,
              const EObjKind    kind,
              const TObjId      o1,
              const TObjId      o2)
{
    const bool isDls = (OK_DLS == kind);
    SE_BREAK_IF(!isDls && OK_SLS != kind);

    if (sh.objBinding(o1).next != sh.objBinding(o2).next)
        // 'next' selector mismatch
        return false;

    if (sh.objBinding(o1).peer != sh.objBinding(o2).peer)
        // 'peer' selector mismatch
        return false;

    if (!segMatchNextValues(sh, roots, o1, o2))
        // end-point value mismatch
        return false;

    if (isDls) {
        const TObjId peer1 = dlSegPeer(sh, o1);
        const TObjId peer2 = dlSegPeer(sh, o2);
        if (!segMatchNextValues(sh, roots, peer1, peer2))
            // end-point value mismatch
            return false;
    }

    // compare the data
    DataMatchVisitor visitor(o1, o2);
    buildIgnoreList(sh, o1, visitor.ignoreList);
    const TObjPair item(o1, o2);
    return traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

class PointingObjectsFinder {
    SymHeap::TContObj &dst_;

    public:
        PointingObjectsFinder(SymHeap::TContObj &dst): dst_(dst) { }

        bool operator()(const SymHeap &sh, TObjId obj) const {
            const TValueId addr = sh.placedAt(obj);
            SE_BREAK_IF(addr <= 0);

            sh.usedBy(dst_, addr);
            return /* continue */ true;
        }
};

void gatherPointingObjects(const SymHeap            &sh,
                           SymHeap::TContObj        &dst,
                           const TObjId             root,
                           bool                     toInsideOnly)
{
    const PointingObjectsFinder visitor(dst);

    if (!toInsideOnly)
        visitor(sh, root);

    traverseSubObjs(sh, root, visitor, /* leavesOnly */ false);
}

bool segMayBePrototype(const SymHeap        &sh,
                       const EObjKind       kind,
                       const TObjId         seg,
                       const TObjId         upSeg)
{
    SE_BREAK_IF(OK_DLS != kind && OK_SLS != kind);
    const TObjId peer = (OK_DLS == kind)
        ? dlSegPeer(sh, seg)
        : static_cast<TObjId>(OBJ_INVALID);

    // gather pointing objects
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, seg, /* toInsideOnly */ false);
    if (OBJ_INVALID != peer)
        gatherPointingObjects(sh, refs, peer, /* toInsideOnly */ false);

    // declare set of allowed pointing objects
    std::set<TObjId> allowedRoots;
    allowedRoots.insert(seg);
    allowedRoots.insert(upSeg);
    if (OBJ_INVALID != peer)
        allowedRoots.insert(peer);

    // wait, upSeg can already be a segment at this point!
    if (OK_DLS == sh.objKind(upSeg))
        allowedRoots.insert(dlSegPeer(sh, upSeg));

    // go through all pointing objects and validate them
    BOOST_FOREACH(const TObjId obj, refs) {
        const TObjId root = objRoot(sh, obj);
        if (!hasKey(allowedRoots, root))
            return false;
    }

    // no intruder found!
    return true;
}

bool segConsiderPrototype(const SymHeap     &sh,
                          const TObjPair    &roots,
                          const TValueId    v1,
                          const TValueId    v2)
{
    TObjId o1 = sh.pointsTo(v1);
    TObjId o2 = sh.pointsTo(v2);
    SE_BREAK_IF(o1 <= 0 || o2 <= 0);

    EObjKind kind = sh.objKind(o1);
    if (sh.objKind(o2) != kind)
        // object kind mismatch
        return false;

    if (OK_HEAD == kind) {
        // jump to root, which should be a segment
        o1 = objRoot(sh, o1);
        o2 = objRoot(sh, o2);
        SE_BREAK_IF(o1 <= 0 || o2 <= 0);
        SE_BREAK_IF(OK_SLS != sh.objKind(o1) && OK_DLS != sh.objKind(o1));
        SE_BREAK_IF(OK_SLS != sh.objKind(o2) && OK_DLS != sh.objKind(o2));

        kind = sh.objKind(o1);
        if (sh.objKind(o2) != kind)
            // object kind mismatch
            return false;
    }

    const struct cl_type *clt1 = sh.objType(o1);
    const struct cl_type *clt2 = sh.objType(o2);
    SE_BREAK_IF(!clt1 || !clt2);
    if (*clt1 != *clt2)
        // type mismatch
        return false;

    return segEqual(sh, roots, kind, o1, o2)
        && segMayBePrototype(sh, kind, o1, roots.first)
        && segMayBePrototype(sh, kind, o2, roots.second);
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

    // attempt to dig some type-info for the new unknown value
    const struct cl_type *clt1 = sh.valType(v1);
    const struct cl_type *clt2 = sh.valType(v2);

    // should be safe to ignore
    SE_BREAK_IF(clt1 && clt2 && clt1 != clt2);

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

    if (UV_ABSTRACT == code)
        return mergeAbstractValues(sh, roots, v1, v2);

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
                valNew = segCloneIfNeeded(sh, valOld);
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
void duplicateUnknownValues(SymHeap &sh, TObjId obj) {
    UnknownValuesDuplicator visitor;
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

    if (isSls) {
        // perform SLS abstraction!
        CL_DEBUG("    AAA initiating SLS abstraction of length " << len);
        debugPlotInit("SLS");
        debugPlot(sh);

        for (int i = 0; i < len; ++i) {
            slSegAbstractionStep(sh, &obj, bf);
            debugPlot(sh);
        }

        CL_DEBUG("<-- successfully abstracted SLS");
        return true;
    }
    else {
        // perform DLS abstraction!
        CL_DEBUG("    AAA initiating DLS abstraction of length " << len);
        debugPlotInit("DLS");
        debugPlot(sh);

        for (int i = 0; i < len; ++i) {
            dlSegAbstractionStep(sh, &obj, bf);
            debugPlot(sh);
        }

        CL_DEBUG("<-- successfully abstracted DLS");
        return true;
    }
}

bool matchSegBinding(const SymHeap              &sh,
                     const TObjId               obj,
                     const SegBindingFields     &bfDiscover)
{
    const EObjKind kind = sh.objKind(obj);
    if (OK_CONCRETE == kind)
        // nothing to match actually
        return true;

    const SegBindingFields bf = sh.objBinding(obj);
    if (bf.head != bfDiscover.head)
        // head mismatch
        return false;

    switch (kind) {
        case OK_SLS:
            return (bf.next == bfDiscover.next);

        case OK_DLS:
            return (bf.next == bfDiscover.peer)
                && (bf.peer == bfDiscover.next);

        default:
            SE_TRAP;
            return false;
    }
}

bool preserveHeadPtr(const SymHeap                &sh,
                     const SegBindingFields       &bf,
                     const TObjId                 obj)
{
    const TValueId valPrev = sh.valueOf(subObjByChain(sh, obj, bf.peer));
    const TValueId valNext = sh.valueOf(subObjByChain(sh, obj, bf.next));
    if (valPrev <= 0 && valNext <= 0)
        // no valid address anyway
        return false;

    const TValueId addrHead = sh.placedAt(subObjByChain(sh, obj, bf.head));
    if (valPrev == addrHead || valNext == addrHead)
        // head pointer detected
        return true;

    const TValueId addrRoot = sh.placedAt(obj);
    if (valPrev == addrRoot || valNext == addrHead)
        // root pointer detected
        return true;

    // found nothing harmful
    return false;
}

bool validateSinglePointingObject(const SymHeap             &sh,
                                  const SegBindingFields    &bf,
                                  const TObjId              obj,
                                  const TObjId              prev,
                                  const TObjId              next)
{
    if (obj == subObjByChain(sh, prev, bf.next))
        return true;

    const bool isDls = !bf.peer.empty();
    if (isDls && obj == subObjByChain(sh, next, bf.peer))
        return true;

    // TODO
    return false;
}

bool validatePointingObjects(const SymHeap              &sh,
                             const SegBindingFields     &bf,
                             const TObjId               root,
                             TObjId                     prev,
                             const TObjId               next,
                             const SymHeap::TContValue  &protoAddrs,
                             const bool                 toInsideOnly = false)
{
    std::set<TObjId> allowedReferers;
    if (OK_DLS == sh.objKind(root))
        // retrieve peer's pointer to this object (if any)
        allowedReferers.insert(peerPtrFromSeg(sh, dlSegPeer(sh, root)));

    if (OK_DLS == sh.objKind(prev))
        // jump to peer in case of DLS
        prev = dlSegPeer(sh, prev);

    // collect all object pointing at/inside the object
    SymHeap::TContObj refs;
    gatherPointingObjects(sh, refs, root, toInsideOnly);

    // consider also up-links from nested prototypes
    BOOST_FOREACH(const TValueId protoAt, protoAddrs) {
        const TObjId seg = sh.pointsTo(protoAt);
        const TObjId nextPtr = nextPtrFromSeg(sh, seg);
        allowedReferers.insert(nextPtr);
    }

    BOOST_FOREACH(const TObjId obj, refs) {
        if (validateSinglePointingObject(sh, bf, obj, prev, next))
            continue;

        if (hasKey(allowedReferers, obj))
            continue;

        // someone points at/inside who should not
        return false;
    }

    // no problems encountered
    return true;
}

bool validateSegEntry(const SymHeap              &sh,
                      const SegBindingFields     &bf,
                      const TObjId               entry,
                      const TObjId               next,
                      const SymHeap::TContValue  &protoAddrs)
{
    const TFieldIdxChain &icHead = bf.head;
    if (icHead.empty()) {
        // no Linux lists involved
        return validatePointingObjects(sh, bf, entry, OBJ_INVALID, next,
                                       protoAddrs, /* toInsideOnly */ true);
    }

    // jump to the head sub-object
    const TObjId head = subObjByChain(sh, entry, icHead);
    SE_BREAK_IF(entry == head || head <= 0);

    // FIXME: this is too strict for the _hlist_ variant of Linux lists
    if (!validatePointingObjects(sh, bf, head, OBJ_INVALID, next,
                                 /* TODO */ SymHeap::TContValue(),
                                 /* toInsideOnly */ true))
        return false;

    // now check that nothing but head is pointed from outside
    // FIXME: suboptimal due to performance
    SymHeap::TContObj refsAll, refsHead;
    const PointingObjectsFinder visAll(refsAll), visHead(refsHead);

    // gather all pointers at entry/head
    visAll(sh, entry);
    visHead(sh, head);

    // gather all pointers inside entry/head
    traverseSubObjs(sh, head, visHead, /* leavesOnly */ false);
    traverseSubObjs(sh, entry, visAll, /* leavesOnly */ false);

    // finally compare the sets
    return (refsAll.size() == refsHead.size());
}

TObjId jumpToNextObj(const SymHeap              &sh,
                     const SegBindingFields     &bf,
                     std::set<TObjId>           &haveSeen,
                     TObjId                     obj)
{
    const bool dlSegOnPath = (OK_DLS == sh.objKind(obj));
    if (dlSegOnPath) {
        // jump to peer in case of DLS
        obj = dlSegPeer(sh, obj);
        haveSeen.insert(obj);
    }

    const struct cl_type *clt = sh.objType(obj);
    const TObjId nextPtr = subObjByChain(sh, obj, bf.next);
    SE_BREAK_IF(nextPtr <= 0);

    const TObjId nextHead = sh.pointsTo(sh.valueOf(nextPtr));
    if (nextHead <= 0)
        // no head pointed by nextPtr
        return OBJ_INVALID;

    const TObjId next = subObjByInvChain(sh, nextHead, bf.head);
    if (next <= 0)
        // no suitable next object
        return OBJ_INVALID;

    const struct cl_type *cltNext = sh.objType(next);
    if (!cltNext || *cltNext != *clt)
        // type mismatch
        return OBJ_INVALID;

    if (!matchSegBinding(sh, next, bf))
        // binding mismatch
        return OBJ_INVALID;

    const bool isDls = !bf.peer.empty();
    if (isDls) {
        // check DLS back-link
        const TObjId prevPtr = subObjByChain(sh, next, bf.peer);
        const TObjId head = subObjByChain(sh, obj, bf.head);
        if (sh.valueOf(prevPtr) != sh.placedAt(head))
            // DLS back-link mismatch
            return OBJ_INVALID;
    }

    if (dlSegOnPath
            && !validatePointingObjects(sh, bf, obj, /* prev */ obj, next,
                                        /* TODO */ SymHeap::TContValue()))
        // never step over a peer object that is pointed from outside!
        return OBJ_INVALID;

    return next;
}

bool matchData(const SymHeap                &sh,
               const SegBindingFields       &bf,
               const TObjId                 o1,
               const TObjId                 o2,
               TProtoAddrs                  *protoAddrs)
{
    const TObjId nextPtr = subObjByChain(sh, o1, bf.next);

    DataMatchVisitor visitor(o1, o2);
    visitor.ignoreList.insert(nextPtr);
    visitor.protoAddrs = protoAddrs;

    if (!bf.peer.empty()) {
        const TObjId prevPtr = subObjByChain(sh, o1, bf.peer);
        visitor.ignoreList.insert(prevPtr);
    }

    const TObjPair item(o1, o2);
    return traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);
}

bool slSegAvoidSelfCycle(const SymHeap &sh, const TObjId o1, const TObjId o2) {
    const TValueId v1 = sh.placedAt(o1);
    const TValueId v2 = sh.placedAt(o2);

    return haveSeg(sh, v1, v2, OK_SLS)
        || haveSeg(sh, v2, v1, OK_SLS);
}

unsigned /* len */ segDiscover(const SymHeap            &sh,
                               const SegBindingFields   &bf,
                               const TObjId             entry)
{
    if (preserveHeadPtr(sh, bf, entry))
        // special quirk for head pointers
        return 0;

    // we use std::set to detect loops
    std::set<TObjId> haveSeen;
    haveSeen.insert(entry);
    TObjId prev = entry;

    const bool isDls = !bf.peer.empty();
    if (isDls) {
        // avoid DLS self-loop
        const TObjId prevPtr = subObjByChain(sh, entry, bf.peer);
        TObjId prev = sh.pointsTo(sh.valueOf(prevPtr));
        prev = subObjByInvChain(sh, prev, bf.head);
        if (0 < prev) {
            haveSeen.insert(prev);
            if (OK_DLS == sh.objKind(prev))
                haveSeen.insert(dlSegPeer(sh, prev));
        }
    }

    TObjId obj = jumpToNextObj(sh, bf, haveSeen, entry);
    if (!insertOnce(haveSeen, obj))
        // loop detected
        return 0;

    // main loop of segDiscover()
    std::vector<TObjId> path;
    while (OBJ_INVALID != obj) {
        // compare the data
        TProtoAddrs protoAddrs;
        if (!matchData(sh, bf, prev, obj, &protoAddrs)) {
            CL_DEBUG("    DataMatchVisitor refuses to create a segment!");
            break;
        }

        if (prev == entry
                && !validateSegEntry(sh, bf, entry, obj, protoAddrs[0]))
            // invalid entry
            break;

        // look ahead
        TObjId next = jumpToNextObj(sh, bf, haveSeen, obj);
        if (!insertOnce(haveSeen, next))
            // loop detected
            break;

        if (!validatePointingObjects(sh, bf, obj, prev, next, protoAddrs[1]))
            // someone points to inside who should not
            break;

        // enlarge the path by one
        path.push_back(obj);
        prev = obj;
        obj = next;
    }

    if (path.empty())
        // found nothing
        return 0;

    if (slSegAvoidSelfCycle(sh, entry, path.back()))
        // avoid creating self-cycle of two SLS segments
        return path.size() - 1;

    return path.size();
}

bool digSegmentHead(TFieldIdxChain          &dst,
                    const SymHeap           &sh,
                    const struct cl_type    *cltRoot,
                    TObjId                  obj)
{
    TFieldIdxChain invIc;
    while (*cltRoot != *sh.objType(obj)) {
        int nth;
        obj = sh.objParent(obj, &nth);
        if (OBJ_INVALID == obj)
            // head not found
            return false;

        invIc.push_back(nth);
    }

    // head found, now reverse the index chain (if any)
    dst.clear();
    std::copy(invIc.rbegin(), invIc.rend(), std::back_inserter(dst));
    return true;
}

struct PtrFinder {
    TObjId              target;
    TFieldIdxChain      icFound;

    bool operator()(const SymHeap &sh, TObjId sub, TFieldIdxChain ic) {
        const TValueId val = sh.valueOf(sub);
        if (val <= 0)
            return /* continue */ true;

        const TObjId obj = sh.pointsTo(val);
        if (obj != target)
            return /* continue */ true;

        // target found!
        icFound = ic;
        return /* break */ false;
    }
};

void digBackLink(SegBindingFields           &bf,
                 const SymHeap              &sh,
                 const TObjId               next,
                 TObjId                     root)
{
    PtrFinder visitor;
    visitor.target = /* head */ subObjByChain(sh, root, bf.head);
    if (traverseSubObjsIc(sh, next, visitor))
        // not found
        return;

    // join the idx chain with head
    const TFieldIdxChain &fromHeadToBack = visitor.icFound;
    bf.peer = bf.head;
    std::copy(fromHeadToBack.begin(),
              fromHeadToBack.end(),
              std::back_inserter(bf.peer));

    if (bf.peer == bf.next)
        // next and prev pointers have to be two distinct pointers, withdraw it
        bf.peer.clear();
}

typedef std::vector<SegBindingFields> TBindingCandidateList;

class ProbeEntryVisitor {
    private:
        TBindingCandidateList   &dst_;
        const TObjId            root_;
        const struct cl_type    *clt_;

    public:
        ProbeEntryVisitor(TBindingCandidateList         &dst,
                          const SymHeap                 &sh,
                          const TObjId                  root):
            dst_(dst),
            root_(root),
            clt_(sh.objType(root))
        {
            SE_BREAK_IF(!clt_);
        }

        bool operator()(const SymHeap &sh, TObjId sub, TFieldIdxChain ic) const
        {
            const TValueId val = sh.valueOf(sub);
            if (val <= 0)
                return /* continue */ true;

            const TObjId next = sh.pointsTo(val);
            if (next <= 0)
                return /* continue */ true;

            if (!isComposite(sh.objType(next)))
                // we take only composite types in case of segment head for now
                return /* continue */ true;

            SegBindingFields bf;
            if (!digSegmentHead(bf.head, sh, clt_, next))
                return /* continue */ true;

            // entry candidate found, check the back-link in case of DLL
            bf.next = ic;
#if !SE_DISABLE_DLS
            digBackLink(bf, sh, next, root_);
#endif

#if SE_DISABLE_SLS
            // allow only DLS abstraction
            if (bf.peer.empty())
                return /* continue */ true;
#endif

            // append a candidate
            dst_.push_back(bf);
            return /* continue */ true;
        }
};

struct SegCandidate {
    TObjId                      entry;
    TBindingCandidateList       bfs;
};

typedef std::vector<SegCandidate> TSegCandidateList;

bool performBestAbstraction(SymHeap &sh, const TSegCandidateList &candidates)
{
    const unsigned cnt = candidates.size();
    if (!cnt)
        // no candidates given
        return false;

    CL_DEBUG("--> initiating segment discovery, "
            << cnt << " entry candidate(s) given");

    // go through entry candidates
    unsigned            bestLen = 0;
    unsigned            bestIdx = 0;
    SegBindingFields    bestBinding;

    for (unsigned idx = 0; idx < cnt; ++idx) {

        // go through binding candidates
        const SegCandidate &segc = candidates[idx];
        BOOST_FOREACH(const SegBindingFields &bf, segc.bfs) {
            const unsigned len = segDiscover(sh, bf, segc.entry);
            if (len <= bestLen)
                continue;

            // update best candidate
            bestIdx = idx;
            bestLen = len;
            bestBinding = bf;
        }
    }

    if (!bestLen) {
        CL_DEBUG("<-- no new segment found");
        return false;
    }

    // pick up the best candidate
    const SegCandidate &segc = candidates[bestIdx];
    return considerAbstraction(sh, bestBinding, segc.entry, bestLen);
}

bool abstractIfNeededCore(SymHeap &sh) {
    TSegCandidateList candidates;

    // go through all potential segment entries
    SymHeapCore::TContObj roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        if (sh.cVar(0, obj))
            // skip static/automatic objects
            continue;

        const TValueId addr = sh.placedAt(obj);
        if (VAL_INVALID == addr)
            // no valid object anyway
            continue;

        // use ProbeEntryVisitor visitor to validate the potential segment entry
        SegCandidate segc;
        const ProbeEntryVisitor visitor(segc.bfs, sh, obj);
        traverseSubObjsIc(sh, obj, visitor);
        if (segc.bfs.empty())
            // found nothing
            continue;

        // append a segment candidate
        segc.entry = obj;
        candidates.push_back(segc);
    }

    return performBestAbstraction(sh, candidates);
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
    (void) sh;
#else
    while (abstractIfNeededCore(sh))
        ;
#endif
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
    duplicateUnknownValues(sh, obj);

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
