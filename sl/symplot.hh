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


#ifndef H_GUARD_SYM_PLOT_H
#define H_GUARD_SYM_PLOT_H

/**
 * @file symplot.hh
 * SymPlot - symbolic heap plotter
 */

#include "symheap.hh"

#include <string>

/// create a plot named "name-NNNN.dot", starting from all live objects
bool plotHeap(
        const SymHeap                   &sh,
        const std::string               &name);

/// create a plot named "name-NNNN.dot", starting from the given starting points
bool plotHeap(
        const SymHeap                   &sh,
        const std::string               &name,
        const TValList                  &startingPoints);

// TODO: remove this
namespace CodeStorage {
    struct Fnc;
}

// TODO: remove this
class SymBackTrace;

/// this plotting interface is @b deprecated, it will go away
class SymPlot {
    public:
        SymPlot(const SymHeap &sh);
        ~SymPlot();
        bool plot(const std::string &name);
        bool plotHeapValue(const std::string &name, TValId value);
        bool plotStackFrame(const std::string           &name,
                            const CodeStorage::Fnc      &fnc,
                            const SymBackTrace          *bt);
    private:
        struct Private;
        Private *d;
        friend class ObjectDigger;
};

#endif /* H_GUARD_SYM_PLOT_H */
