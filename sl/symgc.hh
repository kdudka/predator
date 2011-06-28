/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYMGC_H
#define H_GUARD_SYMGC_H

/**
 * @file symgc.hh
 * collectJunk - implementation of a garbage collector for the symbolic heap
 */

#include "symid.hh"

class SymHeap;

/**
 * check if a sub-heap reachable from the given value is also reachable from
 * somewhere else.  If not, such a sub-heap is considered as garbage and
 * removed.
 * @param sh instance of the symbolic heap to search in
 * @param val ID of the heap value to check for junk
 * @return true if any junk has been detected/collected
 */
bool collectJunk(SymHeap &sh, TValId val); 

bool destroyRootAndCollectJunk(SymHeap &sh, const TValId root);

#endif /* H_GUARD_SYMGC_H */
