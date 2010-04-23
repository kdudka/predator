/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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

#include "config.h"
#include "symheap.hh"

#include <cl/cl_msg.hh>
#include <cl/code_listener.h>

#include "symdump.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>
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

class NeqDb {
    private:
        typedef std::pair<TValueId /* valLt */, TValueId /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;

    public:
        bool areNeq(TValueId valLt, TValueId valGt) {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            return hasKey(cont_, item);
        }
        void add(TValueId valLt, TValueId valGt) {
            if (valLt == valGt)
                TRAP;

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.insert(item);
        }
};

class EqIfDb {
    public:
        typedef boost::tuple<
                TValueId  /* valCond */,
                TValueId  /* valLt   */,
                TValueId  /* ValGt   */,
                bool      /* neg     */>
            TPred;

        typedef std::vector<TPred>                          TDst;

    private:
        typedef std::set<TPred>                             TSet;
        typedef std::map<TValueId /* valCond */, TSet>      TMap;
        TMap cont_;

    public:
        void add(TPred pred) {
            const TValueId valCond = pred.get<0>();
            TSet &ref = cont_[valCond];
            ref.insert(pred);
        }

        void lookupOnce(TDst &dst, TValueId valCond) {
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

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapCore
struct SymHeapCore::Private {
    struct Object {
        TValueId            address;
        TValueId            value;

        Object(): address(VAL_INVALID), value(VAL_INVALID) { }
    };

    struct Value {
        EUnknownValue       code;
        TObjId              target;
        std::set<TObjId>    usedBy;

        Value(): code(UV_KNOWN), target(OBJ_INVALID) { }
    };

    std::vector<Object>     objects;
    std::vector<Value>      values;

    NeqDb                   neqDb;
    EqIfDb                  eqIfDb;

    void releaseValueOf(TObjId obj);
};

void SymHeapCore::Private::releaseValueOf(TObjId obj) {
    // this method is strictly private, range checks should be already done
    const TValueId val = this->objects[obj].value;
    if (val <= 0)
        return;

    Value &ref = this->values.at(val);
    if (1 != ref.usedBy.erase(obj))
        // *** offset detected ***
        TRAP;
}

SymHeapCore::SymHeapCore():
    d(new Private)
{
    d->objects.resize(/* OBJ_RETURN */ 1);
    d->values.resize(/* VAL_NULL */ 1);

    // (un)initialize OBJ_RETURN
    Private::Object &ref = d->objects[OBJ_RETURN];
    ref.value = this->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);

    // store backward reference to OBJ_RETURN
    Private::Value &refValue = d->values[ref.value];
    refValue.usedBy.insert(OBJ_RETURN);
}

SymHeapCore::SymHeapCore(const SymHeapCore &ref):
    d(new Private(*ref.d))
{
}

SymHeapCore::~SymHeapCore() {
    delete d;
}

SymHeapCore& SymHeapCore::operator=(const SymHeapCore &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
}

TValueId SymHeapCore::valueOf(TObjId obj) const {
    // handle special cases first
    switch (obj) {
        case OBJ_INVALID:
            return VAL_INVALID;

        case OBJ_LOST:
        case OBJ_DELETED:
        case OBJ_DEREF_FAILED:
            return /* XXX */ const_cast<SymHeapCore *>(this)
                ->valCreate(UV_DEREF_FAILED, OBJ_DEREF_FAILED);

        case OBJ_UNKNOWN:
            return /* XXX */ const_cast<SymHeapCore *>(this)
                ->valCreate(UV_UNKNOWN, OBJ_UNKNOWN);

        default:
            break;
    }

    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        return VAL_INVALID;

    const Private::Object &ref = d->objects[obj];
    return ref.value;
}

TValueId SymHeapCore::placedAt(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return VAL_INVALID;

    const Private::Object &ref = d->objects[obj];
    return ref.address;
}

TObjId SymHeapCore::pointsTo(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return OBJ_INVALID;

    const Private::Value &ref = d->values[val];
    return ref.target;
}

void SymHeapCore::usedBy(TContObj &dst, TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return;

    const Private::Value &ref = d->values[val];
    BOOST_FOREACH(TObjId obj, ref.usedBy) {
        dst.push_back(obj);
    }
}

/// return how many objects use the value
unsigned SymHeapCore::usedByCount(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return 0; // means: "not used"
    return d->values[val].usedBy.size();
}

TObjId SymHeapCore::objCreate() {
    // acquire object ID
    const size_t last = std::max(d->objects.size(), d->values.size());
    const TObjId obj = static_cast<TObjId>(last);
    d->objects.resize(obj + 1);

    // obtain value pair
    const TValueId address = this->valCreate(UV_KNOWN, obj);
    const TValueId value   = this->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);

    // keeping a reference here may cause headaches in case of reallocation
    d->objects[obj].address = address;
    d->objects[obj].value   = value;

