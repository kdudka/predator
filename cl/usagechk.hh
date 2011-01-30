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

#ifndef H_GUARD_USAGECHK_H
#define H_GUARD_USAGECHK_H

#include <cl/cl_msg.hh>

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

        void read(TKey key, const TValue &val, const struct cl_loc *loc) {
            Usage &u = map_[key];
            u.read = true;
            if (!u.loc.file) {
                CL_LOC_SETIF(u.loc, loc);
                u.val = val;
            }
        }

        void write(TKey key, const TValue &val, const struct cl_loc *loc) {
            Usage &u = map_[key];
            u.written = true;
            if (!u.loc.file) {
                CL_LOC_SETIF(u.loc, loc);
                u.val = val;
            }
        }

        void emitPendingMessages(const struct cl_loc *loc);

    private:
#ifndef BUILDING_DOX
        struct Usage {
            bool            read;
            bool            written;
            struct cl_loc   loc;
            TValue          val;

            Usage():
                read(false),
                written(false),
                loc(cl_loc_unknown)
            {
            }
        };

        typedef std::map<int, Usage> TMap;
        TMap                map_;
        std::string         what_;
#endif
};

template <class TKey, class TValue>
inline void UsageChecker<TKey, TValue>::emitPendingMessages(
        const struct cl_loc *loc)
{
    typename TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        const Usage &u = i->second;

        if (!u.read) {
            CL_WARN_MSG(cl_loc_fallback(&u.loc, loc),
                    "warning: unused "
                    << what_ << u.val);
        }

        if (!u.written) {
            CL_ERROR_MSG(cl_loc_fallback(&u.loc, loc),
                    "error: uninitialized "
                    << what_ << u.val);
        }
    }
}

#endif /* H_GUARD_USAGECHK_H */
