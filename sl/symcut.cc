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
#include <cl/clutil.hh>
#include <cl/code_listener.h>
#include <cl/storage.hh>

#include "symplot.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "worklist.hh"

#include <iomanip>
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

class UniBlockWriter {
    private:
        SymHeap         &dst_;
        const TValId    rootDst_;

    public:
        UniBlockWriter(SymHeap &dst, const TValId rootDst):
            dst_(dst),
            rootDst_(rootDst)
        {
        }

        bool operator()(const SymHeap &src, const UniformBlock &bl) {
            const TValId addrDst = dst_.valByOffset(rootDst_, bl.off);

            const TValId tplValue = translateValProto(dst_, src, bl.tplValue);
            dst_.writeUniformBlock(addrDst, tplValue, bl.size);

            return /* continue */ true;
        }
};

struct DeepCopyData {
    typedef std::map<TValId     /* seg */, TMinLen   /* len */> TSegLengths;
    typedef std::pair<ObjHandle /* src */, ObjHandle /* dst */> TItem;
    typedef std::set<CVar>                                      TCut;

    SymHeap             &src;
    SymHeap             &dst;
    TCut                &cut;
    const bool          digBackward;

    TSegLengths         segLengths;
    TValMap             valMap;

    WorkList<TItem>     wl;

    DeepCopyData(const SymHeap &src_, SymHeap &dst_, TCut &cut_,
                 bool digBackward_):
        src(/* XXX */ const_cast<SymHeap &>(src_)),
        dst(dst_),
        cut(cut_),
        digBackward(digBackward_)
    {
    }
};

class DCopyObjVisitor {
    private:
        DeepCopyData &dc_;

    public:
        DCopyObjVisitor(DeepCopyData &dc): dc_(dc) { }

        bool operator()(const ObjHandle item[2]) {
            const ObjHandle &objSrc = item[/* src */ 0];
            const ObjHandle &objDst = item[/* dst */ 1];

            const TValId srcAt = objSrc.placedAt();
            const TValId dstAt = objDst.placedAt();
            dc_.valMap[srcAt] = dstAt;

            const DeepCopyData::TItem schedItem(objSrc, objDst);
            dc_.wl.schedule(schedItem);

            return /* continue */ true;
        }
};

void digSubObjs(DeepCopyData &dc, TValId addrSrc, TValId addrDst)
{
    const EValueTarget code = dc.src.valTarget(addrSrc);
    if (isPossibleToDeref(code)) {
        // copy uniform blocks
        UniBlockWriter blVisitor(dc.dst, addrDst);
        traverseUniformBlocks(dc.src, addrSrc, blVisitor);
    }

    SymHeap *const heaps[] = { &dc.src, &dc.dst };
    const TValId roots[] = { addrSrc, addrDst };

    DCopyObjVisitor objVisitor(dc);
    traverseLiveObjsGeneric<2>(heaps, roots, objVisitor);
}

TValId /* rootDstAt */ addObjectIfNeeded(DeepCopyData &dc, TValId rootSrcAt) {
    if (VAL_NULL == rootSrcAt)
        return VAL_NULL;

    TValMap &valMap = dc.valMap;
    TValMap::iterator itRootSrc = valMap.find(rootSrcAt);
    if (valMap.end() != itRootSrc)
        // good luck, we have already handled the value before
        return itRootSrc->second;

    CL_BREAK_IF(VAL_ADDR_OF_RET == rootSrcAt);
    SymHeap &src = dc.src;
    SymHeap &dst = dc.dst;

    CVar cv;
    if (isProgramVar(src.valTarget(rootSrcAt))) {
        // program variable
        cv = src.cVarByRoot(rootSrcAt);
#if DEBUG_SYMCUT
        const size_t orig = dc.cut.size();
#endif
        // enlarge the cut if needed
        dc.cut.insert(cv);
#if DEBUG_SYMCUT
        if (dc.cut.size() != orig)
            CL_DEBUG("addObjectIfNeeded() is enlarging the cut by cVar #"
                    << cv.uid << ", nestlevel = " << cv.inst);
#endif
        const TValId rootDstAt = dst.addrOfVar(cv, /* createIfNeeded */ true);
        dc.valMap[rootSrcAt] = rootDstAt;
        digSubObjs(dc, rootSrcAt, rootDstAt);
        return rootDstAt;
    }

    // create the object in 'dst'
    const TSizeRange size = src.valSizeOfTarget(rootSrcAt);
    TValId rootDstAt = dst.heapAlloc(size);

    // preserve type-info if known
    const TObjType clt = src.valLastKnownTypeOfTarget(rootSrcAt);
    if (clt)
        dst.valSetLastKnownTypeOfTarget(rootDstAt, clt);

    // preserve prototype level
    const TProtoLevel protoLevel = src.valTargetProtoLevel(rootSrcAt);
    dst.valTargetSetProtoLevel(rootDstAt, protoLevel);

    // preserve metadata of abstract objects
    if (isAbstract(src.valTarget(rootSrcAt))) {
        const EObjKind kind = src.valTargetKind(rootSrcAt);
        const BindingOff off = (OK_OBJ_OR_NULL == kind)
            ? BindingOff(OK_OBJ_OR_NULL)
            : src.segBinding(rootSrcAt);

        dst.valTargetSetAbstract(rootDstAt, kind, off);

#if SE_SYMCUT_PRESERVES_MIN_LENGTHS
        const TMinLen minLength = objMinLength(src, rootSrcAt);
        dc.segLengths[rootDstAt] = minLength;
#endif
    }

    // store mapping of values
    dc.valMap[rootSrcAt] = rootDstAt;

    // look inside
    digSubObjs(dc, rootSrcAt, rootDstAt);
    return rootDstAt;
}

