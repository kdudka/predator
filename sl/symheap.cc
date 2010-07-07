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

// NeqDb/EqIfDb helper
template <class TDst>
void emitOne(TDst &dst, TValueId val) {
#if 0
    // the following condition seems to clash with our needs in SymHeapPlotter
    if (val <= 0)
        return;
#endif
    dst.push_back(val);
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
        void del(TValueId valLt, TValueId valGt) {
            if (valLt == valGt)
                TRAP;

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.erase(item);
        }

        template <class TDst>
        void gatherRelatedValues(TDst &dst, TValueId val) const {
            // FIXME: suboptimal due to performace
            BOOST_FOREACH(const TItem &item, cont_) {
                if (item.first == val)
                    emitOne(dst, item.second);
                else if (item.second == val)
                    emitOne(dst, item.first);
            }
        }

        friend void SymHeapCore::copyRelevantPreds(SymHeapCore &dst,
                                                   const SymHeapCore::TValMap &)
                                                   const;
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

        template <class TDst2>
        void gatherRelatedValues(TDst2 &dst, TValueId val) const;

        friend void SymHeapCore::copyRelevantPreds(SymHeapCore &dst,
                                                   const SymHeapCore::TValMap &)
                                                   const;
};

template <class TDst2>
void EqIfDb::gatherRelatedValues(TDst2 &dst, TValueId val) const {
    // FIXME: suboptimal due to performace
    TMap::const_iterator iter;
    for (iter = cont_.begin(); iter != cont_.end(); ++iter) {
        const TSet &line = iter->second;
        BOOST_FOREACH(const TPred &pred, line) {
            TValueId valCond, valLt, valGt; bool neg;
            boost::tie(valCond, valLt, valGt, neg) = pred;
            // FIXME: some are not realistic
            // TODO: prune and write some documentation of what is
            // possible and what is not
            if (valCond == val) {
                emitOne(dst, valLt);
                emitOne(dst, valGt);
            }
            if (valLt == val) {
                emitOne(dst, valCond);
                emitOne(dst, valGt);
            }
            if (valGt == val) {
                emitOne(dst, valCond);
                emitOne(dst, valLt);
            }
        }
    }
}

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
    TObjId acquireObj();
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

