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

#include "config.h"
#include "symheap.hh"

#include "cl_private.hh"
#include "symdump.hh"
#include "util.hh"
#include "worklist.hh"

#include <map>
#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace {
    // XXX: force linker to pull-in the symdump module into .so
    void pull_in_symdump(void) {
        int sink = ::have_symdump;
        (void) sink;
    }
}

namespace SymbolicHeap {

class NeqDb {
    private:
        typedef std::pair<int /* valLt */, int /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;

    public:
        bool areNeq(int valLt, int valGt) {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            return hasKey(cont_, item);
        }
        void add(int valLt, int valGt) {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.insert(item);
        }
        void del(int valLt, int valGt) {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.erase(item);
        }
};

class EqIfDb {
    public:
        typedef boost::tuple<
                int  /* valCond */,
                int  /* valLt   */,
                int  /* ValGt   */,
                bool /* neg     */>
            TPred;

        typedef std::vector<TPred>                  TDst;

    private:
        typedef std::set<TPred>                     TSet;
        typedef std::map<int /* valCond */, TSet>   TMap;
        TMap cont_;

    public:
        void add(TPred pred) {
            const int valCond = pred.get<0>();
            TSet &ref = cont_[valCond];
            ref.insert(pred);
        }

        void lookupOnce(TDst &dst, int valCond) {
            TMap::iterator iter = cont_.find(valCond);
            if (cont_.end() == iter)
                // no match
                return;

            // stream out all the relevant predicates
            const TSet &ref = iter->second;
            BOOST_FOREACH(const TPred &pred, ref) {
                dst.push_back(pred);
            }

            // delete the db entry afterwards
            cont_.erase(iter);
        }
};

typedef std::vector<int>    TSub;
typedef std::set<int>       TSet;

struct Var {
    const struct cl_type    *clt;
    int /* CodeStorage */   cVarUid;
    int /* val */           placedAt;
    int /* val */           value;
    int /* var */           parent;
    TSub                    subVars;

    // TODO
    Var():
        clt(0),
        cVarUid(-1),
        placedAt(VAL_INVALID),
        value(VAL_INVALID),
        parent(VAL_INVALID)
    {
    }
};

enum EValue {
    EV_HEAP = 0,
    // TODO: EV_HEAP_OBJECT_OF_UNKNOWN_TYPE?
    EV_UNKOWN,
    EV_CUSTOM,
    EV_COMPOSITE
};

struct Value {
    EValue                  code;
    const struct cl_type    *clt;
    int /* obj */           pointsTo;
    TSet                    haveValue;

    // TODO
    Value():
        code(EV_HEAP),
        clt(0),
        pointsTo(OBJ_INVALID)
    {
    }
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
    NeqDb                   neqDb;
    EqIfDb                  eqIfDb;

    // IDs are now unique, overlapping of var/val IDs used to cause headaches
    int                     last;

    // TODO: move elsewhere
    int                     retVal;

    Private();
    void initReturn();

    void releaseValueOf(int obj);
    void indexValueOf(int obj, int val);

    int /* val */ createValue(EValue code, const struct cl_type *clt, int obj,
                              int referrer = OBJ_INVALID);
    int /* var */ createVar(const struct cl_type *clt,
                            int /* CodeStorage */ uid);

    void destroySingleVar(int var);
    void destroyVar(int var);

