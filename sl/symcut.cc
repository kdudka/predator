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
#include "symcut.hh"

#include <cl/cl_msg.hh>
#include <cl/code_listener.h>

#include "worklist.hh"

#include <map>
#include <set>
#include <vector>

#include <boost/foreach.hpp>

#ifndef SE_DISABLE_SYMCUT
#   define SE_DISABLE_SYMCUT 0
#endif

template <class TCut>
void prune(SymHeap &dst, const SymHeap &src, /* NON-const */ TCut &cut) {
    WorkList<TObjId> todo;
    BOOST_FOREACH(CVar cv, cut) {
        const TObjId obj = src.objByCVar(cv);
        if (OBJ_INVALID == obj)
            // invalid cut
            TRAP;

        todo.schedule(obj);
    }

    TObjId obj;
    while (todo.next(obj)) {
        const struct cl_type *clt = src.objType(obj);
        if (!clt)
            TRAP;

        CVar cv;
        if (src.cVar(&cv, obj))
            // the cut may grow eventually
            cut.insert(cv);

        // create image of the object in the target heap
        dst.objCreate(clt, cv);
    }
}

void splitHeapByCVars(SymHeap *srcDst, SymHeap::TContCVar cut,
                      SymHeap *saveSurroundTo)
{
#if SE_DISABLE_SYMCUT
    return;
#endif
    // std::vector -> std::set
    // FIXME: use an algorithm?
    std::set<CVar> cset;
    BOOST_FOREACH(const CVar &cv, cut) {
        cset.insert(cv);
    }

    // cut the first part
    SymHeap dst;
    prune(dst, *srcDst, cset);

    if (!saveSurroundTo) {
        // we're done
        *srcDst = dst;
        return;
    }

    // get the complete list of program variables
    SymHeap::TContCVar all;
    srcDst->gatherCVars(all);

    // FIXME: use an algorithm?
    std::set<CVar> complement;
    BOOST_FOREACH(const CVar &cv, all) {
        if (!hasKey(cset, cv))
            complement.insert(cv);
    }

    // compute the surrounding part of heap
    prune(*saveSurroundTo, *srcDst, complement);
    *srcDst = dst;

    // print some statistics
    const unsigned cntA = cset.size();
    const unsigned cntB = complement.size();
    const unsigned cntOrig = cut.size();
    const unsigned cntTotal = all.size();
    CL_DEBUG("splitHeapByCVars() finished: "
            << cntOrig << " -> " << cntA << " |" << cntB
            << " (" << cntTotal << " program variables in total)");

    // basic sanity check
    if (cntA < cntOrig || cntA + cntB != cntTotal)
        TRAP;
}

template <class TWL>
void digSubObjs(SymHeap &dst, const SymHeap &src, TWL &wl,
                const typename TWL::value_type &item)
{
    const TObjId obj = item.first;
    const TObjId objDst = item.second;

    const struct cl_type *clt = src.objType(obj);
    if (clt != dst.objType(objDst))
        // type mismatch
        TRAP;

    if (!clt)
        // anonymous object of known size
        return;

    if (CL_TYPE_STRUCT != clt->code)
        // we should be set up
        return;

    for (int i = 0; i < clt->item_cnt; ++i) {
        const TObjId sub = src.subObj(obj, i);
        const TObjId subDst = dst.subObj(objDst, i);
        if (sub < 0 || subDst < 0)
            TRAP;

        wl.schedule(sub, subDst);
    }
}

template <class TWL>
void deepCopy(SymHeap &dst, const SymHeap &src, TWL &wl) {
    // injective mapping of object IDs and value IDs
    std::map<TObjId /* src */, TObjId /* dst */> objMap;
    std::map<TValueId /* src */, TValueId /* dst */> valMap;

    // XXX
    typedef std::pair<TObjId /* setVal */, TObjId /* toAddrOf */> TPointToSub;
    std::vector<TPointToSub> delayedPointToSub;

    typename TWL::value_type item;
    while (wl.next(item)) {
        const TObjId obj = item.first;
        const TObjId objDst = item.second;

        if (obj < 0 || objDst < 0)
            // this should have been handled elsewhere
            TRAP;

        // store IDs mapping
        objMap[obj] = objDst;
        valMap[src.placedAt(obj)] = dst.placedAt(objDst);

        // dig sub-objects in case of composite type
        digSubObjs(dst, src, wl, item);

        // read original value
        const TValueId value = src.valueOf(obj);
        if (VAL_INVALID == value)
            TRAP;

        const TObjId compObj = src.valGetCompositeObj(value);
        if (OBJ_INVALID != compObj) {
            // XXX
            delayedPointToSub.push_back(TPointToSub(objDst, compObj));
            continue;
        }

        if (value <= 0) {
            // special value IDs always match
            dst.objSetValue(objDst, value);
            continue;
        }

        const struct cl_type *cltCustom = 0;
        const int custom = src.valGetCustom(&cltCustom, value);
        if (-1 != custom) {
            // custom value, e.g. fnc pointer
            if (!hasKey(valMap, value))
                valMap[value] = dst.valCreateCustom(cltCustom, custom);
            dst.objSetValue(objDst, valMap[value]);
            continue;
        }

        const EUnknownValue code = src.valGetUnknown(value);
        if (UV_KNOWN != code) {
            // custom value, e.g. fnc pointer
            if (!hasKey(valMap, value)) {
                const struct cl_type *cltUnkown = src.valType(value);
                valMap[value] = dst.valCreateUnknown(code, cltUnkown);
            }
            dst.objSetValue(objDst, valMap[value]);
            continue;
        }

        if (src.valPointsToAnon(value))
            // we know size of the target, but not the type etc.
            // TODO
            TRAP;

        if (!hasKey(valMap, value))
            // TODO: schedule for processing
            TRAP;

        // good luck, we already know the mapping
        dst.objSetValue(objDst, valMap[value]);
    }

    // delayed assignment of composite values
    BOOST_FOREACH(const TPointToSub &item, delayedPointToSub) {
        const TObjId obj = item.first;
        const TObjId target = item.second;

        // read the value
        const TValueId value = dst.valueOf(objMap[target]);
        if (VAL_INVALID == value)
            TRAP;

        // assign it to the object
        dst.objSetValue(obj, value);
    }
}

void joinHeapsByCVars(SymHeap *srcDst, const SymHeap *src2)
{
#if SE_DISABLE_SYMCUT
    return;
#endif
    // worklist running through SymHeap objects
    typedef std::pair<TObjId /* src */, TObjId /* dst */> TItem;
    WorkList<TItem> wl;

    // go through all program variables
    SymHeap::TContCVar cVars;
    src2->gatherCVars(cVars);
    BOOST_FOREACH(CVar cv, cVars) {
        const TObjId obj = src2->objByCVar(cv);
        if (OBJ_INVALID == obj)
            // failed to resolve program variable
            TRAP;

        const struct cl_type *clt = src2->objType(obj);
        if (!clt)
            // we should always know type of program variables
            TRAP;

        // copy object itself
        const TObjId objDst = srcDst->objCreate(clt, cv);
        wl.schedule(obj, objDst);
    }

    // go through the worklist
    deepCopy(*srcDst, *src2, wl);
}