    // store backward reference
    Private::Value &ref = d->values[value];
    ref.usedBy.insert(obj);

    return obj;
}

TValueId SymHeapCore::valCreate(EUnknownValue code, TObjId target) {
    if (this->lastObjId() < target)
        // target out of range (we allow OBJ_INVALID here for custom values)
        TRAP;

    // acquire value ID
    const size_t last = std::max(d->objects.size(), d->values.size());
    const TValueId val = static_cast<TValueId>(last);
    d->values.resize(val + 1);

    Private::Value &ref = d->values[val];
    ref.code = code;
    if (UV_KNOWN == code)
        // ignore target for unknown values, they should be not followed anyhow
        ref.target = target;

    return val;
}

TObjId SymHeapCore::lastObjId() const {
    const size_t cnt = d->objects.size() - /* safe because of OBJ_RETURN */ 1;
    return static_cast<TObjId>(cnt);
}

TValueId SymHeapCore::lastValueId() const {
    const size_t cnt = d->values.size() - /* safe because of VAL_NULL */ 1;
    return static_cast<TValueId>(cnt);
}

void SymHeapCore::objSetValue(TObjId obj, TValueId val) {
    if (this->lastObjId() < obj || obj < 0
            || this->lastValueId() < val || val == VAL_INVALID)
        // out of range
        TRAP;

    d->releaseValueOf(obj);
    d->objects[obj].value = val;
    if (val < 0)
        return;

    Private::Value &ref = d->values.at(val);
    ref.usedBy.insert(obj);
}

void SymHeapCore::objDestroy(TObjId obj, TObjId kind) {
    if (this->lastObjId() < obj || obj < 0)
        // out of range (we allow to destroy OBJ_RETURN)
        TRAP;

    switch (kind) {
        case OBJ_DELETED:
        case OBJ_LOST:
            break;

        default:
            TRAP;
    }

    if (OBJ_RETURN != obj) {
        TValueId addr = d->objects[obj].address;
        if (addr <= 0)
            // object about to be destroyed has invalid address
            TRAP;

        d->values.at(addr).target = kind;
    }

    d->releaseValueOf(obj);
    d->objects[obj].address = VAL_INVALID;
    d->objects[obj].value   = VAL_INVALID;
}

EUnknownValue SymHeapCore::valGetUnknown(TValueId val) const {
    switch (val) {
        case VAL_NULL: /* == VAL_FALSE */
        case VAL_TRUE:
            return UV_KNOWN;

        case VAL_INVALID:
            // fall through! (basically equal to "out of range")
        default:
            break;
    }
    if (this->lastValueId() < val || val < 0)
        // out of range
        TRAP;
    return d->values[val].code;
}

TValueId SymHeapCore::valDuplicateUnknown(TValueId val) {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return VAL_INVALID;

    const Private::Value &ref = d->values[val];
    return this->valCreate(ref.code, ref.target);
}

/// change value of all variables with value _val to (fresh) _newval
void SymHeapCore::valReplace(TValueId _val, TValueId _newval) {
    // assert usedByCount(_newval)==0
    if(usedByCount(_newval)>0) {
        // _newval is not fresh
        // FIXME: needs EQ/NEQ update? **** (I do not know)
    }

    // TODO: can be optimized using move of usedBy container and patch of object values

    // collect objects having the value _val
    TContObj rlist;
    this->usedBy(rlist, _val);

    // go through the list and replace the value by _newval
    BOOST_FOREACH(const TObjId obj, rlist) {
        this->objSetValue(obj, _newval);
    }
    // FIXME: solve possible problem with EQ/NEQ database records?
    //        old: any RELOP _val --> new: any !RELOP _newval ?
    // how about (in-place) change from struct to abstract segment?
}