TObjId SymHeapCore::Private::acquireObj() {
    const size_t last = std::max(this->objects.size(), this->values.size());
    const TObjId obj = static_cast<TObjId>(last);
    this->objects.resize(obj + 1);
    return obj;
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
            return VAL_DEREF_FAILED;

        case OBJ_UNKNOWN:
            // not implemented
            TRAP;

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

TObjId SymHeapCore::objDup(TObjId objOrigin) {
    // acquire a new object
    const TObjId obj = d->acquireObj();
    d->objects[obj].address = this->valCreate(UV_KNOWN, obj);

    // copy the value inside, while keeping backward references etc.
    const Private::Object &origin = d->objects.at(objOrigin);
    this->objSetValue(obj, origin.value);
    return obj;
}

TObjId SymHeapCore::objCreate() {
    // acquire object ID
    const TObjId obj = d->acquireObj();

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
    if (UV_KNOWN == code || UV_ABSTRACT == code)
        // ignore target for unknown values, they should be not followed anyhow
        ref.target = target;

    return val;
}

void SymHeapCore::valSetUnknown(TValueId val, EUnknownValue code) {
    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        default:
            // please check if the caller is aware of what he's doing
            TRAP;
    }

    Private::Value &ref = d->values[val];
    if (ref.target <= 0)
        TRAP;

    ref.code = code;
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

    // TODO: can be optimized using move of usedBy container and patch of object
    // values

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
    this->delNeq(_val, _newval);
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

void SymHeapCore::delNeq(TValueId valA, TValueId valB) {
    // just for debugging purposes
#if 0
    if (!d->neqDb.areNeq(valA, valB))
        TRAP;
#endif
    d->neqDb.del(valA, valB);
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
    if (UV_KNOWN == refCond.code || UV_ABSTRACT == refCond.code)
        // valCond is not an unknown value
        TRAP;

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);
    if (this->lastValueId() < valB || valB <= 0)
        // valB can't be an unknown value
        TRAP;

    const Private::Value &refB = d->values[valB];
    if (UV_KNOWN == refB.code)
        // valB is not an unknown value (we count UV_ABSTRACT as unknown here)
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

    if (UV_ABSTRACT == this->valGetUnknown(valA)) {
        // UV_ABSTRACT is treated as _unkown_ value here, it has to be valB
        const TValueId tmp = valA;
        valA = valB;
        valB = tmp;
    }

    // we presume (0 <= valA) and (0 < valB) at this point
    if (this->lastValueId() < valB || valB < 0)
        TRAP;

    const Private::Value &refB = d->values[valB];

    // now look at the kind of valB
    const EUnknownValue code = refB.code;
    if (UV_KNOWN == code) {
        // it should be safe to just compare the IDs in this case
        // NOTE: it's not that when UV_ABSTRACT is involved

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

void SymHeapCore::gatherRelatedValues(TContValue &dst, TValueId val) const {
    d->neqDb.gatherRelatedValues(dst, val);
    d->eqIfDb.gatherRelatedValues(dst, val);
}

template <class TValMap>
bool valMapLookup(const TValMap &valMap, TValueId *pVal) {
    if (*pVal <= VAL_NULL)
        // special values always match, no need for mapping
        return true;

    typename TValMap::const_iterator iter = valMap.find(*pVal);
    if (valMap.end() == iter)
        // mapping not found
        return false;

    // match
    *pVal = iter->second;
    return true;
}

template <class TValMap>
void copyRelevantEqIf(SymHeapCore &dst, const EqIfDb::TPred &pred,
                      const TValMap &valMap)
{
    // FIXME: this code has never run, let's go with a debugger first
    TRAP;

    TValueId valCond, valLt, valGt; bool neg;
    boost::tie(valCond, valLt, valGt, neg) = pred;

    if (!valMapLookup(valMap, &valCond)
            || !valMapLookup(valMap, &valLt)
            || !valMapLookup(valMap, &valGt))
        // not relevant
        return;

    // create the image now!
    dst.addEqIf(valCond, valLt, valGt, neg);
}

void SymHeapCore::copyRelevantPreds(SymHeapCore &dst, const TValMap &valMap)
    const
{
    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb.cont_) {
        TValueId valLt = item.first;
        TValueId valGt = item.second;

        if (!valMapLookup(valMap, &valLt) || !valMapLookup(valMap, &valGt))
            // not relevant
            continue;

        // create the image now!
        dst.addNeq(valLt, valGt);
    }

    // go through EqIfDb
    EqIfDb::TMap &db = d->eqIfDb.cont_;
    EqIfDb::TMap::const_iterator iter;
    for (iter = db.begin(); iter != db.end(); ++iter) {
        const EqIfDb::TSet &line = iter->second;
        BOOST_FOREACH(const EqIfDb::TPred &pred, line) {
            copyRelevantEqIf(dst, pred, valMap);
        }
    }
}

// /////////////////////////////////////////////////////////////////////////////
// CVar lookup container
class CVarMap {
    private:
        typedef std::map<CVar, TObjId>              TMap;
        typedef std::pair<TObjId, int /* cnt */>    TGlItem;
        typedef std::map<int /* uid */, TGlItem>    TGlMap;

        TMap    lc_;
        TGlMap  gl_;

    public:
        void insert(CVar cVar, TObjId obj) {
            lc_[cVar] = obj;

            // gl scope
            TGlItem &ref = gl_[cVar.uid];
            ref.first = obj;
            ref.second ++;
        }

        void remove(CVar cVar) {
            if (1 != lc_.erase(cVar))
                // *** offset detected ***
                TRAP;

            // gl scope
            TGlMap::iterator iter = gl_.find(cVar.uid);
            if (gl_.end() == iter)
                // *** offset detected ***
                TRAP;

            TGlItem &ref = iter->second;
            if (0 == --ref.second)
                gl_.erase(iter);
        }

        TObjId find(CVar cVar) {
            TMap::iterator iterLc = lc_.find(cVar);
            if (lc_.end() != iterLc)
                return iterLc->second;

            // gl scope
            TGlMap::iterator iterGl = gl_.find(cVar.uid);
            if (gl_.end() == iterGl)
                // not found even there
                return OBJ_INVALID;

            const TGlItem &ref = iterGl->second;
            return ref.first;
        }

        template <class TCont>
        void getAll(TCont &dst) {
            TMap::const_iterator i;
            for (i = lc_.begin(); i != lc_.end(); ++i)
                dst.push_back(i->first);
        }

        template <class TFunctor>
        void goThroughObjs(TFunctor &f)
        {
            TMap::const_iterator i;
            for (i = lc_.begin(); i != lc_.end(); ++i)
                f(i->second);
        }
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapTyped
struct SymHeapTyped::Private {
    struct Object {
        const struct cl_type        *clt;
        size_t                      cbSize;
        CVar                        cVar;
        //int                       nth_item; // -1  OR  0 .. parent.item_cnt-1
        TObjId                      parent;
        TContObj                    subObjs;

        Object(): clt(0), cbSize(0), parent(OBJ_INVALID) { }
    };

    struct Value {
        const struct cl_type        *clt;
        TObjId                      compObj;
        bool                        isCustom;
        int                         customData;

        Value(): clt(0), compObj(OBJ_INVALID), isCustom(false) { }
    };

    CVarMap                         cVarMap;

    typedef std::map<int, TValueId> TCValueMap;
    TCValueMap                      cValueMap;

    std::vector<Object>             objects;
    std::vector<Value>              values;
    TContObj                        roots;
};

void SymHeapTyped::resizeIfNeeded() {
    const size_t lastValueId = this->lastValueId();
    if (d->values.size() <= lastValueId)
        d->values.resize(lastValueId + 1);

    const size_t lastObjId = this->lastObjId();
    if (d->objects.size() <= lastObjId)
        d->objects.resize(lastObjId + 1);
}

TValueId SymHeapTyped::createCompValue(const struct cl_type *clt, TObjId obj) {
    const TValueId val = SymHeapCore::valCreate(UV_KNOWN, OBJ_INVALID);
    if (VAL_INVALID == val)
        TRAP;

    this->resizeIfNeeded();
    Private::Value &ref = d->values[val];
    ref.clt         = clt;
    ref.compObj     = obj;

    return val;
}

void SymHeapTyped::initValClt(TObjId obj) {
    // look for object's address
    const TValueId val = SymHeapCore::placedAt(obj);
    if (VAL_INVALID == val)
        TRAP;

    // initialize value's type
    const Private::Object &ref = d->objects[obj];
    d->values.at(val).clt = ref.clt;
}

TObjId SymHeapTyped::createSubVar(const struct cl_type *clt, TObjId parent) {
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

void SymHeapTyped::createSubs(TObjId obj) {
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
                SymHeapCore::objSetValue(obj, this->createCompValue(clt, obj));

                // keeping a reference at this point may cause headaches in case
                // of reallocation
                d->objects[obj].subObjs.resize(cnt);
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type *subClt = clt->items[i].type;
                    const TObjId subObj = this->createSubVar(subClt, obj);
                    //d->objects[subObj].nth_item = i; // postion in struct
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

struct ObjDupStackItem {
    TObjId  srcObj;
    TObjId  dstParent;
    int     nth;
};
TObjId SymHeapTyped::objDup(TObjId obj) {
    CL_DEBUG("SymHeapTyped::objDup() is taking place...");
    TObjId image = OBJ_INVALID;

    ObjDupStackItem item;
    item.srcObj = obj;
    item.dstParent = OBJ_INVALID;

    std::stack<ObjDupStackItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        // duplicate a single object
        const TObjId src = item.srcObj;
        const TObjId dst = SymHeapCore::objDup(src);
        this->resizeIfNeeded();
        if (OBJ_INVALID == image)
            image = dst;

        // copy the metadata
        d->objects[dst] = d->objects[src];
        d->objects[dst].parent = item.dstParent;

        // initialize clt of its address
        this->initValClt(dst);

        // update the reference to self in the parent object
        if (OBJ_INVALID != item.dstParent) {
            Private::Object &refParent = d->objects.at(item.dstParent);
            refParent.subObjs[item.nth] = dst;
        }

        const TContObj subObjs(d->objects[src].subObjs);
        if (subObjs.empty())
            continue;

        // assume composite object
        const struct cl_type *clt = d->objects[src].clt;
        SymHeapCore::objSetValue(dst, this->createCompValue(clt, dst));

        // traverse composite types recursively
        for (unsigned i = 0; i < subObjs.size(); ++i) {
            item.srcObj     = subObjs[i];
            item.dstParent  = dst;
            item.nth        = i;
            todo.push(item);
        }
    }

    const Private::Object &ref = d->objects[obj];
    if (ref.clt && ref.clt->code == CL_TYPE_STRUCT && -1 == ref.parent)
        // if the original was a root object, the new one must also be
        d->roots.push_back(image);

    return image;
}

void SymHeapTyped::objDestroyPriv(TObjId obj) {
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
        const TObjId kind = (-1 == ref.cVar.uid)
            ? OBJ_DELETED
            : OBJ_LOST;
        SymHeapCore::objDestroy(obj, kind);
    }

    // remove self from roots (if ever there)
    remove_if(d->roots.begin(), d->roots.end(),
            bind2nd(std::equal_to<TObjId>(), obj));
}

SymHeapTyped::SymHeapTyped():
    d(new Private)
{
    this->resizeIfNeeded();
}

SymHeapTyped::SymHeapTyped(const SymHeapTyped &ref):
    SymHeapCore(ref),
    d(new Private(*ref.d))
{
}

SymHeapTyped::~SymHeapTyped() {
    delete d;
}

SymHeapTyped& SymHeapTyped::operator=(const SymHeapTyped &ref) {
    SymHeapCore::operator=(ref);
    delete d;
    d = new Private(*ref.d);
    return *this;
}

const struct cl_type* SymHeapTyped::objType(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return 0;

    return d->objects[obj].clt;
}

const struct cl_type* SymHeapTyped::valType(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point a valid obj
        return 0;

    return d->values[val].clt;
}

bool SymHeapTyped::cVar(CVar *dst, TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return false;

    const CVar &cVar = d->objects[obj].cVar;
    if (-1 == cVar.uid)
        // looks like a heap object
        return false;

    if (dst)
        // return its identification if requested to do so
        *dst = cVar;

    // non-heap object
    return true;
}

TObjId SymHeapTyped::objByCVar(CVar cVar) const {
    return d->cVarMap.find(cVar);
}

void SymHeapTyped::gatherCVars(TContCVar &dst) const {
    d->cVarMap.getAll(dst);
}

void SymHeapTyped::gatherRootObjs(TContObj &dst) const {
    dst = d->roots;
}

TObjId SymHeapTyped::valGetCompositeObj(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point a valid obj
        return OBJ_INVALID;

    return d->values[val].compObj;
}

TObjId SymHeapTyped::subObj(TObjId obj, int nth) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return OBJ_INVALID;

    const Private::Object &ref = d->objects[obj];
    const TContObj &subs = ref.subObjs;
    const int cnt = subs.size();

    // this helps to avoid a warning when compiling with DEBUG_SYMID_FORCE_INT
    static const TObjId OUT_OF_RANGE = OBJ_INVALID;

    return (nth < cnt)
        ? subs[nth]
        : /* nth is out of range */ OUT_OF_RANGE;
}

TObjId SymHeapTyped::objParent(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return OBJ_INVALID;

    return d->objects[obj].parent;
}

TObjId subObjByChain(const SymHeapTyped &sh, TObjId obj, TFieldIdxChain ic) {
    BOOST_FOREACH(const int nth, ic) {
        obj = sh.subObj(obj, nth);
        if (OBJ_INVALID == obj)
            break;
    }

    return obj;
}

TObjId SymHeapTyped::objCreate(const struct cl_type *clt, CVar cVar) {
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
    ref.cVar    = cVar;
    if(clt) {
        this->createSubs(obj);
        if (CL_TYPE_STRUCT == clt->code)
            d->roots.push_back(obj);
    }

    if (/* heap object */ -1 != cVar.uid)
        d->cVarMap.insert(cVar, obj);

    this->initValClt(obj);
    return obj;
}

TObjId SymHeapTyped::objCreateAnon(int cbSize) {
    const TObjId obj = SymHeapCore::objCreate();
    this->resizeIfNeeded();
    d->objects[obj].cbSize = cbSize;

    return obj;
}

int SymHeapTyped::objSizeOfAnon(TObjId obj) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        TRAP;

    const Private::Object &ref = d->objects[obj];
    if (ref.clt)
        // we know the type, so that it does not look like an anonymous object
        TRAP;

    return ref.cbSize;
}

