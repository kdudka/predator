/*
 * Copyright (C) 2014 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CONT_SHAPE_VAR_H
#define H_GUARD_CONT_SHAPE_VAR_H

#include "adt_op_match.hh"
#include "fixed_point.hh"

namespace AdtOp {

typedef int                                         TShapeVarId;

extern const TShapeVarId InvalidShapeVar;

/// return a (globally) unique shape var ID
TShapeVarId acquireFreshShapeVar(void);

typedef FixedPoint::TShapeIdent                     TShapeIdent;
typedef std::map<TShapeIdent, TShapeVarId>          TShapeVarByShape;

bool assignShapeVariables(
        TShapeVarByShape           *pDst,
        const TMatchList           &matchList,
        const TOpList              &opList,
        const OpCollection         &coll,
        const TProgState           &progState);

} // namespace AdtOp

#endif /* H_GUARD_CONT_SHAPE_VAR_H */
