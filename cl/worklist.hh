/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_WORKLIST_H
#define H_GUARD_WORKLIST_H

#include "util.hh"

#include <set>
#include <stack>

/// really stupid, but easy to use, DFS implementation
template <class T>
class WorkList {
    public:
        typedef T value_type;

    protected:
        std::stack<T> todo_;
        std::set<T>   seen_;

    public:
        WorkList() { }
        WorkList(const T &item) {
            todo_.push(item);
            seen_.insert(item);
        }

        bool next(T &dst) {
            if (todo_.empty())
                return false;

            dst = todo_.top();
            todo_.pop();
            return true;
        }

        bool schedule(const T &item) {
            if (hasKey(seen_, item))
                return false;

            todo_.push(item);
            seen_.insert(item);
            return true;
        }

        bool seen(const T &item) const {
            return hasKey(seen_, item);
        }

        // FIXME: really bad idea as log as schedule(const T &) is non-virutal
        template <class T1, class T2>
        bool schedule(const T1 &i1, const T2 &i2) {
            return this->schedule(T(i1, i2));
        }

        unsigned cntSeen() const { return seen_.size(); }
        unsigned cntTodo() const { return todo_.size(); }
};

#endif /* H_GUARD_WORKLIST_H */
