/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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

#include "config_cl.h"
#include "builtins.hh"

#include <cl/clutil.hh>
#include <cl/storage.hh>

#include "util.hh"

namespace CodeStorage {

bool isBuiltInFnc(const struct cl_operand &op)
{
    const char *name;
    if (!fncNameFromCst(&name, &op))
        // likely indirect fuction call
        return false;

    // list of builtins
    return STREQ("PT_ASSERT", name)
        || STREQ("VK_ASSERT", name);
}

bool isBuiltInCall(const Insn &insn)
{
    if (insn.code != CL_INSN_CALL)
        return false;

    return isBuiltInFnc(insn.operands[/* fnc */ 1]);
}

} // namespace CodeStorage
