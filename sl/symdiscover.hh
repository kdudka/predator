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

#ifndef H_GUARD_SYMDISCOVER_H
#define H_GUARD_SYMDISCOVER_H

/**
 * @file symdiscover.hh
 * public interface of list segment discovering algorithms
 */

#include "config.h"
#include "shape.hh"

/// probe neighbouring objects and return a list of shape properties candidates
void digShapePropsCandidates(
        TShapePropsList            *pDst,
        SymHeap                    &sh,
        const TObjId                obj);

/// true if we can merge a pair of subsequent objects according to shape props
bool canMergeObjWithNextObj(
        SymHeap                    &sh,
        const TObjId                obj,
        const ShapeProps           &props,
        TObjId                     *pNextObj = 0);

/**
 * Take the given symbolic heap and look for the best possible abstraction in
 * there.  If nothing is found, zero is returned.  Otherwise it returns total
 * length of the best possible abstraction.
 */
bool discoverBestAbstraction(Shape *pDst, SymHeap &sh);

#endif /* H_GUARD_SYMDISCOVER_H */
