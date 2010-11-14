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

    TValMap                     valMap1;
    TValMap                     valMap2;

    WorkList<TValPair>          wl;
    EJoinStatus                 status;

    SymJoinCtx(SymHeap &dst_, const SymHeap &sh1_, const SymHeap &sh2_):
        dst(dst_),
        sh1(sh1_),
        sh2(sh2_),
        status(JS_USE_ANY)
    {
    }
};

class ObjJoinVisitor {
    private:
        SymJoinCtx &ctx_;

    public:
        ObjJoinVisitor(SymJoinCtx &ctx): ctx_(ctx) { }

        bool operator()(const TObjTriple &item) {
            const SymHeap &sh1 = ctx_.sh1;
            const SymHeap &sh2 = ctx_.sh2;
            SymHeap       &dst = ctx_.dst;

            const TObjId obj1   = item[0];
            const TObjId obj2   = item[1];
            const TObjId objDst = item[2];

            // store object IDs mapping
            ctx_.objMap1[obj1]  = objDst;
            ctx_.objMap2[obj2]  = objDst;

            // store object's address
            const TValueId addr = dst.placedAt(objDst);
            ctx_.valMap1[sh1.placedAt(obj1)] = addr;
            ctx_.valMap2[sh2.placedAt(obj2)] = addr;

            const TValueId v1 = sh1.valueOf(obj1);
            const TValueId v2 = sh2.valueOf(obj2);
            if (VAL_NULL == v1 && VAL_NULL == v2) {
                ctx_.dst.objSetValue(objDst, VAL_NULL);
                return /* continue */ true;
            }

            // store mapping of composite object's values
            const TValueId vDst = dst.valueOf(objDst);
            if (OBJ_INVALID != sh1.valGetCompositeObj(v1))
                ctx_.valMap1[v1] = vDst;
            if (OBJ_INVALID != sh2.valGetCompositeObj(v2))
                ctx_.valMap2[v2] = vDst;

            // special values have to match (NULL not treated as special here)
            // TODO: should we consider also join of VAL_TRUE/VAL_FALSE?
            if (v1 < 0 || v2 < 0) {
                if (v1 == v2)
                    return /* continue */ true;

                SJ_DEBUG("<-- special value mismatch: " << SJ_VALP(v1, v2));
                return false;
            }

            if (ctx_.wl.schedule(TValPair(v1, v2)))
                SJ_DEBUG("+++ " << SJ_VALP(v1, v2));

            return /* continue */ true;
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

    SJ_DEBUG("<-- value clt mismatch: " << SJ_VALP(v1, v2));
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

    SJ_DEBUG("<-- object clt mismatch: " << SJ_OBJP(o1, o2));
    return false;
}

void defineValueMapping(
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const TValueId          vDst)
{
    TValMap &vMap1 = ctx.valMap1;
    TValMap &vMap2 = ctx.valMap2;

    // check for redefinition
    SE_BREAK_IF(hasKey(vMap1, v1));
    SE_BREAK_IF(hasKey(vMap2, v2));

    vMap1[v1] = vDst;
    vMap2[v2] = vDst;
}

bool followValuePair(
        SymJoinCtx              &ctx,
        const EUnknownValue     code,
        const TValueId          v1,
        const TValueId          v2)
{
    if (UV_KNOWN != code) {
        SE_BREAK_IF(debugSymJoin);
        return true;
    }

    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    // TODO: compositle values
    // TODO: custom values

    const TObjId o1 = sh1.pointsTo(v1);
    const TObjId o2 = sh2.pointsTo(v2);
    if (!checkNonPosValues(o1, o2)) {
        SJ_DEBUG("<-- target validity mismatch: " << SJ_VALP(v1, v2)
                 << " -> " << SJ_OBJP(o1, o2));
        return false;
    }

    if (o1 < 0 && o2 < 0)
        // already checked
        return true;

    const struct cl_type *clt;
    if (!joinObjClt(&clt, ctx, o1, o2))
        return false;

    const TObjId root1 = objRoot(sh1, o1);
    const TObjId root2 = objRoot(sh2, o2);
    if (hasKey(ctx.objMap1, root1)) {
        SE_BREAK_IF(!hasKey(ctx.objMap2, root2));
        return true;
    }

    SE_BREAK_IF(root1 <= 0 || root2 <= 0);
    if (!joinObjClt(&clt, ctx, root1, root2))
        return false;

    if (!clt) {
        // TODO: anonymous object of known size
        SE_BREAK_IF(debugSymJoin);
        return true;
    }

    const TObjId rootDst = ctx.dst.objCreate(clt);
    return traverseSubObjs(ctx, root1, root2, rootDst);
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
        defineValueMapping(ctx, v1, v2, vDst);
        *pResult = true;
        return true;
    }

    // TODO
    SE_BREAK_IF(debugSymJoin);
    return false;
}

bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValueId          v1,
        const TValueId          v2,
        const EUnknownValue     code1,
        const EUnknownValue     code2)
{
    // TODO
    (void) pResult;
    (void) ctx;
    (void) v1;
    (void) v2;
    (void) code1;
    (void) code2;
    SE_BREAK_IF(debugSymJoin);
    return false;
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
            return followValuePair(ctx, code, v1, v2);

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
    defineValueMapping(ctx, v1, v2, vDst);
    return true;
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
        SJ_DEBUG("<-- unknown value code mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    return joinValueCore(ctx, code1, v1, v2);
}

bool joinPendingValues(SymJoinCtx &ctx) {
    TValPair vp;
    while (ctx.wl.next(vp)) {
        const TValueId v1 = vp.first;
        const TValueId v2 = vp.second;

        TValMap::const_iterator i1 = ctx.valMap1.find(v1);
        TValMap::const_iterator i2 = ctx.valMap2.find(v2);

        const bool hasTarget1 = (ctx.valMap1.end() != i1);
        const bool hasTarget2 = (ctx.valMap2.end() != i2);
        if (hasTarget1 || hasTarget2) {
            const TValueId vDst1 = (hasTarget1)
                ? i1->second
                : static_cast<TValueId>(VAL_INVALID);

            const TValueId vDst2 = (hasTarget2)
                ? i2->second
                : static_cast<TValueId>(VAL_INVALID);

            if ((hasTarget1 != hasTarget2) || (vDst1 != vDst2)) {
                SJ_DEBUG("<-- value mapping mismatch: " << SJ_VALP(v1, v2)
                         "-> " << SJ_VALP(vDst1, vDst2));
                return false;
            }
        }
        else if (!joinValuePair(ctx, v1, v2))
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

    // TODO
    SE_BREAK_IF(debugSymJoin);
    if (!areEqual(sh1, sh2))
        return false;

    *pStatus = ctx.status;
    return true;
}
