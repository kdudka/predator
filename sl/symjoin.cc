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
#include "symjoin.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>

#include "symclone.hh"
#include "symcmp.hh"
#include "symgc.hh"
#include "symseg.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy_if
#include <functional>           // for std::bind
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

static bool debugSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debugSymJoin)                                                     \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

#define SJ_VALP(v1, v2) "(v1 = #" << v1 << ", v2 = #" << v2 << ")"
#define SJ_OBJP(o1, o2) "(o1 = #" << o1 << ", o2 = #" << o2 << ")"

typedef boost::array<TObjId, 3>                                 TObjTriple;

template <class T>
class WorkListWithUndo: public WorkList<T> {
    private:
        typedef WorkList<T> TBase;

    public:
        void undo(const T &item) {
            CL_BREAK_IF(!hasKey(TBase::done_, item));
            TBase::todo_.push(item);
        }
};

struct SymJoinCtx {
    SymHeap                     &dst;
    const SymHeap               &sh1;
    const SymHeap               &sh2;

    typedef std::map<TObjId /* src */, TObjId /* dst */>        TObjMap;
    TObjMap                     objMap1;
    TObjMap                     objMap2;

    TValMapBidir                valMap1;
    TValMapBidir                valMap2;

    WorkListWithUndo<TValPair>  wl;
    EJoinStatus                 status;

    typedef std::map<TObjId /* seg */, unsigned /* len */>      TSegLengths;
    TSegLengths                 segLengths;
    std::set<TValPair>          sharedNeqs;
    std::set<TObjPair>          tieBreaking;

    std::set<TObjTriple>        protoRoots;

    SymJoinCtx(SymHeap &dst_, const SymHeap &sh1_, const SymHeap &sh2_):
        dst(dst_),
        sh1(sh1_),
        sh2(sh2_),
        status(JS_USE_ANY)
    {
        // VAL_NULL should be always mapped to VAL_NULL
        valMap1[0][VAL_NULL] = VAL_NULL;
        valMap1[1][VAL_NULL] = VAL_NULL;
        valMap2[0][VAL_NULL] = VAL_NULL;
        valMap2[1][VAL_NULL] = VAL_NULL;
    }
};

void updateJoinStatus(SymJoinCtx &ctx, const EJoinStatus action) {
    if (JS_USE_ANY == action)
        return;

    EJoinStatus &status = ctx.status;
    switch (status) {
        case JS_THREE_WAY:
            return;

        case JS_USE_ANY:
            status = action;
            return;

        default:
            if (action != status)
                status = JS_THREE_WAY;
    }
}

bool hasExplicitNeq(
        const SymHeap           &shConst,
        const TValueId          v1,
        const TValueId          v2)
{
    // FIXME: const-insane interface of SymHeap::neqOp()
    SymHeap &sh = const_cast<SymHeap &>(shConst);
    return sh.neqOp(SymHeap::NEQ_QUERY_EXPLICIT_NEQ, v1, v2);
}

void gatherSharedPreds(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const TValueId          vDst)
{
    // look for shared Neq predicates
    SymHeap::TContValue rVals1;
    ctx.sh1.gatherRelatedValues(rVals1, v1);
    BOOST_FOREACH(const TValueId rel1, rVals1) {
        if (!hasExplicitNeq(ctx.sh1, v1, rel1))
            // not a Neq in sh1
            continue;

        TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
        TValMap::const_iterator it1 = vMap1.find(rel1);
        if (vMap1.end() == it1)
            // related value has not (yet?) any mapping to dst
            continue;

        const TValueId relDst = it1->second;
        TValMap &vMap2r = ctx.valMap2[/* rtl */ 1];
        TValMap::const_iterator it2r = vMap2r.find(relDst);
        if (vMap2r.end() == it2r)
            // related value has not (yet?) any mapping back to sh2
            continue;

        const TValueId rel2 = it2r->second;
        if (!hasExplicitNeq(ctx.sh2, v2, rel2))
            // not a Neq in sh2
            continue;

        // sort Neq values
        TValueId valLt = vDst;
        TValueId valGt = relDst;
        sortValues(valLt, valGt);

        // insert a shared Neq predicate
        const TValPair neq(valLt, valGt);
        ctx.sharedNeqs.insert(neq);
    }
}

