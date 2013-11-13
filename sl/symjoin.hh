/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_JOIN_H
#define H_GUARD_SYM_JOIN_H

/**
 * @file symjoin.hh
 * @todo some dox
 */

#include "join_status.hh"
#include "symheap.hh"
#include "symtrace.hh"              // for Trace::TIdMapper

struct ShapeProps;

/// @todo some dox
bool joinData(
        SymHeap                 &sh,
        const ShapeProps        &props,
        const TObjId             obj1,
        const TObjId             obj2,
        TObjId                  *pDst            = 0,
        TObjSet                  protoObjs[1][2] = 0,
        EJoinStatus             *pStatus         = 0,
        Trace::TIdMapper        *pIdMapper       = 0);

/// @todo some dox
bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *dst,
        SymHeap                  sh1,
        SymHeap                  sh2,
        const bool               allowThreeWay = true);

/// enable/disable debugging of symjoin
void debugSymJoin(const bool enable);

#endif /* H_GUARD_SYM_JOIN_H */
