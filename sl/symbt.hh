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

#ifndef H_GUARD_SYMBT_H
#define H_GUARD_SYMBT_H

#include <cl/code_listener.h>

/**
 * @file symbt.hh
 * SymBackTrace - backtrace management
 */

class SymProc;

enum EMsgLevel {
    ML_DEBUG,
    ML_WARN,
    ML_ERROR
};

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

/// backtrace management
class SymBackTrace {
    public:
        /**
         * @param stor reference to storage object, used for resolving fnc IDs
         */
        SymBackTrace(const CodeStorage::Storage &stor);
        ~SymBackTrace();

        SymBackTrace(const SymBackTrace &);

        /**
         * @todo consider fitness of this method in the public interface of
         * SymBackTrace
         */
        const CodeStorage::Storage& stor() const;

    public:
        /**
         * enter a call of function, thus enlarge the backtrace by one
         * @param fncId ID of function, which is being called
         * @param loc location of the call related to the caller (definitely @b
         * not location of the called function)
         */
        void pushCall(int fncId, const struct cl_loc *loc);

        /**
         * leave the call of function on top of the backtrace
         * @note it's safe to ignore the return value
         */
        const CodeStorage::Fnc* popCall();

        /// size of the backtrace, aka @b call @b depth
        unsigned size() const;

        /**
         * count occurrences of the given function.  Zero means the function
         * does not occur in the backtrace.  Non-zero means the function occurs
         * in the backtrace.  Moreover, if the value is more than one, the
         * function has been called @b recursively.
         * @note The function is not implemented or used right now, but is going
         * to be as soon as we allow recursion.  We need the returned value to
         * distinguish among different instances of the same local variable.
         */
        int countOccurrencesOfFnc(cl_uid_t fncId) const;

        /**
         * same as countOccurrencesOfFnc(), but operating on top of the
         * backtrace stack
         */
        int countOccurrencesOfTopFnc() const;

        /// return the topmost function in the backtrace
        const CodeStorage::Fnc* topFnc() const;

        /// return location of call of the topmost function in the backtrace
        const struct cl_loc* topCallLoc() const;

    protected:
        /**
         * stream out the backtrace, using CL_NOTE_MSG; or do nothing if the
         * backtrace is trivial
         * @return true if the backtrace is @b not trivial
         */
        bool printBackTrace() const;
        friend class SymProc;
        friend class SymExecEngine;
        friend bool areEqual(const SymBackTrace *, const SymBackTrace *);

    private:
        SymBackTrace& operator=(const SymBackTrace &);

        struct Private;
        Private *d;
};

/// true if the given back traces are equal (or both the pointers are NULL)
bool areEqual(const SymBackTrace *, const SymBackTrace *);

#endif /* H_GUARD_SYMBT_H */
