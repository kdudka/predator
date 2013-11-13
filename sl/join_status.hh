/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_JOIN_STATUS_H
#define H_GUARD_JOIN_STATUS_H

#include <iostream>

/// classification of the result of a join operation
enum EJoinStatus {
    JS_USE_ANY = 0,
    JS_USE_SH1,
    JS_USE_SH2,
    JS_THREE_WAY
};

inline std::ostream& operator<<(std::ostream &str, const EJoinStatus status)
{
    switch (status) {
        case JS_USE_ANY:        return (str << "JS_USE_ANY"  );
        case JS_USE_SH1:        return (str << "JS_USE_SH1"  );
        case JS_USE_SH2:        return (str << "JS_USE_SH2"  );
        case JS_THREE_WAY:      return (str << "JS_THREE_WAY");
        default:
            return (str << static_cast<int>(status));
    }
}

#endif /* H_GUARD_JOIN_STATUS_H */
