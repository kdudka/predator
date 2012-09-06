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

#include "config.h"
#include "plotenum.hh"

#include <cl/cl_msg.hh>

#include <iomanip>

// /////////////////////////////////////////////////////////////////////////////
// implementation of PlotEnumerator
PlotEnumerator *PlotEnumerator::inst_ = 0;

std::string PlotEnumerator::decorate(std::string name)
{
    // obtain a unique ID for the given name
    const int id = map_[name] ++;
#if SYMPLOT_STOP_AFTER_N_STATES
    if (SYMPLOT_STOP_AFTER_N_STATES < id) {
        CL_ERROR("SYMPLOT_STOP_AFTER_N_STATES (" << SYMPLOT_STOP_AFTER_N_STATES
                << ") exceeded, now stopping per user's request...");
        CL_TRAP;
    }
#endif

    // convert the ID to string
    std::ostringstream str;
    str << std::fixed
        << std::setfill('0')
        << std::setw(/* width of the ID suffix */ 4)
        << id;

    // merge name with ID
    name += "-";
    name += str.str();

#ifdef SYMPLOT_STOP_CONDITION
    if (SYMPLOT_STOP_CONDITION(name))
        CL_TRAP;
#endif

    return name;
}