TValId handleValueCore(DeepCopyData &dc, TValId srcAt) {
    TValMap &valMap = dc.valMap;
    TValMap::iterator iterValSrc = valMap.find(srcAt);
    if (valMap.end() != iterValSrc)
        // good luck, we have already handled the value before
        return iterValSrc->second;

    const TValId rootSrcAt = dc.src.valRoot(srcAt);
    const TValId rootDstAt = addObjectIfNeeded(dc, rootSrcAt);

    if (VT_RANGE == dc.src.valTarget(srcAt)) {
        // range offset value
        const IR::Range range = dc.src.valOffsetRange(srcAt);
        const TValId dstAt = dc.dst.valByRange(rootDstAt, range);
        dc.valMap[srcAt] = dstAt;
        return dstAt;
    }

    const TOffset off = dc.src.valOffset(srcAt);
    if (!off)
        return rootDstAt;

    const TValId dstAt = dc.dst.valByOffset(rootDstAt, off);
    dc.valMap[srcAt] = dstAt;
    return dstAt;
}

TValId handleCustomValue(DeepCopyData &dc, const TValId valSrc) {
    // custom value, e.g. fnc pointer
    const CustomValue custom = dc.src.valUnwrapCustom(valSrc);
    const TValId valDst = dc.dst.valWrapCustom(custom);
    dc.valMap[valSrc] = valDst;
    return valDst;
}

void trackUses(DeepCopyData &dc, TValId valSrc) {
    if (!dc.digBackward)
        // optimization
        return;

    SymHeap &sh = dc.src;
    ObjList uses;

    const TValId rootSrcAt = sh.valRoot(valSrc);
    if (VAL_NULL == rootSrcAt)
        // nothing to track actually
        return;

    const EValueTarget code = sh.valTarget(rootSrcAt);
    if (isPossibleToDeref(code))
        sh.pointedBy(uses, rootSrcAt);
    else
        sh.usedBy(uses, valSrc, /* liveOnly */ true);

    // go from the value backward
    BOOST_FOREACH(const ObjHandle &objSrc, uses) {
        const TValId srcAt = objSrc.placedAt();
        const EValueTarget code = sh.valTarget(srcAt);
        if (!isPossibleToDeref(code))
            continue;

        handleValueCore(dc, srcAt);
    }
}

TValId handleValue(DeepCopyData &dc, TValId valSrc) {
    SymHeap &src = dc.src;
    SymHeap &dst = dc.dst;

    if (valSrc <= 0)
        // special value IDs always match
        return valSrc;

    TValMap &valMap = dc.valMap;
    TValMap::iterator iterValSrc = valMap.find(valSrc);
    if (valMap.end() != iterValSrc)
        // good luck, we have already handled the value before
        return iterValSrc->second;

    trackUses(dc, valSrc);

    const EValueTarget code = realValTarget(src, valSrc);
    if (VT_CUSTOM == code)
        // custom value, e.g. fnc pointer
        return handleCustomValue(dc, valSrc);

    if (isAnyDataArea(code) || VAL_NULL == src.valRoot(valSrc))
        // create the target object, if it does not exist already
        return handleValueCore(dc, valSrc);

    // an unkonwn value
    const EValueOrigin vo = src.valOrigin(valSrc);
    const TValId valDst = dst.valCreate(code, vo);
    valMap[valSrc] = valDst;
    return valDst;
}

void deepCopy(DeepCopyData &dc) {
    SymHeap &src = dc.src;
    SymHeap &dst = dc.dst;

    DeepCopyData::TItem item;
    while (dc.wl.next(item)) {
        const ObjHandle &objSrc = item.first;
        const ObjHandle &objDst = item.second;
        CL_BREAK_IF(!objSrc.isValid() || !objDst.isValid());

        // read the address
        const TValId atSrc = objSrc.placedAt();
        CL_BREAK_IF(atSrc <= 0);
        trackUses(dc, atSrc);

        // read the original value
        TValId valSrc = objSrc.value();
        CL_BREAK_IF(VAL_INVALID == valSrc);
        if (isComposite(objDst.objType(), /* includingArray */ false))
            continue;

        // do whatever we need to do with the value
        const TValId valDst = handleValue(dc, valSrc);
        CL_BREAK_IF(VAL_INVALID == valDst);

        // now set object's value
        objDst.setValue(valDst);
    }

    // finally copy all relevant predicates
    src.copyRelevantPreds(dst, dc.valMap);

    typedef DeepCopyData::TSegLengths TSegLengths;
    BOOST_FOREACH(TSegLengths::const_reference item, dc.segLengths) {
        const TValId seg = item.first;
        const TMinLen minLength = item.second;
        dst.segSetMinLength(seg, minLength);
    }
}

