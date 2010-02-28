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

/**
 * @file symstate.hh
 * SymHeapUnion - @b symbolic @b state represented as a union of SymHeap objects
 */

#include <vector>

#include "symheap.hh"

/**
 * symbolic state represented as a union of SymHeap objects (aka disjuncts)
 *
 * During the symbolic execution (see SymExec) we keep such a state per each
 * basic block of the function just being processed.  The result of symbolically
 * executed function is then the SymHeapUnion taken from the basic block
 * containing CL_INSN_RET as soon as the fix-point calculation has terminated.
 */
class SymHeapUnion {
    private:
        typedef std::vector<SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef TList::iterator iterator;

    public:
        /// insert given SymHeap object into the union
        void insert(const SymHeap &heap);

        /// merge given SymHeapUnion object into self
        void insert(const SymHeapUnion &huni);

        /// return count of object stored in the container
        size_t size()          const { return heaps_.size();  }

        /// return STL-like iterator to go through the container
        const_iterator begin() const { return heaps_.begin(); }

        /// return STL-like iterator to go through the container
        const_iterator end()   const { return heaps_.end();   }

        /// @copydoc begin() const
        iterator begin()             { return heaps_.begin(); }

        /// @copydoc begin() const
        iterator end()               { return heaps_.end();   }

    private:
        TList heaps_;
};


#endif /* H_GUARD_SYM_STATE_H */