    void createSubs(int var, const struct cl_type *clt);
};

void SymHeap::Private::initReturn() {
    // create OBJ_RETURN
    Var &var = this->varMap[OBJ_RETURN];

    var.clt         = 0;
    var.cVarUid     = -1;
    var.placedAt    = VAL_INVALID;
    var.value       = this->createValue(EV_UNKOWN, 0, UV_UNINITIALIZED,
                                        OBJ_RETURN);
}

SymHeap::Private::Private():
    last(0),
    retVal(VAL_INVALID)
{
    this->initReturn();
}

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

void SymHeap::Private::releaseValueOf(int obj) {
    // TODO: implement
    const int val = this->varMap[obj].value;
    if (val <= 0)
        return;

    Value &ref = this->valueMap[val];
    TSet &hv = ref.haveValue;
    if (1 != hv.erase(obj))
        TRAP;
}

void SymHeap::Private::indexValueOf(int obj, int val) {
    Value &ref = this->valueMap[val];
    TSet &hv = ref.haveValue;
    hv.insert(obj);
}

int /* val */ SymHeap::Private::createValue(EValue code,
                                            const struct cl_type *clt, int obj,
                                            int referrer)
{
    const int valId = ++last;

    Value &val = valueMap[valId];
    val.code            = code;
    val.clt             = clt;
    val.pointsTo        = obj;

    if (OBJ_INVALID != referrer)
        val.haveValue.insert(referrer);

    return valId;
}

int /* var */ SymHeap::Private::createVar(const struct cl_type *clt,
                                          int /* CodeStorage */ uid)
{
    const int objId = ++(this->last);
    Var &var = this->varMap[objId];

    var.clt         = clt;
    var.cVarUid     = uid;
    var.placedAt    = this->createValue(EV_HEAP, clt, objId);
    var.value       = this->createValue(EV_UNKOWN, 0, UV_UNINITIALIZED, objId);

    return objId;
}

void SymHeap::Private::destroySingleVar(int var) {
    TVarMap::iterator varIter = this->varMap.find(var);
    if (this->varMap.end() == varIter)
        // var not found
        TRAP;

    const Var &refVar = varIter->second;
    const bool isHeapVar = (-1 == refVar.cVarUid || !refVar.clt);

    // keep haveValue() up2date
    this->releaseValueOf(var);

    // mark corresponding value as freed
    const int val = refVar.placedAt;
    Value &ref = this->valueMap[val];
    ref.pointsTo = (isHeapVar)
        ? OBJ_DELETED
        : OBJ_LOST;

    this->varMap.erase(varIter);
}

void SymHeap::Private::destroyVar(int var) {
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
        this->destroySingleVar(var);
    }
}

