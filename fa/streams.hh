/*
 * Copyright (C) 2012 Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 * This file is part of forester.
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

#ifndef _STREAMS_HH_
#define _STREAMS_HH_

/**
 * @file  streams.hh
 *
 * Code for handling various streams for communication with the outside world.
 * Shamelessly copied from Predator.
 */


// Standard library headers
#include <sstream>
#include <string>

// Code Listener headers
#include <cl/code_listener.h>


/**
 * Standard output stream wrapper on top of the code listener interface
 *
 * @param[in]  fnc	      The function used to emit the message - with prototype
 *                        @c (void (*)(const char*)
 * @param[in]  loc        Location (if nullptr, then the location is in @p locStr
 * @param[in]  locStream  Alternative stream with location (if @c loc == nullptr)
 * @param[in]  to_stream  Whatever you need to stream out.  Stuff with defined
 *                        operator<<(std::ostream, ...) is a viable start.
 */
#define FA_MSG_STREAM(fnc, loc, locStream, to_stream) do {    \
/*  if ((cl_debug == (fnc)) && !cl_debug_level())   */        \
/*     break; */                                              \
	                                                            \
	std::ostringstream strMsg;                                  \
	strMsg << to_stream;                                        \
	if (!(loc))                                                 \
	{                                                           \
	  std::ostringstream strLoc;                                \
	  strLoc << locStream;                                      \
	                                                            \
	  Streams::callPrintFnc(                                    \
	    fnc,                                                    \
	    nullptr,                                                \
	    strLoc.str().c_str(),                                   \
	    strMsg.str().c_str());                                  \
	}                                                           \
	else                                                        \
	{                                                           \
		Streams::callPrintFnc(                                    \
	    fnc,                                                    \
	    loc,                                                    \
	    nullptr,                                                \
	    strMsg.str().c_str());                                  \
	}                                                           \
} while (0)


/**
 * wrapper around FA_MSG_STREAM for known location
 *
 * see FA_MSG_STREAM for details
 */
#define FA_MSG_STREAM_LOC(fnc, loc, to_stream)             \
	FA_MSG_STREAM(fnc, (loc), "", to_stream)


/**
 * wrapper around FA_MSG_STREAM decorating the message by @b internal location
 * info
 *
 * see FA_MSG_STREAM for details
 */
#define FA_MSG_STREAM_INTERNAL(fnc, to_stream)                         \
	FA_MSG_STREAM(fnc, nullptr, __FILE__ << ":" << __LINE__ << ": ",     \
	  to_stream << " [internal location]")

/**
 * Emit an @b error message with @b internal location info
 */
#define FA_ERROR(to_stream) \
	FA_MSG_STREAM_INTERNAL(Streams::error, to_stream)

/**
 * Emit a @b warn message with @b internal location info
 */
#define FA_WARN(to_stream) \
	FA_MSG_STREAM_INTERNAL(Streams::warn, to_stream)

/**
 * Emit a @b debug message with @b internal location info
 */
#define FA_DEBUG(to_stream) \
	FA_MSG_STREAM_INTERNAL(Streams::debug, to_stream)

/**
 * Emit a @b log message with @b internal location info
 */
#define FA_LOG(to_stream) \
	FA_MSG_STREAM_INTERNAL(Streams::log, to_stream)

/**
 * Emit a @b note message with @b internal location info
 */
#define FA_NOTE(to_stream) \
	FA_MSG_STREAM_INTERNAL(Streams::note, to_stream)


/// same as FA_DEBUG, but compares the current debug level with the given one
#define FA_DEBUG_AT(level, what) do {                        \
	if (Streams::getDebugLevel() < (level))                    \
		break;                                                   \
	                                                           \
	FA_DEBUG(what);                                            \
} while (0)


/**
 * Emit an @b error message using the given location info
 *
 * @param[in]  loc   location info to use
 * @param[in]  what  whatever you need to stream out
 */
#define FA_ERROR_MSG(loc, what) \
	FA_MSG_STREAM_LOC(Streams::error, (loc), what)

/**
 * Emit a @b warn message using the given location info
 *
 * @param[in]  loc   location info to use
 * @param[in]  what  whatever you need to stream out
 */
#define FA_WARN_MSG(loc, what) \
	FA_MSG_STREAM_LOC(Streams::warn, (loc), what)


/**
 * Emit a @b debug message using the given location info
 *
 * @param[in]  loc   location info to use
 * @param[in]  what  whatever you need to stream out
 */
#define FA_DEBUG_MSG(loc, what) \
	FA_MSG_STREAM_LOC(Streams::debug, (loc), what)

/**
 * Emit a @b log message using the given location info
 *
 * @param[in]  loc   location info to use
 * @param[in]  what  whatever you need to stream out
 */
#define FA_LOG_MSG(loc, what) \
	FA_MSG_STREAM_LOC(Streams::log, (loc), what)

/**
 * Emit a @b note message using the given location info
 *
 * @param[in]  loc   location info to use
 * @param[in]  what  whatever you need to stream out
 */
#define FA_NOTE_MSG(loc, what) \
	FA_MSG_STREAM_LOC(Streams::note, (loc), what)


/// same as FA_DEBUG_MSG, but compares the current debug level with the given one
#define FA_DEBUG_AT_MSG(level, loc, what) do {             \
	if (Streams::getDebugLevel() < (level))                  \
		break;                                                 \
	                                                         \
	FA_DEBUG_MSG((loc), what);                               \
} while (0)


class Streams
{
public:   // data types

	/// Signature of print functions
	typedef void (*TPrintFnc)(const char*, const char*);

public:   // methods

	// deleted
	Streams() = delete;

	static void log  (const char* locStr, const char* msg);
	static void error(const char* locStr, const char* msg);
	static void debug(const char* locStr, const char* msg);
	static void note (const char* locStr, const char* msg);
	static void warn (const char* locStr, const char* msg);

	static void callPrintFnc(
		TPrintFnc          fnc,
		const cl_loc*      loc,
		const char*        locStr,
		const char*        msg);

	/**
	 * @brief  Sets the debugging level to @p lvl
	 *
	 * @param[in]  lvl  The new value of the debugging level
	 */
	static void setDebugLevel(int lvl);

	/**
	 * @brief  Sets the debugging level the same as for Code Listener
	 *
	 * @param[in]  lvl  The new value of the debugging level
	 */
	static void setDebugLevelAsForCL();

	/**
	 * @brief  Gets the current debugging level
	 *
	 * @returns  Current debugging level
	 */
	static int getDebugLevel();

	/**
	 * @brief  Prints to the trace output
	 *
	 * @param[in]  traceStr  A string with a trace
	 */
	static void trace(const char* traceStr);

	/**
	 * @brief  Prints to the microcode trace output
	 *
	 * @param[in]  traceStr  A string with a microcode trace
	 */
	static void traceUcode(const char* traceUcodeStr);

	/**
	 * @brief  Prints to the microcode output
	 *
	 * @param[in]  ucodeStr  A string with a microcode
	 */
	static void ucode(const char* ucodeStr);

	/**
	 * @brief  Prints to the original code output
	 *
	 * @param[in]  origCodeStr  A string with the original code
	 */
	static void origCode(const char* origCodeStr);
};

#endif
