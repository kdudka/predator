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

#ifndef H_GUARD_USAGECHK_H
#define H_GUARD_USAGECHK_H

#include "cl_private.hh"
#include "location.hh"

#include <map>
#include <string>

template <class TKey, class TValue>
class UsageChecker {
    public:
        UsageChecker(std::string what):
            what_(what)
        {
        }

        void reset() {
            map_.clear();
        }

        void read(TKey key, const TValue &val, const Location &loc) {
            Usage &u = map_[key];
            u.read = true;
            if (u.loc.locLine < 0) {
                u.loc = loc;
                u.val = val;
            }
        }

        void write(TKey key, const TValue &val, const Location &loc) {
            Usage &u = map_[key];
            u.written = true;
            if (u.loc.locLine < 0) {
                u.loc = loc;
                u.val = val;
            }
        }

        void emitPendingMessages(const Location &loc);

    private:
        struct Usage {
            bool            read;
            bool            written;
            Location        loc;
            TValue          val;

            Usage(): read(false), written(false) { }
        };

        typedef std::map<int, Usage> TMap;
        TMap                map_;
        std::string         what_;
};

template <class TKey, class TValue>
inline void UsageChecker<TKey, TValue>::emitPendingMessages(const Location &loc)
{
    typename TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        const Usage &u = i->second;

        if (!u.read) {
            CL_MSG_STREAM(cl_warn,
                    LocationWriter(u.loc, &loc)
                    << "warning: unused " << what_ << u.val);
        }

        if (!u.written) {
            CL_MSG_STREAM(cl_error,
                    LocationWriter(u.loc, &loc)
                    << "error: uninitialized " << what_ << u.val);
        }
    }
}

#endif /* H_GUARD_USAGECHK_H */
