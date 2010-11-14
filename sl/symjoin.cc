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
#include <cl/clutil.hh>

#include "symclone.hh"
#include "symcmp.hh"
#include "symseg.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "worklist.hh"

#include <map>

#include <boost/foreach.hpp>

/* static */ bool debugSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debugSymJoin)                                                     \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

#define SJ_VALP(v1, v2) "(v1 = #" << v1 << ", v2 = #" << v2 << ")"
#define SJ_OBJP(v1, v2) "(o1 = #" << v1 << ", o2 = #" << v2 << ")"

typedef boost::array<const SymHeap *, 3>        TSymHeapTriple;
typedef boost::array<TObjId         , 3>        TObjTriple;

struct SymJoinCtx {
    SymHeap                     &dst;
    const SymHeap               &sh1;
    const SymHeap               &sh2;

    typedef std::map<TObjId /* src */, TObjId /* dst */>        TObjMap;
    TObjMap                     objMap1;
    TObjMap                     objMap2;

    TValMapBidir                valMap1;
    TValMapBidir                valMap2;

    WorkList<TValPair>          wl;
    EJoinStatus                 status;

    typedef std::map<TObjId /* seg */, unsigned /* len */>      TSegLengths;
    TSegLengths                 segLengths;

    SymJoinCtx(SymHeap &dst_, const SymHeap &sh1_, const SymHeap &sh2_):
        dst(dst_),
        sh1(sh1_),
        sh2(sh2_),
        status(JS_USE_ANY)
    {
    }
};

