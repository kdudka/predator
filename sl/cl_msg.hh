/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CL_MSG_H
#define H_GUARD_CL_MSG_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define CL_DIE(msg) do { \
    cl_die("fatal error: " msg); \
    abort(); \
} while (0)

#define CL_MSG_STREAM(fnc, to_stream) do { \
    std::ostringstream str; \
    str << to_stream; \
    fnc(str.str().c_str()); \
} while (0)

#define CL_MSG_STREAM_INTERNAL(fnc, to_stream) \
    CL_MSG_STREAM(fnc, __FILE__ << ":" << __LINE__ \
            << ": " << to_stream << " [internal location]")

#define CL_DEBUG(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_debug, "debug: " << to_stream)

#define CL_WARN(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_warn, "warning: " << to_stream)

#define CL_ERROR(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_error, "error: " << to_stream)

#define CL_NOTE(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_note, "note: " << to_stream)

#define CL_DEBUG_MSG(loc, what) \
    CL_MSG_STREAM(cl_debug, (loc) << "debug: " << what)

#define CL_WARN_MSG(loc, what) \
    CL_MSG_STREAM(cl_warn, (loc) << "warning: " << what)

#define CL_ERROR_MSG(loc, what) \
    CL_MSG_STREAM(cl_error, (loc) << "error: " << what)

#define CL_NOTE_MSG(loc, what) \
    CL_MSG_STREAM(cl_note, (loc) << "note: " << what)

void cl_debug(const char *msg);
void cl_warn(const char *msg);
void cl_error(const char *msg);
void cl_note(const char *msg);
void cl_die(const char *msg);

#endif /* H_GUARD_CL_MSG_H */
