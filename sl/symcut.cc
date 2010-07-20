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

#include <iomanip>
#include <map>
#include <set>
#include <stack>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef SE_DISABLE_SYMCUT
#   define SE_DISABLE_SYMCUT 0
#endif

#ifndef DEBUG_SYMCUT
#   define DEBUG_SYMCUT 0
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

struct DeepCopyData {
    typedef std::map<TObjId   /* src */, TObjId   /* dst */>    TObjMap;
    typedef std::map<TValueId /* src */, TValueId /* dst */>    TValMap;
    typedef std::pair<TObjId  /* src */, TObjId   /* dst */>    TItem;
    typedef std::set<CVar>                                      TCut;

    const SymHeap       &src;
    SymHeap             &dst;
    TCut                &cut;

    TObjMap             objMap;
    TValMap             valMap;

    WorkList<TItem>     wl;

    DeepCopyData(const SymHeap &src_, SymHeap &dst_, TCut &cut_):
        src(src_),
        dst(dst_),
        cut(cut_)
    {
    }
};

void add(DeepCopyData &dc, TObjId objSrc, TObjId objDst) {
    dc.objMap[objSrc] = objDst;
    dc.valMap[dc.src.placedAt(objSrc)] = dc.dst.placedAt(objDst);
    dc.wl.schedule(objSrc, objDst);
}

void digSubObjs(DeepCopyData &dc, TObjId objSrc, TObjId objDst)
{
    const SymHeap   &src = dc.src;
    SymHeap         &dst = dc.dst;

    typedef DeepCopyData::TItem TItem;
    std::stack<TItem> todo;
    push(todo, objSrc, objDst);
    while (!todo.empty()) {
        TObjId objSrc, objDst;
        boost::tie(objSrc, objDst) = todo.top();
        todo.pop();

        const struct cl_type *cltSrc = src.objType(objSrc);
        if (dst.objType(objDst) != cltSrc)
            // type mismatch
            TRAP;

        if (!cltSrc)
            // anonymous object of known size
            continue;

        if (CL_TYPE_STRUCT != cltSrc->code)
            // we should be set up
            continue;

        // store mapping of composite value
        dc.valMap[src.valueOf(objSrc)] = dst.valueOf(objDst);

        // go through fields
        for (int i = 0; i < cltSrc->item_cnt; ++i) {
            const TObjId subSrc = src.subObj(objSrc, i);
            const TObjId subDst = dst.subObj(objDst, i);
            if (subSrc < 0 || subDst < 0)
                TRAP;

            add(dc, subSrc, subDst);
            push(todo, subSrc, subDst);
        }
    }
}

TObjId addObjectIfNeeded(DeepCopyData &dc, TObjId objSrc) {
    if (OBJ_RETURN == objSrc)
        // FIXME: safe to ignore??
        return OBJ_RETURN;

    DeepCopyData::TObjMap::iterator iterObjSrc = dc.objMap.find(objSrc);
    if (dc.objMap.end() != iterObjSrc)
        // mapping already known
        return iterObjSrc->second;

    // go to root
    const SymHeap &src = dc.src;
    TObjId rootSrc = objSrc, tmp;
    while (OBJ_INVALID != (tmp = src.objParent(rootSrc)))
        rootSrc = tmp;

    if (!src.objExists(rootSrc))
        // TODO: implement cloning of virtual objects
        TRAP;

    CVar cv;
    if (src.cVar(&cv, rootSrc)) {
        // enlarge the cut if needed
#if DEBUG_SYMCUT
        const size_t orig = dc.cut.size();
#endif
        dc.cut.insert(cv);
#if DEBUG_SYMCUT
        if (dc.cut.size() != orig)
            CL_DEBUG("addObjectIfNeeded() is enlarging the cut by cVar #"
                    << cv.uid << ", nestlevel = " << cv.inst);
#endif
    }

    SymHeap &dst = dc.dst;
    const struct cl_type *clt = src.objType(rootSrc);
    if (!clt) {
        // assume anonymous object of known size
        if (src.objType(objSrc))
            TRAP;

        const int cbSize = src.objSizeOfAnon(objSrc);
        const TObjId objDst = dst.objCreateAnon(cbSize);
        add(dc, objSrc, objDst);
        return objDst;
    }

    const TObjId rootDst = dst.objCreate(clt, cv);
    add(dc, rootSrc, rootDst);
    digSubObjs(dc, rootSrc, rootDst);

    const EObjKind kind = src.objKind(rootSrc);
    switch (kind) {
        case OK_DLS:
        case OK_SLS: {
            const TFieldIdxChain icNext = src.objNextField(rootSrc);
            const TFieldIdxChain icPeer = src.objPeerField(rootSrc);
            dst.objSetAbstract(rootDst, kind, icNext, icPeer);

            const bool shared = src.objShared(rootSrc);
            dst.objSetShared(rootDst, shared);
        }
        // fall through!

        case OK_CONCRETE:
            return dc.objMap[objSrc];
    }

    TRAP;
    return OBJ_INVALID;
}