void prune(const SymHeap &src, SymHeap &dst,
           /* NON-const */ DeepCopyData::TCut &cut, bool forwardOnly = false)
{
    DeepCopyData dc(src, dst, cut, !forwardOnly);
    DeepCopyData::TCut snap(cut);

    // go through all program variables
    BOOST_FOREACH(CVar cv, snap) {
        const TValId srcAt = dc.src.addrOfVar(cv, /* createIfNeeded */ true);
        const TValId dstAt = dc.dst.addrOfVar(cv, /* createIfNeeded */ true);
        dc.valMap[srcAt] = dstAt;
        digSubObjs(dc, srcAt, dstAt);
    }

    if (src.valLastKnownTypeOfTarget(VAL_ADDR_OF_RET))
        // clone VAL_ADDR_OF_RET
        digSubObjs(dc, VAL_ADDR_OF_RET, VAL_ADDR_OF_RET);

    // go through the worklist
    deepCopy(dc);
}

void splitHeapByCVars(
        SymHeap                     *srcDst,
        const TCVarList             &cut,
        SymHeap                     *saveFrameTo)
{
#if SE_DISABLE_SYMCUT
    return;
#endif

#if DEBUG_SYMCUT
    CL_DEBUG("splitHeapByCVars() started: cut by " << cut.size() << " variable(s)");
#endif
    // get the set of live program variables
    DeepCopyData::TCut live;
    gatherProgramVars(live, *srcDst);

    // make an intersection with the cut
    DeepCopyData::TCut cset;
    BOOST_FOREACH(const CVar &cv, cut) {
        if (hasKey(live, cv))
            cset.insert(cv);
    }

    // cut the first part
#if DEBUG_SYMCUT || !defined NDEBUG
    const unsigned cntOrig = cset.size();
#endif
    SymHeap dst(srcDst->stor(), new Trace::TransientNode("splitHeapByCVars()"));
    prune(*srcDst, dst, cset);

    if (!saveFrameTo) {
        // we're done
        *srcDst = dst;
        return;
    }
#if DEBUG_SYMCUT
    CL_DEBUG("splitHeapByCVars() is computing the frame...");
#endif
    // get the complete list of program variables
    TCVarList all;
    gatherProgramVars(all, *srcDst);

    // compute set difference (we cannot use std::set_difference since 'all' is
    // not sorted, which would break the algorithm badly)
    DeepCopyData::TCut complement;
    BOOST_FOREACH(const CVar &cv, all)
        if (!hasKey(cset, cv))
            complement.insert(cv);

    // compute the corresponding frame
    prune(*srcDst, *saveFrameTo, complement);

    // print some statistics
#if DEBUG_SYMCUT || !defined NDEBUG
    const unsigned cntA = cset.size();
    const unsigned cntB = complement.size();
    const unsigned cntTotal = all.size();
#endif

#if DEBUG_SYMCUT
    CL_DEBUG("splitHeapByCVars() finished: "
            << cntOrig << " -> " << cntA << " |" << cntB
            << " (" << cntTotal << " program variables in total)");

    const float ratio = 100.0 * dst.lastId() / srcDst->lastId();
    CL_DEBUG("splitHeapByCVars() resulting heap size: " << std::fixed
            << std::setprecision(2) << std::setw(5) << ratio << "%");
#endif

#ifndef NDEBUG
    // basic sanity check
    if (cntA < cntOrig || cntA + cntB != cntTotal) {
        CL_ERROR("symcut: splitHeapByCVars() failed, attempt to plot heaps...");
        plotHeap(*srcDst,         "cut-input");
        plotHeap( dst,            "cut-output");
        plotHeap(*saveFrameTo,    "cut-frame");
        CL_NOTE("symcut: plot done, please consider analyzing the results");
        CL_TRAP;
    }
#endif

    // update *srcDst (we can't do it sooner because of the plotting above)
    *srcDst = dst;
}

void joinHeapsByCVars(
        SymHeap                     *srcDst,
        const SymHeap               *src2)
{
#if SE_DISABLE_SYMCUT
    return;
#endif
    // gather _all_ program variables of *src2
    DeepCopyData::TCut cset;
    gatherProgramVars(cset, *src2);
    
    // forward-only merge of *src2 into *srcDst
    prune(*src2, *srcDst, cset, /* optimization */ true);
}
