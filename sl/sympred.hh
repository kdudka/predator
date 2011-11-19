/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_PRED_H
#define H_GUARD_SYM_PRED_H

#include "symid.hh"
#include "util.hh"

#include <map>
#include <set>

/// @todo give this class (and consequently this module) a more generic name
class NeqDb {
    public:
        bool empty() const {
            return cont_.empty();
        }

        void add(TValId valLt, TValId valGt);
        void del(TValId valLt, TValId valGt);

        /// @todo give this method a more generic name
        bool areNeq(TValId valLt, TValId valGt) const;

    protected:
        typedef std::pair<TValId /* valLt */, TValId /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;
};

template <class TKey, class TVal>
class PairMap {
    protected:
        typedef std::pair<TKey /* lt */, TKey /* gt */>     TItem;
        typedef std::map<TItem, TVal>                       TMap;
        TMap db_;

    public:
        void add(TKey k1, TKey k2, TVal val) {
            sortValues(k1, k2);
            const TItem key(k1, k2);

            CL_BREAK_IF(hasKey(db_, key));
            db_[key] = val;
        }

        bool chk(TVal *pDst, TKey k1, TKey k2) const {
            sortValues(k1, k2);
            const TItem key(k1, k2);

            typename TMap::const_iterator it = db_.find(key);
            if (db_.end() == it)
                return false;

            *pDst = it->second;
            return true;
        }
};

#endif /* H_GUARD_SYM_PRED_H */
