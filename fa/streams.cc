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
#include <iostream>

// Code Listener headers
#include <cl/cl_msg.hh>

// Forester headers
#include "streams.hh"

// definition of the symbol
int Streams::debugLvl_ = 0;

// anonymous namespace
namespace
{
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
	printToStream(std::cerr, locStr, "warning", msg);
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
	printToStream(std::cerr, locStr, "note", msg);
}

void Streams::callPrintFnc(
	TPrintFnc          fnc,
	const cl_loc*      loc,
	const char*        locStr,
	const char*        msg)
{
	std::ostringstream os;

	if (loc)
	{
		os << *loc;
		locStr = os.str().c_str();
	}

	fnc(locStr, msg);
}

void Streams::setDebugLevel(int lvl)
{
	debugLvl_ = lvl;
}

void Streams::setDebugLevelAsForCL()
{
	debugLvl_ = cl_debug_level();
}

int Streams::getDebugLevel()
{
	return debugLvl_;
}