bool defineValueMapping(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const TValueId          vDst)
{
    const bool hasValue1 = (VAL_INVALID != v1);
    const bool hasValue2 = (VAL_INVALID != v2);
    CL_BREAK_IF(!hasValue1 && !hasValue2);

    const bool ok1 = !hasValue1 || matchPlainValues(ctx.valMap1, v1, vDst);
    const bool ok2 = !hasValue2 || matchPlainValues(ctx.valMap2, v2, vDst);
    if (!ok1 || !ok2) {
        SJ_DEBUG("<-- value mapping mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (hasValue1 && hasValue2)
        gatherSharedPreds(ctx, v1, v2, vDst);

    return true;
}

// read-only (in)consistency check
bool checkValueMapping(
        const SymJoinCtx        &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const bool              allowUnknownMapping)
{
    // read-only value lookup
    const TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
    const TValMap &vMap2 = ctx.valMap2[/* ltr */ 0];
    TValMap::const_iterator i1 = vMap1.find(v1);
    TValMap::const_iterator i2 = vMap2.find(v2);

    const bool hasMapping1 = (vMap1.end() != i1);
    const bool hasMapping2 = (vMap2.end() != i2);
    if (!hasMapping1 && !hasMapping2)
        // we have not enough info yet
        return allowUnknownMapping;

    const TValueId vDst1 = (hasMapping1)
        ? i1->second
        : static_cast<TValueId>(VAL_INVALID);

    const TValueId vDst2 = (hasMapping2)
        ? i2->second
        : static_cast<TValueId>(VAL_INVALID);

    if (hasMapping1 && hasMapping1 && (vDst1 == vDst2))
        // mapping already known and known to be consistent
        return true;

    SJ_DEBUG("<-- value mapping mismatch: " << SJ_VALP(v1, v2)
             "-> " << SJ_VALP(vDst1, vDst2));
    return false;
}

/// (OBJ_INVALID == objDst) means read-only!!!
bool joinFreshObjTripple(
        SymJoinCtx              &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const TObjId            objDst)
{
    const bool segClone = (OBJ_INVALID == obj1 || OBJ_INVALID == obj2);
    const bool readOnly = (OBJ_INVALID == objDst);
    CL_BREAK_IF(segClone && readOnly);
    CL_BREAK_IF(obj1 < 0 && obj2 < 0);

    const TValueId v1 = ctx.sh1.valueOf(obj1);
    const TValueId v2 = ctx.sh2.valueOf(obj2);
    if (VAL_NULL == v1 && VAL_NULL == v2)
        // both values are VAL_NULL, nothing more to join here
        return true;

    if (VAL_NULL == v1 || VAL_NULL == v2) {
        if (segClone)
            // same as above, but now only one value of v1 and v2 is valid
            return true;

        if (!checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ true))
            // mapping already inconsistent
            return false;
    }

    const TObjId cObj1 = ctx.sh1.valGetCompositeObj(v1);
    const TObjId cObj2 = ctx.sh2.valGetCompositeObj(v2);
    if (!segClone && (OBJ_INVALID == cObj1) != (OBJ_INVALID == cObj2)) {
        SJ_DEBUG("<-- scalar vs. composite value " << SJ_VALP(v1, v2));
        return false;
    }

    if (OBJ_INVALID != cObj1 || OBJ_INVALID != cObj2) {
        // store mapping of composite object's values
        const TValueId vDst = ctx.dst.valueOf(objDst);
        return readOnly
            || defineValueMapping(ctx, v1, v2, vDst);
    }

    if (segClone) {
        const bool isGt1 = (OBJ_INVALID == obj2);
        const TValMapBidir &vm = (isGt1) ? ctx.valMap1 : ctx.valMap2;
        const TValueId val = (isGt1) ? v1 : v2;
        if (hasKey(vm[/* lrt */ 0], val))
            return true;
    }
    else {
        // special values have to match (NULL not treated as special here)
        // TODO: should we consider also join of VAL_TRUE/VAL_FALSE?
        if (v1 < 0 || v2 < 0) {
            if (v1 == v2)
                return true;

            SJ_DEBUG("<-- special value mismatch " << SJ_VALP(v1, v2));
            return false;
        }

        if (readOnly)
            return checkValueMapping(ctx, v1, v2,
                                     /* allowUnknownMapping */ true);
    }

    if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
        return true;

    if (ctx.wl.schedule(TValPair(v1, v2)))
        SJ_DEBUG("+++ " << SJ_VALP(v1, v2) << " <- " << SJ_OBJP(obj1, obj2));

    return true;
}

struct ObjJoinVisitor {
    SymJoinCtx              &ctx;
    std::set<TObjId>        blackList;

    ObjJoinVisitor(SymJoinCtx &ctx_): ctx(ctx_) { }

    bool operator()(const TObjTriple &item) {
        const TObjId obj1   = item[0];
        const TObjId obj2   = item[1];
        const TObjId objDst = item[2];

        // store object IDs mapping
        if (OBJ_INVALID != obj1)
            ctx.objMap1[obj1] = objDst;

        if (OBJ_INVALID != obj2)
            ctx.objMap2[obj2] = objDst;

        // store object's addresses
        const TValueId addr1 = ctx.sh1.placedAt(obj1);
        const TValueId addr2 = ctx.sh2.placedAt(obj2);
        const TValueId dstAt = ctx.dst.placedAt(objDst);
        if (!defineValueMapping(ctx, addr1, addr2, dstAt))
            return false;

        // check black-list
        if (hasKey(blackList, objDst))
            return /* continue */ true;

        return /* continue */ joinFreshObjTripple(ctx, obj1, obj2, objDst);
    }
};

class SegMatchVisitor {
    private:
        SymJoinCtx &ctx_;

    public:
        SegMatchVisitor(SymJoinCtx &ctx): ctx_(ctx) { }

        bool operator()(const boost::array<TObjId, 2> &item) {
            const TObjId obj1   = item[0];
            const TObjId obj2   = item[1];

            return joinFreshObjTripple(ctx_, obj1, obj2,
                                       /* read-only */ OBJ_INVALID);
        }
};

bool traverseSubObjs(
        SymJoinCtx              &ctx,
        const TObjId            root1,
        const TObjId            root2,
        const TObjId            rootDst,
        const SegBindingFields  *bfBlackList = 0)
{
    typedef boost::array<const SymHeap *, 3>        TSymHeapTriple;
#ifndef NDEBUG
    // all three types have to match!
    const struct cl_type *clt1   = ctx.sh1.objType(root1);
    const struct cl_type *clt2   = ctx.sh2.objType(root2);
    const struct cl_type *cltDst = ctx.dst.objType(rootDst);
    CL_BREAK_IF(!cltDst || (OBJ_INVALID == root1 && OBJ_INVALID == root2));
    CL_BREAK_IF(OBJ_INVALID != root1 && *clt1 != *cltDst);
    CL_BREAK_IF(OBJ_INVALID != root2 && *clt2 != *cltDst);
#endif
    TObjTriple roots;
    roots[/* sh1 */ 0] = root1;
    roots[/* sh2 */ 1] = root2;
    roots[/* dst */ 2] = rootDst;

    TSymHeapTriple sht;
    sht[0] = &ctx.sh1;
    sht[1] = &ctx.sh2;
    sht[2] = &ctx.dst;

    // initialize visitor
    ObjJoinVisitor objVisitor(ctx);
    if (bfBlackList)
        buildIgnoreList(objVisitor.blackList, ctx.dst, rootDst, *bfBlackList);

    else if ((&ctx.sh1 == &ctx.sh2)) {
        // we are called from joinData()
        if (root1 == root2)
            // do not follow shared data
            return true;

        ctx.protoRoots.insert(roots);
    }

    // guide the visitors through them
    return objVisitor(roots)
        && traverseSubObjs<3>(sht, roots, objVisitor);
}

bool segMatchLookAhead(
        SymJoinCtx              &ctx,
        const TObjId            root1,
        const TObjId            root2)
{
    boost::array<TObjId, 2> roots;
    roots[/* sh1 */ 0] = root1;
    roots[/* sh2 */ 1] = root2;

    boost::array<const SymHeap *, 2> sht;
    sht[0] = &ctx.sh1;
    sht[1] = &ctx.sh2;

    // guide the visitors through them
    SegMatchVisitor visitor(ctx);
    return visitor(roots)
        && traverseSubObjs<2>(sht, roots, visitor);
}

bool joinValClt(
        const struct cl_type    **pDst,
        const SymJoinCtx        &ctx,
        const TValueId          v1,
        const TValueId          v2)
{
    const struct cl_type *clt1 = ctx.sh1.valType(v1);
    const struct cl_type *clt2 = ctx.sh2.valType(v2);
    if (joinClt(clt1, clt2, pDst))
        return true;

    SJ_DEBUG("<-- value clt mismatch " << SJ_VALP(v1, v2));
    return false;
}

bool joinObjClt(
        const struct cl_type    **pDst,
        const SymJoinCtx        &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const struct cl_type *clt1 = ctx.sh1.objType(o1);
    const struct cl_type *clt2 = ctx.sh2.objType(o2);
    if (joinClt(clt1, clt2, pDst))
        return true;

    SJ_DEBUG("<-- object clt mismatch " << SJ_OBJP(o1, o2));
    return false;
}

bool joinObjKind(
        EObjKind                *pDst,
        const SymJoinCtx        &ctx,
        const TObjId            o1,
        const TObjId            o2,
        const EJoinStatus       action)
{
    const EObjKind kind1 = ctx.sh1.objKind(o1);
    const EObjKind kind2 = ctx.sh2.objKind(o2);
    if (OK_CONCRETE == kind1) {
        CL_BREAK_IF(action == JS_USE_SH1);
        *pDst = kind2;
        return true;
    }

    if (OK_CONCRETE == kind2) {
        CL_BREAK_IF(action == JS_USE_SH2);
        *pDst = kind1;
        return true;
    }

    if (kind1 == kind2) {
        CL_BREAK_IF(action != JS_USE_ANY);
        *pDst = kind1;
        return true;
    }

    switch (action) {
        case JS_USE_SH1:
            *pDst = kind1;
            return true;

        case JS_USE_SH2:
            *pDst = kind2;
            return true;

        default:
            SJ_DEBUG("<-- object kind mismatch " << SJ_OBJP(o1, o2));
            return false;
    }
}

bool joinSegBinding(
        SegBindingFields        *pBf,
        const SymJoinCtx        &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const bool isSeg1 = objIsSeg(ctx.sh1, o1);
    const bool isSeg2 = objIsSeg(ctx.sh2, o2);
    if (!isSeg1 && !isSeg2)
        // nothing to join here
        return true;

    if (isSeg1 && isSeg2) {
        const SegBindingFields bf = ctx.sh1.objBinding(o1);
        if (bf == ctx.sh2.objBinding(o2)) {
            *pBf = bf;
            return true;
        }

        SJ_DEBUG("<-- segment binding mismatch " << SJ_OBJP(o1, o2));
        return false;
    }

    if (isSeg1) {
        *pBf = ctx.sh1.objBinding(o1);
        return true;
    }

    if (isSeg2) {
        *pBf = ctx.sh2.objBinding(o2);
        return true;
    }

    // not reachable
    CL_TRAP;
    return false;
}

bool joinProtoFlag(
        bool                    *pDst,
        const SymJoinCtx        &ctx,
        const TObjId            root1,
        const TObjId            root2)
{
    if (OBJ_INVALID == root2) {
        *pDst = ctx.sh1.objIsProto(root1);
        return true;
    }

    if (OBJ_INVALID == root1) {
        *pDst = ctx.sh2.objIsProto(root2);
        return true;
    }

    *pDst = ctx.sh1.objIsProto(root1);
    if (ctx.sh2.objIsProto(root2) == *pDst)
        return true;

    if (&ctx.sh1 == &ctx.sh2) {
        // we are called from joinData()
        *pDst = true;
        return true;
    }

    SJ_DEBUG("<-- prototype vs shared: " << SJ_OBJP(root1, root2));
    return false;
}

bool createObject(
        SymJoinCtx              &ctx,
        const struct cl_type    *clt,
        const TObjId            root1,
        const TObjId            root2,
        const EJoinStatus       action)
{
    EObjKind kind;
    if (!joinObjKind(&kind, ctx, root1, root2, action))
        return false;

    SegBindingFields bf;
    if (!joinSegBinding(&bf, ctx, root1, root2))
        return false;

    bool isProto;
    if (!joinProtoFlag(&isProto, ctx, root1, root2))
        return false;

    updateJoinStatus(ctx, action);

    // preserve 'prototype' flag
    const TObjId rootDst = ctx.dst.objCreate(clt);
    ctx.dst.objSetProto(rootDst, isProto);

    if (OK_CONCRETE != kind) {
        // abstract object
        ctx.dst.objSetAbstract(rootDst, kind, bf);

        // compute minimal length of the resulting segment
        const unsigned len1 = objMinLength(ctx.sh1, root1);
        const unsigned len2 = objMinLength(ctx.sh2, root2);
        ctx.segLengths[rootDst] = std::min(len1, len2);
    }

    return traverseSubObjs(ctx, root1, root2, rootDst);
}

bool joinAnonObjects(
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const int cbSize1 = ctx.sh1.objSizeOfAnon(o1);
    const int cbSize2 = ctx.sh2.objSizeOfAnon(o2);
    if (cbSize1 != cbSize2) {
        SJ_DEBUG("<-- anon object size mismatch " << SJ_OBJP(o1, o2));
        return false;
    }

    // create the join object
    const TObjId anon = ctx.dst.objCreateAnon(cbSize1);
    ctx.objMap1[o1] = anon;
    ctx.objMap2[o2] = anon;
    return defineValueMapping(ctx,
            ctx.sh1.placedAt(o1),
            ctx.sh2.placedAt(o2),
            ctx.dst.placedAt(anon));
}

bool followObjPair(
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2,
        const EJoinStatus       action,
        const bool              readOnly = false)
{
    const struct cl_type *clt;
    if (!joinObjClt(&clt, ctx, o1, o2))
        return false;

    // jump to roots
    const TObjId root1 = objRoot(ctx.sh1, o1);
    const TObjId root2 = objRoot(ctx.sh2, o2);
    if (hasKey(ctx.objMap1, root1)) {
        const TObjId rootDst = ctx.objMap1[root1];
        SymJoinCtx::TObjMap::const_iterator i2 = ctx.objMap2.find(root2);
        if (ctx.objMap2.end() == i2 || i2->second != rootDst) {
            SJ_DEBUG("<-- object root mismatch " << SJ_OBJP(root1, root2));
            return false;
        }

        // join mapping of object's address
        const TValueId addr1 = ctx.sh1.placedAt(root1);
        const TValueId addr2 = ctx.sh2.placedAt(root2);
        const TValueId dstAt = ctx.dst.placedAt(rootDst);
        return defineValueMapping(ctx, addr1, addr2, dstAt);
    }

    CL_BREAK_IF(root1 <= 0 || root2 <= 0);
    if (!joinObjClt(&clt, ctx, root1, root2))
        return false;

    if (!clt) {
        // anonymous object of known size
        return !readOnly
            && joinAnonObjects(ctx, root1, root2);
    }

    if (readOnly)
        // do not create any object, just check if it was possible
        return segMatchLookAhead(ctx, root1, root2);

    return createObject(ctx, clt, root1, root2, action);
}

bool followValuePair(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2)
{
    const struct cl_type *clt1, *clt2;
    const int cVal1 = ctx.sh1.valGetCustom(&clt1, v1);
    const int cVal2 = ctx.sh2.valGetCustom(&clt2, v2);
    if ((OBJ_INVALID == cVal1) != (OBJ_INVALID == cVal2) || (cVal1 != cVal2)) {
        SJ_DEBUG("<-- custom values mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (OBJ_INVALID != cVal1) {
        // matching pair of custom values
        const struct cl_type *clt;
        if (!joinClt(clt1, clt2, &clt)) {
            SJ_DEBUG("<-- custom value clt mismatch " << SJ_VALP(v1, v2));
            return false;
        }

        const TValueId vDst = ctx.dst.valCreateCustom(clt, cVal1);
        return defineValueMapping(ctx, v1, v2, vDst);
    }

    const TObjId o1 = ctx.sh1.pointsTo(v1);
    const TObjId o2 = ctx.sh2.pointsTo(v2);
    if (!checkNonPosValues(o1, o2)) {
        SJ_DEBUG("<-- target validity mismatch: " << SJ_VALP(v1, v2)
                 << " -> " << SJ_OBJP(o1, o2));
        return false;
    }

    if (0 < o1)
        return followObjPair(ctx, o1, o2, JS_USE_ANY);

    // special handling for OBJ_DELETED/OBJ_LOST
    CL_BREAK_IF(o1 != OBJ_DELETED && o1 != OBJ_LOST);
    const struct cl_type *clt;
    if (!joinValClt(&clt, ctx, v1, v2))
        return false;

    SymHeap &dst = ctx.dst;
    const TObjId objTmp = dst.objCreate(clt);
    const TValueId vDst = dst.placedAt(objTmp);

    // FIXME: avoid using of friend?
    SymHeapCore &core = dynamic_cast<SymHeapCore &>(dst);
    core.objDestroy(objTmp, /* OBJ_DELETED/OBJ_LOST */ o1);
    return defineValueMapping(ctx, v1, v2, vDst);
}

bool joinSegmentWithAny(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TObjId            root1,
        const TObjId            root2,
        const EJoinStatus       action)
{
    SJ_DEBUG(">>> joinSegmentWithAny" << SJ_OBJP(root1, root2));
    if (followObjPair(ctx, root1, root2, action, /* read-only */ true)) {
        // go ahead, try it read-write!
        *pResult = followObjPair(ctx, root1, root2, action);
        return true;
    }

    SJ_DEBUG("<<< joinSegmentWithAny" << SJ_OBJP(root1, root2));
    return false;
}

bool insertSegmentCloneHelper(
        SymJoinCtx                  &ctx,
        const SymHeap               &shGt,
        const TValueId              valGt,
        const SymJoinCtx::TObjMap   &objMapGt,
        const EJoinStatus           action)
{
    const TObjId objGt = objRootByVal(shGt, valGt);
    if (objGt < 0 || hasKey(objMapGt, objGt))
        // nothing to clone here
        return true;

    const struct cl_type *clt = shGt.objType(objGt);
    if (!clt)
        // TODO: clone anonymous prototypes?
        return true;

    SJ_DEBUG("+i+ insertSegmentClone: cloning object #" << objGt <<
             ", clt = " << *clt <<
             ", action = " << action);

    const TObjId root1 = (JS_USE_SH1 == action)
        ? objGt
        : static_cast<TObjId>(OBJ_INVALID);

    const TObjId root2 = (JS_USE_SH2 == action)
        ? objGt
        : static_cast<TObjId>(OBJ_INVALID);

    // clone the object
    ctx.tieBreaking.insert(TObjPair(root1, root2));
    if (createObject(ctx, clt, root1, root2, action))
        return true;

    SJ_DEBUG("<-- insertSegmentClone: failed to create object "
             << SJ_OBJP(root1, root2));
    return false;
}

bool insertSegmentClone(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EJoinStatus       action)
{
    SJ_DEBUG(">>> insertSegmentClone" << SJ_VALP(v1, v2));
    const bool isGt1 = (JS_USE_SH1 == action);
    const bool isGt2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(isGt1 == isGt2);

    // resolve the existing segment in shGt
    const SymHeap &shGt = (isGt1) ? ctx.sh1 : ctx.sh2;
    const TObjId seg = objRootByVal(shGt, (isGt1) ? v1 : v2);
    TObjId peer = seg;
    if (OK_DLS == shGt.objKind(seg))
        peer = dlSegPeer(shGt, seg);

    // resolve the 'next' pointer and check its validity
    const TValueId backGt = shGt.valueOf(nextPtrFromSeg(shGt, seg));
    const TValueId nextGt = shGt.valueOf(nextPtrFromSeg(shGt, peer));
    const TValueId nextLt = (isGt2) ? v1 : v2;
    if (!checkNonPosValues(nextGt, nextLt) || !checkValueMapping(ctx, 
                (isGt1) ? nextGt : nextLt,
                (isGt2) ? nextGt : nextLt,
                /* allowUnknownMapping */ true))
    {
        SJ_DEBUG("<-- insertSegmentClone: value mismatch "
                 "(nextLt = #" << nextLt << ", nextGt = #" << nextGt << ")");
        return false;
    }

    const SymJoinCtx::TObjMap &objMapGt = (isGt1) ? ctx.objMap1 : ctx.objMap2;

    TValPair vp(
            (isGt1) ? shGt.placedAt(seg) : static_cast<TValueId>(VAL_INVALID),
            (isGt2) ? shGt.placedAt(seg) : static_cast<TValueId>(VAL_INVALID));

    ctx.wl.schedule(vp);
    while (ctx.wl.next(vp)) {
        const TValueId valGt = (isGt1) ? vp.first : vp.second;
        const TValueId valLt = (isGt2) ? vp.first : vp.second;
        if (VAL_INVALID != valLt) {
            // process the rest of ctx.wl rather in joinPendingValues()
            ctx.wl.undo(vp);
            break;
        }

        if (backGt == valGt || nextGt == valGt)
            // do not go byond the segment, just follow its data
            continue;

        const EUnknownValue code = shGt.valGetUnknown(valGt);
        if (UV_UNINITIALIZED == code || UV_UNKNOWN == code) {
            // clone unknown value
            const struct cl_type *const clt = shGt.valType(valGt);
            const TValueId vDst = ctx.dst.valCreateUnknown(code, clt);
            if (defineValueMapping(ctx, vp.first, vp.second, vDst))
                return false;
        }
        else if (!insertSegmentCloneHelper(ctx, shGt, valGt, objMapGt, action))
            // clone failed
            return false;
    }

    if (VAL_NULL == nextGt && VAL_NULL == nextLt)
        // do not follow VAL_NULL pair
        return true;

    // FIXME: what about DLS back-link here?
    const TValPair next(
            (isGt1) ? nextGt : nextLt,
            (isGt2) ? nextGt : nextLt);

    if (ctx.wl.schedule(next)) {
        SJ_DEBUG("+++ " << SJ_VALP(next.first, next.second)
                 << " <- insertSegmentClone");
    }

    return true;
}

bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EUnknownValue     code1,
        const EUnknownValue     code2)
{
    const bool isAbs1 = (UV_ABSTRACT == code1);
    const bool isAbs2 = (UV_ABSTRACT == code2);
    const EJoinStatus subStatus = (isAbs1)
        ? JS_USE_SH1
        : JS_USE_SH2;

    const TObjId root1 = objRootByVal(ctx.sh1, v1);
    const TObjId root2 = objRootByVal(ctx.sh2, v2);
    if (0 < root1 && 0 < root2) {
        if (isAbs1 && isAbs2)
            return joinSegmentWithAny(pResult, ctx, root1, root2, JS_USE_ANY);

        CL_BREAK_IF(isAbs1 == isAbs2);
        if (joinSegmentWithAny(pResult, ctx, root1, root2, subStatus))
            return true;
    }

    *pResult = insertSegmentClone(ctx, v1, v2, subStatus);
    return true;
}

bool joinValuePair(SymJoinCtx &ctx, const TValueId v1, const TValueId v2) {
    const EUnknownValue code1 = ctx.sh1.valGetUnknown(v1);
    const EUnknownValue code2 = ctx.sh2.valGetUnknown(v2);

    EUnknownValue code;
    if (joinUnknownValuesCode(&code, code1, code2)) {
        // create unknown value
        const struct cl_type *clt;
        if (!joinValClt(&clt, ctx, v1, v2))
            return false;

        const TValueId vDst = ctx.dst.valCreateUnknown(code, clt);
        return defineValueMapping(ctx, v1, v2, vDst);
    }

    bool result;
    if ((UV_ABSTRACT == code1 || UV_ABSTRACT == code2)
            && joinAbstractValues(&result, ctx, v1, v2, code1, code2))
        return result;

    if (code1 != code2) {
        SJ_DEBUG("<-- unknown value code mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    return followValuePair(ctx, v1, v2);
}

bool joinPendingValues(SymJoinCtx &ctx) {
    TValPair vp;
    while (ctx.wl.next(vp)) {
        const TValueId v1 = vp.first;
        const TValueId v2 = vp.second;

        if (!joinValuePair(ctx, v1, v2))
            return false;
    }

    return true;
}

bool joinCVars(SymJoinCtx &ctx) {
    SymHeap         &dst = ctx.dst;
    const SymHeap   &sh1 = ctx.sh1;
    const SymHeap   &sh2 = ctx.sh2;

    // gather program variables
    SymHeap::TContCVar cVars1, cVars2;
    sh1.gatherCVars(cVars1);
    sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2) {
        SJ_DEBUG("<-- different program variables");
        return false;
    }

    // go through all program variables
    BOOST_FOREACH(const CVar &cv, cVars1) {
        const TObjId root1 = sh1.objByCVar(cv);
        const TObjId root2 = sh2.objByCVar(cv);

        // create a corresponding program variable in the resulting heap
        const struct cl_type *clt = sh1.objType(root1);
        const TObjId rootDst = dst.objCreate(clt, cv);

        // look at the values inside
        if (!traverseSubObjs(ctx, root1, root2, rootDst))
            return false;
    }

    // all OK
    return true;
}

/// known to work only for TObjId/TValueId
template <class TMap>
typename TMap::mapped_type roMapLookup(
        const TMap                          &roMap,
        const typename TMap::mapped_type    id)
{
    if (id <= 0)
        return id;

    typename TMap::const_iterator iter = roMap.find(id);
    return (roMap.end() == iter)
        ? static_cast<typename TMap::mapped_type>(-1)
        : iter->second;
}

template <class TItem, class TBlackList>
void setDstValuesCore(
        SymJoinCtx              &ctx,
        const TItem             &rItem,
        const TBlackList        &blackList)
{
    const TObjId objDst = rItem.first;
    CL_BREAK_IF(objDst < 0);
    if (hasKey(blackList, objDst))
        return;

    const TObjPair &orig = rItem.second;
    const TObjId obj1 = orig.first;
    const TObjId obj2 = orig.second;
    CL_BREAK_IF(OBJ_INVALID == obj1 && OBJ_INVALID == obj2);

    const TValueId v1 = ctx.sh1.valueOf(obj1);
    const TValueId v2 = ctx.sh2.valueOf(obj2);

    const bool isComp1 = (OBJ_INVALID != ctx.sh1.valGetCompositeObj(v1));
    const bool isComp2 = (OBJ_INVALID != ctx.sh2.valGetCompositeObj(v2));
    if (isComp1 || isComp2) {
        // do not bother by composite values
        CL_BREAK_IF(OBJ_INVALID != obj1 && !isComp1);
        CL_BREAK_IF(OBJ_INVALID != obj2 && !isComp2);
        return;
    }

    const TValueId vDstBy1 = roMapLookup(ctx.valMap1[/* ltr */ 0], v1);
    const TValueId vDstBy2 = roMapLookup(ctx.valMap2[/* ltr */ 0], v2);
    if (vDstBy1 == vDstBy2) {
        if (&ctx.sh1 == &ctx.sh2 && VAL_INVALID == vDstBy1)
            // we are called from joinData() and perhaps hit the binding fields
            return;

        // the values are equal --> pick any
        ctx.dst.objSetValue(objDst, vDstBy1);
        return;
    }

    // tie breaking
    bool use1 = false;
    bool use2 = false;
    if (obj2 < 0)
        use1 = true;
    else if (obj1 < 0)
        use2 = true;
    else {
        const TObjId target1 = objRootByVal(ctx.sh1, v1);
        const TObjId target2 = objRootByVal(ctx.sh2, v2);

        const TObjPair rp1(target1, OBJ_INVALID);
        const TObjPair rp2(OBJ_INVALID, target2);

        use1 = hasKey(ctx.tieBreaking, rp1);
        use2 = hasKey(ctx.tieBreaking, rp2);
    }

    CL_BREAK_IF(use1 == use2);
    const TValueId vDst = (use1) ? vDstBy1 : vDstBy2;

    // set the value
    ctx.dst.objSetValue(objDst, vDst);
}

void setDstValues(SymJoinCtx &ctx, const std::set<TObjId> *blackList = 0) {
    typedef SymJoinCtx::TObjMap TObjMap;
    typedef std::map<TObjId /* objDst */, TObjPair> TMap;
    TMap rMap;

    // reverse mapping for ctx.objMap1
    BOOST_FOREACH(TObjMap::const_reference ref, ctx.objMap1) {
        const TObjId objDst = ref.second;
        if (!hasKey(rMap, objDst))
            rMap[objDst].second = OBJ_INVALID;

        // objDst -> obj1
        rMap[objDst].first = ref.first;
    }

    // reverse mapping for ctx.objMap2
    BOOST_FOREACH(TObjMap::const_reference ref, ctx.objMap2) {
        const TObjId objDst = ref.second;
        if (!hasKey(rMap, objDst))
            rMap[objDst].first = OBJ_INVALID;

        // objDst -> obj2
        rMap[objDst].second = ref.first;
    }

    std::set<TObjId> emptyBlackList;
    if (!blackList)
        blackList = &emptyBlackList;

    BOOST_FOREACH(TMap::const_reference rItem, rMap) {
        setDstValuesCore(ctx, rItem, blackList);
    }
}

bool matchPreds(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValMapBidir      &vMap)
{
    return sh1.matchPreds(sh2, vMap[/* ltr */ 0])
        && sh2.matchPreds(sh1, vMap[/* rtl */ 1]);
}

void handleDstPreds(SymJoinCtx &ctx) {
    // go through all segments and initialize minLength
    BOOST_FOREACH(SymJoinCtx::TSegLengths::const_reference ref, ctx.segLengths)
    {
        const TObjId    seg = ref.first;
        const unsigned  len = ref.second;
        segSetMinLength(ctx.dst, seg, len);
    }

    BOOST_FOREACH(const TValPair neq, ctx.sharedNeqs) {
        TValueId valLt, valGt;
        boost::tie(valLt, valGt) = neq;

        const TObjId targetLt = ctx.dst.pointsTo(valLt);
        const TObjId targetGt = ctx.dst.pointsTo(valGt);
        if (hasKey(ctx.segLengths, targetLt)
                || hasKey(ctx.segLengths, targetGt))
            // preserve segment length
            continue;

        ctx.dst.neqOp(SymHeap::NEQ_ADD, valLt, valGt);
    }

    // cross-over check of Neq predicates
    if (!matchPreds(ctx.sh1, ctx.dst, ctx.valMap1))
        updateJoinStatus(ctx, JS_USE_SH2);
    if (!matchPreds(ctx.sh2, ctx.dst, ctx.valMap2))
        updateJoinStatus(ctx, JS_USE_SH1);
}

bool segDetectSelfLoopHelper(
        const SymHeap           &sh,
        std::set<TObjId>        &haveSeen,
        TObjId                  seg)
{
    // remember original kind of object
    const EObjKind kind = sh.objKind(seg);

    // find a loop-less path
    std::set<TObjId> path;
    while (insertOnce(path, seg)) {
        TObjId peer = seg;
        if (OK_DLS == kind) {
            // jump to peer in case of DLS
            peer = dlSegPeer(sh, seg);
            if (!insertOnce(path, peer))
                break;
        }

        const TValueId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
        TObjId next = sh.pointsTo(valNext);
        if (next < 0)
            // no valid next object --> no loop
            return false;

        const EObjKind kindNext = sh.objKind(next);
        if (kindNext != kind && kindNext != OK_HEAD)
            // no compatible next segment --> no loop
            return false;

        seg = objRoot(sh, next);
        if (kind != sh.objKind(seg))
            // no compatible next segment --> no loop
            return false;

        // optimization
        haveSeen.insert(seg);
    }

    // loop detected!
    return true;
}

// FIXME: not tested
bool segDetectSelfLoop(const SymHeap &sh) {
    using namespace boost::lambda;

    // gather all root objects
    SymHeap::TContObj roots;
    sh.gatherRootObjs(roots);

    // filter segment roots from there
    std::set<TObjId> segRoots;
    std::copy_if(roots.begin(), roots.end(),
                 std::inserter(segRoots, segRoots.begin()),
                 bind(objIsSeg, std::cref(sh), _1, false));

    // go through all entries
    std::set<TObjId> haveSeen;
    BOOST_FOREACH(const TObjId seg, segRoots) {
        if (!insertOnce(haveSeen, seg))
            continue;

        if (segDetectSelfLoopHelper(sh, haveSeen, seg))
            // cycle detected!
            return true;
    }

    // found nothing harmful
    return false;
}

bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *pDst,
        const SymHeap           &sh1,
        const SymHeap           &sh2)
{
    // FIXME: provide SymHeap::clear() to achieve this?
    *pDst = SymHeap();

    // initialize symbolic join ctx
    SymJoinCtx ctx(*pDst, sh1, sh2);

    // start with program variables
    if (!joinCVars(ctx)) {
        CL_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // go through all values in them
    if (!joinPendingValues(ctx)) {
        CL_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // time to preserve all 'hasValue' edges and shared Neq predicates
    setDstValues(ctx);
    handleDstPreds(ctx);

    if (JS_THREE_WAY == ctx.status && segDetectSelfLoop(ctx.dst)) {
        // purely segmental loops cause us problems
        CL_DEBUG(">J< segment cycle detected, cancelling three-way join...");
        return false;
    }

    if (debugSymJoin) {
        // catch possible regression at this point
        CL_BREAK_IF((JS_USE_ANY == ctx.status) != areEqual(sh1, sh2));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh1, ctx.dst));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh2, ctx.dst));
    }

    if (JS_THREE_WAY == ctx.status) {
#if SE_DISABLE_THREE_WAY_JOIN
        CL_WARN("three-way join disabled by configuration, recompile "
                "with SE_DISABLE_THREE_WAY_JOIN == 0 to enable it");
        return false;
#else
        CL_WARN("three-way join enabled by configuration, recompile "
                "with SE_DISABLE_THREE_WAY_JOIN == 1 to disable it");
#endif
    }

    // all OK
    *pStatus = ctx.status;
    return true;
}

