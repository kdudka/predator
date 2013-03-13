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

#ifndef H_GUARD_SHAPE_H
#define H_GUARD_SHAPE_H

#include "symheap.hh"
#include "util.hh"                  // for RETURN_IF_COMPARED

/// describe how the shape looks like
struct ShapeProps {
    EObjKind            kind;
    BindingOff          bOff;
};

inline bool operator<(const ShapeProps &a, const ShapeProps &b)
{
    // compare kind of shape
    RETURN_IF_COMPARED(a, b, kind);

    // the compare offsets
    return a.bOff < b.bOff;
}

inline bool operator==(const ShapeProps &a, const ShapeProps &b)
{
    return a.kind == b.kind
        && a.bOff == b.bOff;
}

inline bool operator!=(const ShapeProps &a, const ShapeProps &b)
{
    return !operator==(a, b);
}

/// inductive definition of a container shape
struct Shape {
    TObjId              entry;
    ShapeProps          props;
    unsigned            length; ///< count of objects (both regions or abstract)
};

inline bool operator<(const Shape &a, const Shape &b)
{
    RETURN_IF_COMPARED(a, b, entry);
    RETURN_IF_COMPARED(a, b, props);
    return a.length < b.length;
}

inline bool operator==(const Shape &a, const Shape &b)
{
    return a.entry == b.entry
        && a.props == b.props
        && a.length == b.length;
}

inline bool operator!=(const Shape &a, const Shape &b)
{
    return !operator==(a, b);
}

/// list of shape properties (kind, binding offsets) candidates
typedef std::vector<ShapeProps>             TShapePropsList;

/// set of shapes given by their inductive definition
typedef std::set<Shape>                     TShapeSet;

/// list of shapes given by their inductive definition
typedef std::vector<Shape>                  TShapeList;

/// list of shapes grouped by heap index they occur in
typedef std::vector<TShapeList>             TShapeListByHeapIdx;

/// return the set of objects that the given shape consists of
void objSetByShape(TObjSet *pDst, const SymHeap &sh, const Shape &shape);

/// return the last object (the opposite of entry) of the given shape
TObjId lastObjOfShape(const SymHeap &sh, const Shape &shape);

#endif /* H_GUARD_SHAPE_H */