void SymHeapCore::valReplaceUnknown(TValueId val, TValueId replaceBy) {
    typedef std::pair<TValueId /* val */, TValueId /* replaceBy */> TItem;
    TItem item(val, replaceBy);

    WorkList<TItem> wl(item);
    while (wl.next(item)) {
        boost::tie(val, replaceBy) = item;

        // ensure there hasn't been any inequality defined among the pair
        if (d->neqDb.areNeq(val, replaceBy)) {
            CL_ERROR("inconsistency detected among values #" << val
                    << " and #" << replaceBy);
            TRAP;
        }

        // collect objects having the value 'val'
        TContObj rlist;
        this->usedBy(rlist, val);

        // go through the list and replace the value by 'replaceBy'
        BOOST_FOREACH(const TObjId obj, rlist) {
            this->objSetValue(obj, replaceBy);
        }

        // handle all EqIf predicates
        EqIfDb::TDst eqIfs;
        d->eqIfDb.lookupOnce(eqIfs, val);
        BOOST_FOREACH(const EqIfDb::TPred &pred, eqIfs) {
            TValueId valCond, valLt, valGt; bool neg;
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

void SymHeapCore::addNeq(TValueId valA, TValueId valB) {
    d->neqDb.add(valA, valB);
}

void SymHeapCore::addEqIf(TValueId valCond, TValueId valA, TValueId valB,
                          bool neg)
{
    if (VAL_INVALID == valA || VAL_INVALID == valB)
        TRAP;

    if (this->lastValueId() < valCond || valCond <= 0)
        // value ID is either out of range, or invalid
        TRAP;

    // TODO: check somehow if the type of valCond is CL_TYPE_BOOL at this point
    const Private::Value &refCond = d->values[valCond];
    if (UV_KNOWN == refCond.code)
        // valCond is not an unknown value
        TRAP;

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);
    if (this->lastValueId() < valB || valB <= 0)
        // valB can't be an unknown value
        TRAP;

    const Private::Value &refB = d->values[valB];
    if (UV_KNOWN == refB.code)
        // valB is not an unknown value
        TRAP;

    // all seems fine to store the predicate
    d->eqIfDb.add(EqIfDb::TPred(valCond, valA, valB, neg));
}

namespace {
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

bool SymHeapCore::proveEq(bool *result, TValueId valA, TValueId valB) const {
    if (VAL_INVALID == valA || VAL_INVALID == valB)
        // we can prove nothing for invalid values
        return false;

    if (valA == valB) {
        // identical value IDs ... the prove is done
        *result = true;
        return true;
    }

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);

    // non-heap comparison of bool values
    if (proveEqBool(result, valA, valB)) {
        // FIXME: not tested, worth to check with a debugger first
        TRAP;
        return true;
    }

    // we presume (0 <= valA) and (0 < valB) at this point
    if (this->lastValueId() < valB || valB < 0)
        TRAP;

    const Private::Value &refB = d->values[valB];

    // now look at the kind of valB
    const EUnknownValue code = refB.code;
    if (UV_KNOWN == code) {
        // it should be safe to just compare the IDs in this case
        // NOTE: we in fact know (valA != valB) at this point, look above
        *result = /* (valA == valB) */ false;
        return true;
    }

    if (d->neqDb.areNeq(valA, valB)) {
        // good luck, we have explicit info the values are not equal
        *result = false;
        return true;
    }

    // giving up, really no idea if the values are equal or not...
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap1
struct SymHeap1::Private {
    struct Object {
        const struct cl_type        *clt;
        size_t                      cbSize;
        int /* CodeStorage */       cVar;
        int                         nth_item; // -1  OR  0 .. parent.item_cnt-1
        TObjId                      parent;
        TContObj                    subObjs;

        Object(): clt(0), cbSize(0), cVar(-1), nth_item(-1), parent(OBJ_INVALID) { }
    };

    struct Value {
        const struct cl_type        *clt;
        TObjId                      compObj;
        bool                        isCustom;
        int                         customData;

        Value(): clt(0), compObj(OBJ_INVALID), isCustom(false) { }
    };

    /// @attention this will break as soon as we allow recursion
    typedef std::map<int, TObjId>   TCVarMap;
    TCVarMap                        cVarMap;

    typedef std::map<int, TValueId> TCValueMap;
    TCValueMap                      cValueMap;

    std::vector<Object>             objects;
    std::vector<Value>              values;
};

void SymHeap1::resizeIfNeeded() {
    const size_t lastValueId = this->lastValueId();
    if (d->values.size() <= lastValueId)
        d->values.resize(lastValueId + 1);

    const size_t lastObjId = this->lastObjId();
    if (d->objects.size() <= lastObjId)
        d->objects.resize(lastObjId + 1);
}

TValueId SymHeap1::createCompValue(const struct cl_type *clt, TObjId obj) {
    const TValueId val = SymHeapCore::valCreate(UV_KNOWN, OBJ_INVALID);
    if (VAL_INVALID == val)
        TRAP;

    this->resizeIfNeeded();
    Private::Value &ref = d->values[val];
    ref.clt         = clt;
    ref.compObj     = obj;

    return val;
}

void SymHeap1::initValClt(TObjId obj) {
    // look for object's address
    const TValueId val = SymHeapCore::placedAt(obj);
    if (VAL_INVALID == val)
        TRAP;

    // initialize value's type
    const Private::Object &ref = d->objects[obj];
    d->values.at(val).clt = ref.clt;
}

TObjId SymHeap1::createSubVar(const struct cl_type *clt, TObjId parent) {
    const TObjId obj = SymHeapCore::objCreate();
    if (OBJ_INVALID == obj)
        TRAP;

    this->resizeIfNeeded();
    Private::Object &ref = d->objects[obj];
    ref.clt         = clt;
    ref.parent      = parent;

    this->initValClt(obj);
    return obj;
}

void SymHeap1::createSubs(TObjId obj) {
    const struct cl_type *clt = d->objects.at(obj).clt;

    typedef std::pair<TObjId, const struct cl_type *> TPair;
    typedef std::stack<TPair> TStack;
    TStack todo;

    // we use explicit stack to avoid recursion
    push(todo, obj, clt);
    while (!todo.empty()) {
        boost::tie(obj, clt) = todo.top();
        todo.pop();
        if (!clt)
            // missing type-info
            TRAP;

        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_ARRAY:
                CL_WARN("CL_TYPE_ARRAY is not supported by SymHeap1 for now");
                break;

            case CL_TYPE_CHAR:
                CL_WARN("CL_TYPE_CHAR is not supported by SymHeap1 for now");
                break;

            case CL_TYPE_BOOL:
            case CL_TYPE_INT:
            case CL_TYPE_PTR:
                break;

            case CL_TYPE_STRUCT: {
                const int cnt = clt->item_cnt;
                SymHeapCore::objSetValue(obj, this->createCompValue(clt, obj));

                // keeping a reference at this point may cause headaches in case
                // of reallocation
                d->objects[obj].subObjs.resize(cnt);
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type *subClt = clt->items[i].type;
                    const TObjId subObj = this->createSubVar(subClt, obj);
                    d->objects[subObj].nth_item = i; // postion in struct
                    d->objects[obj].subObjs[i] = subObj;
                    push(todo, subObj, subClt);
                }
                break;
            }

            default:
                TRAP;
        }
    }
}

// FIXME why resize?
TValueId SymHeap1::valueOf(TObjId obj) const {
    const TValueId val = SymHeapCore::valueOf(obj);
    const_cast<SymHeap1 *>(this)->resizeIfNeeded();
    return val;
}

void SymHeap1::destroyObj(TObjId obj) {
    typedef std::stack<TObjId> TStack;
    TStack todo;

    // we are using explicit stack to avoid recursion
    todo.push(obj);
    while (!todo.empty()) {
        obj = todo.top();
        todo.pop();

        // schedule all subvars for removal
        const Private::Object &ref = d->objects.at(obj);
        BOOST_FOREACH(TObjId subObj, ref.subObjs) {
            todo.push(subObj);
        }

        // remove current
        const TObjId kind = (-1 == ref.cVar)
            ? OBJ_DELETED
            : OBJ_LOST;
        SymHeapCore::objDestroy(obj, kind);
    }
}

SymHeap1::SymHeap1():
    d(new Private)
{
    this->resizeIfNeeded();
}

SymHeap1::SymHeap1(const SymHeap1 &ref):
    SymHeapCore(ref),
    d(new Private(*ref.d))
{
}

SymHeap1::~SymHeap1() {
    delete d;
}

SymHeap1& SymHeap1::operator=(const SymHeap1 &ref) {
    SymHeapCore::operator=(ref);
    delete d;
    d = new Private(*ref.d);
    return *this;
}

const struct cl_type* SymHeap1::objType(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return 0;

    return d->objects[obj].clt;
}

const struct cl_type* SymHeap1::valType(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point a valid obj
        return 0;

    return d->values[val].clt;
}

int SymHeap1::cVar(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return -1;

    return d->objects[obj].cVar;
}

/// @attention this will break as soon as we allow recursion
TObjId SymHeap1::objByCVar(int uid) const {
    Private::TCVarMap::iterator iter = d->cVarMap.find(uid);
    if (d->cVarMap.end() == iter)
        return OBJ_INVALID;
    else
        return iter->second;
}

void SymHeap1::gatherCVars(TCont &dst) const {
    Private::TCVarMap::const_iterator i;
    for (i = d->cVarMap.begin(); i != d->cVarMap.end(); ++i)
        dst.push_back(i->first);
}

TObjId SymHeap1::valGetCompositeObj(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point a valid obj
        return OBJ_INVALID;

    return d->values[val].compObj;
}

TObjId SymHeap1::subObj(TObjId obj, int nth) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return OBJ_INVALID;

    const Private::Object &ref = d->objects[obj];
    const TContObj &subs = ref.subObjs;
    const int cnt = subs.size();
    return (nth < cnt)
        ? subs[nth]
        : /* nth is out of range */ OBJ_INVALID;
}

