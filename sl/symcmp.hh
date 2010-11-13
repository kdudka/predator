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

#include "symid.hh"
#include <map>
#include <vector>

class SymHeap;

/// either intra-heap or inter-heap value mapping
typedef std::map<TValueId /* src */, TValueId /* dst */>    TValMap;

/// @todo some dox
bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        TValMap                 *srcToDst = 0,
        TValMap                 *dstToSrc = 0);

typedef std::pair<TValueId, TValueId>                       TValPair;

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

#endif /* H_GUARD_SYM_CMP_H */