TValueId handleValue(DeepCopyData &dc, TValueId valSrc, bool digBackward) {
    const SymHeap   &src = dc.src;
    SymHeap         &dst = dc.dst;

    if (/* optimization */ digBackward) {
        // go from the value backward
        SymHeap::TContObj uses;
        src.usedBy(uses, valSrc);
        BOOST_FOREACH(TObjId objSrc, uses) {
            addObjectIfNeeded(dc, objSrc);
        }
    }

    const TObjId compSrc = src.valGetCompositeObj(valSrc);
    if (OBJ_INVALID != compSrc) {
        // value of a composite object
        const TObjId compDst = addObjectIfNeeded(dc, compSrc);
        return dst.valueOf(compDst);
    }

    if (valSrc <= 0)
        // special value IDs always match
        return valSrc;

    DeepCopyData::TValMap &valMap = dc.valMap;
    DeepCopyData::TValMap::iterator iterValSrc = valMap.find(valSrc);
    if (valMap.end() != iterValSrc)
        // good luck, we have already handled the value before
        return iterValSrc->second;

    const struct cl_type *cltCustom = 0;
    const int custom = src.valGetCustom(&cltCustom, valSrc);
    if (-1 != custom) {
        // custom value, e.g. fnc pointer
        const TValueId valDst = dst.valCreateCustom(cltCustom, custom);
        valMap[valSrc] = valDst;
        return valDst;
    }

    const EUnknownValue code = src.valGetUnknown(valSrc);
    switch (code) {
        case UV_ABSTRACT:
            // will be handled later
        case UV_KNOWN:
            break;

        default: {
            // a proper unkonwn value
            const struct cl_type *cltUnkown = src.valType(valSrc);
            const TValueId valDst = dst.valCreateUnknown(code, cltUnkown);
            valMap[valSrc] = valDst;
            return valDst;
        }
    }

    // now is the time to "dereference" the value
    const TObjId targetSrc = src.pointsTo(valSrc);
    if (OBJ_INVALID == targetSrc)
        TRAP;

    if (targetSrc < 0) {
        // special handling for OBJ_DELETED/OBJ_LOST
        switch (targetSrc) {
            case OBJ_DELETED:
            case OBJ_LOST:
                break;
            default:
                TRAP;
        }

        // FIXME: really safe to ignore (cltValSrc == 0) ??
        const struct cl_type *cltValSrc = src.valType(valSrc);
        const TObjId objTmp = dst.objCreate(cltValSrc);
        const TValueId valDst = dst.placedAt(objTmp);

        // FIXME: avoid using of friend?
        SymHeapCore &core = dynamic_cast<SymHeapCore &>(dst);
        core.objDestroy(objTmp, /* OBJ_DELETED/OBJ_LOST */ targetSrc);

        valMap[valSrc] = valDst;
        return valDst;
    }

    // create the target object, if it does not exist already
    const TObjId targetDst = addObjectIfNeeded(dc, targetSrc);
    const TValueId valDst = dst.placedAt(targetDst);

    if (UV_ABSTRACT == code)
        // preserve UV_ABSTRACT code
        dst.valSetUnknown(valDst, UV_ABSTRACT);

    // return target object's address
    return valDst;
}

