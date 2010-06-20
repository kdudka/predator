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

#include "symbt.hh"
#include "symplot.hh"

#include <cl/cl_msg.hh>
#include <cl/code_listener.h>
#include <cl/storage.hh>

#include "worklist.hh"

#include <map>
#include <set>
#include <stack>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef SE_DISABLE_SYMCUT
#   define SE_DISABLE_SYMCUT 0
#endif

template <class TSet, class TList>
void fillSet(TSet &dst, const TList &src)
{
    // FIXME: use an algorithm?
    typedef typename TList::const_reference TRef;
    BOOST_FOREACH(TRef ref, src) {
        dst.insert(ref);
    }
}

template <class TObjMap, class TValMap, class TWL>
void digSubObjs(const SymHeap &src, SymHeap &dst,
                TObjMap &objMap, TValMap &valMap, TWL &wl,
                TObjId objSrc, TObjId objDst)
{
    typedef typename TWL::value_type TItem;
    std::stack<TItem> todo;
    push(todo, objSrc, objDst);
    while (!todo.empty()) {
        TObjId objSrc, objDst;
        boost::tie(objSrc, objDst) = todo.top();
        todo.pop();

        const struct cl_type *clt = src.objType(objSrc);
        if (clt != dst.objType(objDst))
            // type mismatch
            TRAP;

        if (!clt)
            // anonymous object of known size
            continue;

        if (CL_TYPE_STRUCT != clt->code)
            // we should be set up
            continue;

        if (CL_TYPE_STRUCT != clt->code) {
            for (int i = 0; i < clt->item_cnt; ++i) {
                const TObjId subSrc = src.subObj(objSrc, i);
                const TObjId subDst = dst.subObj(objDst, i);
                if (subSrc < 0 || subDst < 0)
                    TRAP;

                push(todo, subSrc, subDst);
                if (!wl.schedule(subSrc, subDst))
                    TRAP;

                objMap[subSrc] = subDst;
                valMap[src.placedAt(subSrc)] = dst.placedAt(subDst);
            }
        }
    }
}

template <class TCut, class TObjMap, class TValMap, class TWL>
TObjId addObjectIfNeeded(const SymHeap &src, SymHeap &dst, TCut &cut,
                         TObjMap &objMap, TValMap &valMap, TWL &wl,
                         TObjId objSrc)
{
    if (OBJ_RETURN == objSrc)
        // FIXME: safe to ignore??
        return OBJ_RETURN;

    typename TObjMap::iterator iter = objMap.find(objSrc);
    if (objMap.end() != iter)
        // mapping already known
        return iter->second;

    // got to root
    TObjId rootSrc = objSrc, tmp;
    while (OBJ_INVALID != (tmp = src.objParent(rootSrc)))
        rootSrc = tmp;

    CVar cv;
    if (src.cVar(&cv, rootSrc))
        // enlarge the cut if needed
        cut.insert(cv);

    const struct cl_type *clt = src.objType(rootSrc);
    if (clt) {
        const TObjId rootDst = dst.objCreate(clt, cv);
        objMap[rootSrc] = rootDst;
        valMap[src.placedAt(rootSrc)] = dst.placedAt(rootDst);
        wl.schedule(rootSrc, rootDst);
        digSubObjs(src, dst, objMap, valMap, wl, rootSrc, rootDst);
        return objMap[objSrc];
    }

    // assume anonymous object of known size
    const TValueId at = src.placedAt(objSrc);
    if (!src.valPointsToAnon(at))
        TRAP;

    const int cbSize = src.objSizeOfAnon(objSrc);
    const TObjId objDst = dst.objCreateAnon(cbSize);
    objMap[objSrc] = objDst;
    valMap[src.placedAt(objSrc)] = dst.placedAt(objDst);
    wl.schedule(objSrc, objDst);
    return objDst;
}

