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

#ifndef H_GUARD_SYM_BIN_H
#define H_GUARD_SYM_BIN_H

/**
 * @file symbin.hh
 * implementation of built-in functions
 */

#include <vector>

class SymExecCore;
class SymState;

namespace CodeStorage {
    struct Insn;
}

/// list of indexes of operands in an instruction
typedef std::vector<unsigned /* idx */>         TOpIdxList;

/// list of operands which have dereference semantics for a detected built-in
const TOpIdxList& opsWithDerefSemanticsInCallInsn(const CodeStorage::Insn &);

/**
 * analyze the given @b call instruction and handle any recognized built-in
 * function eventually.  There is nothing done if no built-in is recognized
 * that case is signalized by the return value.
 * @param dst where to store results of the execution of the built-in
 * @param core an instance of SymExecCore used in read/write mode
 * @param insn an instruction that should be analyzed and eventually executed
 * @return return true if a built-in has been recognized; false otherwise
 */
bool handleBuiltIn(SymState                     &dst,
                   SymExecCore                  &core,
                   const CodeStorage::Insn      &insn);

#endif /* H_GUARD_SYM_BIN_H */
