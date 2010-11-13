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

#include "symcmp.hh"
#include "symstate.hh"

#include <boost/foreach.hpp>

/* static */ bool debugSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debugSymJoin)                                                     \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *pDst,
        const SymHeap           &sh1,
        const SymHeap           &sh2)
{
    // FIXME: provide SymHeap::clear() to achieve this?
    *pDst = SymHeap();
    SymHeap &dst = *pDst;

    // gather program variables
    SymHeap::TContCVar cVars1, cVars2;
    sh1.gatherCVars(cVars1);
    sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2) {
        SJ_DEBUG("<-- different program variables");
        return false;
    }

    // object IDs mapping
    std::map<TObjId, TObjId>    objMap1;
    std::map<TObjId, TObjId>    objMap2;

    // go through all program variables
    BOOST_FOREACH(const CVar &cv, cVars1) {
        const TObjId obj1 = sh1.objByCVar(cv);
        const TObjId obj2 = sh2.objByCVar(cv);

        // create a corresponding program variable in the resulting heap
        const struct cl_type *clt = sh1.objType(obj1);
        const TObjId obj = dst.objCreate(clt, cv);

        // store object IDs mapping
        objMap1[obj1] = obj;
        objMap2[obj2] = obj;
    }

    // TODO
    //SE_BREAK_IF(debugSymJoin);
    if (!areEqual(sh1, sh2))
        return false;

    // TODO
    *pStatus = JS_USE_ANY;
    return true;
}
