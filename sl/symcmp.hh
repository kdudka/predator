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

#ifndef H_GUARD_SYM_CMP_H
#define H_GUARD_SYM_CMP_H

/**
 * @file symcmp.hh
 * various algorithms for comparison of symbolic (sub-)heaps
 */

#include "symheap.hh"
#include <map>
#include <vector>

/// @todo some dox
bool joinUnknownValuesCode(
        EUnknownValue           *pDst,
        const EUnknownValue     code1,
        const EUnknownValue     code2);

/// either intra-heap or inter-heap value mapping
typedef std::map<TValId /* src */, TValId /* dst */>        TValMap;

/// @todo some dox
bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        TValMap                 *srcToDst = 0,
        TValMap                 *dstToSrc = 0);

typedef std::pair<TValId, TValId>                           TValPair;

class ISubMatchVisitor {
    public:
        virtual ~ISubMatchVisitor() { }
        virtual bool handleValuePair(bool *wantTraverse, TValPair vp) = 0;
};

typedef std::vector<TValPair>                               TValPairList;

bool matchSubHeaps(
        const SymHeap           &sh,
        const TValPairList      &startingPoints,
        ISubMatchVisitor        *visitor = 0);

inline bool checkNonPosValues(int a, int b) {
    if (0 < a && 0 < b)
        // we'll need to properly compare positive values
        return true;

    // non-positive values always have to match, bail out otherwise
    return (a == b);
}

typedef TValMap                                             TValMapBidir[2];

bool matchPlainValues(
        TValMapBidir            valMapping,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValId            v1,
        const TValId            v2);

#endif /* H_GUARD_SYM_CMP_H */
