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

#include "symseg.hh"
#include "symutil.hh"
#include "worklist.hh"

struct ProtoFinder {
    std::set<TValId> protos;

    bool operator()(const ObjHandle &sub) {
        const TValId val = sub.value();
        if (val <= 0)
            return /* continue */ true;

        SymHeapCore *sh = sub.sh();
        const TValId root = sh->valRoot(val);
        if (sh->valTargetProtoLevel(root))
            protos.insert(root);

        return /* continue */ true;
    }
};

// visitor
class ProtoCollector {
    private:
        TValList               &protoList_;
        const bool              skipDlsPeers_;
        TObjSet                 ignoreList_;
        WorkList<TValId>        wl_;

    public:
        ProtoCollector(TValList &dst, bool skipDlsPeers):
            protoList_(dst),
            skipDlsPeers_(skipDlsPeers)
        {
        }

        TObjSet& ignoreList() {
            return ignoreList_;
        }

        bool operator()(const ObjHandle &obj);
};

bool ProtoCollector::operator()(const ObjHandle &obj) {
    if (hasKey(ignoreList_, obj))
        return /* continue */ true;

    const TValId val = obj.value();
    if (val <= 0)
        return /* continue */ true;

    SymHeap &sh = *static_cast<SymHeap *>(obj.sh());
    if (!isPossibleToDeref(sh.valTarget(val)))
        return /* continue */ true;

    // check if we point to prototype, or shared data
    if (!sh.valTargetProtoLevel(val))
        return /* continue */ true;

    TValId proto = sh.valRoot(val);
    wl_.schedule(proto);
    while (wl_.next(proto)) {
        ProtoFinder visitor;
        traverseLivePtrs(sh, proto, visitor);
        BOOST_FOREACH(const TValId protoAt, visitor.protos)
            wl_.schedule(protoAt);

            if (skipDlsPeers_ && isDlSegPeer(sh, proto))
                // we are asked to return only one part of each DLS
                continue;

        protoList_.push_back(proto);
    }

    return /* continue */ true;
}

bool collectPrototypesOf(
        TValList                   &dst,
        SymHeap                    &sh,
        const TValId                root,
        const bool                  skipDlsPeers)
{
    CL_BREAK_IF(sh.valOffset(root));
    if (!isAbstract(sh.valTarget(root)))
        // only abstract objects are allowed to have prototypes
        return false;

    ProtoCollector collector(dst, skipDlsPeers);
    buildIgnoreList(collector.ignoreList(), sh, root);
    return traverseLivePtrs(sh, root, collector);
}
