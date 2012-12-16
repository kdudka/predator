/*
 * Copyright (C) 2012 Ondrej Lengal
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PROGRAMCONFIG_HH_
#define _PROGRAMCONFIG_HH_

// Boost headers
#include <boost/algorithm/string.hpp>

// Forester headers
#include "streams.hh"

struct ProgramConfig
{
public:   // data members

	std::string dbRoot;             ///< box database root directory
	bool        printUcode;         ///< printing microcode?
	bool        printOrigCode;      ///< printing the original code?
	bool        onlyCompile;        ///< only compiling?
	bool        printTrace;         ///< printing trace for errors?
	bool        printUcodeTrace;    ///< printing microcode trace for errors?

private:  // methods

	void processArg(const std::string& arg);

public:   // methods

	ProgramConfig(const std::string& confStr = "") :
		dbRoot(""),
		printUcode(false),
		printOrigCode(false),
		onlyCompile(false),
		printTrace(false),
		printUcodeTrace(false)
	{
		std::vector<std::string> args;
		boost::split(args, confStr, boost::is_any_of(";"));
		for (const std::string& arg : args)
		{
			processArg(arg);
		}
	}
};


#endif