TObjId SymHeap1::objParent(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return OBJ_INVALID;

    return d->objects[obj].parent;
}

/////////////////////////////////////////////////////////////////////////////
// utility functions
/////////////////////////////////////////////////////////////////////////////

/// count sub-objects in composite objects (single-level only)
int countSubObjects(const SymHeap1 &sh, TObjId o)
{
    const struct cl_type *clt = sh.objType(o);
    if(!clt)
        TRAP;
    const enum cl_type_e code = clt->code;
    switch (code) {
        case CL_TYPE_STRUCT:
            return clt->item_cnt;
        //TODO: ARRAY
        default:
            return 0; // not composite
    }
    TRAP;
    return 0;
}

/// return true if used as target of pointer variable(s)
bool isTarget(const SymHeap1 &sh, TObjId obj) {
    TValueId address = sh.placedAt(obj);
    return (sh.usedByCount(address) > 0); // address is used
}

/// count number of pointer targets in object [recursive]
unsigned countTargets(const SymHeap1 &sh, TObjId obj) {
    unsigned count = isTarget(sh,obj);  // 0|1
    int num_items = countSubObjects(sh,obj);
    if(num_items==0) // not a composite (structure/array)
        return count;
    // go through all items and count number of pointed ones
    // FIXME: aliasing problem (struct === first item) counted as 2 targets
    for( int i=0; i<num_items; ++i ) {
        TObjId o = sh.subObj(obj,i);
        if(o!=OBJ_INVALID)      // can be sparse
            count += countTargets(sh,o);
    }
    return count;
}

