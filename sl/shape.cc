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

#include "shape.hh"

#include "symutil.hh"

void objSetByShape(TObjSet *pDst, const SymHeap &sh, const Shape &shape)
{
    const ShapeProps &props = shape.props;
    const EObjKind kind = props.kind;
    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            break;

        default:
            CL_BREAK_IF("invalid call of objSetByShape()");
    }

    const TOffset offNext = props.bOff.next;
    SymHeap &shWritable = const_cast<SymHeap &>(sh);
    TObjId obj = shape.entry;

    for (unsigned i = 0; i < shape.length; ++i) {
        pDst->insert(obj);
        obj = nextObj(shWritable, obj, offNext);
    }
}

TObjId lastObjOfShape(const SymHeap &sh, const Shape &shape)
{
    const ShapeProps &props = shape.props;
    const EObjKind kind = props.kind;
    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            break;

        default:
            CL_BREAK_IF("invalid call of lastObjOfShape()");
    }

    const TOffset offNext = props.bOff.next;
    SymHeap &shWritable = const_cast<SymHeap &>(sh);

    TObjId obj = shape.entry;
    for (unsigned i = 1U; i < shape.length; ++i)
        obj = nextObj(shWritable, obj, offNext);

    return obj;
}
