/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYMGC_H
#define H_GUARD_SYMGC_H

/**
 * @file symgc.hh
 * collectJunk - implementation of a garbage collector for the symbolic heap
 */

#include "symheap.hh"

#include <boost/foreach.hpp>

#define REQUIRE_GC_ACTIVITY(sh, obj, fnc) do {                                 \
    if (collectJunk(sh, obj))                                                  \
        break;                                                                 \
    CL_ERROR(#fnc "() failed to collect garbage, " #obj " still referenced");  \
    CL_BREAK_IF("REQUIRE_GC_ACTIVITY has not been successful");                \
} while (0)

/// collect and remove all junk reachable from the given object
bool /* found */ collectJunk(SymHeap &sh, TObjId obj, TObjSet *leakObjs = 0);

/// same as collectJunk(), but does not consider prototypes to be junk objects
bool collectSharedJunk(SymHeap &sh, TObjId obj, TObjSet *leakObjs = 0);

bool destroyObjectAndCollectJunk(
        SymHeap                 &sh,
        const TObjId             obj,
        TObjSet                *leakObjs = 0);

/// @todo some dox
class LeakMonitor {
    public:
        LeakMonitor(SymHeap &sh):
            sh_(sh),
            snap_(sh.stor(), sh.traceNode())
        {
        }

        void enter();
        void leave();

        template <class TCont>
        bool collectJunkFrom(const TCont &killedPtrs) {
            bool leaking = false;
            BOOST_FOREACH(TValId val, killedPtrs) {
                const TObjId obj = sh_.objByAddr(val);
                if (collectJunk(sh_, obj, &leakObjs_))
                    leaking = true;
            }

            return leaking;
        }

        bool /* leaking */ destroyObject(const TObjId obj) {
            return destroyObjectAndCollectJunk(sh_, obj, &leakObjs_);
        }

        bool /* leaking */ importLeakObjs(TObjSet *leakObjs);


    private:
        SymHeap     &sh_;
        SymHeap     snap_;
        TObjSet     leakObjs_;
};

/// enable/disable debugging of the garbage collector
void debugGarbageCollector(const bool enable);

#endif /* H_GUARD_SYMGC_H */