void deepCopy(DeepCopyData &dc, bool digBackward) {
    const SymHeap   &src = dc.src;
    SymHeap         &dst = dc.dst;

    DeepCopyData::TItem item;
    while (dc.wl.next(item)) {
        const TObjId objSrc = item.first;
        const TObjId objDst = item.second;

        if (objSrc < 0 || objDst < 0)
            // this should have been handled elsewhere
            TRAP;

        if (objSrc == OBJ_RETURN && objDst == OBJ_RETURN)
            // FIXME: really safe to ignore?
            continue;

        // read the address
        const TValueId atSrc = src.placedAt(objSrc);
        if (atSrc <=0)
            TRAP;

        if (/* optimization */ digBackward) {
            // go from the value backward
            SymHeap::TContObj uses;
            src.usedBy(uses, atSrc);
            BOOST_FOREACH(TObjId objSrc, uses) {
                addObjectIfNeeded(dc, objSrc);
            }
        }

        // read the original value
        TValueId valSrc = src.valueOf(objSrc);
        if (VAL_INVALID == valSrc)
            TRAP;

        // do whatever we need to do with the value
        const TValueId valDst = handleValue(dc, valSrc, digBackward);
        if (VAL_INVALID == valDst)
            TRAP;

        // now set object's value
        dst.objSetValue(objDst, valDst);

        if (/* optimization */ digBackward) {
            // now poke all values related by Neq or EqIf predicates
            SymHeap::TContValue relatedVals;
            src.gatherRelatedValues(relatedVals, valSrc);
            BOOST_FOREACH(TValueId relValSrc, relatedVals) {
                if (valSrc <=0 || relValSrc <= 0)
                    continue;
#if DEBUG_SYMCUT
                CL_DEBUG("deepCopy() is traversing a predicate: #"
                        << valSrc << " -> #" << relValSrc);
#endif
                handleValue(dc, relValSrc, digBackward);
            }
        }
    }

    // finally copy all relevant Neq/EqIf predicates
    src.copyRelevantPreds(dst, dc.valMap);
}

void prune(const SymHeap &src, SymHeap &dst,
           /* NON-const */ DeepCopyData::TCut &cut, bool forwardOnly = false)
{
    DeepCopyData dc(src, dst, cut);
    DeepCopyData::TCut snap(cut);

    // go through all program variables
    BOOST_FOREACH(CVar cv, snap) {
        const TObjId objSrc = dc.src.objByCVar(cv);
        if (OBJ_INVALID == objSrc)
            // failed to resolve program variable
            TRAP;

        const struct cl_type *cltObjSrc = dc.src.objType(objSrc);
        if (!cltObjSrc)
            // we should always know type of program variables
            TRAP;

        addObjectIfNeeded(dc, objSrc);
    }

    // go through the worklist
    deepCopy(dc, !forwardOnly);
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
    CL_DEBUG("splitHeapByCVars() started: cut by " << cut.size() << " variable(s)");

    // std::vector -> std::set
    DeepCopyData::TCut cset;
    fillSet(cset, cut);

    // cut the first part
    SymHeap dst;
    prune(*srcDst, dst, cset);

    if (!saveSurroundTo) {
        // we're done
        *srcDst = dst;
        return;
    }
#if DEBUG_SYMCUT
    CL_DEBUG("splitHeapByCVars() is computing the surround...");
#endif
    // get the complete list of program variables
    SymHeap::TContCVar all;
    srcDst->gatherCVars(all);

    // FIXME: use an algorithm?
    DeepCopyData::TCut complement;
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

    const float ratio = 100.0 * dst.lastValueId() / srcDst->lastValueId();
    CL_DEBUG("splitHeapByCVars() resulting heap size: " << std::fixed
            << std::setprecision(2) << std::setw(5) << ratio << "%");

    // basic sanity check
    if (cntA < cntOrig || cntA + cntB != cntTotal) {
        CL_ERROR("symcut: splitHeapByCVars() failed, attempt to plot heaps...");
        plotHeap(bt, *srcDst,         "prune-input");
        plotHeap(bt,  dst,            "prune-output");
        plotHeap(bt, *saveSurroundTo, "prune-surround");
        CL_NOTE("symcut: plot done, please consider analyzing the results");
        TRAP;
    }

    // update *srcDst (we can't do it sooner because of the plotting above)
    *srcDst = dst;
}

void joinHeapsByCVars(const SymBackTrace *bt, SymHeap *srcDst,
                      const SymHeap *src2)
{
    (void) bt;
#if SE_DISABLE_SYMCUT
    return;
#endif
    // gather _all_ program variables of *src2
    SymHeap::TContCVar all;
    src2->gatherCVars(all);

    // std::vector -> std::set
    DeepCopyData::TCut cset;
    fillSet(cset, all);
    
    // forward-only merge of *src2 into *srcDst
    prune(*src2, *srcDst, cset, /* optimization */ true);
}