template <class TWL, class TCut, class TObjMap, class TValMap>
void deepCopy(const SymHeap &src, SymHeap &dst, TCut &cut,
              TObjMap &objMap, TValMap &valMap, TWL &wl)
{
    typename TWL::value_type item;
    while (wl.next(item)) {
        const TObjId objSrc = item.first;
        const TObjId objDst = item.second;

        if (objSrc < 0 || objDst < 0)
            // this should have been handled elsewhere
            TRAP;

        if (objSrc != OBJ_RETURN) {
            // store value IDs mapping
            const TValueId at = src.placedAt(objSrc);
            const TValueId atDst = dst.placedAt(objDst);
            valMap[at] = atDst;

            // go from the value backward
            // FIXME: overkill in case of joinHeapsByCVars (performance impact)
            SymHeap::TContObj uses;
            src.usedBy(uses, at);
            BOOST_FOREACH(TObjId objSrc, uses) {
                addObjectIfNeeded(src, dst, cut, objMap, valMap, wl, objSrc);
            }
        }

        // read the original value
        const TValueId valSrc = src.valueOf(objSrc);
        if (VAL_INVALID == valSrc)
            TRAP;

        // go from the value backward
        // FIXME: overkill in case of joinHeapsByCVars (performance impact)
        SymHeap::TContObj uses;
        src.usedBy(uses, valSrc);
        BOOST_FOREACH(TObjId objSrc, uses) {
            addObjectIfNeeded(src, dst, cut, objMap, valMap, wl, objSrc);
        }

        const TObjId compSrc = src.valGetCompositeObj(valSrc);
        if (OBJ_INVALID != compSrc) {
            const TObjId compDst =
                addObjectIfNeeded(src, dst, cut, objMap, valMap, wl, compSrc);

            const TValueId valDst = dst.valueOf(compDst);
            valMap[valSrc] = valDst;
            dst.objSetValue(objDst, valDst);
            continue;
        }

        if (valSrc <= 0) {
            // special value IDs always match
            dst.objSetValue(objDst, valSrc);
            continue;
        }

        typename TValMap::iterator iter = valMap.find(valSrc);
        if (valMap.end() != iter) {
            // good luck, we have already handled the target value
            dst.objSetValue(objDst, iter->second);
            continue;
        }

        const struct cl_type *cltCustom = 0;
        const int custom = src.valGetCustom(&cltCustom, valSrc);
        if (-1 != custom) {
            // custom value, e.g. fnc pointer
            const TValueId valDst = dst.valCreateCustom(cltCustom, custom);
            valMap[valSrc] = valDst;
            dst.objSetValue(objDst, valDst);
            continue;
        }

        const EUnknownValue code = src.valGetUnknown(valSrc);
        if (UV_KNOWN != code) {
            // custom value, e.g. fnc pointer
            const struct cl_type *cltUnkown = src.valType(valSrc);
            const TValueId valDst = dst.valCreateUnknown(code, cltUnkown);
            valMap[valSrc] = valDst;
            dst.objSetValue(objDst, valDst);
            continue;
        }

        if (src.valPointsToAnon(valSrc)) {
            // we know size of the target, but not the type etc.
            const TObjId anon = src.pointsTo(valSrc);
            const int cbSize = src.objSizeOfAnon(anon);
            const TObjId anonDst = dst.objCreateAnon(cbSize);
            objMap[anon] = anonDst;
            const TValueId valDst = dst.placedAt(anonDst);
            valMap[src.placedAt(anon)] = valDst;
            dst.objSetValue(objDst, valDst);
            continue;
        }

        const TObjId target = src.pointsTo(valSrc);
        if (OBJ_INVALID == target)
            TRAP;

        if (target < 0) {
            switch (target) {
                case OBJ_DELETED:
                case OBJ_LOST:
                    break;
                default:
                    TRAP;
            }

            // FIXME: really safe to ignore (valClt == 0) ??
            const struct cl_type *valClt = src.valType(valSrc);
            const TObjId objTmp = dst.objCreate(valClt);
            const TValueId valDst = dst.placedAt(objTmp);

            // FIXME: avoid using of friend?
            dynamic_cast<SymHeapCore &>(dst).objDestroy(objTmp, target);

            valMap[valSrc] = valDst;
            dst.objSetValue(objDst, valDst);
            continue;
        }

        // traverse recursively
        const TObjId targetDst =
            addObjectIfNeeded(src, dst, cut, objMap, valMap, wl, target);
        const TValueId at = dst.placedAt(targetDst);
        dst.objSetValue(objDst, at);
    }
}

template <class TCut>
void prune(const SymHeap &src, SymHeap &dst, /* NON-const */ TCut &cut) {
    // injective mapping of object IDs and value IDs
    std::map<TObjId /* src */, TObjId /* dst */> objMap;
    std::map<TValueId /* src */, TValueId /* dst */>  valMap;

    // worklist running through SymHeap objects
    typedef std::pair<TObjId /* src */, TObjId /* dst */> TItem;
    WorkList<TItem> wl;

    // go through all program variables
    BOOST_FOREACH(CVar cv, cut) {
        const TObjId obj = src.objByCVar(cv);
        if (OBJ_INVALID == obj)
            // failed to resolve program variable
            TRAP;

        const struct cl_type *clt = src.objType(obj);
        if (!clt)
            // we should always know type of program variables
            TRAP;

        addObjectIfNeeded(src, dst, cut, objMap, valMap, wl, obj);
    }

    // go through the worklist
    deepCopy(src, dst, cut, objMap, valMap, wl);
}

namespace {
    void plotHeap(const SymBackTrace *bt, const SymHeap &heap, const char *name)
    {
        const CodeStorage::Storage &stor = bt->stor();
        SymHeapPlotter plotter(stor, heap);
        plotter.plot(name);
    }
}

void splitHeapByCVars(const SymBackTrace *bt, SymHeap *srcDst,
                      const SymHeap::TContCVar &cut, SymHeap *saveSurroundTo)
{
#if SE_DISABLE_SYMCUT
    return;
#endif
    // std::vector -> std::set
    std::set<CVar> cset;
    fillSet(cset, cut);

    // cut the first part
    SymHeap dst;
    prune(*srcDst, dst, cset);

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
    prune(*srcDst, *saveSurroundTo, complement);

    // print some statistics
    const unsigned cntA = cset.size();
    const unsigned cntB = complement.size();
    const unsigned cntOrig = cut.size();
    const unsigned cntTotal = all.size();
    CL_DEBUG("splitHeapByCVars() finished: "
            << cntOrig << " -> " << cntA << " |" << cntB
            << " (" << cntTotal << " program variables in total)");

    // basic sanity check
    if (cntA < cntOrig || cntA + cntB != cntTotal) {
        CL_ERROR("symcut: prune() failed, attempt to plot heaps...");
        plotHeap(bt, *srcDst, "prune-input");
        plotHeap(bt, dst, "prune-output");
        plotHeap(bt, *saveSurroundTo, "prune-surround");
        CL_NOTE("symcut: plot done, please consider analyzing the results");
        TRAP;
    }
    *srcDst = dst;
}

void joinHeapsByCVars(const SymBackTrace *bt, SymHeap *srcDst,
                      const SymHeap *src2)
{
    (void) bt;
#if SE_DISABLE_SYMCUT
    return;
#endif
    // gather program variables
    SymHeap::TContCVar all;
    src2->gatherCVars(all);

    // std::vector -> std::set
    std::set<CVar> cset;
    fillSet(cset, all);
    
    // FIXME: performance impact
    prune(*src2, *srcDst, cset);
}
