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
#include "worklist.hh"

#include <map>

#include <boost/foreach.hpp>

/* static */ bool debugSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debugSymJoin)                                                     \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

#define SJ_VALP(v1, v2) "(v1 = #" << v1 << ", v2 = #" << v2 << ")"

typedef boost::array<const SymHeap *, 3>        TSymHeapTriple;
//typedef boost::array<TValueId       , 3>        TValTriple;
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

    SymJoinCtx(SymHeap &dst_, const SymHeap &sh1_, const SymHeap &sh2_):
        dst(dst_),
        sh1(sh1_),
        sh2(sh2_)
    {
    }
};

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
            const TValueId addr = ctx_.dst.placedAt(objDst);
            ctx_.valMap1[ctx_.sh1.placedAt(obj1)] = addr;
            ctx_.valMap2[ctx_.sh2.placedAt(obj2)] = addr;

            const TValueId v1 = ctx_.sh1.valueOf(obj1);
            const TValueId v2 = ctx_.sh2.valueOf(obj2);
            if (VAL_NULL == v1 && VAL_NULL == v2) {
                ctx_.dst.objSetValue(objDst, VAL_NULL);
                return /* continue */ true;
            }

            // special values have to match (NULL not treated as special here)
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

bool joinUnkownValues(SymJoinCtx &ctx, const TValueId v1, const TValueId v2) {
    // TODO
    (void) ctx;
    (void) v1;
    (void) v2;
    SE_BREAK_IF(debugSymJoin);
    return true;
}

bool joinAbstractValues(SymJoinCtx &ctx, const TValueId v1, const TValueId v2) {
    // TODO
    (void) ctx;
    (void) v1;
    (void) v2;
    SE_BREAK_IF(debugSymJoin);
    return true;
}

bool joinValueCore(
        SymJoinCtx              &ctx,
        const EUnknownValue     code,
        const TValueId          v1,
        const TValueId          v2)
{
    TValMap::const_iterator i1 = ctx.valMap1.find(v1);
    TValMap::const_iterator i2 = ctx.valMap2.find(v2);

    const bool hasTarget1 = (ctx.valMap1.end() != i1);
    const bool hasTarget2 = (ctx.valMap2.end() != i2);
    SE_BREAK_IF(hasTarget1 && hasTarget2);

    if (hasTarget1) {
        // sync mapping of v2
        ctx.valMap2[v2] = i1->second;
        return true;
    }

    if (hasTarget2) {
        // sync mapping of v1
        ctx.valMap1[v1] = i2->second;
        return true;
    }

    SE_BREAK_IF(hasTarget1 || hasTarget2);
    switch (code) {
        case UV_KNOWN:
            SE_BREAK_IF(debugSymJoin);
            return true;

        case UV_UNINITIALIZED:
            break;

        default:
            SE_TRAP;
            return true;
    }

    SymHeap         &dst = ctx.dst;
    const SymHeap   &sh1 = ctx.sh1;
    const SymHeap   &sh2 = ctx.sh2;

    const struct cl_type *clt1 = sh1.valType(v1);
    const struct cl_type *clt2 = sh2.valType(v2);
    if (clt1 && clt2 && (*clt1 != *clt2)) {
        SJ_DEBUG("<-- unknown value clt mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    // create a new unknown value
    const TValueId valDst = dst.valCreateUnknown(code, clt1);
    ctx.valMap1[v1] = valDst;
    ctx.valMap2[v2] = valDst;
    return true;
}

bool joinValuePair(SymJoinCtx &ctx, const TValueId v1, const TValueId v2) {
    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    const EUnknownValue code1 = sh1.valGetUnknown(v1);
    const EUnknownValue code2 = sh2.valGetUnknown(v2);
    if (UV_UNKNOWN == code1 || UV_UNKNOWN == code2)
        return joinUnkownValues(ctx, v1, v2);

    if (UV_ABSTRACT == code1 || UV_ABSTRACT == code2)
        return joinAbstractValues(ctx, v1, v2);

    if (code1 != code2) {
        SJ_DEBUG("<-- unknown value code mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    switch (code1) {
        case UV_KNOWN:
        case UV_UNINITIALIZED:
            return joinValueCore(ctx, code1, v1, v2);

        default:
            SE_TRAP;
            return false;
    }
}

bool joinPendingValues(SymJoinCtx &ctx) {
    TValPair vp;
    while (ctx.wl.next(vp)) {
        const TValueId v1 = vp.first;
        const TValueId v2 = vp.second;

        TValMap::const_iterator i1 = ctx.valMap1.find(v1);
        TValMap::const_iterator i2 = ctx.valMap2.find(v2);

        if ((ctx.valMap1.end() != i1) && (ctx.valMap2.end() != i2)) {
            const TValueId vDst1 = i1->second;
            const TValueId vDst2 = i2->second;
            if (vDst1 != vDst2) {
                SJ_DEBUG("<-- value mapping mismatch: " << SJ_VALP(v1, v2)
                         "-> " << SJ_VALP(vDst1, vDst2));
                return false;
            }

            continue;
        }

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

    ObjJoinVisitor objVisitor(ctx);

    // go through all program variables
    BOOST_FOREACH(const CVar &cv, cVars1) {
        TObjTriple root;
        root[/* sh1 */ 0] = sh1.objByCVar(cv);
        root[/* sh2 */ 1] = sh2.objByCVar(cv);

        // create a corresponding program variable in the resulting heap
        const struct cl_type *clt = sh1.objType(root[0]);
        SE_BREAK_IF(!clt);
        root[/* dst */ 2] = dst.objCreate(clt, cv);

        TSymHeapTriple sht;
        sht[0] = &ctx.sh1;
        sht[1] = &ctx.sh2;
        sht[2] = &ctx.dst;

        // guide the visitors
        if (!objVisitor(root) || !traverseSubObjs<3>(sht, root, objVisitor))
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
    SymHeap &dst = *pDst;
    SymJoinCtx ctx(dst, sh1, sh2);

    // start with program variables
    if (!joinCVars(ctx)) {
        SE_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    if (!joinPendingValues(ctx)) {
        SE_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // TODO
    SE_BREAK_IF(debugSymJoin);
    if (!areEqual(sh1, sh2))
        return false;

    // TODO
    *pStatus = JS_USE_ANY;
    return true;
}
