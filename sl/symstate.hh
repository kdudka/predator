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

#ifndef H_GUARD_SYM_STATE_H
#define H_GUARD_SYM_STATE_H

#include <vector>

#include "symheap.hh"

class SymHeapUnion {
    private:
        typedef std::vector<SymbolicHeap::SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef TList::iterator iterator;

    public:
        void insert(const SymbolicHeap::SymHeap &heap);
        void insert(const SymHeapUnion &huni);

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator begin() const { return heaps_.begin(); }

        /**
         * return STL-like iterator to go through the container
         */
        const_iterator end()   const { return heaps_.end();   }

        /**
         * return count of object stored in the container
         */
        size_t size()          const { return heaps_.size();  }

        iterator begin()             { return heaps_.begin(); }
        iterator end()               { return heaps_.end();   }

    private:
        TList heaps_;
};


#endif /* H_GUARD_SYM_STATE_H */
