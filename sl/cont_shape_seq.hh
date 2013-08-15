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

#ifndef H_GUARD_CONT_SHAPE_SEQ_H
#define H_GUARD_CONT_SHAPE_SEQ_H

#include "fixed_point.hh"

namespace FixedPoint {

struct ShapeSeq {
    TShapeIdent             beg;            /// first shape in the sequence
    TShapeIdent             end;            /// last shape in the sequence
};

typedef std::vector<ShapeSeq>                       TShapeSeqList;
typedef std::vector<TShapeIdent>                    TShapeIdentList;

void collectShapeSequences(TShapeSeqList *pDst, const GlobalState &glState);

/// expand all shapes in the closed interval <beg,end>
void expandShapeSequence(
        TShapeIdentList            *pDst,
        const ShapeSeq             &seq,
        const GlobalState          &glState);

} // namespace FixedPoint

#endif /* H_GUARD_CONT_SHAPE_SEQ_H */
