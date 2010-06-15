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

#ifndef H_GUARD_SYM_CALL_H
#define H_GUARD_SYM_CALL_H

/**
 * @file symcall.hh
 * classes SymCallCache and SymCallCtx for function @b call @b optimization
 * during the symbolic execution
 */

#include "symheap.hh"

class SymBackTrace;
class SymHeapUnion;

namespace CodeStorage {
    struct Insn;
}

/**
 * function call context, which represents a cache entry of SymCallCache
 * @note these objects can't be created/destroyed out of SymCallCache
 */
class SymCallCtx {
    public:
        /**
         * check if we need to execute the function call in this context.  If @b
         * true, you need to execute the call, starting with entry(), and insert
         * all results into rawResults().  If @b false, you don't need to
         * execute the function call and you can use the already cached result.
         */
        bool needExec() const;

        /**
         * a pre-computed symbolic heap valid for the entry of the eventual
         * function call.  Do not use this method if needExec() has returned @b
         * false.
         */
        const SymHeap& entry() const;

        /**
         * a place for raw results of a function call, later polished and merged
         * into the target state.  Do not use this method if needExec() has
         * returned @b false.
         */
        SymHeapUnion& rawResults();

        /**
         * merge the (either cached, or just computed) results of the
         * corresponding function call into the target state
         * @param dst target state
         */
        void flushCallResults(SymHeapUnion &dst);

    private:
        /// @note these objects can't be created/destroyed out of SymCallCache
        SymCallCtx();

        /// @note these objects can't be created/destroyed out of SymCallCache
        ~SymCallCtx();

        /// @note these objects can't be created/destroyed out of SymCallCache
        friend class /* SymCallCache helper */ PerFncCache;
        friend class SymCallCache;

        /// object copying is @b not allowed
        SymCallCtx(const SymCallCtx &);

        /// object copying is @b not allowed
        SymCallCtx& operator=(const SymCallCtx &);

    private:
        struct Private;
        Private *d;
};

/// persistent cache for results of fncs called during the symbolic execution
class SymCallCache {
    public:
        /**
         * create long term cache, this should happen once per SymExec lifetime
         * @param bt an instance of symbolic backtrace used to distinguish among
         * instances of automatic variables and to print backtraces when
         * necessary/suitable
         */
        SymCallCache(SymBackTrace *bt);
        ~SymCallCache();

        /**
         * cache entry point.  This returns either existing, or a newly created
         * call context.
         * @param heap a symbolic heap valid for call entry
         * @param insn a call instruction which is about to be executed
         * @return reference to the corresponding SymCallCtx object
         */
        SymCallCtx& getCallCtx(SymHeap heap, const CodeStorage::Insn &insn);

    private:
        /// object copying is @b not allowed
        SymCallCache(const SymCallCache &);

        /// object copying is @b not allowed
        SymCallCache& operator=(const SymCallCache &);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_CALL_H */
