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

#ifndef H_GUARD_SYMSEG_H
#define H_GUARD_SYMSEG_H

/**
 * @file symseg.hh
 * @todo some dox
 */

#include "config.h"
#include "symid.hh"

class SymHeap;

bool haveDlSeg(SymHeap &sh, TValueId atAddr, TValueId pointingTo);

bool haveDlSegAt(SymHeap &sh, TValueId atAddr, TValueId peerAddr);

TObjId nextPtrFromSeg(const SymHeap &sh, TObjId seg);

TObjId dlSegPeer(const SymHeap &sh, TObjId dls);

bool dlSegNotEmpty(const SymHeap &sh, TObjId dls);

bool segNotEmpty(const SymHeap &sh, TObjId seg);

void segDestroy(SymHeap &sh, TObjId seg);

#endif /* H_GUARD_SYMSEG_H */
