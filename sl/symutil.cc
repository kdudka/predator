/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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
#include "symutil.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "symbt.hh"
#include "symheap.hh"
#include "symproc.hh"
#include "symstate.hh"
#include "util.hh"

#include <stack>

#include <boost/foreach.hpp>

void moveKnownValueToLeft(
        const SymHeapCore           &sh,
        TValId                      &valA,
        TValId                      &valB)
{
    sortValues(valA, valB);
    if (valA <= 0 || VAL_TRUE == valA)
        return;

    const EValueTarget code = sh.valTarget(valA);
    if (VT_ON_HEAP == code || isProgramVar(code)
            || /* FIXME */ VT_CUSTOM == code
            || /* FIXME */ isGone(code))
        return;

    const TValId tmp = valA;
    valA = valB;
    valB = tmp;
}

void getPtrValues(TValList &dst, const SymHeap &sh, TValId at) {
    TObjList ptrs;
    sh.gatherLivePointers(ptrs, at);
    BOOST_FOREACH(const TObjId obj, ptrs) {
        const TValId val = sh.valueOf(obj);
        if (0 < val)
            dst.push_back(sh.valueOf(obj));
    }
}

bool initSingleVariable(
        SymHeap                         &sh,
        const TObjId                     obj)
{
    const struct cl_type *clt = sh.objType(obj);
    CL_BREAK_IF(!clt);

    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_ARRAY:
            CL_DEBUG("CL_TYPE_ARRAY is not supported by VarInitializer");
            return /* continue */ true;

        case CL_TYPE_UNION:
        case CL_TYPE_STRUCT:
            CL_TRAP;

        default:
            break;
    }

    // no initializer given, nullify the variable
    sh.objSetValue(obj, /* also equal to VAL_FALSE */ VAL_NULL);
    return /* continue */ true;
}

class InitVarLegacyWrapper {
    private:
        SymHeap                                 &sh_;
        const TValId                            root_;
        const TObjType                          rootClt_;
        const struct cl_initializer            *initial_;

    public:
        InitVarLegacyWrapper(
                SymHeap                         &sh,
                const TValId                    root,
                const TObjType                  clt,
                const struct cl_initializer    *initial):
            sh_(sh),
            root_(root),
            rootClt_(clt),
            initial_(initial)
        {
            CL_BREAK_IF(sh.valOffset(root));
        }

        bool operator()(TFieldIdxChain ic, const struct cl_type_item *item)
            const
        {
            const TObjType clt = item->type;
            if (isComposite(clt))
                return /* continue */ true;

            const TOffset off = offsetByIdxChain(rootClt_, ic);
            const TValId at = sh_.valByOffset(root_, off);
            const TObjId obj = sh_.objAt(at, clt);
            CL_BREAK_IF(obj <= 0);

            CL_BREAK_IF(initial_);
            initSingleVariable(sh_, obj);
            return /* continue */ true;
        }
};

void initVariable(
        SymHeap                     &sh,
        SymBackTrace                &bt,
        const CodeStorage::Var      &var)
{
    if (!isOnStack(var)) {
        // nullify a gl variable
        const CVar cv(var.uid, /* gl variable */ 0);
        const TValId at = sh.addrOfVar(cv);
        const TObjType clt = var.type;

        if (isComposite(clt)) {
            const InitVarLegacyWrapper visitor(sh, at, clt, /* nullify */ 0);
            traverseTypeIc(var.type, visitor, /* digOnlyComposite */ true);
        }
        else {
            const TObjId obj = sh.objAt(at, clt);
            initSingleVariable(sh, obj);
        }
    }

    SymExecCore core(sh, &bt, SymExecCoreParams());
    core.setLocation(&var.loc);
    BOOST_FOREACH(const CodeStorage::Insn *insn, var.initials) {
        CL_DEBUG_MSG(&var.loc,
                "(I) executing an explicit var initializer: " << *insn);
        SymHeapList dst;

        if (!core.exec(dst, *insn))
            CL_BREAK_IF("initVariable() malfunction");

        CL_BREAK_IF(1 != dst.size());
        SymHeap &result = const_cast<SymHeap &>(dst[/* the only result */ 0]);
        sh.swap(result);
    }
}

class PointingObjectsFinder {
    public:
        // we have to use std::set, a vector is not sufficient in all cases
        typedef std::set<TObjId> TResults;

    private:
        TResults results_;

    public:
        const TResults& results() const { return results_; }

        bool operator()(const SymHeap &sh, TObjId obj) {
            const TValId addr = sh.placedAt(obj);
            CL_BREAK_IF(addr <= 0);

            TObjList refs;
            sh.usedBy(refs, addr);
            std::copy(refs.begin(), refs.end(),
                      std::inserter(results_, results_.begin()));

            return /* continue */ true;
        }
};

void redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo)
{
    // go through all objects pointing at/inside pointingTo
    TObjList refs;
    sh.pointedBy(refs, pointingTo);
    BOOST_FOREACH(const TObjId obj, refs) {
        const TValId referrerAt = sh.valRoot(sh.placedAt(obj));
        if (VAL_INVALID != pointingFrom && pointingFrom != referrerAt)
            // pointed from elsewhere, keep going
            continue;

        // check the current link
        const TValId nowAt = sh.valueOf(obj);
        const TOffset offToRoot = sh.valOffset(nowAt);
        CL_BREAK_IF(sh.valOffset(redirectTo));

        // redirect accordingly
        const TValId result = sh.valByOffset(redirectTo, offToRoot);
        sh.objSetValue(obj, result);
    }
}
