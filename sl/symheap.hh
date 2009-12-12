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

#ifndef H_GUARD_SYM_HEAP_H
#define H_GUARD_SYM_HEAP_H

#include "storage.hh"

#include <vector>

namespace SymbolicHeap {

// special OBJs
enum {
    OBJ_INVALID       = -1
};

// special VALs
enum {
    VAL_NULL          =  0,
    VAL_INVALID       = -1,
    VAL_UNINITIALIZED = -2,
    VAL_UNKNOWN       = -3
};

// needs review
class SymHeap {
    public:
        SymHeap();
        ~SymHeap();

    public:
        typedef std::vector<int> TCont;

    public:
        // basic heap lookup
        int /* val */ valueOf(int obj);
        int /* val */ placedAt(int obj);
        int /* obj */ pointsTo(int val);
        const TCont& /* obj[] */ haveValue(int val);
        const TCont& /* obj[] */ notEqualTo(int obj);
        bool notEqual(int obj1, int obj2);

    public:
        // static info lookup
        const struct cl_type* /* clt */ objType(int obj);
        const struct cl_type* /* clt */ valType(int val);
        int /* CodeStorage var uid */ cVar(int var);
        int /* var */ varByCVar(int /* CodeStorage var */ uid);

    public:
        // projection info lookup
        int /* var */ subVar(int var, int nth);
        int /* val */ subVal(int val, int nth);
        int /* var */ varParent(int var);
        int /* val */ valParent(int val);

    public:
        // heap objects construction
        int /* var */ varCreate(const struct cl_type *clt,
                                int /* CodeStorage var */ uid);

        int /* sls */ slsCreate(const struct cl_type *clt,
                                const struct cl_accessor *selector);

    public:
        // heap objects alternation and destruction
        void objSetValue(int obj, int val);
        void objDestroy(int obj);

    public:
        // inequality set alternation
        void addNeq(int obj1, int obj2);
        void delNeq(int obj1, int obj2);

    private:
        struct Private;
        Private *d;
};

} // namespace SymbolicHeap

#endif /* H_GUARD_SYM_HEAP_H */
