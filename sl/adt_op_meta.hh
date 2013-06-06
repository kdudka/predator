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

#ifndef H_GUARD_ADT_OP_META_H
#define H_GUARD_ADT_OP_META_H

#include "fixed_point.hh"
#include "util.hh"                          // for RETURN_IF_COMPARED

namespace AdtOp {

//// enumeration of supported meta-operations (only pointer operations for now)
enum EMetaOperation {
    MO_INVALID = 0,
    MO_ALLOC,                               /// a new object is introduced
    MO_FREE,                                /// an existing object disappears
    MO_SET,                                 /// a value is written to a field
    MO_UNSET                                /// a tracked field is invalidated
};

//// description of a meta-operation (only pointer operations for now)
struct MetaOperation {
    EMetaOperation      code;               /// kind of meta-operation
    TObjId              obj;                /// object ID we operate on
    TOffset             off;                /// offset we write at (set/unset)
    TObjId              tgtObj;             /// object we take address of (set)
    TValId              tgtOff;             /// tgt offset of the address (set)
    ETargetSpecifier    tgtTs;              /// tgt specifier of the addr (set)
};

/// required to be defined by the implementation of std::set
inline bool operator<(const MetaOperation &a, const MetaOperation &b)
{
    // compare lexicographically
    RETURN_IF_COMPARED(a, b, code);
    RETURN_IF_COMPARED(a, b, obj);
    RETURN_IF_COMPARED(a, b, off);
    RETURN_IF_COMPARED(a, b, tgtObj);
    RETURN_IF_COMPARED(a, b, tgtOff);
    RETURN_IF_COMPARED(a, b, tgtTs);

    // the items are incomparable
    return false;
}

/// set of meta-operations (which should be independent on each other)
typedef std::set<MetaOperation>                     TMetaOpSet;

bool diffHeaps(TMetaOpSet *pDst, const SymHeap &sh1, const SymHeap &sh2);

} // namespace AdtOp

#endif /* H_GUARD_ADT_OP_META_H */
