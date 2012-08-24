/*
 * Copyright (C) 2010-2011 Kamil Dudka <kdudka@redhat.com>
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

// Standard library headers
#include <iomanip>
#include <sstream>

// Forester headers
#include "plotenum.hh"

// /////////////////////////////////////////////////////////////////////////////
// implementation of PlotEnumerator
PlotEnumerator *PlotEnumerator::inst_ = nullptr;

std::string PlotEnumerator::decorate(std::string name)
{
	// obtain a unique ID for the given name
	const int id = map_[name] ++;

	// convert the ID to string
	std::ostringstream str;
	str << std::fixed
		<< std::setfill('0')
		<< std::setw(/* width of the ID suffix */ 4)
		<< id;

	// merge name with ID
	name += "-";
	name += str.str();

	return name;
}