class GhostMapper {
    private:
        TValMap                 &vMap_;

    public:
        GhostMapper(TValMap &vMap):
            vMap_(vMap)
        {
        }

        bool operator()(const SymHeap &sh, const TObjPair &item) {
            // obtain addresses
            const TValueId addrReal  = sh.placedAt(item.first);
            const TValueId addrGhost = sh.placedAt(item.second);
            CL_BREAK_IF(addrReal < 0 || addrGhost < 0);
            CL_BREAK_IF(addrReal == addrGhost);

            // wait, first we need to translate the address into ctx.dst world
            const TValueId image = roMapLookup(vMap_, addrReal);
            CL_BREAK_IF(image <= 0);

            // introduce ghost mapping
            CL_BREAK_IF(hasKey(vMap_, addrGhost));
            vMap_[addrGhost] = image;

            return /* continue */ true;
        }
};

void mapGhostAddressSpace(
        SymJoinCtx              &ctx,
        const TObjId            objReal,
        const TObjId            objGhost,
        const EJoinStatus       action)
{
    const SymHeap &sh = ctx.sh1;
    CL_BREAK_IF(&sh != &ctx.sh2);
    CL_BREAK_IF(objReal < 0 || objGhost < 0);

    TValMapBidir &vMap = (JS_USE_SH1 == action)
        ? ctx.valMap1
        : ctx.valMap2;

    GhostMapper visitor(vMap[/* ltr */ 0]);
    const TObjPair root(objReal, objGhost);

    visitor(sh, root);
    traverseSubObjs(sh, root, visitor, /* leavesOnly */ false);
}

