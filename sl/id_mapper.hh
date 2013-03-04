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

#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

enum EDirection {
    D_LEFT_TO_RIGHT,
    D_RIGHT_TO_LEFT
};

template <typename TId>
class IdMapper {
    public:
        typedef std::vector<TId> TVector;

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

        bool /* changed */ insert(const TId left, const TId right);

        template <EDirection>
        void query(TVector *pDst, const TId id) const;

    private:
        typedef boost::bimaps::multiset_of<TId>     TMulti;
        typedef boost::bimap<TMulti, TMulti>        TBiMap;
        typedef typename TBiMap::value_type         TPair;

        ENotFoundAction             nfa_;
        TBiMap                      biMap_;
};

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
};

template <class TBiMap>
struct IdMapperLib<TBiMap, D_RIGHT_TO_LEFT> {
    typedef typename TBiMap::right_map TMap;

    static TMap& mapFromBiMap(TBiMap &bm)             { return bm.right; }

    static const TMap& mapFromBiMap(const TBiMap &bm) { return bm.right; }
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

#endif /* H_GUARD_ID_MAPPER_H */