/// return true if pointer value
bool isPointer(const SymHeap1 &sh, TValueId val) {
    const struct cl_type *clt = sh.valType(val);
    if(!clt)
        TRAP;
    const enum cl_type_e code = clt->code;
    return code == CL_TYPE_PTR;
}

/// return true if pointer variable
bool isPointer(const SymHeap1 &sh, TObjId obj) {
    const struct cl_type *clt = sh.objType(obj);
    if(!clt)
        TRAP;
    const enum cl_type_e code = clt->code;
    return code == CL_TYPE_PTR;
}

/// count number of pointers in object [recursive]
unsigned countPointers(const SymHeap1 &sh, TObjId obj) {
    unsigned count = isPointer(sh,obj);  // 0|1
    int num_items = countSubObjects(sh,obj);
    if(num_items==0) // not a composite (structure/array)
        return count;
    // go through all items and count number of pointers
    for( int i=0; i<num_items; ++i ) {
        TObjId o = sh.subObj(obj,i);
        if(o!=OBJ_INVALID)      // can be sparse
            count += countPointers(sh,o);
    }
    return count;
}

/////////////////////////////////////////////////////////////////////////////

TObjId SymHeap1::objCreate(const struct cl_type *clt, int uid) {
    if(clt) {
        const enum cl_type_e code = clt->code;
        switch (code) {
            case CL_TYPE_INT:
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
                // TODO: handle also other types somehow?
                TRAP;
        }
    }

    const TObjId obj = SymHeapCore::objCreate();
    if (OBJ_INVALID == obj)
        return OBJ_INVALID;

    this->resizeIfNeeded();
    Private::Object &ref = d->objects[obj];
    ref.clt     = clt;
    ref.cVar    = uid;
    if(clt)
        this->createSubs(obj);

    if (/* heap object */ -1 != uid)
        d->cVarMap[uid] = obj;

    this->initValClt(obj);
    return obj;
}

TObjId SymHeap1::objCreateAnon(int cbSize) {
    const TObjId obj = SymHeapCore::objCreate();
    this->resizeIfNeeded();
    d->objects[obj].cbSize = cbSize;

    return obj;
}

int SymHeap1::objSizeOfAnon(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        TRAP;

    const Private::Object &ref = d->objects[obj];
    if (ref.clt)
        // we know the type, so that it does not look like an anonymous object
        TRAP;

    return ref.cbSize;
}

bool SymHeap1::valPointsToAnon(TValueId val) const {
    if (val <= 0)
        return false;

    const TObjId obj = SymHeapCore::pointsTo(val);
    if (obj <= 0)
        // value does not point to anything (OBJ_RETURN also makes no sense)
        return false;

    const Private::Object &ref = d->objects.at(obj);
    return !ref.clt;
}

void SymHeap1::objDefineType(TObjId obj, const struct cl_type *clt) {
    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        TRAP;

    Private::Object &ref = d->objects[obj];
    if (ref.clt)
        // type redefinition not allowed for now
        TRAP;

    // delayed object's type definition
    ref.clt = clt;
    this->createSubs(obj);

    if (OBJ_RETURN == obj)
        // OBJ_RETURN has no address
        return;

    // delayed value's type definition
    this->initValClt(obj);
}

void SymHeap1::objDestroy(TObjId obj) {
    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        TRAP;

    Private::Object &ref = d->objects[obj];
    const int uid = ref.cVar;
    if (uid != /* heap object */ -1
            && 1 != d->cVarMap.erase(uid))
        // *** offset detected while removing cVar reference ***
        TRAP;

    this->destroyObj(obj);
    if (OBJ_RETURN == obj) {
        // (un)initialize OBJ_RETURN for next wheel
        const TValueId uv = this->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);
        SymHeapCore::objSetValue(OBJ_RETURN, uv);
        d->objects[OBJ_RETURN].clt = 0;
    }
}

TValueId SymHeap1::valCreateUnknown(EUnknownValue code,
                                   const struct cl_type *clt)
{
    const TValueId val = SymHeapCore::valCreate(code, OBJ_UNKNOWN);
    if (VAL_INVALID == val)
        return VAL_INVALID;

    this->resizeIfNeeded();
    d->values[val].clt = clt;
    return val;
}

