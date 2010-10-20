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

#include "symheap.hh"

struct cl_type;
class SymHeap;

#ifndef BUILDING_DOX
    namespace CodeStorage {
        struct Storage;
    }

    // for internal use only
    void dump_plot_set_stor(const CodeStorage::Storage &);
#endif

/// dump @b type-info given as clt
void dump_clt(const struct cl_type *clt);

/// dump a chain of @b accessors
void dump_ac(const struct cl_accessor *ac);

/// dump a chain of @b selectors
void dump_ic(const struct cl_type *clt, TFieldIdxChain ic);

/// dump @b kind of the given object
void dump_kind(const SymHeap &heap, TObjId obj);

/// dump a symbolic heap @b object
void dump_obj(const SymHeap &heap, TObjId obj);

/// dump a symbolic heap @b value
void dump_value(const SymHeap &heap, TValueId value);

/// dump a symbolic heap value, including all (directly) @b referred @b objects
void dump_value_refs(const SymHeap &heap, TValueId value);

/// dump a static/automatic @b variable, see CodeStorage
void dump_cvar(const SymHeap &heap, int cVar);

/// dump a @b all static/automatic @b variables of the given symbolic heap
void dump_heap(const SymHeap &heap);

/// dump any @b existing @b positive ID as either heap object or a heap value
void dump_id(const SymHeap &heap, int id);

/// plot the given heap to file "symdump-NNNN.dot"
void dump_plot(const SymHeapCore *sh);

/// plot the given heap to file "symdump-NNNN.dot"
void dump_plot(const SymHeapCore &sh);

/// plot the given heap to file "NAME-NNNN.dot"
void dump_plot(const SymHeapCore &sh, const char *name);

#endif /* H_GUARD_SYM_DUMP_H */
