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


#ifndef H_GUARD_SYM_DUMP_H
#define H_GUARD_SYM_DUMP_H

/**
 * @file symdump.hh
 * collection of dump_* functions operating on a SymHeap object, handy when @b
 * debugging
 */

class SymHeap;
class SymHeapCore;

namespace Trace {
    class Node;
}

/// plot the given heap to file "symdump-NNNN.dot"
void dump_plot(const SymHeapCore *sh);

/// plot the given heap to file "symdump-NNNN.dot"
void dump_plot(const SymHeapCore &sh);

/// plot the given heap to file "NAME-NNNN.dot"
void dump_plot(const SymHeapCore &sh, const char *name);

/// plot a trace graph with the given end-point
void dump_trace(Trace::Node *endPoint);

/// plot the corresponding trace graph
void dump_trace(const SymHeapCore &);

#endif /* H_GUARD_SYM_DUMP_H */
