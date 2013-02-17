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

/// describe how the shape looks like
struct ShapeProps {
    EObjKind            kind;
    BindingOff          bOff;
};

/// inductive definition of a container shape
struct Shape {
    TObjId              entry;
    ShapeProps          props;
    unsigned            length; ///< count of objects (both regions or abstract)
};

/// list of shape properties (kind, binding offsets) candidates
typedef std::vector<ShapeProps>             TShapePropsList;

/// list of shapes given by their inductive definition
typedef std::vector<Shape>                  TShapeList;

#endif /* H_GUARD_SHAPE_H */
