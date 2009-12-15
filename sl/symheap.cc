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
    bool                    custom;

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

    // TODO: move elsewhere
    int                     retVal;

    Private():
        lastObj(0),
        lastVal(0),
        retVal(VAL_INVALID)
    {
    }

    int /* val */ createValue(const struct cl_type *clt, int obj,
                              bool custom = false);
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

int /* val */ SymHeap::Private::createValue(const struct cl_type *clt, int obj,
                                            bool custom)
{
    const int valId = ++lastVal;

    Value &val = valueMap[valId];
    val.clt             = clt;
    val.pointsTo        = obj;
    val.custom          = custom;

    return valId;
}

int /* val */ SymHeap::valueOf(int obj) const {
    switch (obj) {
        case OBJ_INVALID:
            return VAL_INVALID;

        case OBJ_DELETED:
        case OBJ_UNKNOWN:
            return VAL_UNKNOWN;

        default:
            break;
    }

    TVarMap::iterator iter = d->varMap.find(obj);
    if (d->varMap.end() == iter)
        return VAL_INVALID;

    Var &var = iter->second;
    return var.value;
}

int /* val */ SymHeap::placedAt(int obj) const {
    TVarMap::iterator iter = d->varMap.find(obj);
    if (d->varMap.end() == iter)
        return OBJ_INVALID;

    Var &var = iter->second;
    return var.placedAt;
}

int /* obj */ SymHeap::pointsTo(int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return VAL_INVALID;

    Value &value = iter->second;
    if (value.custom)
        TRAP;

    return value.pointsTo;
}

const SymHeap::TCont& /* obj[] */ SymHeap::haveValue(int val) const {
    // TODO
    TRAP;
    static TCont nonsense;
    return nonsense;
}

const SymHeap::TCont& /* obj[] */ SymHeap::notEqualTo(int obj) const {
    // TODO
    TRAP;
    static TCont nonsense;
    return nonsense;
}

bool SymHeap::notEqual(int obj1, int obj2) const {
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

int /* var */ SymHeap::varByCVar(int /* CodeStorage var */ uid) const {
    TIdMap::iterator iter = d->cVarIdMap.find(uid);
    if (d->cVarIdMap.end() == iter)
        return OBJ_INVALID;
    else
        return iter->second;
}

void SymHeap::gatherCVars(TCont &out) const {
    TIdMap::const_iterator ii;
    for (ii = d->cVarIdMap.begin(); ii != d->cVarIdMap.end(); ++ii)
        out.push_back(ii->first);
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
    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_INT:
            CL_DEBUG("CL_TYPE_INT treated as pointer");
            // go through!

        case CL_TYPE_BOOL:
        case CL_TYPE_PTR:
            break;

        default:
            TRAP;
    }

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
    // first look for Var object
    TVarMap::iterator varIter = d->varMap.find(obj);
    if (d->varMap.end() != varIter) {
        // obj is a Var object
        Var &var = varIter->second;
        const int val = var.placedAt;

        if (-1 != var.cVarUid)
            // TODO: carve out the error messages from this module
            CL_MSG_STREAM_INTERNAL(cl_error,
                    "error: attempt to free non-heap object");

        // TODO: check for possible free() of non-root

        // TODO: destroy complex objects recursively
        d->varMap.erase(varIter);

        // mark corresponding value as freed
        Value &ref = d->valueMap[val];
        ref.pointsTo = OBJ_DELETED;
        return;
    }

    // then look for Sls object
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

int /* val */ SymHeap::valCreateCustom(const struct cl_type *clt, int cVal)
{
    return d->createValue(clt, cVal, true);
}

bool SymHeap::valIsCustom(int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        // Oops, value not found at all
        TRAP;

    Value &value = iter->second;
    return value.custom;
}

int /* cVal */ SymHeap::valGetCustom(const struct cl_type **pClt, int val) const
{
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        // custom value not found, this should never happen
        TRAP;

    Value &value = iter->second;
    if (!value.custom)
        // nope, this does not look like a custom value
        TRAP;

    if (pClt)
        // TODO: this deserves a comment in the public header
        *pClt = value.clt;

    return /* cVar */ value.pointsTo;
}

void SymHeap::setReturnValue(int val) {
    if (0 < val && !hasKey(d->valueMap, val))
        // invalid value ought to be returned
        TRAP;

    d->retVal = val;
}

int /* val */ SymHeap::getReturnValue() {
    return d->retVal;
}

} // namespace SymbolicHeap
