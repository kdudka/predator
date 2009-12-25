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
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

namespace SymbolicHeap {

typedef std::vector<int> TSub;

struct Var {
    const struct cl_type    *clt;
    int /* CodeStorage */   cVarUid;
    int /* val */           placedAt;
    int /* val */           value;
    TSub                    subVars;

    // the following line helps to recognize an uninitialized instance
    Var(): clt(0) { }
};

struct Value {
    const struct cl_type    *clt;
    int /* obj */           pointsTo;
    bool                    custom;
    TSub                    subValues;

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
    TIdMap                  cValIdMap;
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
    int /* var */ createVar(const struct cl_type *clt,
                            int /* CodeStorage */ uid);

    void destroySingleVar(int var, bool isHeapVar);
    void destroyVar(int var, bool isHeapVar);

    void createSubs(Var &refVar, const struct cl_type *clt);
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

int /* var */ SymHeap::Private::createVar(const struct cl_type *clt,
                                          int /* CodeStorage */ uid)
{
    const int objId = ++(this->lastObj);
    Var &var = this->varMap[objId];

    var.clt         = clt;
    var.cVarUid     = uid;
    var.placedAt    = this->createValue(clt, objId);
    var.value       = VAL_UNINITIALIZED;

    return objId;
}

void SymHeap::Private::destroySingleVar(int var, bool isHeapVar) {
    TVarMap::iterator varIter = this->varMap.find(var);
    if (this->varMap.end() == varIter)
        // var not found
        TRAP;

    Var &refVar = varIter->second;
    if (isHeapVar && -1 != refVar.cVarUid)
        // TODO: carve out the error messages from this module
        CL_MSG_STREAM_INTERNAL(cl_error,
                "error: attempt to free non-heap object");

    if (!isHeapVar && -1 == refVar.cVarUid)
        // wrong method used to destroy a heap object
        TRAP;

    // TODO: check for possible free() of non-root
    // TODO: check for JUNK!

    // mark corresponding value as freed
    const int val = refVar.placedAt;
    Value &ref = this->valueMap[val];
    ref.pointsTo = (isHeapVar)
        ? OBJ_DELETED
        : OBJ_LOST;

    // TODO: destroy complex objects recursively
    this->varMap.erase(varIter);
}

void SymHeap::Private::destroyVar(int var, bool isHeapVar) {
    typedef std::stack<int /* var */> TStack;
    TStack todo;

    // we are using explicit stack to avoid recursion
    todo.push(var);
    while (!todo.empty()) {
        const int var = todo.top();
        todo.pop();

        TVarMap::iterator varIter = this->varMap.find(var);
        if (this->varMap.end() == varIter)
            // var not found
            TRAP;

        // schedule all subvars for removal
        Var &refVar = varIter->second;
        TSub &subs = refVar.subVars;
        BOOST_FOREACH(int subVar, subs) {
            todo.push(subVar);
        }

        // remove current
        this->destroySingleVar(var, isHeapVar);
    }
}

void SymHeap::Private::createSubs(Var &refVar, const struct cl_type *clt) {
    typedef std::pair<Var *, const struct cl_type *> TPair;
    typedef std::stack<TPair> TStack;
    TStack todo;

    // we are using explicit stack to avoid recursion
    todo.push(&refVar, clt);
    while (!todo.empty()) {
        Var *pVar;
        const struct cl_type *clt;
        boost::tie(pVar, clt) = todo.top();
        todo.pop();

        // FIXME: check whether clt may be NULL at this point
        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_CHAR:
                CL_MSG_STREAM_INTERNAL(cl_warn, "warning: "
                        "CL_TYPE_CHAR are not supported by SymHeap for now");
                break;

            case CL_TYPE_PTR:
                break;

            case CL_TYPE_STRUCT: {
                const int cnt = clt->item_cnt;
                pVar->subVars.resize(cnt);
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type *subClt = clt->items[i].type;
                    const int obj = this->createVar(subClt, -1);
                    pVar->subVars[i] = obj;

                    Var &var = /* FIXME: suboptimal */ this->varMap[obj];
                    todo.push(&var, subClt);
                }
                break;
            }

            default:
                TRAP;
        }
    }
}

