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

#ifndef H_GUARD_CONT_SHAPE_H
#define H_GUARD_CONT_SHAPE_H

#include "symheap.hh"

class SymState;

namespace ContShape {

/// describe how the shape looks like
struct Props {
    EObjKind            kind;
    BindingOff          bOff;
};

/// inductive definition of a container shape
struct Shape {
    TObjId              entry;
    Props               props;
};

/// list of container shapes given by their inductive definition
typedef std::vector<Shape>                  TShapeList;

/// list of container shapes grouped by heap index they occur in
typedef std::vector<TShapeList>             TShapeListByHeapIdx;

void detectContShapes(TShapeListByHeapIdx *pDst, const SymState &);

} // namespace ContShape

#endif /* H_GUARD_CONT_SHAPE_H */