bool dlSegCheckProtoConsistency(SymJoinCtx &ctx) {
    BOOST_FOREACH(const TObjTriple &proto, ctx.protoRoots) {
        const TObjId obj1   = proto[0];
        const TObjId obj2   = proto[1];
        if (OK_DLS != ctx.sh1.objKind(obj1))
            // we are intersted only DLSs here
            continue;

        CL_BREAK_IF(OK_DLS != ctx.sh2.objKind(obj2));
        CL_BREAK_IF(OK_DLS != ctx.dst.objKind(proto[/* dst */ 2]));
        const TObjId peer1 = dlSegPeer(ctx.sh1, obj1);
        const TObjId peer2 = dlSegPeer(ctx.sh2, obj2);

        const TObjId peerDstBy1 = roMapLookup(ctx.objMap1, peer1);
        const TObjId peerDstBy2 = roMapLookup(ctx.objMap2, peer2);
        if (peerDstBy1 != peerDstBy2) {
            SJ_DEBUG("<-- DLS prototype peer mismatch " << SJ_OBJP(obj1, obj2));
            return false;
        }

        TObjTriple protoPeer;
        protoPeer[0] = peer1;
        protoPeer[1] = peer2;
        protoPeer[2] = peerDstBy1;
        if (hasKey(ctx.protoRoots, protoPeer))
            continue;

        SJ_DEBUG("<-- DLS prototype peer not a prototype "
                 << SJ_OBJP(obj1, obj2));
        return false;
    }

    return true;
}

