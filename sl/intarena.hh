/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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

template <typename TInt, typename TObj>
class IntervalArena {
    public:
        typedef std::set<TObj>                  TSet;

        // for compatibility with STL
        typedef std::pair<TInt, TInt>           key_type;
        typedef std::pair<key_type, TObj>       value_type;

    public:
        void add(const key_type &, const TObj);
        void sub(const key_type &, const TObj);
        bool intersects(TSet &dst, const key_type &key) const;

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

    private:
        typedef std::map<TInt, value_type>      TCont;
        TCont                                   cont_;
};

template <typename TInt, typename TObj>
void IntervalArena<TInt, TObj>::add(const key_type &key, const TObj obj)
{
    CL_BREAK_IF("please implement");
}

template <typename TInt, typename TObj>
void IntervalArena<TInt, TObj>::sub(const key_type &key, const TObj obj)
{
    CL_BREAK_IF("please implement");
}

template <typename TInt, typename TObj>
bool IntervalArena<TInt, TObj>::intersects(TSet &dst, const key_type &key) const
{
    CL_BREAK_IF("please implement");
}

#endif /* H_GUARD_INTARENA_H */
