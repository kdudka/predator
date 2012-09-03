/*
 * Copyright (C) 2010-2012 Kamil Dudka <kdudka@redhat.com>
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


#ifndef H_GUARD_PLOT_ENUM_H
#define H_GUARD_PLOT_ENUM_H

/**
 * @file plotenum.hh
 * 
 * This file was shamelessly copied from Predator.
 */

// Standard library headers
#include <string>
#include <map>

// singleton
class PlotEnumerator
{
	public:

		static PlotEnumerator* instance()
		{
		    return (inst_) ?
					(inst_) :
					(inst_ = new PlotEnumerator);
		}
		
		// generate kind of more unique name
		std::string decorate(std::string name);
	
	private:
		static PlotEnumerator *inst_;

		PlotEnumerator() :
			map_{}
		{ }
		// FIXME: should we care about the destruction?
	
	private:
		typedef std::map<std::string, int> TMap;
		TMap map_;
};

#endif /* H_GUARD_PLOT_ENUM_H */
