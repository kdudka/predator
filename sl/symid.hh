/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_SYM_ID_H
#define H_GUARD_SYM_ID_H

#include <climits>

// well, this is redundant in this header, but saves a few lines of code
class SymHeap;

// special OBJs
typedef enum {
    OBJ_RETURN        =  0,
    OBJ_INVALID       = -1,
    OBJ_DELETED       = -2,
    OBJ_UNKNOWN       = -3,
    OBJ_DEREF_FAILED  = -4,
    OBJ_LOST          = -5,

    OBJ_MAX_ID        = /* XXX */ UINT_MAX
} TObjId;

// special VALs
typedef enum {
    VAL_NULL          =  0,
    VAL_INVALID       = -1,

    VAL_TRUE          = (/* XXX */ OBJ_LOST - 1),
    VAL_FALSE         = VAL_NULL,

    VAL_MAX_ID        = /* XXX */ UINT_MAX
} TValueId;

#endif /* H_GUARD_SYM_ID_H */