int /* val */ SymHeap::valueOf(int obj) const {
    switch (obj) {
        case OBJ_INVALID:
            return VAL_INVALID;

        case OBJ_LOST:
        case OBJ_DELETED:
        case OBJ_DEREF_FAILED:
            return VAL_DEREF_FAILED;

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
        return VAL_INVALID;

    Var &var = iter->second;
    return var.placedAt;
}

int /* obj */ SymHeap::pointsTo(int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return OBJ_INVALID;

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

const struct cl_type* /* clt */ SymHeap::objType(int obj) const {
    // first look for Var object
    TVarMap::iterator varIter = d->varMap.find(obj);
    if (d->varMap.end() != varIter) {
        // obj is a Var object
        Var &var = varIter->second;
        return var.clt;
    }

    // then look for Sls object
    // TODO
    TRAP;
    return 0;
}

const struct cl_type* /* clt */ SymHeap::valType(int val) const {
    // TODO
    TRAP;
    return 0;
}

int /* CodeStorage var uid */ SymHeap::cVar(int var) const {
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

int /* var */ SymHeap::subVar(int var, int nth) const {
    TVarMap::iterator iter = d->varMap.find(var);
    if (d->varMap.end() == iter)
        return OBJ_INVALID;

    const Var &refVar = iter->second;
    const TSub &subs = refVar.subVars;
    const int cnt = subs.size();
    return (nth < cnt)
        ? subs[nth]
        : OBJ_INVALID;
}

int /* val */ SymHeap::subVal(int val, int nth) const {
    // TODO
    TRAP;
    return VAL_INVALID;
}

int /* var */ SymHeap::varParent(int var) const {
    // TODO
    TRAP;
    return OBJ_INVALID;
}

int /* val */ SymHeap::valParent(int val) const {
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
        case CL_TYPE_STRUCT:
            break;

        default:
            TRAP;
    }

    const int objId = d->createVar(clt, uid);

    if (/* heap object */ -1 != uid)
        d->cVarIdMap[uid] = objId;

    return objId;
}

int /* var */ SymHeap::varCreateAnon(int cbSize) {
    return d->createVar(0, /* FIXME: use union for this? */ cbSize);
}

int SymHeap::varSizeOfAnon(int var) const {
    TVarMap::iterator iter = d->varMap.find(var);
    if (d->varMap.end() == iter)
        // not even a variable
        TRAP;

    Var &ref = iter->second;
    if (ref.clt)
        // not anonoymous variable at all
        TRAP;

    return /* cbSize */ ref.cVarUid;
}

bool SymHeap::valPointsToAnon(int val) const {
    if (val <= 0)
        return false;

    TValueMap::iterator valIter = d->valueMap.find(val);
    if (d->valueMap.end() == valIter)
        // not even a value
        TRAP;

    Value &value = valIter->second;
    if (value.custom)
        return false;

    TVarMap::iterator iter = d->varMap.find(value.pointsTo);
    if (d->varMap.end() == iter)
        // not even a variable
        TRAP;

    const Var &ref = iter->second;
    return !ref.clt;
}

void SymHeap::varDefineType(int var, const struct cl_type *clt) {
    TVarMap::iterator varIter = d->varMap.find(var);
    if (d->varMap.end() == varIter)
        // var not found
        TRAP;

    Var &refVar = varIter->second;
    if (refVar.clt)
        // type redefinition not allowed
        TRAP;

    refVar.cVarUid = /* heap object */ -1;
    refVar.clt     = clt;
    d->createSubs(refVar, clt);

    TValueMap::iterator valIter = d->valueMap.find(refVar.placedAt);
    if (d->valueMap.end() == valIter)
        TRAP;

    Value &value = valIter->second;
    if (value.clt || value.custom)
        TRAP;

    value.clt = clt;
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
        d->destroyVar(obj, true);
        return;
    }

    // then look for Sls object
    // TODO
    TRAP;
}

void SymHeap::varDestroyNonHeap(int var) {
    d->destroyVar(var, false);
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
    TIdMap::iterator ii = d->cValIdMap.find(cVal);
    if (d->cValIdMap.end() == ii) {
        const int val = d->createValue(clt, cVal, true);
        d->cValIdMap[cVal] = val;
        return val;
    }

    // custom value already defined, we has to reuse it
    const int val = ii->second;
    TValueMap::iterator vi = d->valueMap.find(val);
    if (d->valueMap.end() == vi)
        TRAP;

    const Value &ref = vi->second;
    if (!ref.custom)
        // heap corruption
        TRAP;

    if (ref.clt != clt)
        // type mismatch
        TRAP;

    return val;
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

int /* val */ SymHeap::getReturnValue() const {
    return d->retVal;
}

} // namespace SymbolicHeap
