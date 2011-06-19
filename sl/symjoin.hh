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

#ifndef H_GUARD_SYM_JOIN_H
#define H_GUARD_SYM_JOIN_H

/**
 * @file symjoin.hh
 * @todo some dox
 */

#include "symheap.hh"

#include <iostream>

/// @todo some dox
enum EJoinStatus {
    JS_USE_ANY = 0,
    JS_USE_SH1,
    JS_USE_SH2,
    JS_THREE_WAY
};

inline std::ostream& operator<<(std::ostream &str, const EJoinStatus status) {
    switch (status) {
        case JS_USE_ANY:        return (str << "JS_USE_ANY"  );
        case JS_USE_SH1:        return (str << "JS_USE_SH1"  );
        case JS_USE_SH2:        return (str << "JS_USE_SH2"  );
        case JS_THREE_WAY:      return (str << "JS_THREE_WAY");
        default:
            return (str << static_cast<int>(status));
    }
}

/// replacement of matchData() from symdiscover
bool joinDataReadOnly(
        EJoinStatus             *pStatus,
        const SymHeap           &sh,
        const BindingOff        &bf,
        const TValId            addr1,
        const TValId            addr2,
        TValList                protoRoots[1][2]);

/// replacement of abstractNonMatchingValues() from symabstract
bool joinData(
        SymHeap                 &sh,
        const TValId            dst,
        const TValId            src,
        const bool              bidir);

/// @todo some dox
bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *dst,
        const SymHeap           &sh1,
        const SymHeap           &sh2);

/// enable/disable debugging of symjoin
void debugSymJoin(const bool enable);

#endif /* H_GUARD_SYM_JOIN_H */