TValueId SymHeap1::valCreateCustom(const struct cl_type *clt, int cVal) {
    Private::TCValueMap::iterator iter = d->cValueMap.find(cVal);
    if (d->cValueMap.end() == iter) {
        // cVal not found, create a new wrapper for it
        const TValueId val = SymHeapCore::valCreate(UV_KNOWN, OBJ_INVALID);
        if (VAL_INVALID == val)
            return VAL_INVALID;

        this->resizeIfNeeded();
        Private::Value &ref = d->values[val];
        ref.clt         = clt;
        ref.isCustom    = true;
        ref.customData  = cVal;

        return val;
    }

    // custom value already wrapped, we have to reuse it
    const TValueId val = iter->second;
    const Private::Value &ref = d->values.at(val);
    if (!ref.isCustom)
        // heap corruption
        TRAP;

    if (ref.clt != clt)
        // type mismatch
        TRAP;

    return val;
}

int SymHeap1::valGetCustom(const struct cl_type **pClt, TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return -1;

    const Private::Value &ref = d->values[val];
    if (!ref.isCustom)
        // not a custom value
        return -1;

    if (pClt)
        // provide type info if requested to do so
        *pClt = ref.clt;

    return ref.customData;
}


/// return abstract offset of object in struct
int  SymHeap1::nthItemOf(TObjId o) const {
    return d->objects[o].nth_item;
}



















// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap2

// FIXME: make clone operation explicit, use SymHeap* as handle only

struct SymHeap2::Private {
    struct Object {
        TObjKind        kind;
        const struct cl_type *clt;      // SymHeap1 clt is 0 for error detection
        int             nextid;
        TAbstractLen    alen;
        TObjId          lambda;

        Object(): kind(SLS), clt(0), nextid(-1), alen(PE), lambda(OBJ_INVALID) { }
    };

    std::map<TObjId,Object> sl_segments;
};


/// create an empty symbolic heap
SymHeap2::SymHeap2() :
    d(new Private)
{
}

/// destruction of the symbolic heap invalidates all IDs of its entities
SymHeap2::~SymHeap2() {
    delete d;
}

/// copy construction
SymHeap2::SymHeap2(const SymHeap2 &o) :
    SymHeap1(o),
    d(new Private(*o.d))
{
    //CL_WARN("***copy_ctr\n") ;
}

/// assign heaps
SymHeap2& SymHeap2::operator=(const SymHeap2 &o)
{
    if(&o==this)
        return *this;
    SymHeap1::operator=(o);
    delete d;
    d = new Private(*o.d);
    return *this;
}

// methods

/// get kind of object
//TODO: move to base class
TObjKind SymHeap2::objKind(TObjId obj) const {
    if(d->sl_segments.count(obj)==0)
       return VAR; // non-abstract
    return d->sl_segments[obj].kind; // abstract object
}

/// test if object is abstract segment
bool SymHeap2::objIsAbstract(TObjId obj) const {
    return objKind(obj)!=VAR;
}

/// get identification of next pointer in structure
int SymHeap2::slsGetNextId(TObjId obj) const {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    return d->sl_segments[obj].nextid;
}


/// returns id of prototype object (lambda representation)  TODO: can be undefined?
TObjId SymHeap2::slsGetLambdaId(TObjId obj) const {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    return d->sl_segments[obj].lambda;
}

/// sets id of prototype object (lambda representation)  TODO: can be undefined?
void SymHeap2::slsSetLambdaId(TObjId obj, TObjId lambda) {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    d->sl_segments[obj].lambda=lambda;
}

/// get abstract length of listsegment
TAbstractLen SymHeap2::slsGetLength(TObjId obj) const {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    return d->sl_segments[obj].alen;
}

void SymHeap2::slsSetLength(TObjId obj, TAbstractLen alen) {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    d->sl_segments[obj].alen=alen;
}

// return type info for listsegment
const struct cl_type * SymHeap2::slsType(TObjId obj) const {
    if(d->sl_segments.count(obj)==0)
        TRAP;
    return d->sl_segments[obj].clt;
}


/// concretize abstract segment - nonempty variant
TObjId SymHeap2::Concretize_NE(TObjId ao)
{
    if(!objIsAbstract(ao))
        TRAP;

    // TODO: should we keep the same aoID?

    // 1. create struct (clone prototype?)
    const struct cl_type * clt = slsType(ao);
    TObjId o = objCreate(clt,-1);  // create structure


    // 2. recycle listsegment, change to possibly empty
    slsSetLength(ao,PE);

    // 3. relink all pointer2segment to pointer2struct
    valReplace(placedAt(ao),placedAt(o));

    // 4. link sruct-next to new listsegment
    TObjId next = subObj(o,slsGetNextId(ao));
    objSetValue(next,placedAt(ao));

    return o;
}


