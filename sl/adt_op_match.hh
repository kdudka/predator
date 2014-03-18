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

#ifndef H_GUARD_ADT_OP_MATCH_H
#define H_GUARD_ADT_OP_MATCH_H

#include "adt_op.hh"
#include "fixed_point.hh"

#include <list>

namespace AdtOp {

typedef FixedPoint::StateRewriter                   TInsnWriter;

enum EFootprintPort {
    FP_SRC,
    FP_DST,
    FP_TOTAL
};

typedef std::list<FixedPoint::THeapIdent>           THeapIdentSeq;

struct FootprintMatch {
    TFootprintIdent                 footprint;
    ShapeProps                      props;
    ShapeProps                      tplProps;

    /// list of locations (instructions) matched by the template + dst location
    THeapIdentSeq                   matchedHeaps;

    /// list of instructions included as not implementing the operation
    THeapIdentSeq                   skippedHeaps;

    FixedPoint::TObjectMapper       objMap          [FP_TOTAL];

    FootprintMatch(const TFootprintIdent footprint_):
        footprint(footprint_)
    {
    }
};

typedef std::vector<FootprintMatch>                 TMatchList;
typedef FixedPoint::GlobalState                     TProgState;

void matchTemplates(
        TMatchList                 *pDst,
        const OpCollection         &coll,
        const TProgState           &progState);

typedef std::vector<FixedPoint::TLocIdx>            TInsnList;
typedef std::vector<TInsnList>                      TInsnListByTplIdx;

void collectReplacedInsns(
        TInsnListByTplIdx          *pDst,
        const TMatchList           &matchList,
        const TProgState           &progState);

void selectApplicableMatches(
        TMatchList                 *pMatchList,
        const TProgState           &progState);

TObjId selectMappedObjByTs(
        const SymHeap              &sh,
        const BindingOff           &bOff,
        const TObjList             &objList,
        ETargetSpecifier            ts);

typedef int                                         TOpIdx;
typedef int                                         TMatchIdx;
typedef std::vector<TMatchIdx>                      TMatchIdxList;
typedef std::vector<TMatchIdxList>                  TOpList;

bool collectOpList(
        TOpList                    *pDst,
        const TMatchList           &matchList);

} // namespace AdtOp

#endif /* H_GUARD_ADT_OP_MATCH_H */
