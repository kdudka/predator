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
#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/foreach.hpp>

enum EDirection {
    D_LEFT_TO_RIGHT,
    D_RIGHT_TO_LEFT
};

template <typename TId>
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
            return biMap_.empty();
        }

        unsigned size() const
        {
            return biMap_.size();
        }

        bool /* changed */ insert(const TId left, const TId right);

        template <EDirection>
        void query(TVector *pDst, const TId id) const;

        template <EDirection>
        void composite(const IdMapper &by);

        void prettyPrint(std::ostream &) const;

    private:
        typedef boost::bimaps::multiset_of<TId>     TMulti;
        typedef boost::bimap<TMulti, TMulti>        TBiMap;
        typedef typename TBiMap::value_type         TPair;

        ENotFoundAction             nfa_;
        TBiMap                      biMap_;
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

template <typename TId>
bool /* changed */ IdMapper<TId>::insert(const TId left, const TId right)
{
    const TPair item(left, right);
    return biMap_.insert(item)./* changed */second;
}

template <class TBiMap, EDirection>
struct IdMapperLib { };

template <class TBiMap>
struct IdMapperLib<TBiMap, D_LEFT_TO_RIGHT> {
    typedef typename TBiMap::left_map TMap;

    static TMap& mapFromBiMap(TBiMap &bm)             { return bm.left; }

    static const TMap& mapFromBiMap(const TBiMap &bm) { return bm.left; }

    template <typename TId>
    static void insertTo(TBiMap *pBiMap, const TId a, const TId b) {
        const typename TBiMap::value_type item(a, b);
        pBiMap->insert(item);
    }
};

template <class TBiMap>
struct IdMapperLib<TBiMap, D_RIGHT_TO_LEFT> {
    typedef typename TBiMap::right_map TMap;

    static TMap& mapFromBiMap(TBiMap &bm)             { return bm.right; }

    static const TMap& mapFromBiMap(const TBiMap &bm) { return bm.right; }

    template <typename TId>
    static void insertTo(TBiMap *pBiMap, const TId a, const TId b) {
        const typename TBiMap::value_type item(b, a);
        pBiMap->insert(item);
    }
};

template <typename TId>
template <EDirection DIR>
void IdMapper<TId>::query(TVector *pDst, const TId id) const
{
    // resolve types according to DIR
    typedef IdMapperLib<TBiMap, DIR>                TLib;
    typedef typename TLib::TMap                     TMap;
    typedef typename TMap::const_iterator           TIter;

    // find first
    const TMap &m = TLib::mapFromBiMap(biMap_);
    const TIter beg = m.find(id);
    if (beg == m.end()) {
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
    const TIter end = m.upper_bound(id);
    CL_BREAK_IF(beg == end);

    // copy the image to the given vector
    for (TIter it = beg; it != end; ++it) {
        CL_BREAK_IF(id != it->first);
        pDst->push_back(it->second);
    }
}

template <typename TId>
template <EDirection DIR>
void IdMapper<TId>::composite(const IdMapper<TId> &by)
{
    if (by.nfa_ < nfa_)
        nfa_ = by.nfa_;

    TBiMap result;

    // resolve types according to DIR
    typedef IdMapperLib<TBiMap, DIR>                TLib;
    typedef typename TLib::TMap                     TMap;

    // iterate through the mapping of 'this'
    const TMap &m = TLib::mapFromBiMap(biMap_);
    BOOST_FOREACH(typename TMap::const_reference item, m) {
        const TId a = item.first;
        const TId b = item.second;
        TVector cList;
        this->query<DIR>(&cList, b);
        BOOST_FOREACH(const TId c, cList)
            TLib::insertTo(&result, a, c);
    }

    if (NFA_RETURN_IDENTITY == nfa_) {
        // iterate through the mapping of 'by'
        const TMap &mBy = TLib::mapFromBiMap(by.biMap_);
        BOOST_FOREACH(typename TMap::const_reference item, mBy) {
            const TId b = item.first;
            const TId c = item.second;

            // reverse lookup
            TVector aList;
            if (D_LEFT_TO_RIGHT == DIR)
                by.query<D_RIGHT_TO_LEFT>(&aList, b);
            else
                by.query<D_LEFT_TO_RIGHT>(&aList, b);

            BOOST_FOREACH(const TId a, aList)
                TLib::insertTo(&result, a, c);
        }
    }

    // finally replace the mapping of 'this' by the result
    biMap_.swap(result);
}

template <typename TId>
void IdMapper<TId>::prettyPrint(std::ostream &str) const
{
    unsigned i = 0U;
    BOOST_FOREACH(typename TBiMap::left_const_reference item, biMap_.left) {
        if (i++)
            str << ", ";

        str << item.first << " -> " << item.second;
    }
}

#endif /* H_GUARD_ID_MAPPER_H */
