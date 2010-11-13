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

typedef boost::array<const SymHeap *, 3>        TSymHeapTriple;
typedef boost::array<TValueId       , 3>        TValTriple;
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

    WorkList<TValTriple>        wl;

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

            return /* continue */ true;
        }
};

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
        objVisitor(root);
        if (!traverseSubObjs<3>(sht, root, objVisitor))
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

    // TODO
    SE_BREAK_IF(debugSymJoin);
    if (!areEqual(sh1, sh2))
        return false;

    // TODO
    *pStatus = JS_USE_ANY;
    return true;
}
