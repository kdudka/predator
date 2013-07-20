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
#include "adt_op_match.hh"

#include "cont_shape_seq.hh"

namespace AdtOp {

void matchFootprints(
        TMatchList                 *pDst,
        const OpCollection         &coll,
        const TProgState           &progState)
{
    FixedPoint::TShapeSeqList ssList;
    FixedPoint::collectShapeSequences(&ssList, progState);

    // TODO
    (void) pDst;
    (void) coll;
    (void) progState;
    CL_BREAK_IF("please implement");
}

} // namespace AdtOp
