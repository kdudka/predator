/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CL_PRIVATE_H
#define H_GUARD_CL_PRIVATE_H

#include <cl/code_listener.h>

class ICodeListener;

/**
 * wrap ICodeListener object so that it can be exposed to pure C world
 */
struct cl_code_listener* cl_create_listener_wrap(ICodeListener *);

/**
 * retrieve wrapped ICodeListener object
 */
ICodeListener* cl_obtain_from_wrap(struct cl_code_listener *);

/**
 * evaluates as true if the given (struct cl_loc *) pLoc is valid location info
 */
#define CL_LOC_VALID(pLoc) ((pLoc) && (pLoc)->file)

/**
 * rewrites dst by *pSrc, if (struct cl_loc *) pSrc is valid location info
 */
#define CL_LOC_SETIF(dst, pSrc) do {        \
    if (CL_LOC_VALID(pSrc))                 \
        (dst) = *(pSrc);                    \
} while (0)

/**
 * initialize location info by the given file name
 */
#define CL_LOC_SET_FILE(dst, _file) do {    \
    (dst) = cl_loc_unknown;                 \
    (dst).file = (_file);                   \
} while (0)

#endif /* H_GUARD_CL_PRIVATE_H */
