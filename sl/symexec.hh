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

#ifndef H_GUARD_SYM_EXEC_H
#define H_GUARD_SYM_EXEC_H

/**
 * @file symexec.hh
 * SymExec - top level loop of the @b symbolic @b execution
 */

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

/**
 * top level loop of the @b symbolic @b execution
 * @todo design some interface to interact with the surrounding environment,
 * something to set/get state of the global variables, and something set/get
 * function arguments and the return value
 */
class SymExec {
    public:
        /**
         * load the static info about the analyzed code
         * @param stor all-in-one static info about the analyzed code, see
         * CodeStorage for details
         */
        SymExec(CodeStorage::Storage &stor);
        ~SymExec();

        /// return true if the @b fast @b mode is enabled
        bool fastMode() const;

        /// enable/disable the @b fast @b mode
        void setFastMode(bool);

        // TODO: some functions operating on d->stateZero??
        // TODO: we should be able to define return variable somehow

        /**
         * symbolically @b execute a function
         * @param fnc a function requested to be executed
         */
        void exec(const CodeStorage::Fnc &fnc /*, TODO: results? */);

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
