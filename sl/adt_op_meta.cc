/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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

#include "config.h"
#include "adt_op_meta.hh"

#include "fixed_point.hh"

using FixedPoint::TObjectMapper;

namespace AdtOp {

template <EDirection dir>
bool projectMetaOperation(MetaOperation &mo, TObjectMapper idMap)
{
    // TODO
    return false;
}

bool diffHeaps(TMetaOpSet *pDst, const SymHeap &sh1, const SymHeap &sh2)
{
    // TODO
    (void) pDst;
    (void) sh1;
    (void) sh2;
    return false;
}

} // namespace AdtOp
