/*
 * Copyright (C) 2012 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_PROTOTYPE_H
#define H_GUARD_PROTOTYPE_H

#include "config.h"
#include "symheap.hh"

/// collect prototypes of the given object, return true on success
bool collectPrototypesOf(
        TObjSet                    &dst,
        SymHeap                    &sh,
        const TObjId                obj);

/// increment prototype level of a single object while taking care of DLS peers
void objIncrementProtoLevel(SymHeap &sh, TObjId);

/// decrement prototype level of a single object while taking care of DLS peers
void objDecrementProtoLevel(SymHeap &sh, TObjId);

/// decrement prototype level of owned prototype objects
void decrementProtoLevel(SymHeap &sh, const TObjId obj);

/**
 * returns true if no concrete object points to another object of a higher level
 * @note this runs in debug build only
 */
bool protoCheckConsistency(const SymHeap &sh);

#endif /* H_GUARD_PROTOTYPE_H */
