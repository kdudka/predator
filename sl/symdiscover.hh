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
#include "symutil.hh"

#ifndef BUILDING_DOX
// exported only for debugging purposes
bool segMatchSmallList(
        const SymHeap           &sh,
        const TObjId            segUp,
        const TObjId            conUp,
        const TValueId          segVal,
        const TValueId          conVal);
#endif

typedef SymHeap::TContObj TProtoRoots[2];

// experimental, will be documented later eventually
bool considerGenericPrototype(
        const SymHeap           &sh,
        const TObjPair          &roots,
        const TValueId          v1,
        const TValueId          v2,
        TProtoRoots             *protoRoots);

/**
 * Take the given symbolic heap and look for the best possible abstraction in
 * there.  If nothing is found, zero is returned.  Otherwise it returns total
 * length of the best possible abstraction.  In that case, *bf is set to the
 * corresponding binding fields (head, next, peer) and *entry is set to the
 * corresponding starting point of the abstraction.
 *
 * In case of success (non-zero return value), you can determine the type of the
 * detected abstraction by *bf.  If bf->peer is empty, it means a SLS
 * abstraction;  DLS otherwise.  If bf->head is empty, it means a regular list
 * segment abstraction;  Linux list segment otherwise.
 *
 * In case of failure (zero return value), *bf and *entry are not touched at
 * all.
 */
unsigned /* len */ discoverBestAbstraction(
        const SymHeap           &sh,
        SegBindingFields        *bf,
        TObjId                  *entry);

#endif /* H_GUARD_SYMDISCOVER_H */
