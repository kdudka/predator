/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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
#include "adt_op.hh"

#include "symplot.hh"

#include <iomanip>
#include <sstream>

#include <boost/foreach.hpp>

namespace AdtOp {

// /////////////////////////////////////////////////////////////////////////////
// implementation of OpFootprint
OpFootprint::OpFootprint(const SymHeap &input, const SymHeap &output):
    input_(input),
    output_(output)
{
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of OpTemplate
void OpTemplate::plot() const
{
    unsigned idx = 0U;

    BOOST_FOREACH(const OpFootprint *fprint, fList_) {
        // convert the ID to string
        std::ostringstream str;
        str << name_ << "-"
            << std::fixed
            << std::setfill('0')
            << std::setw(/* width of the idx suffix */ 2)
            << (idx++);

        plotHeap(fprint->input(), str.str() + "-in");
        plotHeap(fprint->output(), str.str() + "-out");
    }
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of OpCollection
void OpCollection::plot() const
{
    BOOST_FOREACH(const OpTemplate *tpl, tList_)
        tpl->plot();
}

} // namespace AdtOp
