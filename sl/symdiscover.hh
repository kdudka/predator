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
#include "symheap.hh"

/// return true if the given binding is a DLS binding
inline bool isDlsBinding(const BindingOff &off) {
    return (off.next != off.prev);
}

/**
 * Take the given symbolic heap and look for the best possible abstraction in
 * there.  If nothing is found, zero is returned.  Otherwise it returns total
 * length of the best possible abstraction.  In that case, *bf is set to the
 * corresponding binding fields (head, next, peer) and *entry is set to the
 * corresponding starting point of the abstraction.
 *
 * In case of success (non-zero return value), you can determine the type of the
 * detected abstraction by *bf.  If bf->peer is equal to bf->next, it means a
 * SLS abstraction;  DLS otherwise.  If bf->head is zero, it means a regular
 * list segment abstraction;  Linux list segment otherwise.
 *
 * In case of failure (zero return value), *bf and *entry are undefined.
 */
unsigned /* len */ discoverBestAbstraction(
        SymHeap                 &sh,
        BindingOff              *bf,
        TValId                  *entry);

#endif /* H_GUARD_SYMDISCOVER_H */