bool joinDataCore(
        SymJoinCtx              &ctx,
        const SegBindingFields  &bf,
        const TObjId            o1,
        const TObjId            o2)
{
    const SymHeap &sh = ctx.sh1;
    CL_BREAK_IF(&sh != &ctx.sh2);

    const struct cl_type *clt;
    if (!joinClt(ctx.sh1.objType(o1), sh.objType(o2), &clt) || !clt) {
#ifndef NDEBUG
        // why are we called actually?
        CL_TRAP;
#endif
        return false;
    }

    // TODO
    //CL_BREAK_IF(OK_DLS == sh.objKind(o1) || OK_DLS == sh.objKind(o2));

    const TObjId rootDst = ctx.dst.objCreate(clt);
    if (!traverseSubObjs(ctx, o1, o2, rootDst, &bf))
        return false;

    // never step over DLS peer
    if (OK_DLS == sh.objKind(o1)) {
        const TObjId peer = dlSegPeer(sh, o1);
        if (peer != o2)
            mapGhostAddressSpace(ctx, o1, peer, JS_USE_SH1);
    }
    if (OK_DLS == sh.objKind(o2)) {
        const TObjId peer = dlSegPeer(sh, o2);
        if (peer != o1)
            mapGhostAddressSpace(ctx, o2, peer, JS_USE_SH2);
    }

    if (!joinPendingValues(ctx))
        return false;

    // check consistency of DLS prototype peers
    dlSegCheckProtoConsistency(ctx);

    std::set<TObjId> blackList;
    buildIgnoreList(blackList, ctx.dst, rootDst, bf);
    setDstValues(ctx, &blackList);

    handleDstPreds(ctx);

    if (JS_THREE_WAY == ctx.status) {
        CL_WARN("three-way join not yet allowed for joinDataCore()");
        return false;
    }

    return true;
}

