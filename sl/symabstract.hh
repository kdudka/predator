/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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

#ifndef H_GUARD_SYMABSTRACT_H
#define H_GUARD_SYMABSTRACT_H

/**
 * @file symabstract.hh
 * list segment based abstraction/concretization of heap objects
 */

#include "config.h"
#include "symheap.hh"

#include <list>

/**
 * container for symbolic heaps scheduled for processing.  It's feed by
 * concretizeObj() and consumed by SymExecCore::concretizeLoop().
 */
typedef std::list<SymHeap> TSymHeapList;

/**
 * concretize the given @b abstract object.  If the result is non-deterministic,
 * more than one symbolic heap can be produced.
 * @param srcDst an instance of symbolic heap, used in read/write mode
 * @param atAddr address of the @b abstract heap object that should be
 * concretized
 * @param dst an extra container used to store extra results that are caused by
 * non-deterministic decisions
 * @note the first result is always stored into srcDst, the use of dst is
 * optional and should be eventually handled separately by caller
 */
void concretizeObj(SymHeap &srcDst, TValId atAddr, TSymHeapList &dst);

/**
 * analyze the given symbolic heap and consider abstraction of some shapes that
 * we know ho to rewrite to their more abstract way of existence
 * @param sh an instance of symbolic heap, used in read/write mode
 */
void abstractIfNeeded(SymHeap &sh);

/**
 * attempt to splice out a chain of (possibly empty) empty list segments when
 * the caller has some explicit info that there are in fact no list segments
 * @note The operation may fail under some circumstances, caller should be
 * probably ready for both variants.
 * @note This operation may be used on traversing of a non-deterministic
 * condition during symbolic execution.
 * @param sh an instance of symbolic heap, used in read/write mode
 * @param atAddr address of the list segment that should be eliminated
 * @param pointingTo target point of the segment, it will be equal with atAddr
 * as long as the operation succeeds
 */
bool spliceOutAbstractPath(SymHeap &sh, TValId atAddr, TValId pointingTo);

/// enable/disable debugging of symabstract
void debugSymAbstract(const bool enable);

#endif /* H_GUARD_SYMABSTRACT_H */
