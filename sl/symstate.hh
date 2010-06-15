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
 * SymHeapUnion - @b symbolic @b state represented as a union of SymHeap objects
 */

#include <vector>

#include "symheap.hh"

/**
 * symbolic state represented as a union of SymHeap objects (aka disjuncts)
 *
 * During the symbolic execution (see SymExec) we keep such a state per each
 * basic block of the function just being processed.  The result of a
 * symbolically executed function is then the SymHeapUnion taken from the basic
 * block containing CL_INSN_RET as soon as the fix-point calculation has
 * terminated.
 */
class SymHeapUnion {
    private:
        typedef std::vector<SymHeap> TList;

    public:
        typedef TList::const_iterator const_iterator;
        typedef TList::iterator iterator;

    public:
        SymHeapUnion();
        virtual ~SymHeapUnion();

        SymHeapUnion(const SymHeapUnion &);
        SymHeapUnion& operator=(const SymHeapUnion &);

        /**
         * look for the given symbolic heap, return its index if found, -1
         * otherwise
         */
        int lookup(const SymHeap &heap) const;

        /// insert given SymHeap object into the union
        virtual void insert(const SymHeap &heap);

        /// merge given SymHeapUnion object into self
        virtual void insert(const SymHeapUnion &huni);

        /// return count of object stored in the container
        size_t size()          const { return heaps_.size();  }

        /// return nth SymHeap object, 0 <= nth < size()
        const SymHeap& operator[](int nth) const {
            return heaps_[nth];
        }

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

    private:
        struct Private;
        Private *d;
};

/**
 * Extension of SymHeapUnion, which distinguishes among already processed
 * symbolic heaps and symbolic heaps scheduled for processing.  Newly inserted
 * symbolic heaps are always marked as scheduled.  They can be marked as done
 * later, using the setDone() method.
 */
class SymHeapScheduler: public SymHeapUnion {
    public:
        /// import of SymHeapUnion rewrites the base and invalidates all flags
        SymHeapScheduler& operator=(const SymHeapUnion &huni) {
            static_cast<SymHeapUnion &>(*this) = huni;
            done_.clear();
            done_.resize(huni.size(), false);
            return *this;
        }

        virtual void insert(const SymHeap &heap) {
            const size_t last = this->size();
            SymHeapUnion::insert(heap);
            if (this->size() == last)
                // nothing has been changed
                return;

            // schedule the just inserted SymHeap for processing
            done_.push_back(false);
        }

    public:
        /// check if the nth symbolic heap has been already processed
        bool isDone(int nth) const {
            return done_[nth];
        }

        /// mark the nth symbolic heap as processed
        void setDone(int nth) {
            done_[nth] = true;
        }

    private:
        std::vector<bool> done_;
};


#endif /* H_GUARD_SYM_STATE_H */
