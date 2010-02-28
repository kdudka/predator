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

/**
 * @file cl_msg.hh
 * Macros for emitting @b error, @b warning, @b info and @b debug messages
 * through the code listener interface
 */

#include <cstdlib>      // needed for abort()
#include <sstream>      // needed for std::ostringstream
#include <string>       // needed for operator<<(std::ostream, std::string)

/**
 * emit a fatal error message and ask the code listener peer to shoot down the
 * process
 * @param msg a message to emit before the process dies, it must be a string
 * literal
 */
#define CL_DIE(msg) do { \
    cl_die("fatal error: " msg); \
    abort(); \
} while (0)

/**
 * standard output stream wrapper on top of the code listener interface
 * @param fnc a function used to emit the message - cl_debug(), cl_warn(),
 * cl_error() or cl_note()
 * @param to_stream whatever you need to stream out.  Stuff with defined
 * operator<<(std::ostream, ...) is a viable start.
 */
#define CL_MSG_STREAM(fnc, to_stream) do { \
    std::ostringstream str; \
    str << to_stream; \
    fnc(str.str().c_str()); \
} while (0)

/**
 * wrapper around CL_MSG_STREAM decorating the message by @b internal location
 * info
 *
 * see cl_msg.hh::CL_MSG_STREAM for details
 */
#define CL_MSG_STREAM_INTERNAL(fnc, to_stream) \
    CL_MSG_STREAM(fnc, __FILE__ << ":" << __LINE__ \
            << ": " << to_stream << " [internal location]")

/**
 * emit a @b debug message with @b internal location info
 */
#define CL_DEBUG(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_debug, "debug: " << to_stream)

/**
 * emit a @b warning message with @b internal location info
 */
#define CL_WARN(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_warn, "warning: " << to_stream)

/**
 * emit an @b error message with @b internal location info
 */
#define CL_ERROR(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_error, "error: " << to_stream)

/**
 * emit a @b note message with @b internal location info
 */
#define CL_NOTE(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_note, "note: " << to_stream)

/**
 * emit a @b debug message using the given location info
 * @param loc location info to use
 * @param what whatever you need to stream out
 */
#define CL_DEBUG_MSG(loc, what) \
    CL_MSG_STREAM(cl_debug, (loc) << "debug: " << what)

/**
 * emit a @b warning message using the given location info
 * @param loc location info to use
 * @param what whatever you need to stream out
 */
#define CL_WARN_MSG(loc, what) \
    CL_MSG_STREAM(cl_warn, (loc) << "warning: " << what)

/**
 * emit an @b error message using the given location info
 * @param loc location info to use
 * @param what whatever you need to stream out
 */
#define CL_ERROR_MSG(loc, what) \
    CL_MSG_STREAM(cl_error, (loc) << "error: " << what)

/**
 * emit a @b note message using the given location info
 * @param loc location info to use
 * @param what whatever you need to stream out
 */
#define CL_NOTE_MSG(loc, what) \
    CL_MSG_STREAM(cl_note, (loc) << "note: " << what)

/// emit raw debug message
void cl_debug(const char *msg);

/// emit raw warning message
void cl_warn(const char *msg);

/// emit raw error message
void cl_error(const char *msg);

/// emit raw note message
void cl_note(const char *msg);

/// emit raw fatal error and ask cl peer to shoot down the process
void cl_die(const char *msg);

#endif /* H_GUARD_CL_MSG_H */
