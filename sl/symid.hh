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

#ifndef H_GUARD_SYM_ID_H
#define H_GUARD_SYM_ID_H

/**
 * @file symid.hh
 * types used for identification of SymHeap entities - symid.hh::TObjId and
 * symid.hh::TValId
 */

#include "config.h"

#include <climits>          // needed for UINT_MAX
#include <utility>          // needed for std::pair

/**
 * SymHeap @b object @b ID
 * @note Though the type is defined as enumeration for now, the code should not
 * rely on that fact, as it may be subject for change in the future.
 */
enum TObjId {
    /**
     * special enumeration value denoting a failure of a method which may return
     * a valid object.  OBJ_INVALID can @b never denote a valid object.  @n @n
     */
    OBJ_INVALID       = -1,

    /**
     * special enumeration value denoting a result of invalid dereference,
     * either direct or implied.  OBJ_DEREF_FAILED can @b never denote a valid
     * object.  This enumeration value is used for sort of error recovery,
     * especially to avoid flood of error messages triggered by a single error
     * in the analyzed program. @n @n
     */
    OBJ_DEREF_FAILED  = -2,

    /**
     * special enumeration value denoting we in fact @b know @b nothing about
     * the requested object.  OBJ_UNKNOWN may or may not stand for an existing
     * object. @n @n
     */
    OBJ_UNKNOWN       = -3,

    /**
     * sort of non-portable hack, abusing enumeration type for integral purposes
     */
    OBJ_MAX_ID        = /* XXX */ UINT_MAX
};

/**
 * SymHeap @b value @b ID
 * @note Though the type is defined as enumeration for now, the code should not
 * rely on that fact, as it may be subject for change in the future.
 */
enum TValId {
    /**
     * special enumeration value denoting a NULL pointer.  This value can't be
     * followed by SymHeap::pointsTo().  This enumeration value @b coincides
     * with @b VAL_FALSE and can't be anyhow distinguished without the owning
     * object.  @n @n
     */
    VAL_NULL          =  0,

    /// @todo some dox
    VAL_ADDR_OF_RET   =  1,

    /**
     * special enumeration value denoting a failure of a method which may return
     * a value ID.  VAL_INVALID can @b never denote a valid value.  @n @n
     */
    VAL_INVALID       = -1,

    /**
     * special enumeration value denoting a Boolean false value.  This
     * enumeration value @b coincides with @b VAL_NULL and can't be anyhow
     * distinguished without the owning object.  @n @n
     */
    VAL_FALSE         = VAL_NULL,

    /**
     * special enumeration value denoting a Boolean true value.
     * @n @n
     */
    VAL_TRUE          = (OBJ_UNKNOWN - 1),

    /**
     * @copydoc symid.hh::OBJ_MAX_ID
     */
    VAL_MAX_ID        = /* XXX */ UINT_MAX
};

typedef std::pair<TObjId, TObjId>           TObjPair;
typedef std::pair<TValId, TValId>           TValPair;

#endif /* H_GUARD_SYM_ID_H */
