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

#ifndef H_GUARD_CLDEBUG_H
#define H_GUARD_CLDEBUG_H

#include <iostream>

struct cl_operand;

namespace CodeStorage {
    struct Insn;
}

void operandToStream(std::ostream &str, const struct cl_operand &op);
void insnToStream(std::ostream &str, const CodeStorage::Insn &insn);

inline std::ostream& operator<<(std::ostream &str, const struct cl_operand &op)
{
    operandToStream(str, op);
    return str;
}

inline std::ostream& operator<<(std::ostream &str, const CodeStorage::Insn &in)
{
    insnToStream(str, in);
    return str;
}

#endif /* H_GUARD_CLDEBUG_H */