void SymHeap::Private::createSubs(int var, const struct cl_type *clt) {
    typedef std::pair<int /* var */, const struct cl_type *> TPair;
    typedef std::stack<TPair> TStack;
    TStack todo;

    // we use explicit stack to avoid recursion
    push(todo, var, clt);
    while (!todo.empty()) {
        int var;
        const struct cl_type *clt;
        boost::tie(var, clt) = todo.top();
        todo.pop();

        // FIXME: check whether clt may be NULL at this point
        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_ARRAY:
                CL_WARN("CL_TYPE_ARRAY is not supported by SymHeap for now");
                break;

            case CL_TYPE_CHAR:
                CL_WARN("CL_TYPE_CHAR is not supported by SymHeap for now");
                break;

            case CL_TYPE_BOOL:
            case CL_TYPE_INT:
            case CL_TYPE_PTR:
                break;

            case CL_TYPE_STRUCT: {
                const int cnt = clt->item_cnt;
                Var &ref = this->varMap[var];
                ref.value = this->createValue(EV_COMPOSITE, clt, var,
                                              /* TODO: check */ var);
                ref.subVars.resize(cnt);
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type *subClt = clt->items[i].type;
                    const int subVar = this->createVar(subClt, -1);
                    ref.subVars[i] = subVar;

                    Var &subRef = /* FIXME: suboptimal */ this->varMap[subVar];
                    subRef.parent = var;
                    push(todo, subVar, subClt);
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
            return d->createValue(EV_UNKOWN, 0, UV_DEREF_FAILED);

        case OBJ_UNKNOWN:
            TRAP;

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
    if (EV_HEAP != value.code)
        TRAP;

    return value.pointsTo;
}

void SymHeap::haveValue(TCont /* obj[] */ &dst, int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return;

    Value &value = iter->second;
    BOOST_FOREACH(int obj, value.haveValue) {
        dst.push_back(obj);
    }
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
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return 0;

    const Value &ref = iter->second;
    return ref.clt;
}

int /* CodeStorage var uid */ SymHeap::cVar(int var) const {
    TVarMap::iterator iter = d->varMap.find(var);
    if (d->varMap.end() == iter)
        return -1;

    const Var &ref = iter->second;
    return (ref.clt)
        ? ref.cVarUid
        : /* anonymous object of known size */ -1;
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

int /* var */ SymHeap::varParent(int var) const {
    TVarMap::iterator iter = d->varMap.find(var);
    if (d->varMap.end() == iter)
        return OBJ_INVALID;

    const Var &refVar = iter->second;
    return refVar.parent;
}

int /* obj */ SymHeap::valGetCompositeObj(int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        return OBJ_INVALID;

    const Value &ref = iter->second;
    return (EV_COMPOSITE == ref.code)
        ? /* FIXME: use union instead */ ref.pointsTo
        : OBJ_INVALID;
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

        case CL_TYPE_ARRAY:
            if (CL_TYPE_CHAR == clt->items[0].type->code)
                // make it possible to at least ignore strings
                break;
            // fall thorough!
        default:
            TRAP;
    }

    const int objId = d->createVar(clt, uid);
    d->createSubs(objId, clt);

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
    if (EV_HEAP != value.code)
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
    d->createSubs(var, clt);

    if (OBJ_RETURN == var)
        return;

    TValueMap::iterator valIter = d->valueMap.find(refVar.placedAt);
    if (d->valueMap.end() == valIter)
        TRAP;

    Value &value = valIter->second;
    if (value.clt || EV_HEAP != value.code)
        TRAP;

    value.clt = clt;
}

int /* sls */ SymHeap::slsCreate(const struct cl_type *clt,
                                 const struct cl_accessor *selector)
{
    // TODO
    (void) clt;
    (void) selector;
    TRAP;
    return OBJ_INVALID;
}

void SymHeap::objSetValue(int obj, int val) {
    d->releaseValueOf(obj);
    d->indexValueOf(obj, val);

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
        const Var &var = varIter->second;
        const int uid = var.cVarUid;
        if (/* heap object */ -1 != uid)
            d->cVarIdMap.erase(uid);

        d->destroyVar(obj);
        if (OBJ_RETURN == obj)
            d->initReturn();

        return;
    }

    // then look for Sls object
    // TODO
    TRAP;
}

int /* val */ SymHeap::valCreateUnknown(EUnknownValue code,
                                        const struct cl_type *clt)
{
    return d->createValue(EV_UNKOWN, clt, static_cast<int>(code));
}

EUnknownValue SymHeap::valGetUnknown(int val) const {
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        // value not found, this should never happen
        TRAP;

    Value &value = iter->second;
    return (EV_UNKOWN == value.code)
        ? static_cast<EUnknownValue>(value.pointsTo)
        : UV_KNOWN;
}

void SymHeap::valReplaceUnknown(int val, int /* val */ replaceBy) {
    typedef std::pair<int /* val */, int /* replaceBy */> TItem;
    TItem item(val, replaceBy);

    WorkList<TItem> wl(item);
    while (wl.next(item)) {
        boost::tie(val, replaceBy) = item;

        // remove inqueality if any
        d->neqDb.del(val, replaceBy);

        // collect objects having the value valDst
        TCont rlist;
        this->haveValue(rlist, val);

        // go through the list and replace the value by valSrc
        BOOST_FOREACH(const int obj, rlist) {
            this->objSetValue(obj, replaceBy);
        }

        // handle all EqIf prdicates
        // FIXME: not tested at all
        EqIfDb::TDst eqIfs;
        d->eqIfDb.lookupOnce(eqIfs, val);
        BOOST_FOREACH(const EqIfDb::TPred &pred, eqIfs) {
            int valCond, valLt, valGt; bool neg;
            boost::tie(valCond, valLt, valGt, neg) = pred;

            // deduce if the values are equal or not equal
            bool areEqual = false;
            switch (replaceBy) {
                case VAL_FALSE:
                    areEqual = neg;
                    break;

                case VAL_TRUE:
                    areEqual = !neg;
                    break;

                default:
                    TRAP;
            }

            if (areEqual)
                wl.schedule(TItem(valGt, valLt));
            else
                d->neqDb.add(valLt, valGt);
        }
    }
}

int /* val */ SymHeap::valDuplicateUnknown(int /* val */ tpl)
{
    TValueMap::iterator iter = d->valueMap.find(tpl);
    if (d->valueMap.end() == iter)
        // value not found, this should never happen
        TRAP;

    Value &value = iter->second;
    return d->createValue(EV_UNKOWN, value.clt, value.pointsTo);
}

int /* val */ SymHeap::valCreateCustom(const struct cl_type *clt, int cVal)
{
    TIdMap::iterator ii = d->cValIdMap.find(cVal);
    if (d->cValIdMap.end() == ii) {
        const int val = d->createValue(EV_CUSTOM, clt, cVal);
        d->cValIdMap[cVal] = val;
        return val;
    }

    // custom value already defined, we have to reuse it
    const int val = ii->second;
    TValueMap::iterator vi = d->valueMap.find(val);
    if (d->valueMap.end() == vi)
        TRAP;

    const Value &ref = vi->second;
    if (EV_CUSTOM != ref.code)
        // heap corruption
        TRAP;

    if (ref.clt != clt)
        // type mismatch
        TRAP;

    return val;
}

int /* cVal */ SymHeap::valGetCustom(const struct cl_type **pClt, int val) const
{
    TValueMap::iterator iter = d->valueMap.find(val);
    if (d->valueMap.end() == iter)
        // value not found, this should never happen
        TRAP;

    Value &value = iter->second;
    if (EV_CUSTOM != value.code)
        // not a custom value
        return VAL_INVALID;

    if (pClt)
        // TODO: this deserves a comment in the public header
        *pClt = value.clt;

    return /* cVal */ value.pointsTo;
}

void SymHeap::addEqIf(int valCond, int valA, int valB, bool neg) {
    if (VAL_INVALID == valA || VAL_INVALID == valB)
        TRAP;

    TValueMap::iterator iter = d->valueMap.find(valCond);
    if (d->valueMap.end() == iter)
        // valCond not found, this should never happen
        TRAP;

    const Value *pValue = &iter->second;
    if (!pValue->clt || !pValue->clt->code == CL_TYPE_BOOL)
        // valCond is not a bool value
        TRAP;

    if (EV_UNKOWN != pValue->code)
        // valCond is not an unknown value
        TRAP;

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);
    if (valB <= 0)
        // valB can't be an unknown value
        TRAP;

    iter = d->valueMap.find(valB);
    if (d->valueMap.end() == iter)
        // valB not found, this should never happen
        TRAP;

    pValue = &iter->second;
    if (EV_UNKOWN != pValue->code)
        // valB is not an unknown value
        TRAP;

    // all seems fine to store the predicate
    d->eqIfDb.add(EqIfDb::TPred(valCond, valA, valB, neg));
}

