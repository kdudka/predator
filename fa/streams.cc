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

// Standard library headers
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

// Code Listener headers
#include <cl/cl_msg.hh>

// Forester headers
#include "streams.hh"


// anonymous namespace
namespace
{
	/// file descriptors (need to be open by the shell when executing)
	enum fd_e : int
	{
		FD_UCODE       = 3,             ///< FD for microcode output
		FD_TRACE       = 4,             ///< FD for trace output
		FD_TRACE_UCODE = 5,             ///< FD for microcode trace output
		FD_ORIG_CODE   = 6              ///< FD for orignal code output
	};

	/// debugging level
	int debugLvl = 0;

	/**
	 * @brief  
	 *
	 * @param[in]  eol  Should there be an end of line after the message?
	 */
	inline void printToStream(
		std::ostream&       os,
		const char*         locStr,
		const char*         label,
		const char*         msg,
		bool                eol        = true)
	{
		// Assertions
		assert(nullptr != locStr);
		assert(nullptr != label);
		assert(nullptr != msg);

		os << locStr << label << ": " << msg << (eol? "\n" : "");
	}

	/**
	 * @brief  Writes a C-string to a stream given by a file descriptor
	 * 
	 * @param[in]  fd   The file descriptor of the output stream
	 * @param[in]  str  The string to be written to the stream
	 *
	 * @returns  @p EXIT_SUCCESS if OK, otherwise something else :-)
	 */
	int writeToFD(
		int                      fd,
		const char*              str)
	{
		// Assertions
		assert(nullptr != str);

		// first duplicate the target file descriptor so that we can later close it
		// with fclose()
		int tmpFD = dup(fd);
		if (-1 == tmpFD)
		{
			FA_WARN("Cannot duplicate the file descriptor " << fd << "!");
			return EXIT_FAILURE;
		}

		FILE* file = fdopen(tmpFD, "w");
		if (!file)
		{
			FA_WARN("Cannot open the file descriptor " << fd << "for writing!");
			return EXIT_FAILURE;
		}

		size_t len = strlen(str);

		if (fwrite(str, 1, len, file) != len)
		{
			FA_WARN("Error writing to the file descriptor " << fd << "!");
			return EXIT_FAILURE;
		}

		if (fclose(file))
		{
			FA_WARN("Error closing the file descriptor " << fd << "!");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}
}

void Streams::error(
	const char* locStr,
	const char* msg)
{
	std::ostringstream os;
	printToStream(os, locStr, "error", msg, false);
	cl_error(os.str().c_str());
}

void Streams::warn(
	const char* locStr,
	const char* msg)
{
	std::ostringstream os;
	printToStream(os, locStr, "warning", msg, false);
	cl_warn(os.str().c_str());
}

void Streams::debug(
	const char* locStr,
	const char* msg)
{
	printToStream(std::cerr, locStr, "debug", msg);
}

void Streams::log(
	const char* locStr,
	const char* msg)
{
	printToStream(std::clog, locStr, "log", msg);
}

void Streams::note(
	const char* locStr,
	const char* msg)
{
	std::ostringstream os;
	printToStream(os, locStr, "note", msg, false);
	cl_note(os.str().c_str());
}

void Streams::callPrintFnc(
	TPrintFnc          fnc,
	const cl_loc*      loc,
	const char*        locStr,
	const char*        msg)
{
	std::ostringstream os;
	std::string str;

	if (loc)
	{
		os << *loc;
		str = os.str();
		locStr = str.c_str();
	}

	fnc(locStr, msg);
}

void Streams::setDebugLevel(int lvl)
{
	::debugLvl = lvl;
}

void Streams::setDebugLevelAsForCL()
{
	::debugLvl = cl_debug_level();
}

int Streams::getDebugLevel()
{
	return ::debugLvl;
}

void Streams::trace(
	const char*        traceStr)
{
	writeToFD(FD_TRACE, traceStr);
}

void Streams::traceUcode(
	const char*        traceUcodeStr)
{
	writeToFD(FD_TRACE_UCODE, traceUcodeStr);
}

void Streams::ucode(
	const char*        ucodeStr)
{
	writeToFD(FD_UCODE, ucodeStr);
}

void Streams::origCode(
	const char*        origCodeStr)
{
	writeToFD(FD_ORIG_CODE, origCodeStr);
}
