/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_NEQ_H
#define H_GUARD_SYM_NEQ_H

#include "symid.hh"

#include <set>

class NeqDb {
    public:
        bool empty() const {
            return cont_.empty();
        }

        void add(TValId valLt, TValId valGt);
        void del(TValId valLt, TValId valGt);
        bool areNeq(TValId valLt, TValId valGt) const;

    protected:
        typedef std::pair<TValId /* valLt */, TValId /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;
};

#endif /* H_GUARD_SYM_NEQ_H */
