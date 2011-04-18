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


#ifndef H_GUARD_SYM_PLOT_H
#define H_GUARD_SYM_PLOT_H

/**
 * @file symplot.hh
 * SymPlot - symbolic heap plotter
 */

#include "symid.hh"

#include <string>

namespace CodeStorage {
    struct Fnc;
    struct Storage;
}

class SymBackTrace;
class SymHeap;

/**
 * @file symplot.hh
 * symbolic heap @b plotter
 * @note it should be safe to call the plot* methods more times per one instance
 * of SymPlot.  The already plotted entities should be not plotted again in that
 * case.
 */
class SymPlot {
    public:
        /**
         * initialize plotter by static-info and symbolic heap
         * @param stor all-in-one info about the analyzed code, see CodeStorage
         * @param heap a symbolic heap which is going to be plotted
         */
        SymPlot(const CodeStorage::Storage &stor, const SymHeap &heap);
        ~SymPlot();

        /**
         * plot the whole symbolic heap, starting from static/automatic
         * variables
         * @param name base name of the resulting dot file, which will be
         * decorated by a serial number
         */
        bool plot(const std::string &name);

        /**
         * plot a sub-heap reachable from the given heap value
         * @param name base name of the resulting dot file, which will be
         * decorated by a serial number
         * @param value ID of a value denoting the requested sub-heap
         */
        bool plotHeapValue(const std::string &name, TValId value);

        /**
         * plot a sub-heap reachable from automatic variables of the given
         * function
         * @param name base name of the resulting dot file, which will be
         * decorated by a serial number
         * @param fnc a function to look for the automatic variables
         * @param bt an instance of symbolic backtrace used to distinguish among
         * instances of automatic variables
         */
        bool plotStackFrame(const std::string           &name,
                            const CodeStorage::Fnc      &fnc,
                            const SymBackTrace          *bt);

    private:
        /// object copying is @b not allowed
        SymPlot(const SymPlot &);

        /// object copying is @b not allowed
        SymPlot& operator=(const SymPlot &);

    private:
        struct Private;
        Private *d;

        // internal helper of SymPlot::Private::digObjCore()
        friend class ObjectDigger;
};

#endif /* H_GUARD_SYM_PLOT_H */
