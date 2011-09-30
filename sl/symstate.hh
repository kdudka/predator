/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
 * @todo update dox
 */

#include <set>
#include <vector>

#include "symheap.hh"

namespace CodeStorage {
    class Block;
}

class SymState {
    private:
        typedef std::vector<SymHeap> TList;

    public:
        typedef TList::const_iterator           const_iterator;
        typedef TList::iterator                 iterator;

    public:
        virtual ~SymState() { }

        virtual void clear() {
            heaps_.clear();
        }

        virtual void swap(SymState &other) {
            heaps_.swap(other.heaps_);
        }

        /**
         * look for the given symbolic heap, return its index if found, -1
         * otherwise
         */
        virtual int lookup(const SymHeap &heap) const = 0;

        /// insert given SymHeap object into the state
        virtual bool insert(const SymHeap &heap);

        /// insert a heap and replace the given one by some garbage eventually
        virtual bool insertFast(SymHeap &heap);

        /// merge the content of the given SymState object into the state
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
        /// insert @b new SymHeap that @ must be guaranteed to be not yet in
        virtual void insertNew(const SymHeap &sh) {
            heaps_.push_back(sh);
        }

        /// aggressive optimization
        virtual void insertNewFast(SymHeap &sh) {
            const unsigned last = heaps_.size();
            heaps_.push_back(SymHeap(sh.stor()));
            heaps_[last].swap(sh);
        }

        virtual void eraseExisting(int nth) {
            heaps_.erase(heaps_.begin() + nth);
        }

        virtual void swapExisting(int nth, SymHeap &sh) {
            SymHeap &existing = heaps_.at(nth);
            existing.swap(sh);
        }

        /// lookup/insert optimization in SymCallCache implementation
        friend class PerFncCache;

    private:
        TList heaps_;
};

class SymHeapList: public SymState {
    public:
        SymHeapList() { }

        SymHeapList(const SymState &ref):
            // safe to slice the base, as we have no data anyway
            SymState(ref)
        {
        }

        SymHeapList& operator=(const SymState &ref) {
            // safe to slice the base, as we have no data anyway
            *static_cast<SymState *>(this) = ref;

            return *this;
        }

        virtual int lookup(const SymHeap &) const {
            return /* not found */ -1;
        }
};

/**
 * symbolic state represented as a union of SymHeap objects (aka disjuncts)
 *
 * During the symbolic execution (see SymExec) we keep such a state per each
 * basic block of the function just being processed.  The result of a
 * symbolically executed function is then the SymState taken from the basic
 * block containing CL_INSN_RET as soon as the fix-point calculation has
 * terminated.
 */
class SymHeapUnion: public SymState {
    public:
        virtual int lookup(const SymHeap &sh) const;
};

class SymStateWithJoin: public SymHeapUnion {
    public:
        virtual bool insert(const SymHeap &sh);
        virtual bool insertFast(SymHeap &sh);

    private:
        void packSuffix(unsigned idx);
        bool insertCore(SymHeap &sh, const bool feelFreeToOverwrite);
};

/**
 * Extension of SymStateWithJoin, which distinguishes among already processed
 * symbolic heaps and symbolic heaps scheduled for processing.  Newly inserted
 * symbolic heaps are always marked as scheduled.  They can be marked as done
 * later, using the setDone() method.
 */
class SymStateMarked: public SymStateWithJoin {
    public:
        /// import of SymState rewrites the base and invalidates all flags
        SymStateMarked& operator=(const SymState &huni) {
            static_cast<SymState &>(*this) = huni;
            done_.clear();
            done_.resize(huni.size(), false);
            return *this;
        }

        virtual void clear() {
            SymStateWithJoin::clear();
            done_.clear();
        }

        virtual void swap(SymState &otherBase) {
            SymStateMarked &other = dynamic_cast<SymStateMarked &>(otherBase);
            SymStateWithJoin::swap(other);
            done_.swap(other.done_);
        }

    protected:
        virtual void insertNew(const SymHeap &sh) {
            SymStateWithJoin::insertNew(sh);

            // schedule the just inserted SymHeap for processing
            done_.push_back(false);
        }

        virtual void insertNewFast(SymHeap &sh) {
            SymStateWithJoin::insertNewFast(sh);

            // schedule the just inserted SymHeap for processing
            done_.push_back(false);
        }

        virtual void eraseExisting(int nth) {
            SymStateWithJoin::eraseExisting(nth);
            done_.erase(done_.begin() + nth);
        }

        virtual void swapExisting(int nth, SymHeap &sh) {
            SymStateWithJoin::swapExisting(nth, sh);

            // an already inserted heap has been generalized, we need to
            // schedule it once again
            done_.at(nth) = false;
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
 * block.  It's used by SymExecEngine and PathTracer classes.
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

        /// aggressive optimization
        bool insertFast(
                    const CodeStorage::Block                *dst,
                    const CodeStorage::Block                *src,
                    SymHeap                                 &sh);

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

class IStatsProvider {
    public:
        virtual ~IStatsProvider() { }
        virtual void printStats() const = 0;
};

class BlockScheduler: public IStatsProvider {
    public:
        typedef const CodeStorage::Block       *TBlock;
        typedef std::set<TBlock>                TBlockSet;
        typedef std::vector<TBlock>             TBlockList;

    public:
        BlockScheduler();
        BlockScheduler(const BlockScheduler &);
        ~BlockScheduler();

        const TBlockSet& todo() const;

        TBlockList done() const;

        unsigned cntWaiting() const;

        bool schedule(const TBlock bb);

        bool getNext(TBlock *dst);

        virtual void printStats() const;

    private:
        // not implemented
        BlockScheduler& operator=(const BlockScheduler &);

        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_STATE_H */
