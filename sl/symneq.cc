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

#include "config.h"
#include "symneq.hh"

#include "util.hh"

bool NeqDb::areNeq(TValId valLt, TValId valGt) const {
    sortValues(valLt, valGt);
    TItem item(valLt, valGt);
    return hasKey(cont_, item);
}

void NeqDb::add(TValId valLt, TValId valGt) {
    CL_BREAK_IF(valLt == valGt);

    sortValues(valLt, valGt);
    TItem item(valLt, valGt);
    cont_.insert(item);
}

void NeqDb::del(TValId valLt, TValId valGt) {
    CL_BREAK_IF(valLt == valGt);

    sortValues(valLt, valGt);
    TItem item(valLt, valGt);
    cont_.erase(item);
}