bool joinDataReadOnly(
        const SymHeap           &sh,
        const SegBindingFields  &bf,
        const TObjId            o1,
        const TObjId            o2,
        SymHeap::TContObj       protoRoots[1][2])
{
    SymHeap tmp;
    SymJoinCtx ctx(
            /* dst */ tmp,
            /* sh1 */ sh,
            /* sh2 */ sh);

    if (!joinDataCore(ctx, bf, o1, o2))
        return false;

    if (protoRoots) {
        BOOST_FOREACH(const TObjTriple &proto, ctx.protoRoots) {
            (*protoRoots)[0].push_back(proto[0]);
            (*protoRoots)[1].push_back(proto[1]);
        }
    }

    return true;
}

struct JoinValueVisitor {
    SymJoinCtx                  &ctx;
    std::set<TObjId>            ignoreList;
    bool                        bidir;

    JoinValueVisitor(SymJoinCtx &ctx_):
        ctx(ctx_)
    {
    }

    bool operator()(SymHeap &sh, TObjPair item) const {
        const TObjId dst = item.first;
        const TObjId src = item.second;
        if (hasKey(ignoreList, dst))
            return /* continue */ true;

        const TValueId oldDst = sh.valueOf(dst);
        const TValueId oldSrc = sh.valueOf(src);

        const TValueId newDst = roMapLookup(ctx.valMap1[/* ltr */ 0], oldDst);
        const TValueId newSrc = roMapLookup(ctx.valMap2[/* ltr */ 0], oldSrc);

        CL_BREAK_IF(newDst != newSrc);
        if (VAL_INVALID == newDst)
            // shared data
            return true;

        sh.objSetValue(dst, newDst);
        if (collectJunk(sh, oldDst))
            CL_DEBUG("    JoinValueVisitor drops a sub-heap (oldDst)");

        if (!this->bidir)
            return /* continue */ true;

        sh.objSetValue(src, newSrc);
        if (collectJunk(sh, oldSrc))
            CL_DEBUG("    JoinValueVisitor drops a sub-heap (oldSrc)");

        return /* continue */ true;
    }
};