namespace {
    // FIXME: not tested
    bool proveEqBool(bool *result, int valA, int valB) {
        if ((valA == valB) && (VAL_TRUE == valA || VAL_FALSE == valA)) {
            // values are equal
            *result = true;
            return true;
        }

        // we presume (VAL_TRUE < VAL_FALSE) and (valA <= valB) at this point
        if (VAL_TRUE == valA && VAL_FALSE == valB) {
            // values are not equal
            *result = false;
            return true;
        }

        // we don't really know if the values are equal or not
        return false;
    }
}

bool SymHeap::proveEq(bool *result, int valA, int valB) const {
    if (VAL_INVALID == valA || VAL_INVALID == valB)
        // we can prove nothing for invalid values
        return false;

    if (valA == valB) {
        // value IDs match exatctly ... the prove is done
        *result = true;
        return true;
    }

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);

    // non-heap comparison of bool values
    if (proveEqBool(result, valA, valB))
        return true;

    // we presume (0 <= valA) and (0 < valB) at this point
    TValueMap::iterator iter = d->valueMap.find(valB);
    if (d->valueMap.end() == iter)
        // valB not found, this should never happen
        TRAP;

    // now look at the type of valB
    const Value &valueB = iter->second;
    const EValue code = valueB.code;
    switch (code) {
        case EV_HEAP:
        case EV_CUSTOM:
        case EV_COMPOSITE:
            // it should be safe to just the compare IDs in this case
            // NOTE: we in fact know (valA != valB) at this point, look above
            *result = (valA == valB);
            return true;

        case EV_UNKOWN:
            break;
    }

    if (d->neqDb.areNeq(valA, valB)) {
        // good luck, we have explicit info the values are not equal
        *result = false;
        return true;
    }

    // giving up, really no idea if the values are equal or not...
    return false;
}

} // namespace SymbolicHeap
