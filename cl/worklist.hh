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

#include <queue>
#include <set>
#include <stack>

template <class T, class TShed> struct WorkListLib { };

template <class T>
struct WorkListLib<T, std::stack<T> > {
    static typename std::stack<T>::reference top(std::stack<T> &cont) {
        return cont.top();
    }
};

template <class T>
struct WorkListLib<T, std::queue<T> > {
    static typename std::queue<T>::reference top(std::queue<T> &cont) {
        return cont.front();
    }
};

/// really stupid, but easy to use, DFS implementation
template <class T, class TSched = std::stack<T> >
class WorkList {
    public:
        typedef T value_type;

    protected:
        TSched        todo_;
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

            dst = WorkListLib<T, TSched>::top(todo_);
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

        unsigned cntSeen() const { return seen_.size(); }
        unsigned cntTodo() const { return todo_.size(); }
};

#endif /* H_GUARD_WORKLIST_H */