void recoverPointersToSelf(
        SymHeap                 &sh,
        const TObjId            dst,
        const TObjId            src,
        const TObjId            ghost,
        const bool              bidir)
{
    redirectInboundEdges(sh,
            /* pointingFrom */  dst,
            /* pointingTo   */  ghost,
            /* redirectTo   */  dst);

    if (!bidir)
        return;

    redirectInboundEdges(sh,
            /* pointingFrom */  src,
            /* pointingTo   */  ghost,
            /* redirectTo   */  src);
}

/// future replacment of matchData() from symdiscover
bool joinData(
        SymHeap                 &sh,
        const TObjId            dst,
        const TObjId            src,
        const bool              bidir)
{
    SymJoinCtx ctx(
            /* dst */ sh,
            /* sh1 */ sh,
            /* sh2 */ sh);

    // dst is expected to be a segment
    CL_BREAK_IF(OK_SLS != sh.objKind(dst) && OK_DLS != sh.objKind(dst));
    const SegBindingFields bf(sh.objBinding(dst));

    if (!joinDataCore(ctx, bf, dst, src))
        // TODO: collect the already created dangling object and return the heap
        //       in a more consistent shape!
        return false;

    const TObjId dstGhost = roMapLookup(ctx.objMap1, dst);
    CL_BREAK_IF(dstGhost != roMapLookup(ctx.objMap2, src));

    const unsigned cntProto = ctx.protoRoots.size();
    if (cntProto)
        CL_DEBUG("    joinData() merges " << cntProto << " prototype objects");

    // go through prototypes
    BOOST_FOREACH(const TObjTriple &proto, ctx.protoRoots) {
        const TObjId protoDst   = proto[/* sh1 */ 0];
        const TObjId protoSrc   = proto[/* sh2 */ 1];
        const TObjId protoGhost = proto[/* dst */ 2];

        if (objIsSeg(sh, protoDst))
            // remove Neq predicates, their targets are going to vanish soon
            segSetMinLength(sh, protoDst, 0);

        if (bidir && objIsSeg(sh, protoSrc))
            // remove Neq predicates, their targets are going to vanish soon
            segSetMinLength(sh, protoSrc, 0);

        if (objIsSeg(sh, protoGhost))
            // temporarily remove Neq predicates
            segSetMinLength(sh, protoGhost, 0);

        redirectInboundEdges(sh,
                /* pointingFrom */  protoGhost,
                /* pointingTo   */  dstGhost,
                /* redirectTo   */  dst);

        sh.objSetProto(protoGhost, true);
    }

    JoinValueVisitor visitor(ctx);
    visitor.bidir = bidir;
    buildIgnoreList(visitor.ignoreList, sh, dst);

    // traverse all sub-objects
    const TObjPair item(dst, src);
    traverseSubObjs(sh, item, visitor, /* leavesOnly */ true);

    // pointers to self should remain pointers to self
    recoverPointersToSelf(sh, dst, src, dstGhost, bidir);

    // restore minimal length of segment prototypes
    BOOST_FOREACH(const TObjTriple &proto, ctx.protoRoots) {
        typedef SymJoinCtx::TSegLengths TLens;
        const TLens &lens = ctx.segLengths;

        const TObjId protoDst = proto[/* dst */ 2];
        TLens::const_iterator it = lens.find(protoDst);
        if (lens.end() == it)
            continue;

        const unsigned len = it->second;
        if (len)
            segSetMinLength(sh, protoDst, len);
    }

    if (collectJunk(sh, sh.placedAt(dstGhost)))
        CL_DEBUG("    JoinValueVisitor drops a sub-heap (dstGhost)");

    return true;
}

