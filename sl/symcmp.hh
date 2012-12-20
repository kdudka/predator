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

/// either intra-heap or inter-heap value mapping
typedef TValMap                                             TValMapBidir[2];

/// @todo some dox
bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2);

inline bool checkNonPosValues(int a, int b)
{
    if (0 < a && 0 < b)
        // we'll need to properly compare positive values
        return true;

    // non-positive values always have to match, bail out otherwise
    return (a == b);
}

template <typename TMap>
bool mapBidir(
        TMap                            bMap[2],
        const typename TMap::key_type   v1,
        const typename TMap::key_type   v2)
{
    // left-to-right check
    TMap &ltr = bMap[/* ltr */ 0];
    const typename TMap::iterator iter1 = ltr.find(v1);
    if (iter1 != ltr.end())
        // substitution already defined, check if it applies seamlessly
        return iter1->second == v2;

    // right-to-left check
    TMap &rtl = bMap[/* rtl */ 1];
    const typename TMap::iterator iter2 = rtl.find(v2);
    if (iter2 != rtl.end())
        // substitution already defined, check if it applies seamlessly
        return iter2->second == v1;

    // not found --> define a new substitution
    ltr[v1] = v2;
    rtl[v2] = v1;
    return true;
}

#endif /* H_GUARD_SYM_CMP_H */
