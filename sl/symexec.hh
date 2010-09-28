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

#ifndef H_GUARD_SYM_EXEC_H
#define H_GUARD_SYM_EXEC_H

/**
 * @file symexec.hh
 * SymExec - top level algorithm of the @b symbolic @b execution
 */

class SymState;

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

struct SymExecParams {
    bool fastMode;          ///< enable/disable the @b fast @b mode
    bool skipPlot;          ///< simply ignore all ___sl_plot* calls
    bool ptrace;            ///< enable path tracing (a bit chatty)

    SymExecParams():
        fastMode(false),
        skipPlot(false),
        ptrace(false)
    {
    }
};

/**
 * top level algorithm of the @b symbolic @b execution
 *
 * for now, @b fast @b mode means that OOM analysis is omitted
 */
class SymExec {
    public:
        /**
         * load the static info about the analyzed code
         * @param stor all-in-one static info about the analyzed code, see
         * @param params initialization parameters of the symbolic execution
         */
        SymExec(const CodeStorage::Storage &stor, const SymExecParams &params);
        ~SymExec();

        const CodeStorage::Storage& stor() const;
        const SymExecParams& params() const;

        /**
         * initial state.  By default, there is only one symbolic heap in that
         * state.  SymExec takes care of creation and initialization of
         * global/static variables in it.  You can change the symbolic heap
         * and/or add another one.  But be sure that all the disjuncts contain
         * all global/static variables.
         * @attention The state can't be changed during the symbolic execution.
         * In particular, call of SymExec::exec(fnc, stateZero()) is really bad
         * idea.  You need to use a temporary SymState object in that case.
         */
        SymState& stateZero();

        /**
         * symbolically @b execute a function, starting from stateZero()
         * @param fnc a function requested to be executed
         * @param results a container for results of the symbolic execution
         */
        void exec(const CodeStorage::Fnc &fnc, SymState &results);

    private:
        /// object copying is @b not allowed
        SymExec(const SymExec &);

        /// object copying is @b not allowed
        SymExec& operator=(const SymExec &);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_EXEC_H */
