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
 * types used for identification of SymHeap entities - symid.hh::TFldId and
 * symid.hh::TValId
 */

#include "config.h"

#include <climits>          // needed for UINT_MAX
#include <utility>          // needed for std::pair

/**
 * SymHeap @b field @b ID
 * @note Though the type is defined as enumeration for now, the code should not
 * rely on that fact, as it may be subject for change in the future.
 */
enum TFldId {
    /**
     * special enumeration value denoting a failure of a method which may return
     * a valid object.  FLD_INVALID can @b never denote a valid object.  @n @n
     */
    FLD_INVALID       = -1,

    /**
     * special enumeration value denoting a result of invalid dereference,
     * either direct or implied.  FLD_DEREF_FAILED can @b never denote a valid
     * object.  This enumeration value is used for sort of error recovery,
     * especially to avoid flood of error messages triggered by a single error
     * in the analyzed program. @n @n
     */
    FLD_DEREF_FAILED  = -2,

    /**
     * special enumeration value denoting we in fact @b know @b nothing about
     * the requested object.  FLD_UNKNOWN may or may not stand for an existing
     * object. @n @n
     */
    FLD_UNKNOWN       = -3,

    /**
     * sort of non-portable hack, abusing enumeration type for integral purposes
     */
    FLD_MAX_ID        = /* XXX */ UINT_MAX
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
     * special enumeration value denoting a Boolean true value or (int) 1.
     */
    VAL_TRUE          = 1,

    /**
     * @copydoc symid.hh::FLD_MAX_ID
     */
    VAL_MAX_ID        = /* XXX */ UINT_MAX
};

typedef std::pair<TValId, TValId>           TValPair;

/**
 * SymHeap @b object @b ID
 * @note Though the type is defined as enumeration for now, the code should not
 * rely on that fact, as it may be subject for change in the future.
 */
enum TObjId {
    /// for signalling error states only
    OBJ_INVALID       = -1,

    /// target of VAL_NULL (a.k.a. NULL object)
    OBJ_NULL          = (VAL_TRUE + 1),

    /// return value of the function currently being executed
    OBJ_RETURN        = (OBJ_NULL + 1),

    /// @copydoc symid.hh::FLD_MAX_ID
    OBJ_MAX_ID        = /* XXX */ UINT_MAX
};

typedef std::pair<TObjId, TObjId>           TObjPair;

#endif /* H_GUARD_SYM_ID_H */
