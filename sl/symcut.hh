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

#ifndef H_GUARD_SYM_CUT_H
#define H_GUARD_SYM_CUT_H

/**
 * @file symcut.hh
 * split/join algorithms operating on symbolic heap - splitHeapByCVars() and
 * joinHeapsByCVars()
 */

#include "symheap.hh"

class SymBackTrace;

/**
 * split symbolic heap into two parts regarding the list of program variables
 * @note In the corner case, the result may be identical to the input.  Then
 * the heap pointed by saveSurroundTo will be empty.
 * @param srcDst the instance of heap to operate on, it has to contain all
 * program variables that are specified by cut.
 * @param cut list of program variables to cut the heap by
 * @param saveSurroundTo if not null, it must point to a fresh instance of
 * SymHeap; it will be used to stored the (possibly empty) part of heap that is
 * cut off
 */
void splitHeapByCVars(
        SymHeap                     *srcDst,
        const TCVarList             &cut,
        SymHeap                     *saveSurroundTo = 0);

/**
 * split two disjunct symbolic heaps together, going from program variables
 * @param srcDst the instance of heap to operate on
 * @param src2 the other instance of heap, which is used read-only
 */
void joinHeapsByCVars(
        SymHeap                     *srcDst,
        const SymHeap               *src2);

#endif /* H_GUARD_SYM_CUT_H */