void updateJoinStatus(SymJoinCtx &ctx, const EJoinStatus action) {
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

bool defineValueMapping(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const TValueId          vDst)
{
    const bool ok1 = matchPlainValues(ctx.valMap1, v1, vDst);
    const bool ok2 = matchPlainValues(ctx.valMap2, v2, vDst);
    if (ok1 && ok2)
        // no inconsistency so far
        return true;

    SJ_DEBUG("<-- value mapping mismatch " << SJ_VALP(v1, v2));
    return false;
}

/// (OBJ_INVALID == objDst) means read-only!!!
bool joinFreshObjTripple(
        SymJoinCtx              &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const TObjId            objDst)
{
    const bool readOnly = (OBJ_INVALID == objDst);

    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;
    SymHeap       &dst = ctx.dst;

    const TValueId v1 = sh1.valueOf(obj1);
    const TValueId v2 = sh2.valueOf(obj2);
    if (VAL_NULL == v1 && VAL_NULL == v2) {
        if (!readOnly)
            dst.objSetValue(objDst, VAL_NULL);
        return true;
    }

    const TObjId cObj1 = sh1.valGetCompositeObj(v1);
    const TObjId cObj2 = sh2.valGetCompositeObj(v2);
    if ((OBJ_INVALID == cObj1) != (OBJ_INVALID == cObj2)) {
        SJ_DEBUG("<-- scalar vs. composite value " << SJ_VALP(v1, v2));
        return false;
    }

    if (!readOnly && OBJ_INVALID != cObj1) {
        // store mapping of composite object's values
        const TValueId vDst = dst.valueOf(objDst);
        if (!defineValueMapping(ctx, v1, v2, vDst))
            return false;
    }

    // special values have to match (NULL not treated as special here)
    // TODO: should we consider also join of VAL_TRUE/VAL_FALSE?
    if (v1 < 0 || v2 < 0) {
        if (v1 == v2)
            return true;

        SJ_DEBUG("<-- special value mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    // TODO: rewrite
#if 1
    TValMap::const_iterator i1 = ctx.valMap1[/* ltr */ 0].find(v1);
    TValMap::const_iterator i2 = ctx.valMap2[/* ltr */ 0].find(v2);

    const bool hasTarget1 = (ctx.valMap1[/* ltr */ 0].end() != i1);
    const bool hasTarget2 = (ctx.valMap2[/* ltr */ 0].end() != i2);
    if (hasTarget1 || hasTarget2) {
        const TValueId vDst1 = (hasTarget1)
            ? i1->second
            : static_cast<TValueId>(VAL_INVALID);

        const TValueId vDst2 = (hasTarget2)
            ? i2->second
            : static_cast<TValueId>(VAL_INVALID);

        if (hasTarget1 && hasTarget1 && (vDst1 == vDst2))
            return true;

        SJ_DEBUG("<-- value mapping mismatch: " << SJ_VALP(v1, v2)
                 "-> " << SJ_VALP(vDst1, vDst2));
        return false;
    }
#endif
    if (!readOnly && ctx.wl.schedule(TValPair(v1, v2)))
        SJ_DEBUG("+++ " << SJ_VALP(v1, v2) << " <- " << SJ_OBJP(obj1, obj2));

    return true;
}

class ObjJoinVisitor {
    private:
        SymJoinCtx &ctx_;

    public:
        ObjJoinVisitor(SymJoinCtx &ctx): ctx_(ctx) { }

        bool operator()(const TObjTriple &item) {
            const TObjId obj1   = item[0];
            const TObjId obj2   = item[1];
            const TObjId objDst = item[2];

            // store object IDs mapping
            ctx_.objMap1[obj1]  = objDst;
            ctx_.objMap2[obj2]  = objDst;

            // store object's address
            const TValueId addr1 = ctx_.sh1.placedAt(obj1);
            const TValueId addr2 = ctx_.sh2.placedAt(obj2);
            const TValueId dstAt = ctx_.dst.placedAt(objDst);
            if (!defineValueMapping(ctx_, addr1, addr2, dstAt))
                return false;

            return /* continue */ joinFreshObjTripple(ctx_, obj1, obj2, objDst);
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
        const TObjId            rootDst)
{
#if SE_SELF_TEST
    // all three types have to match!
    const struct cl_type *clt1   = ctx.sh1.objType(root1);
    const struct cl_type *clt2   = ctx.sh2.objType(root2);
    const struct cl_type *cltDst = ctx.dst.objType(rootDst);
    SE_BREAK_IF(!clt1 || !clt2 || !cltDst);
    SE_BREAK_IF(*clt1 != *cltDst);
    SE_BREAK_IF(*clt2 != *cltDst);
#endif
    TObjTriple root;
    root[/* sh1 */ 0] = root1;
    root[/* sh2 */ 1] = root2;
    root[/* dst */ 2] = rootDst;

    TSymHeapTriple sht;
    sht[0] = &ctx.sh1;
    sht[1] = &ctx.sh2;
    sht[2] = &ctx.dst;

    // guide the visitors through them
    ObjJoinVisitor objVisitor(ctx);
    return objVisitor(root)
        && traverseSubObjs<3>(sht, root, objVisitor);
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

bool joinClt(
        const struct cl_type    **pDst,
        const struct cl_type    *clt1,
        const struct cl_type    *clt2)
{
    const bool anon1 = !clt1;
    const bool anon2 = !clt2;
    if (anon1 && anon2) {
        *pDst = 0;
        return true;
    }

    if (anon1 != anon2)
        return false;

    SE_BREAK_IF(anon1 || anon2);
    if (*clt1 != *clt2)
        return false;

    *pDst = clt1;
    return true;
}

bool joinValClt(
        const struct cl_type    **pDst,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2)
{
    const struct cl_type *clt1 = ctx.sh1.valType(v1);
    const struct cl_type *clt2 = ctx.sh2.valType(v2);
    if (joinClt(pDst, clt1, clt2))
        return true;

    SJ_DEBUG("<-- value clt mismatch " << SJ_VALP(v1, v2));
    return false;
}

bool joinObjClt(
        const struct cl_type    **pDst,
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const struct cl_type *clt1 = ctx.sh1.objType(o1);
    const struct cl_type *clt2 = ctx.sh2.objType(o2);
    if (joinClt(pDst, clt1, clt2))
        return true;

    SJ_DEBUG("<-- object clt mismatch " << SJ_OBJP(o1, o2));
    return false;
}

bool joinObjKind(
        EObjKind                *pDst,
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2,
        const EJoinStatus       action)
{
    const EObjKind kind1 = ctx.sh1.objKind(o1);
    const EObjKind kind2 = ctx.sh2.objKind(o2);
    if (OK_CONCRETE == kind1) {
        SE_BREAK_IF(action == JS_USE_SH1);
        *pDst = kind2;
        return true;
    }

    if (OK_CONCRETE == kind2) {
        SE_BREAK_IF(action == JS_USE_SH2);
        *pDst = kind1;
        return true;
    }

    if (kind1 == kind2) {
        SE_BREAK_IF(action != JS_USE_ANY);
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
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const bool isSeg1 = (OK_CONCRETE != ctx.sh1.objKind(o1));
    const bool isSeg2 = (OK_CONCRETE != ctx.sh2.objKind(o2));
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
    SE_TRAP;
    return false;
}

unsigned joinSegLength(
        SymJoinCtx              &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    const unsigned len1 = (OK_CONCRETE == sh1.objKind(o1))
        ? /* OK_CONCRETE */ 1
        : segMinLength(sh1, o1);

    const unsigned len2 = (OK_CONCRETE == sh2.objKind(o2))
        ? /* OK_CONCRETE */ 1
        : segMinLength(sh2, o2);

    return (len1 < len2)
        ? len1
        : len2;
}

bool createObject(
        SymJoinCtx              &ctx,
        const struct cl_type    *clt,
        const TObjId            root1,
        const TObjId            root2,
        const EJoinStatus       action)
{
    const SymHeap   &sh1 = ctx.sh1;
    const SymHeap   &sh2 = ctx.sh2;
    SymHeap         &dst = ctx.dst;

    EObjKind kind;
    if (!joinObjKind(&kind, ctx, root1, root2, action))
        return false;

    SegBindingFields bf;
    if (!joinSegBinding(&bf, ctx, root1, root2))
        return false;

    const bool isProto = sh1.objIsProto(root1);
    if (isProto != sh2.objIsProto(root2)) {
        SJ_DEBUG("<-- prototype vs shared: " << SJ_OBJP(root1, root2));
        return false;
    }

    updateJoinStatus(ctx, action);

    // preserve 'prototype' flag
    const TObjId rootDst = ctx.dst.objCreate(clt);
    dst.objSetProto(rootDst, isProto);

    if (OK_CONCRETE != kind) {
        dst.objSetAbstract(rootDst, kind, bf);
        ctx.segLengths[rootDst] = joinSegLength(ctx, root1, root2);
    }

    return traverseSubObjs(ctx, root1, root2, rootDst);
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

    const TObjId root1 = objRoot(ctx.sh1, o1);
    const TObjId root2 = objRoot(ctx.sh2, o2);
    if (hasKey(ctx.objMap1, root1)) {
        const TObjId rootDst = ctx.objMap1[root1];
        SymJoinCtx::TObjMap::const_iterator i2 = ctx.objMap2.find(root2);
        if (ctx.objMap2.end() == i2 || i2->second != rootDst) {
            SJ_DEBUG("<-- object root mismatch " << SJ_OBJP(root1, root2));
            return false;
        }

        const TValueId addr1 = ctx.sh1.placedAt(root1);
        const TValueId addr2 = ctx.sh2.placedAt(root2);
        const TValueId dstAt = ctx.dst.placedAt(rootDst);
        return defineValueMapping(ctx, addr1, addr2, dstAt);
    }

    SE_BREAK_IF(root1 <= 0 || root2 <= 0);
    if (!joinObjClt(&clt, ctx, root1, root2))
        return false;

    if (!clt) {
        // TODO: anonymous object of known size
#if SE_SELF_TEST
        SE_TRAP;
#endif
        return false;
    }

    if (readOnly)
        return segMatchLookAhead(ctx, root1, root2);
    else
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
        if (!joinClt(&clt, clt1, clt2)) {
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
#if SE_SELF_TEST
    switch (o1) {
        case OBJ_DELETED:
        case OBJ_LOST:
            if (o1 == o2)
                break;
        default:
            SE_TRAP;
    }
#endif

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

bool joinUnkownValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EUnknownValue     code1,
        const EUnknownValue     code2)
{
    SE_BREAK_IF((code1 == code2) && UV_UNKNOWN != code1);
    const struct cl_type *clt;
    if (!joinValClt(&clt, ctx, v1, v2)) {
        *pResult = false;
        return true;
    }

    const bool isUnknown1 = (UV_UNKNOWN == code1);
    const bool isUnknown2 = (UV_UNKNOWN == code2);
    if (isUnknown1 && isUnknown2) {
        const TValueId vDst = ctx.dst.valCreateUnknown(UV_UNKNOWN, clt);
        *pResult = defineValueMapping(ctx, v1, v2, vDst);
        return true;
    }

    // TODO
    SE_BREAK_IF(debugSymJoin);
    return false;
}

bool joinSegmentWithAny(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TObjId            root1,
        const TObjId            root2,
        const EJoinStatus       action)
{
    SJ_DEBUG(">>> joinSegmentWithAny" << SJ_OBJP(root1, root2));
    if (followObjPair(ctx, root1, root2, action, /* roSegMatch */ true)) {
        // go ahead, try it read-write!
        *pResult = followObjPair(ctx, root1, root2, action);
        return true;
    }

    SJ_DEBUG("<<< joinSegmentWithAny" << SJ_OBJP(root1, root2));
    return false;
}

bool insertSegmentClone(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EJoinStatus       action)
{
    // TODO
    (void) pResult;
    (void) ctx;
    (void) v1;
    (void) v2;
    (void) action;

    //SE_BREAK_IF(debugSymJoin);
    return false;
}

// FIXME: highly experimental
bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EUnknownValue     code1,
        const EUnknownValue     code2)
{
    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    const bool isAbs1 = (UV_ABSTRACT == code1);
    const bool isAbs2 = (UV_ABSTRACT == code2);
    const EJoinStatus subStatus = (isAbs1)
        ? JS_USE_SH1
        : JS_USE_SH2;

    const TObjId root1 = objRoot(sh1, sh1.pointsTo(v1));
    const TObjId root2 = objRoot(sh2, sh2.pointsTo(v2));
    if (root1 < 0 || root2 < 0)
        return insertSegmentClone(pResult, ctx, v1, v2, subStatus);

    if (isAbs1 && isAbs2)
        return joinSegmentWithAny(pResult, ctx, root1, root2, JS_USE_ANY);

    SE_BREAK_IF(isAbs1 == isAbs2);
    if (joinSegmentWithAny(pResult, ctx, root1, root2, subStatus))
        return true;

    return insertSegmentClone(pResult, ctx, v1, v2, subStatus);
}

bool joinValueCore(
        SymJoinCtx              &ctx,
        const EUnknownValue     code,
        const TValueId          v1,
        const TValueId          v2)
{
    switch (code) {
        case UV_ABSTRACT:
        case UV_KNOWN:
            return followValuePair(ctx, v1, v2);

        case UV_UNKNOWN:
        case UV_UNINITIALIZED:
            break;
    }

    // join clt of unknown values
    const struct cl_type *clt;
    if (!joinValClt(&clt, ctx, v1, v2))
        return false;

    // create a new unknown value
    const TValueId vDst = ctx.dst.valCreateUnknown(code, clt);
    return defineValueMapping(ctx, v1, v2, vDst);
}

bool joinValuePair(SymJoinCtx &ctx, const TValueId v1, const TValueId v2) {
    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    const EUnknownValue code1 = sh1.valGetUnknown(v1);
    const EUnknownValue code2 = sh2.valGetUnknown(v2);

    bool result;
    if ((UV_UNKNOWN == code1 || UV_UNKNOWN == code2)
            && joinUnkownValues(&result, ctx, v1, v2, code1, code2))
        return result;

    if ((UV_ABSTRACT == code1 || UV_ABSTRACT == code2)
            && joinAbstractValues(&result, ctx, v1, v2, code1, code2))
        return result;

    if (code1 != code2) {
        SJ_DEBUG("<-- unknown value code mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    return joinValueCore(ctx, code1, v1, v2);
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

void setDstValues(SymJoinCtx &ctx) {
    SE_BREAK_IF(ctx.objMap1.size() != ctx.objMap2.size());

    BOOST_FOREACH(SymJoinCtx::TObjMap::const_reference ref, ctx.objMap1) {
        const TObjId objDst = ref.second;
        SE_BREAK_IF(objDst < 0);

        const TObjId o1     = ref.first;
        const TValueId v1   = ctx.sh1.valueOf(o1);

        TValueId vDst = v1;
        if (0 < vDst) {
            TValMap &vMap = ctx.valMap1[/* ltr */ 0];
            TValMap::iterator it1 = vMap.find(v1);
            SE_BREAK_IF(vMap.end() == it1);
            vDst = it1->second;
        }

        SymHeap &dst = ctx.dst;
        const TObjId compObj = dst.valGetCompositeObj(vDst);
        if (OBJ_INVALID != compObj) {
            // composite values already match
            SE_BREAK_IF(compObj != objDst);
            continue;
        }

        // set the value
        dst.objSetValue(objDst, vDst);
    }
}

bool matchPreds(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValMapBidir      &vMap)
{
    // FIXME: too strict for us?
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

    // finally check the predicates
    if (!matchPreds(ctx.sh1, ctx.dst, ctx.valMap1))
        updateJoinStatus(ctx, JS_USE_SH2);
    if (!matchPreds(ctx.sh2, ctx.dst, ctx.valMap2))
        updateJoinStatus(ctx, JS_USE_SH1);
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
        SE_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // go through all values in them
    if (!joinPendingValues(ctx)) {
        SE_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // time to preserve all 'hasValue' edges and *some* Neq predicates
    setDstValues(ctx);
    handleDstPreds(ctx);

    if (debugSymJoin) {
        // catch possible regression at this point
        SE_BREAK_IF((JS_USE_ANY == ctx.status) != areEqual(sh1, sh2));
        SE_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh1, ctx.dst));
        SE_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh2, ctx.dst));
    }

    // all OK
    *pStatus = ctx.status;
    return true;
}
