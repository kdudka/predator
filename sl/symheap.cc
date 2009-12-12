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
    int /* val */           placedAt;
    int /* val */           value;

    // the following line helps to recognize an uninitialized instance
    Var(): clt(0) { }
};

struct Value {
    const struct cl_type    *clt;
    int /* obj */           pointsTo;

    // the following line helps to recognize an uninitialized instance
    Value(): clt(0) { }
};

typedef std::map<int, int> TIdMap;
typedef std::map<int, Var> TVarMap;
typedef std::map<int, Value> TValueMap;

// /////////////////////////////////////////////////////////////////////////////
// SymHeap implementation
struct SymHeap::Private {
    TIdMap                  cVarIdMap;
    TVarMap                 varMap;
    TValueMap               valueMap;

    int                     lastObj;
    int                     lastVal;

    Private():
        lastObj(0),
        lastVal(0)
    {
    }

    int /* val */ createValue(const struct cl_type *clt, int obj);
};

SymHeap::SymHeap():
    d(new Private)
{
}

SymHeap::SymHeap(const SymHeap &ref):
    d(new Private(*ref.d))
{
}

SymHeap::~SymHeap() {
    delete d;
}

SymHeap& SymHeap::operator=(const SymHeap &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
}

int /* val */ SymHeap::Private::createValue(const struct cl_type *clt, int obj)
{
    const int valId = ++lastVal;

    Value &val = valueMap[valId];
    val.clt             = clt;
    val.pointsTo        = obj;

    return valId;
}

int /* val */ SymHeap::valueOf(int obj) {
    TVarMap::iterator iter = d->varMap.find(obj);
    if (d->varMap.end() == iter)
        return OBJ_INVALID;

    Var &var = iter->second;
    return var.value;
}

int /* val */ SymHeap::placedAt(int obj) {
    TVarMap::iterator iter = d->varMap.find(obj);
    if (d->varMap.end() == iter)
        return OBJ_INVALID;

    Var &var = iter->second;
    return var.placedAt;
}

int /* obj */ SymHeap::pointsTo(int val) {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return VAL_INVALID;

    Value &value = iter->second;
    return value.pointsTo;
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
    TIdMap::iterator iter = d->cVarIdMap.find(uid);
    if (d->cVarIdMap.end() == iter)
        return OBJ_INVALID;
    else
        return iter->second;
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
    var.placedAt    = d->createValue(clt, objId);
    var.value       = VAL_UNINITIALIZED;

    if (/* heap object */ -1 != uid)
        d->cVarIdMap[uid] = objId;

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
    // first look for Var object
    TVarMap::iterator varIter = d->varMap.find(obj);
    if (d->varMap.end() != varIter) {
        // obj is a Var object
        Var &var = varIter->second;
        var.value = val;
        return;
    }

    // then look for Sls object
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
