/*
 * Copyright (C) 2012 Pavel Raiskup <pavel@raiskup.cz>
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

#ifndef H_GUARD_FIXPOINT_H
#define H_GUARD_FIXPOINT_H

#include "util.hh"

#include <set>
#include <list>

/// really stupid, but easy to use, DFS implementation
template <class T>
class FixPoint {
    public:
        typedef T value_type;

    protected:
        // just simulate c++11 unordered set (with worst complexity)
        std::set<T> todo_;
        std::list<T> todo_order_;

        int access_counter_;

    public:
        FixPoint() :
            access_counter_(0)
        {
        }

        FixPoint(const T &item) {
            todo_.insert(item);
            todo_order_.push_back(item);
        }

        bool next(T &dst) {
            if (todo_.empty())
                return false;

            dst = *todo_order_.begin();
            // delete from internal structures
            todo_order_.pop_front();
            todo_.erase(dst);

            access_counter_++;
            return true;
        }

        bool schedule(const T &item) {
            if (hasKey(todo_, item))
                return false;

            todo_.insert(item);
            todo_order_.push_back(item);
            return true;
        }

        int steps() { return access_counter_; }
};

#endif /* H_GUARD_FIXPOINT_H */
