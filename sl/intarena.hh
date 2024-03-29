/*
 * Copyright (C) 2011-2022 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_INTARENA_H
#define H_GUARD_INTARENA_H

#include "config.h"

#include <map>
#include <set>
#include <vector>

#define IA_AGGRESSIVE_OPTIMIZATION          0

/// ad-hoc implementation;  wastes memory, performance, and human resources
template <typename TInt, typename TFld>
class IntervalArena {
    public:
        typedef std::set<TFld>                      TSet;

        // for compatibility with STL
        typedef std::pair<TInt, TInt>               key_type;
        typedef std::pair<key_type, TFld>           value_type;

        typedef std::vector<key_type>               TKeySet;

    private:
        typedef std::set<TFld>                      TLeaf;
        typedef std::map</* beg */ TInt, TLeaf>     TLine;
        typedef std::map</* end */ TInt, TLine>     TCont;
        TCont                                       cont_;

    public:
        void add(const key_type &, TFld);
        void sub(const key_type &, TFld);
        void intersects(TSet &dst, const key_type &key) const;
        void exactMatch(TSet &dst, const key_type &key) const;

        /// return the set of all keys that map to this object
        void reverseLookup(TKeySet &dst, TFld) const;

        void clear() {
            cont_.clear();
        }

        IntervalArena& operator+=(const value_type &item) {
            this->add(item.first, item.second);
            return *this;
        }

        IntervalArena& operator-=(const value_type &item) {
            this->sub(item.first, item.second);
            return *this;
        }
};

template <typename TInt, typename TFld>
void IntervalArena<TInt, TFld>::add(const key_type &key, const TFld fld)
{
    const TInt beg = key.first;
    const TInt end = key.second;
    CL_BREAK_IF(end <= beg);

    cont_[end][beg].insert(fld);
}

template <typename TInt, typename TFld>
void IntervalArena<TInt, TFld>::sub(const key_type &key, const TFld fld)
{
    const TInt winBeg = key.first;
    const TInt winEnd = key.second;
    CL_BREAK_IF(winEnd <= winBeg);

    std::vector<value_type> recoverList;

    const typename TCont::iterator itEnd = cont_.end();
    typename TCont::iterator it =
        cont_.lower_bound(winBeg + /* right-open interval given as key */ 1);

    while (itEnd != it) {
        TLine &line = it->second;
#if !IA_AGGRESSIVE_OPTIMIZATION
        if (line.empty()) {
            // skip orphans
            ++it;
            continue;
        }
#endif
        typename TLine::iterator lineIt = line.begin();
        TInt beg = lineIt->first;
        if (winEnd <= beg) {
            // we are beyond the window already
            ++it;
            continue;
        }

        const TInt end = it->first;
        bool anyHit = false;

        const typename TLine::iterator lineItEnd = line.end();
        do {
            // make sure the basic window axioms hold
            CL_BREAK_IF(winEnd <= beg);
            CL_BREAK_IF(end <= winBeg);

            // remove the object from the current leaf (if found)
            TLeaf &os = lineIt->second;
            if (os.erase(fld)) {
                anyHit = true;

                if (beg < winBeg) {
                    // schedule "the part above" for re-insertion
                    const key_type key(beg, winBeg);
                    const value_type item(key, fld);
                    recoverList.push_back(item);
                }
            }

#if IA_AGGRESSIVE_OPTIMIZATION
            if (os.empty())
                // FIXME: Can we remove items from std::map during traversal??
                line.erase(lineIt++);
            else
#endif
            ++lineIt;

            if (lineItEnd == lineIt)
                // end of line
                break;

            beg = lineIt->first;
        }
        while (beg < winEnd);

        if (anyHit) {
            if (winEnd < end) {
                // schedule "the part beyond" for re-insertion
                const key_type key(winEnd, end);
                const value_type item(key, fld);
                recoverList.push_back(item);
            }

#if IA_AGGRESSIVE_OPTIMIZATION
            if (line.empty()) {
                // FIXME: Can we remove items from std::map during traversal??
                cont_.erase(it++);
                continue;
            }
#endif
        }

        ++it;
    }

    // go through the recoverList and re-insert the missing parts
    for (const value_type &rItem : recoverList) {
        const key_type &key = rItem.first;
        const TFld fld = rItem.second;
        const TInt beg = key.first;
        const TInt end = key.second;

        cont_[end][beg].insert(fld);
    }
}

template <typename TInt, typename TFld>
void IntervalArena<TInt, TFld>::intersects(TSet &dst, const key_type &key) const
{
    const TInt winBeg = key.first;
    const TInt winEnd = key.second;
    CL_BREAK_IF(winEnd <= winBeg);

    typename TCont::const_iterator it =
        cont_.lower_bound(winBeg + /* right-open interval given as key */ 1);

    for (; cont_.end() != it; ++it) {
        const TLine &line = it->second;
#if !IA_AGGRESSIVE_OPTIMIZATION
        if (line.empty())
            // skip orphans
            continue;
#endif
        typename TLine::const_iterator lineIt = line.begin();
        TInt beg = lineIt->first;
        if (winEnd <= beg)
            // we are beyond the window already
            continue;

        const typename TLine::const_iterator lineItEnd = line.end();
        do {
            // make sure the basic window axioms hold
            CL_BREAK_IF(winEnd <= beg);
            CL_BREAK_IF(/* end */ it->first <= winBeg);

            const TLeaf &os = lineIt->second;
            std::copy(os.begin(), os.end(), std::inserter(dst, dst.begin()));

            // increment for next wheel
            if (lineItEnd == ++lineIt)
                // end of line
                break;

            beg = lineIt->first;
        }
        while (beg < winEnd);
    }
}

// FIXME: brute-force method
// FIXME: no assumptions can be made about the output format
template <typename TInt, typename TFld>
void IntervalArena<TInt, TFld>::reverseLookup(TKeySet &dst, const TFld fld)
    const
{
    key_type key;

    for (typename TCont::const_reference item : cont_) {
        key/* end */.second = item/* end */.first;
        const TLine &line = item.second;

        for (typename TLine::const_reference lineItem : line) {
            const TLeaf &leaf = lineItem.second;
            if (!hasKey(leaf, fld))
                continue;

            key/* beg */.first = lineItem/* beg */.first;
            dst.push_back(key);
        }
    }
}

template <typename TInt, typename TFld>
void IntervalArena<TInt, TFld>::exactMatch(TSet &dst, const key_type &key) const
{
    typedef typename TCont::const_iterator TEndIt;
    const TEndIt itEnd = cont_.find(/* end */ key.second);
    if (cont_.end() == itEnd)
        // upper bound not found
        return;

    const TLine &line = itEnd->second;
    const typename TLine::const_iterator itBeg = line.find(/* beg */ key.first);
    if (line.end() == itBeg)
        // lower bound not found
        return;

    const TLeaf &leaf = itBeg->second;
    std::copy(leaf.begin(), leaf.end(), std::inserter(dst, dst.begin()));
}

#endif /* H_GUARD_INTARENA_H */
