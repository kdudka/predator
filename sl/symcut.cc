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

#include "config.h"
#include "symcut.hh"

#ifndef SE_DISABLE_SYMCUT
#   define SE_DISABLE_SYMCUT 0
#endif

void splitHeapByCVars(SymHeap *srcDst, const SymHeap::TContCVar &cut,
                      SymHeap *saveSurroundTo)
{
#if SE_DISABLE_SYMCUT
    return;
#endif

    // TODO
    (void) srcDst;
    (void) cut;
    (void) saveSurroundTo;
    TRAP;
}

void joinHeapsByCVars(SymHeap *srcDst, const SymHeap *src2)
{
#if SE_DISABLE_SYMCUT
    return;
#endif

    // TODO
    (void) srcDst;
    (void) src2;
    TRAP;
}
