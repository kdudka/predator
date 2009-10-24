/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_LOCATION_H
#define H_GUARD_LOCATION_H

#include <string>

struct Location {
    std::string currentFile;
    std::string locFile;
    int         locLine;
    int         locColumn;

    Location();
    Location(const Location *loc);
    Location(const struct cl_location *loc);
    Location& operator=(const struct cl_location *loc);
};

struct LocationWriter {
    Location                        loc;
    Location                        last;

    LocationWriter(const Location &loc_, const Location *last_ = 0):
        loc(loc_),
        last(last_)
    {
    }

    LocationWriter(const struct cl_location *loc_, const Location *last_ = 0):
        loc(loc_),
        last(last_)
    {
    }
};
std::ostream& operator<<(std::ostream &, const LocationWriter &);

#endif /* H_GUARD_LOCATION_H */
