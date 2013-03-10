/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_ID_MAPPER_H
#define H_GUARD_ID_MAPPER_H

#include "config.h"

#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

enum EDirection {
    D_LEFT_TO_RIGHT,
    D_RIGHT_TO_LEFT
};

template <typename TId,
         TId MIN = std::numeric_limits<TId>::min(),
         TId MAX = std::numeric_limits<TId>::max()>
class IdMapper {
    public:
        typedef std::vector<TId> TVector;

        /// do not change the order, composite() relies on it
        enum ENotFoundAction {
            NFA_TRAP_TO_DEBUGGER,
            NFA_RETURN_NOTHING,
            NFA_RETURN_IDENTITY
        };

    public:
        IdMapper():
            nfa_(NFA_TRAP_TO_DEBUGGER)
        {
        }

        IdMapper(const ENotFoundAction nfa):
            nfa_(nfa)
        {
        }

        void setNotFoundAction(const ENotFoundAction nfa)
        {
            nfa_ = nfa;
        }

        bool empty() const
        {
            CL_BREAK_IF(biSearch_[0].empty() != biSearch_[1].empty());
            return biSearch_[D_LEFT_TO_RIGHT].empty();
        }

        unsigned size() const
        {
            CL_BREAK_IF(biSearch_[0].size() != biSearch_[1].size());
            return biSearch_[D_LEFT_TO_RIGHT].size();
        }

        bool /* changed */ insert(const TId left, const TId right);

        template <EDirection>
        void query(TVector *pDst, const TId id) const;

        template <EDirection>
        void composite(const IdMapper &by);

        void prettyPrint(std::ostream &) const;

    private:
        typedef std::pair<TId, TId>                 TPair;
        typedef std::set<TPair>                     TSearch;
        typedef TSearch                             TBidirSearch[2];
        typedef typename TSearch::const_iterator    TIter;

        ENotFoundAction             nfa_;
        TBidirSearch                biSearch_;
};

template <EDirection DIR, typename TBiMap, class TDst, class TSrc>
void project(
        const TBiMap               &biMap,
        TDst                       *pDstCont,
        const TSrc                 &srcCont)
{
    BOOST_FOREACH(typename TSrc::value_type src, srcCont) {
        typename TBiMap::TVector dstVect;
        biMap.template query<DIR>(&dstVect, src);
        BOOST_FOREACH(const typename TDst::value_type dst, dstVect)
            pDstCont->insert(dst);
    }
}

template <typename TId, TId MIN, TId MAX>
bool IdMapper<TId, MIN, MAX>::insert(const TId left, const TId right)
{
    const TPair itemL(left, right);
    const TPair itemR(right, left);
    CL_BREAK_IF(hasKey(biSearch_[0], itemL) != hasKey(biSearch_[1], itemR));

    const bool changed = biSearch_[D_LEFT_TO_RIGHT].insert(itemL).second;
    if (!changed)
        return false;

    biSearch_[D_RIGHT_TO_LEFT].insert(itemR);
    return true;
}

template <typename TId, TId MIN, TId MAX>
template <EDirection DIR>
void IdMapper<TId, MIN, MAX>::query(TVector *pDst, const TId id) const
{
    BOOST_STATIC_ASSERT(MIN < MAX);

    const TSearch &search = biSearch_[DIR];

    const TPair begItem(id, MIN);
    const TIter beg = search.lower_bound(begItem);
    if (beg == search.end() || beg->first != id) {
        // not found
        switch (nfa_) {
            case NFA_TRAP_TO_DEBUGGER:
                CL_BREAK_IF("IdMapper failed to resolve the requested ID");
                // fall through!

            case NFA_RETURN_NOTHING:
                return;

            case NFA_RETURN_IDENTITY:
                pDst->push_back(id);
                return;
        }
    }

    // find last (end points one item _beyond_ the last one)
    const TPair endItem(id, MAX);
    const TIter end = search.upper_bound(endItem);
    CL_BREAK_IF(beg == end);

    // copy the image to the given vector
    for (TIter it = beg; it != end; ++it) {
        CL_BREAK_IF(id != it->first);
        pDst->push_back(it->second);
    }
}

template <typename TId, TId MIN, TId MAX>
template <EDirection DIR>
void IdMapper<TId, MIN, MAX>::composite(const IdMapper<TId, MIN, MAX> &by)
{
    if (by.nfa_ < nfa_)
        nfa_ = by.nfa_;

    IdMapper<TId, MIN, MAX> result;

    // iterate through the mapping of 'this'
    const TSearch &m = biSearch_[DIR];
    BOOST_FOREACH(typename TSearch::const_reference item, m) {
        const TId a = item.first;
        const TId b = item.second;
        TVector cList;
        this->query<DIR>(&cList, b);
        BOOST_FOREACH(const TId c, cList)
            result.insert(a, c);
    }

    if (NFA_RETURN_IDENTITY == nfa_) {
        // iterate through the mapping of 'by'
        const TSearch &mBy = by.biSearch_[DIR];
        BOOST_FOREACH(typename TSearch::const_reference item, mBy) {
            const TId b = item.first;
            const TId c = item.second;

            // reverse lookup
            TVector aList;
            if (D_LEFT_TO_RIGHT == DIR)
                by.query<D_RIGHT_TO_LEFT>(&aList, b);
            else
                by.query<D_LEFT_TO_RIGHT>(&aList, b);

            BOOST_FOREACH(const TId a, aList)
                result.insert(a, c);
        }
    }

    // finally replace the mapping of 'this' by the result
    biSearch_[0].swap(result.biSearch_[D_RIGHT_TO_LEFT == DIR]);
    biSearch_[1].swap(result.biSearch_[D_LEFT_TO_RIGHT == DIR]);
}

template <typename TId, TId MIN, TId MAX>
void IdMapper<TId, MIN, MAX>::prettyPrint(std::ostream &str) const
{
    unsigned i = 0U;
    const TSearch &m = biSearch_[D_LEFT_TO_RIGHT];
    BOOST_FOREACH(typename TSearch::const_reference item, m) {
        if (i++)
            str << ", ";

        str << item.first << " -> " << item.second;
    }
}

#endif /* H_GUARD_ID_MAPPER_H */
