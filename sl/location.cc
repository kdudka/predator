/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#include "code_listener.h"
#include "location.hh"

#include <iostream>

Location::Location():
    locLine(-1),
    locColumn(-1)
{
}

Location::Location(const Location *loc):
    locLine(-1),
    locColumn(-1)
{
    if (loc) {
        currentFile     = loc->currentFile;
        locFile         = loc->locFile;
        locLine         = loc->locLine;
        locColumn       = loc->locColumn;
    }
}

Location::Location(const struct cl_location *loc):
    locLine(-1),
    locColumn(-1)
{
    if (loc)
        this->operator=(loc);
}

Location& Location::operator=(const struct cl_location *loc) {
    if (loc->file)
        locFile = loc->file;
    else
        locFile.clear();

    locLine = loc->line;
    locColumn = loc->column;

    return *this;
}

// TODO: review
std::ostream& operator<<(std::ostream &str, const LocationWriter &lw) {
    const Location &loc     = lw.loc;
    const Location &last    = lw.last;

    if ((0 < loc.locLine) && !loc.locFile.empty())
        str << loc.locFile;
    else if ((0 < last.locLine) && !last.locFile.empty())
        str << last.locFile;
    else if (!loc.currentFile.empty())
        str << loc.currentFile;
    else if (!last.currentFile.empty())
        str << last.currentFile;
    else
        str << "<unknown file>";

    str << ":";

    if (0 < loc.locLine)
        str << loc.locLine;
    else if (0 < last.locLine)
        str << last.locLine;
    else
        str << "<unknown line>";

    str << ":";

    if ((0 < loc.locLine) && (0 < loc.locColumn))
        str << loc.locColumn << ":";
    else if ((0 < last.locLine) && (0 < last.locColumn))
        str << last.locColumn << ":";

    str << " " << std::flush;
    return str;
}
