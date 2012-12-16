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

// Forester headers
#include "programconfig.hh"

void ProgramConfig::processArg(const std::string& arg)
{
	using std::string;

	if (arg.empty())
		return;

	std::vector<string> data;
	boost::split(data, arg, boost::is_any_of(":"));

	// assert there is at least one part
	assert(!data.empty());

	const std::string& key = data[0];

	//      ***************  unary arguments ****************
	if (std::string("print-ucode") == key)
	{
		this->printUcode = true;
		FA_LOG("Config::processArg: \"print-ucode\" mode requested");
		return;
	}

	if (std::string("print-orig-code") == key)
	{
		this->printOrigCode = true;
		FA_LOG("Config::processArg: \"print-orig-code\" mode requested");
		return;
	}

	if (std::string("only-compile") == key)
	{
		this->onlyCompile = true;
		FA_LOG("Config::processArg: \"only-compile\" mode requested");
		return;
	}

	if (std::string("print-trace") == key)
	{
		this->printTrace = true;
		FA_LOG("Config::processArg: \"print-trace\" mode requested");
		return;
	}

	if (std::string("print-ucode-trace") == key)
	{
		this->printUcodeTrace = true;
		FA_LOG("Config::processArg: \"print-ucode-trace\" mode requested");
		return;
	}

	//      ***************  binary arguments ****************
	if (std::string("db-root") == key)
	{
		if (data.size() != 2)
		{
			throw std::invalid_argument("use \"db-root:<path>\"");
		}

		this->dbRoot = data[1];
		FA_LOG("Config::processArg: \"db-root\" is \"" + this->dbRoot + "\"");
		return;
	}

	FA_WARN("unhandled argument: \"" << arg << "\"");
}