//TODO: method of top SymHeap will be better layer for this kind of operation
/// concretize abstract segment - empty variant
void SymHeap2::Concretize_E(TObjId abstract_object)
{
    // checking if possible
    if(slsGetLength(abstract_object) != PE)
        TRAP;

    // get values
    TValueId v = valueOf(abstract_object);   // we expect correct type of pointer value
    TValueId a = placedAt(abstract_object);

    // join pointer2abstract-value and next-value
    // TODO: verify
    valReplace(a,v);  // all users of a now use v

    // delete listsegment object
    slsDestroy(abstract_object);

    // WARNING: resulting value can be abstract, too
}


/// concretize abstract object pointed by ptr value
void Concretize(SymHeap &sh, TObjId &ao, std::list<SymHeap> &todo)
{
    if (!sh.objIsAbstract(ao))
        TRAP;                   // something is wrong at caller

    // ASSERT: ao is target abstract object

    // 1. create empty variant (possibly empty segments only)
    if(sh.slsGetLength(ao) == PE) {
        SymHeap sh0(sh);        // clone
        sh0.Concretize_E(ao);
        todo.push_back(sh0);    // process this state later
    }

    // 2. nonempty variant - modify existing heap
    ao = sh.Concretize_NE(ao);
}

/// dig root object
TObjId objRoot(SymHeap1 &sh, TObjId obj) {
    TObjId root = obj, next;
    while (OBJ_INVALID != (next = sh.objParent(root)))
        root = next;
    return root;
}

/// compare types and return common type pointer if equal (or 0 if not)
const cl_type * slsCompatibleType(SymHeap2 &sh, TObjId a, TObjId b) {
    const cl_type * atype = sh.objType(a);         // type of struct or 0 for segment
    const cl_type * btype = sh.objType(b);
    TObjKind akind = sh.objKind(a);
    TObjKind bkind = sh.objKind(b);
    if (akind != VAR)
        atype = sh.slsType(a);
    if (bkind != VAR)
        btype = sh.slsType(b);

    if(!atype || !btype)
        TRAP;   // FIXME: something suspicious in our type-info subsystem

    if (atype!=btype)
        return 0;

    if (atype->code!=CL_TYPE_STRUCT)    // we can abstract structs only
        return 0;

    return atype;       // type for both objects
}

