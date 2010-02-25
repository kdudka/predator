/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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


#ifndef H_GUARD_SYM_DUMP_H
#define H_GUARD_SYM_DUMP_H

extern int have_symdump;

struct cl_type;
namespace SymbolicHeap {
    struct SymHeap;
}

void dump_clt(const struct cl_type *clt);
void dump_obj(const SymbolicHeap::SymHeap &heap, int obj);
void dump_value(const SymbolicHeap::SymHeap &heap, int value);
void dump_value_refs(const SymbolicHeap::SymHeap &heap, int value);
void dump_cvar(const SymbolicHeap::SymHeap &heap, int cVar);
void dump_heap(const SymbolicHeap::SymHeap &heap);
void dump_any(const SymbolicHeap::SymHeap &heap, int anyId);

#endif /* H_GUARD_SYM_DUMP_H */
