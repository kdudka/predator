/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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
#include "prototype.hh"

#include <cl/cl_msg.hh>

#include "symseg.hh"
#include "symutil.hh"
#include "worklist.hh"

struct ProtoFinder {
    std::set<TObjId> protos;

    bool operator()(const FldHandle &sub) {
        const TValId val = sub.value();
        if (val <= 0)
            return /* continue */ true;

        SymHeapCore *sh = sub.sh();
        if (sh->objProtoLevel(sh->objByAddr(val)))
            protos.insert(sh->objByAddr(val));

        return /* continue */ true;
    }
};

// visitor
class ProtoCollector {
    private:
        TObjSet                &protoList_;
        TFldSet                 ignoreList_;
        WorkList<TObjId>        wl_;

    public:
        ProtoCollector(TObjSet &dst):
            protoList_(dst)
        {
        }

        TFldSet& ignoreList() {
            return ignoreList_;
        }

        bool operator()(const FldHandle &);
};

bool ProtoCollector::operator()(const FldHandle &fld)
{
    if (hasKey(ignoreList_, fld))
        return /* continue */ true;

    const TValId val = fld.value();
    if (val <= 0)
        return /* continue */ true;

    SymHeap &sh = *static_cast<SymHeap *>(fld.sh());
    if (!isAnyDataArea(sh.valTarget(val)))
        return /* continue */ true;

    // check if we point to prototype, or shared data
    if (!sh.objProtoLevel(sh.objByAddr(val)))
        return /* continue */ true;

    TObjId proto = sh.objByAddr(val);
    wl_.schedule(proto);
    while (wl_.next(proto)) {
        ProtoFinder visitor;
        traverseLiveFields(sh, proto, visitor);
        BOOST_FOREACH(const TObjId proto, visitor.protos)
            wl_.schedule(proto);

        protoList_.insert(proto);
    }

    return /* continue */ true;
}

bool collectPrototypesOf(
        TObjSet                    &dst,
        SymHeap                    &sh,
        const TObjId                obj)
{
    if (OK_REGION == sh.objKind(obj))
        // only abstract objects are allowed to have prototypes
        return false;

    ProtoCollector collector(dst);
    buildIgnoreList(collector.ignoreList(), sh, obj);
    return traverseLiveFields(sh, obj, collector);
}

void objIncrementProtoLevel(SymHeap &sh, TObjId obj)
{
    const TProtoLevel level = sh.objProtoLevel(obj);
    sh.objSetProtoLevel(obj, level + 1);
}

void objDecrementProtoLevel(SymHeap &sh, TObjId obj)
{
    const TProtoLevel level = sh.objProtoLevel(obj);
    sh.objSetProtoLevel(obj, level - 1);
}

void decrementProtoLevel(SymHeap &sh, const TObjId obj)
{
    TObjSet protoList;
    collectPrototypesOf(protoList, sh, obj);
    BOOST_FOREACH(const TObjId proto, protoList)
        objDecrementProtoLevel(sh, proto);
}

bool protoCheckConsistency(const SymHeap &sh)
{
    TObjList allObjs;
    sh.gatherObjects(allObjs);
    BOOST_FOREACH(const TObjId obj, allObjs) {
        if (OK_REGION != sh.objKind(obj))
            continue;

        const TProtoLevel rootLevel = sh.objProtoLevel(obj);

        FldList ptrs;
        sh.gatherLiveFields(ptrs, obj);
        BOOST_FOREACH(const FldHandle &fld, ptrs) {
            const TObjId sub = sh.objByAddr(fld.value());
            if (OBJ_INVALID == sub)
                continue;

            const TProtoLevel level = sh.objProtoLevel(sub);
            if (level <= rootLevel)
                continue;

            CL_ERROR("nesting level bump on a non-abstract object detected");
            return false;
        }
    }

    // all OK
    return true;
}