/// abstract two objects connected by given value if possible
void SymHeap2::Abstract(TValueId ptrValue)
{
    // TODO: this is too simple

    // check, if abstraction possible:
    if (this->usedByCount(ptrValue) != 1)
        TRAP;                   // precondition failed

    // TODO: function for this special case?
    TContObj set;
    usedBy(set, ptrValue);
    TObjId firstn = set[0];     // value referenced by this sub-object only

    TObjId first = objRoot(*this, firstn);      // whole struct id
    if (first == firstn && !objIsAbstract(first))
        return;                 // not a structure, nor a segment

    // link pointers can point INSIDE structs (TODO)
    TObjId secondtarget = pointsTo(ptrValue);   // pointer target id
    TObjId second = objRoot(*this, secondtarget);       // whole struct id

    if (first == second)
        return;                 // cyclic list can not be abstracted

    // first some type checking
    const cl_type *common_type = slsCompatibleType(*this, first, second);
    if (!common_type)
        return;                 // can not abstract nonhomogeneous list

    TObjKind firstkind  = objKind(first);
    TObjKind secondkind = objKind(second);
    // there are 4 possible variants:
    // struct struct
    // struct seg
    // seg    struct
    // seg    seg

// TODO: add more abstract operations and simplify next "spaghetti" code

    if (firstkind == VAR && secondkind == VAR)
    {
//CL_WARN("Abstract(value) VAR+VAR ********************************");
        // FIXME: only simplest case works for now
        // 1. determine nextptr
        int nextid = nthItemOf(firstn);
        if (nextid == -1)
            TRAP;
        // 2. check, if single item pointed
        // TODO: abstract sublists etc
        if (countTargets(*this, first) != 1)
            return;
        if (countTargets(*this, second) != 1)
            return;

        // FIXME: check pointers from structs
        //        only nextptr allowed now
        if (countPointers(*this, first) != 1)
            return;
        if (countPointers(*this, second) != 1)
            return;

        // create nonempty listsegment from 2 structs
        // lsNE(first,second.nextptr)
        TObjId s = slsCreate(common_type, nextid, NE);
        TObjId nextptr = subObj(second, nextid);
        objSetValue (s, valueOf(nextptr));

        TValueId firstaddr   = placedAt(first); // TODO: offset
        TValueId segmentaddr = placedAt(s);
        valReplace(firstaddr, segmentaddr);

        // TODO: prototype values and lambda

        // cleaning
        objDestroy(first);
        objDestroy(second);

//        CL_WARN("abstract struct+struct --- success");
        return;
    }
    else if (firstkind == VAR && secondkind == SLS)
    {
//CL_WARN("Abstract(value) VAR+SLS ********************************");
        // 1. determine nextptr
        int nextid = nthItemOf(firstn);
        if (nextid == -1)
            TRAP;
        if(nextid != slsGetNextId(second))
            return;     // struct and segment not compatible

        // 2. check, if single item pointed, etc
        // TODO: abstract sublists etc
        if (countTargets(*this, first) != 1)
            return;
        // FIXME: check pointers from structs
        //        only nextptr allowed now
        if (countPointers(*this, first) != 1)
            return;

        // extend listsegment to nonempty
        // lsNE(first,second.nextptr)
        TValueId firstaddr   = placedAt(first); // TODO: offset
        TValueId segmentaddr = placedAt(second);
        valReplace(firstaddr, segmentaddr);
        slsSetLength(second,NE);

        // TODO: prototype values and lambda

        // cleaning
        objDestroy(first);

//        CL_WARN("abstract struct+sls --- success");
        return;
    }
    else if (firstkind == SLS && secondkind == VAR)
    {
//CL_WARN("Abstract(value) SLS+VAR ********************************");
        // 1. determine nextptr
        int nextid = slsGetNextId(first);
        if (nextid == -1)
            TRAP;

        // 2. check, if single item pointed
        // TODO: abstract sublists etc
        if (countTargets(*this, second) != 1)
            return;

        // FIXME: check pointers from structs
        //        only nextptr allowed now
        if (countPointers(*this, second) != 1)
            return;

        //TODO: check struct for nextptr

        // extend listsegment to nonempty
        // lsNE(first,second.nextptr)
        TObjId nextptr = subObj(second, nextid);
        objSetValue (first, valueOf(nextptr));
        slsSetLength(first,NE);

        // TODO: prototype values and lambda

        // cleaning
        objDestroy(second);

//        CL_WARN("abstract sls+struct --- success");
        return;
    }
    else if (firstkind == SLS && secondkind == SLS)
    {
//CL_WARN("Abstract(value) SLS+SLS ********************************");
        // 1. determine nextptr
        int nextid = slsGetNextId(first);
        if (nextid == -1)
            TRAP;
        if(nextid!=slsGetNextId(second))
            return;     // incompatible listsegments

        // TODO: prototype values and lambda

        // 2 listsegments to 1
        // lsNE(first,second.nextptr)
        objSetValue (first, valueOf(second));
        if(slsGetLength(second)==NE)
            slsSetLength(first,NE);     // else leave old length

        // cleaning
        objDestroy(second);

//        CL_WARN("abstract sls+sls --- success");
        return;
    }
}

/// search te heap and abstract all objects possible
void Abstract(SymHeap &sh)
{
    // TODO:   better interface?   SymHeap Abstract(const SymHeap&); // copy
    int i;
    for(i=0; i<=sh.lastObjId(); ++i) {
        TObjId o = static_cast<TObjId>(i);
        TValueId addr = sh.placedAt(o);
        if(addr==VAL_INVALID)
            continue;   // no address value => invalid object id

        const struct cl_type *clt = sh.objType(o);
        if(!clt && sh.objIsAbstract(o))
            clt = sh.slsType(o);

        if(!clt)        // FIXME: *** why is this possible?
            continue;   // no type-info

        if(clt->code != CL_TYPE_STRUCT)
            continue;   // we can abstract structs only

        // object is of type struct variable or segment
        if(sh.usedByCount(addr)==1)     // single use of object address
            sh.Abstract(addr);  // it changes the heap (TODO: check if not a problem)
    } // for each object-id
}

/// create sls, needs to set value and lambda later
TObjId SymHeap2::slsCreate(const struct cl_type *clt, int nextid, TAbstractLen alen) {
    TObjId id = objCreate(/* no type */ 0, /* heap-object */ -1);
    d->sl_segments[id].clt = clt;
    d->sl_segments[id].nextid = nextid;
    d->sl_segments[id].alen = alen;
    d->sl_segments[id].kind = SLS;
    d->sl_segments[id].lambda = OBJ_INVALID;
    return id;
}

/// clone sls object
TObjId SymHeap2::slsClone(TObjId ls) {
    // TODO: assert
    const cl_type *typ = slsType(ls);

    // create ls
    TObjId id = slsCreate(typ, d->sl_segments[ls].nextid, d->sl_segments[ls].alen);
    // set next ptr value
    objSetValue(id, valueOf(ls));
    // clone all prototype-values

    d->sl_segments[ls].lambda = d->sl_segments[id].lambda;
    //TODO: check completness

    return id;
}

/// delete sls
void SymHeap2::slsDestroy(TObjId id) {
    // TODO: check if unlinked
    d->sl_segments.erase(id);
    objDestroy(id);
}

// vim: tw=120
