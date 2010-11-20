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

#include "symheap.hh"

/// replacement of matchData() from symdiscover
bool joinDataReadOnly(
        const SymHeap           &sh,
        const SegBindingFields  &bf,
        const TObjId            o1,
        const TObjId            o2,
        SymHeap::TContObj       protoRoots[1][2]);

/// replacement of abstractNonMatchingValues() from symabstract
bool joinData(
        SymHeap                 &sh,
        const TObjId            dst,
        const TObjId            src,
        const bool              bidir);

/// @todo some dox
enum EJoinStatus {
    JS_USE_ANY = 0,
    JS_USE_SH1,
    JS_USE_SH2,
    JS_THREE_WAY
};

/// @todo some dox
bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *dst,
        const SymHeap           &sh1,
        const SymHeap           &sh2);

#endif /* H_GUARD_SYM_JOIN_H */
