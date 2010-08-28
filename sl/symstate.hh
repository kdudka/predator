/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_STATE_H
#define H_GUARD_SYM_STATE_H

/**
 * @file symstate.hh
 * SymState - @b symbolic @b state represented as a union of SymHeap objects
 */

#include <vector>

#include "symheap.hh"

namespace CodeStorage {
    class Block;
}

/**
 * symbolic state represented as a union of SymHeap objects (aka disjuncts)
 *
 * During the symbolic execution (see SymExec) we keep such a state per each
 * basic block of the function just being processed.  The result of a
 * symbolically executed function is then the SymState taken from the basic
 * block containing CL_INSN_RET as soon as the fix-point calculation has
 * terminated.
 */
class SymState {
    private:
        typedef std::vector<SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef TList::iterator iterator;

    public:
        virtual ~SymState() { }

        virtual void clear() {
            heaps_.clear();
        }

        /**
         * look for the given symbolic heap, return its index if found, -1
         * otherwise
         */
        int lookup(const SymHeap &heap) const;

        /// insert given SymHeap object into the union
        void insert(const SymHeap &heap);

        /// merge given SymState object into self
        void insert(const SymState &huni);

        /// return count of object stored in the container
        size_t size()          const { return heaps_.size();  }

        /// return nth SymHeap object, 0 <= nth < size()
        const SymHeap& operator[](int nth) const {
            return heaps_.at(nth);
        }

        /// return STL-like iterator to go through the container
        const_iterator begin() const { return heaps_.begin(); }

        /// return STL-like iterator to go through the container
        const_iterator end()   const { return heaps_.end();   }

        /// @copydoc begin() const
        iterator begin()             { return heaps_.begin(); }

        /// @copydoc begin() const
        iterator end()               { return heaps_.end();   }

    protected:
        /// insert @b new SymHeap that @ must be guaranteed to be not in yet
        virtual void insertNew(const SymHeap &sh) {
            heaps_.push_back(sh);
        }

        /// lookup/insert optimization in SymCallCache implementation
        friend class PerFncCache;

    private:
        TList heaps_;
};

/**
 * Extension of SymState, which distinguishes among already processed
 * symbolic heaps and symbolic heaps scheduled for processing.  Newly inserted
 * symbolic heaps are always marked as scheduled.  They can be marked as done
 * later, using the setDone() method.
 */
class SymStateMarked: public SymState {
    public:
        /// import of SymState rewrites the base and invalidates all flags
        SymStateMarked& operator=(const SymState &huni) {
            static_cast<SymState &>(*this) = huni;
            done_.clear();
            done_.resize(huni.size(), false);
            return *this;
        }

        virtual void clear() {
            SymState::clear();
            done_.clear();
        }

    protected:
        virtual void insertNew(const SymHeap &sh) {
            SymState::insertNew(sh);

            // schedule the just inserted SymHeap for processing
            done_.push_back(false);
        }

    public:
        /// check if the nth symbolic heap has been already processed
        bool isDone(int nth) const {
            return done_.at(nth);
        }

        /// mark the nth symbolic heap as processed
        void setDone(int nth) {
            done_.at(nth) = true;
        }

    private:
        std::vector<bool> done_;
};

/**
 * higher-level container that maintains a SymStateMarked object per each basic
 * block.  It's among other used by SymExecEngine and PathTracer classes.
 */
class SymStateMap {
    public:
        typedef std::vector<const CodeStorage::Block *>     TContBlock;

        SymStateMap();
        ~SymStateMap();

        /// state lookup, basically equal to std::map semantic
        SymStateMarked& operator[](const CodeStorage::Block *);

        /**
         * managed insertion of the state that keeps track of the relation among
         * source and destination basic blocks
         * @param dst @b destination basic block (where the insertion occurs)
         * @param src @b source basic block (where the state has grown), may be
         * zero when inserting an initial state to the entry block
         * @param sh an instance of symbolic heap that should be inserted
         */
        bool insert(const CodeStorage::Block                *dst,
                    const CodeStorage::Block                *src,
                    const SymHeap                           &sh);

        /**
         * returns all blocks that inserted something to the given state
         * @param dst a container where the result should be stored to
         * @param ofBlock the basic block we doing the query for
         * @note This is an @b over @b approximation as it operates on the level
         * of abstract states (sets of symbolic heaps).  It does not consider
         * paths along individual symbolic heaps themselves.
         */
        void gatherInboundEdges(TContBlock                  &dst,
                                const CodeStorage::Block    *ofBlock)
            const;

    private:
        /// object copying is @b not allowed
        SymStateMap(const SymStateMap &);

        /// object copying is @b not allowed
        SymStateMap& operator=(const SymStateMap &);

    private:
        struct Private;
        Private *d;
};


#endif /* H_GUARD_SYM_STATE_H */
