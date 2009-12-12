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

#include "symheap.hh"
#include "cl_private.hh"

#include <map>

namespace SymbolicHeap {

struct Var {
    const struct cl_type    *clt;
    int /* CodeStorage */   cVarUid;
    int /* val */           value;
};

typedef std::map<int, Var> TVarMap;

// /////////////////////////////////////////////////////////////////////////////
// SymHeap implementation
struct SymHeap::Private {
    TVarMap varMap;

    int lastObj;
    int lastVal;

    Private():
        lastObj(0),
        lastVal(0)
    {
    }
};

SymHeap::SymHeap():
    d(new Private)
{
}

SymHeap::~SymHeap() {
    delete d;
}

int /* val */ SymHeap::valueOf(int obj) {
    // TODO
    TRAP;
    return VAL_INVALID;
}

int /* val */ SymHeap::placedAt(int obj) {
    // TODO
    TRAP;
    return VAL_INVALID;
}

int /* obj */ SymHeap::pointsTo(int val) {
    // TODO
    TRAP;
    return OBJ_INVALID;
}

const SymHeap::TCont& /* obj[] */ SymHeap::haveValue(int val) {
    // TODO
    TRAP;
    static TCont nonsense;
    return nonsense;;
}

const SymHeap::TCont& /* obj[] */ SymHeap::notEqualTo(int obj) {
    // TODO
    TRAP;
    static TCont nonsense;
    return nonsense;;
}

bool SymHeap::notEqual(int obj1, int obj2) {
    // TODO
    TRAP;
    return false;
}

const struct cl_type* /* clt */ SymHeap::objType(int obj) {
    // TODO
    TRAP;
    return 0;
}

const struct cl_type* /* clt */ SymHeap::valType(int val) {
    // TODO
    TRAP;
    return 0;
}

int /* CodeStorage var uid */ SymHeap::cVar(int var) {
    // TODO
    TRAP;
    return -1;
}

int /* var */ SymHeap::varByCVar(int /* CodeStorage var */ uid) {
    // TODO
    TRAP;
    return OBJ_INVALID;
}

int /* var */ SymHeap::subVar(int var, int nth) {
    // TODO
    TRAP;
    return OBJ_INVALID;
}

int /* val */ SymHeap::subVal(int val, int nth) {
    // TODO
    TRAP;
    return VAL_INVALID;
}

int /* var */ SymHeap::varParent(int var) {
    // TODO
    TRAP;
    return OBJ_INVALID;
}

int /* val */ SymHeap::valParent(int val) {
    // TODO
    TRAP;
    return VAL_INVALID;
}

int /* var */ SymHeap::varCreate(const struct cl_type *clt,
                                 int /* CodeStorage var */ uid)
{
    if (CL_TYPE_PTR != clt->code)
        // not implemented yet
        TRAP;

    int objId = ++(d->lastObj);
    Var &var = d->varMap[objId];

    var.clt         = clt;
    var.cVarUid     = uid;
    var.value       = VAL_UNINITIALIZED;

    return objId;
}

int /* sls */ SymHeap::slsCreate(const struct cl_type *clt,
                                 const struct cl_accessor *selector)
{
    // TODO
    TRAP;
    return OBJ_INVALID;
}

void SymHeap::objSetValue(int obj, int val) {
    // TODO
    TRAP;
}

void SymHeap::objDestroy(int obj) {
    // TODO
    TRAP;
}

void SymHeap::addNeq(int obj1, int obj2) {
    // TODO
    TRAP;
}

void SymHeap::delNeq(int obj1, int obj2) {
    // TODO
    TRAP;
}

} // namespace SymbolicHeap
