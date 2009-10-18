/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_RELOCATOR_H
#define H_GUARD_RELOCATOR_H

#include <map>

template <class TKey, class TValue>
class Relocator {
        typedef std::map<TKey, TValue> TMap;
        TMap    map_;
        TValue  start_;
        TValue  last_;

    public:
        Relocator(TValue start):
            start_(start),
            last_(start)
        {
        }

        void reset() {
            map_.clear();
            last_ = start_;
        }

        TValue lookup(TKey key) {
            typename TMap::iterator i = map_.find(key);
            if (map_.end() != i)
                return i->second;
            else
                return map_[key] = last_++;
        }
};

#endif /* H_GUARD_RELOCATOR_H */