void SymHeapTyped::objDefineType(TObjId obj, const struct cl_type *clt) {
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

void SymHeapTyped::objDestroy(TObjId obj) {
    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        TRAP;

    Private::Object &ref = d->objects[obj];
    const CVar cv = ref.cVar;
    if (cv.uid != /* heap object */ -1)
        d->cVarMap.remove(cv);

    this->objDestroyPriv(obj);
    if (OBJ_RETURN == obj) {
        // (un)initialize OBJ_RETURN for next wheel
        const TValueId uv = this->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);
        SymHeapCore::objSetValue(OBJ_RETURN, uv);
        d->objects[OBJ_RETURN].clt = 0;
    }
}

TValueId SymHeapTyped::valCreateUnknown(EUnknownValue code,
                                   const struct cl_type *clt)
{
    const TValueId val = SymHeapCore::valCreate(code, OBJ_UNKNOWN);
    if (VAL_INVALID == val)
        return VAL_INVALID;

    this->resizeIfNeeded();
    d->values[val].clt = clt;
    return val;
}

TValueId SymHeapTyped::valCreateCustom(const struct cl_type *clt, int cVal) {
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

int SymHeapTyped::valGetCustom(const struct cl_type **pClt, TValueId val) const
{
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

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap
struct SymHeap::Private {
    struct ObjectEx {
        EObjKind            kind;
        int                 level;
        TFieldIdxChain      icBind;
        TFieldIdxChain      icPeer;

        ObjectEx(): kind(OK_CONCRETE), level(-1) { }
    };

    typedef std::map<TObjId, ObjectEx> TObjMap;
    TObjMap objMap;
};

SymHeap::SymHeap():
    d(new Private)
{
}

SymHeap::SymHeap(const SymHeap &ref):
    SymHeapTyped(ref),
    d(new Private(*ref.d))
{
}

SymHeap::~SymHeap() {
    delete d;
}

SymHeap& SymHeap::operator=(const SymHeap &ref) {
    SymHeapTyped::operator=(ref);
    delete d;
    d = new Private(*ref.d);
    return *this;
}

TObjId SymHeap::objDup(TObjId objOld) {
    const TObjId objNew = SymHeapTyped::objDup(objOld);
    Private::TObjMap::iterator iter = d->objMap.find(objOld);
    if (d->objMap.end() != iter) {
        // duplicte metadata of an abstract object
        Private::ObjectEx tmp(iter->second);
        d->objMap[objNew] = tmp;

        // set the pointing value's code to UV_ABSTRACT
        const TValueId addrNew = this->placedAt(objNew);
        SymHeapCore::valSetUnknown(addrNew, UV_ABSTRACT);
    }

    return objNew;
}

EObjKind SymHeap::objKind(TObjId obj) const {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    return (d->objMap.end() == iter)
        ? OK_CONCRETE
        : iter->second.kind;
}

int SymHeap::objAbstractLevel(TObjId obj) const {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    return (d->objMap.end() == iter)
        ? /* OK_CONCRETE */ 0
        : iter->second.level;
}

TFieldIdxChain SymHeap::objBinderField(TObjId obj) const {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    if (d->objMap.end() == iter)
        // invalid call of SymHeap::objBinderField()
        TRAP;

    return iter->second.icBind;
}

TFieldIdxChain SymHeap::objPeerField(TObjId obj) const {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    if (d->objMap.end() == iter)
        // invalid call of SymHeap::objPeerField()
        TRAP;

    return iter->second.icPeer;
}

void SymHeap::objAbstract(TObjId obj, EObjKind kind, TFieldIdxChain icBind,
                          TFieldIdxChain icPeer)
{
    if (hasKey(d->objMap, obj))
        // invalid call of SymHeap::objAbstract()
        TRAP;

    // mark the value as UV_ABSTRACT
    const TValueId addr = this->placedAt(obj);
    SymHeapCore::valSetUnknown(addr, UV_ABSTRACT);

    const TObjId objBind = subObjByChain(*this, obj, icBind);
    const TValueId valNext = this->valueOf(objBind);
    if (addr == valNext)
        // *** self-loop detected ***
        TRAP;

    // initialize abstract object
    Private::ObjectEx &ref = d->objMap[obj];
    ref.kind    = kind;
    ref.level   = /* TODO */ 1;
    ref.icBind  = icBind;
    ref.icPeer  = icPeer;

    // TODO: go through all nested abstract objects and increase the level
}

void SymHeap::objConcretize(TObjId obj) {
    CL_DEBUG("SymHeap::objConcretize() is taking place...");
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    if (d->objMap.end() == iter)
        // invalid call of SymHeap::objConcretize()
        TRAP;

    if (1 != iter->second.level)
        // not implemented
        TRAP;

    // just remove the object ID from the map
    d->objMap.erase(iter);

    // mark the value as UV_KNOWN
    const TValueId addr = this->placedAt(obj);
    SymHeapCore::valSetUnknown(addr, UV_KNOWN);

    // TODO: check if really necessary;  it should be caller's responsibility
#if 0
    // we have just concretized an object, the pointing value can't be NULL by
    // definition --> let's remove the Neq(addr, NULL) if such a predicate
    // exists
    this->delNeq(VAL_NULL, addr);
#endif
}

// vim: tw=80
