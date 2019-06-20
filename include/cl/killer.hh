/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_KILLER_H
#define H_GUARD_KILLER_H

/**
 * @file killer.hh
 * @todo some dox
 */

#include "code_listener.h"

#include <map>
#include <set>

namespace CodeStorage {
    struct Insn;
    struct Storage;

    void killLocalVariables(Storage &stor);

    namespace VarKiller {
        typedef cl_uid_t                            TVar;
        typedef std::set<TVar>                      TSet;
        typedef std::map<TVar, TVar>                TAliasMap;


        /// per-block data
        struct BlockData {
            TSet                                    gen;
            TSet                                    kill;
        };

        void scanInsn(
                BlockData          *pDst,
                const Insn         *insn,
                TAliasMap          *pAliasMap = 0);

    } // namespace VarKiller

} // namespace CodeStorage

#endif /* H_GUARD_KILLER_H */
