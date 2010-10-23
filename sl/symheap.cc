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
#include <cl/clutil.hh>
#include <cl/code_listener.h>

#include "symabstract.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>
#include <map>
#include <set>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#ifndef DEBUG_UNUSED_VALUES
#   define DEBUG_UNUSED_VALUES 0
#endif

// NeqDb/EqIfDb helper
template <class TDst>
void emitOne(TDst &dst, TValueId val) {
#if 0
    // the following condition seems to clash with our needs in SymPlot
    if (val <= 0)
        return;
#endif
    dst.push_back(val);
}

/// alias lookup db (fully symmetric)
class AliasDb {
    private:
        typedef std::set<TValueId>              TSet;
        typedef std::map<TValueId, TSet *>      TMap;
        TMap            cont_;
        const TSet      empty_;

    public:
        typedef TSet                            TLine;

    public:
        AliasDb() { }
        AliasDb(const AliasDb &ref) {
            this->operator=(ref);
        }

        // deep copy
        AliasDb& operator=(const AliasDb &ref) {
            BOOST_FOREACH(TMap::value_type item, ref.cont_) {
                const TSet &origin = *(item.second);
                cont_[item.first] = new TSet(origin);
            }

            return *this;
        }

        ~AliasDb() {
            // we use a set to avoid a double free during the destruction
            std::set<TSet *> done;

            BOOST_FOREACH(TMap::value_type item, cont_) {
                TSet *s = item.second;
                if (hasKey(done, s))
                    // already deleted
                    continue;

                done.insert(s);
                delete s;
            }
        }

        bool lookup(TValueId v1, TValueId v2) const {
            TMap::const_iterator iter = cont_.find(v1);
            if (cont_.end() == iter)
                return false;
            else
                return hasKey(iter->second, v2);
        }

        const TLine& getByValue(TValueId val) const {
            TMap::const_iterator iter = cont_.find(val);
            if (cont_.end() == iter)
                return empty_;
            else
                return *iter->second;
        }

        void add(TValueId v1, TValueId v2);
};

/// cluster values that are related by known offset between each other
class OffsetDb {
    public:
        typedef SymHeapCore::TOffVal                TOffVal;
        typedef SymHeapCore::TOffValCont            TOffValCont;

    private:
        typedef std::map<TOffVal, TValueId>         TOffMap;
        typedef std::map<TValueId, TOffValCont>     TValMap;
        TOffMap                 offMap_;
        TValMap                 valMap_;
        const TOffValCont       empty_;

        void addNoClobber(const TOffVal &ov, TValueId target) {
            // check for redefinition
            SE_BREAK_IF(hasKey(offMap_, ov));

            offMap_[ov] = target;
            valMap_[target].push_back(ov);
        }

    public:
        void add(TOffVal ov, TValueId target) {
            // add the given relation
            this->addNoClobber(ov, target);

            // and now the other way around
            const TValueId src = ov.first;
            ov.first = target;
            ov.second = -ov.second;
            this->addNoClobber(ov, src);
        }

        TValueId lookup(const TOffVal &ov) {
            TOffMap::iterator iter = offMap_.find(ov);
            if (offMap_.end() == iter)
                // not found
                return VAL_INVALID;

            return iter->second;
        }

        int /* offset */ lookup(TValueId v1, TValueId v2) {
            TValMap::iterator iter = valMap_.find(v1);
            if (valMap_.end() == iter)
                return /* not found */ 0;

            BOOST_FOREACH(const TOffVal &ov, iter->second) {
                if (v2 == ov.first)
                    // FIXME: check direction
                    return ov.second;
            }

            return /* not found */ 0;
        }

        const TOffValCont& getOffValues(TValueId val) {
            TValMap::iterator iter = valMap_.find(val);
            if (valMap_.end() == iter)
                // not found
                return empty_;

            return iter->second;
        }
};

class NeqDb {
    private:
        typedef std::pair<TValueId /* valLt */, TValueId /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;

    public:
        bool areNeq(TValueId valLt, TValueId valGt) const {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            return hasKey(cont_, item);
        }
        void add(TValueId valLt, TValueId valGt) {
            SE_BREAK_IF(valLt == valGt);

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.insert(item);
        }
        void del(TValueId valLt, TValueId valGt) {
            SE_BREAK_IF(valLt == valGt);

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.erase(item);
        }

        bool empty() const {
            return cont_.empty();
        }

        void killByValue(TValueId val) {
            // FIXME: suboptimal due to performance
            TCont snap(cont_);
            BOOST_FOREACH(const TItem &item, snap) {
                if (item.first != val && item.second != val)
                    continue;

                CL_DEBUG("NeqDb::killByValue(#" << val
                        << ") removed dangling Neq predicate");

                cont_.erase(item);
            }
        }

        template <class TDst>
        void gatherRelatedValues(TDst &dst, TValueId val) const {
            // FIXME: suboptimal due to performance
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

        friend bool SymHeapCore::matchPreds(const SymHeapCore &,
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
        bool empty() const {
            return cont_.empty();
        }

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
            std::copy(ref.begin(), ref.end(), std::back_inserter(dst));

            // delete the db entry afterwards
            cont_.erase(iter);
        }

        template <class TDst2>
        void gatherRelatedValues(TDst2 &dst, TValueId val) const;

        void killByValue(TValueId val) {
            if (cont_.erase(val)) {
                CL_DEBUG("EqIfDb::killByValue(#" << val
                        << ") removed dangling EqIf predicate");
            }

            // TODO
#if SE_SELF_TEST
            std::vector<TValueId> tmp;
            this->gatherRelatedValues(tmp, val);
            SE_BREAK_IF(!tmp.empty());
#endif
        }

        friend void SymHeapCore::copyRelevantPreds(SymHeapCore &dst,
                                                   const SymHeapCore::TValMap &)
                                                   const;

        friend bool SymHeapCore::matchPreds(const SymHeapCore &,
                                            const SymHeapCore::TValMap &)
                                            const;
};

template <class TDst2>
void EqIfDb::gatherRelatedValues(TDst2 &dst, TValueId val) const {
    // FIXME: suboptimal due to performance
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

void AliasDb::add(TValueId v1, TValueId v2) {
    // first look for existing aliases
    TMap::iterator i1 = cont_.find(v1);
    TMap::iterator i2 = cont_.find(v2);
    const bool hasSet1 = cont_.end() != i1;
    const bool hasSet2 = cont_.end() != i2;

    if (hasSet1 && hasSet2) {
        // connect two clusters of aliases all together
        TSet *s1 = i1->second;
        TSet *s2 = i2->second;

        // FIXME: not tested yet
#if SE_SELF_TEST
        SE_TRAP;
#endif
        // merge sets
        s1->insert(s2->begin(), s2->end());

        // replace all references
        BOOST_FOREACH(TValueId val, *s1) {
            cont_[val] = s1;
        }

        // s2 should be no more referenced at this point
        // (as long as AliasDb works correctly)
        delete s2;
    }

    if (!hasSet1 && !hasSet2) {
        TSet *s = new TSet();
        s->insert(v1);
        s->insert(v2);
        cont_[v1] = s;
        cont_[v2] = s;
        return;
    }

    if (hasSet1) {
        TSet *s = i1->second;
        s->insert(v2);
        cont_[v2] = s;
    }
    else /* if (hasSet2) */ {
        TSet *s = i2->second;
        s->insert(v1);
        cont_[v1] = s;
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
        EUnknownValue                   code;
        TObjId                          target;
        typedef std::set<TObjId>        TUsedBy;
        TUsedBy                         usedBy;

        Value(): code(UV_KNOWN), target(OBJ_INVALID) { }
    };

    std::vector<Object>     objects;
    std::vector<Value>      values;

    AliasDb                 aliasDb;
    OffsetDb                offsetDb;
    NeqDb                   neqDb;
    EqIfDb                  eqIfDb;

    void valueDestructor(TValueId value);
    void releaseValueOf(TObjId obj);
    TObjId acquireObj();
};

void SymHeapCore::Private::valueDestructor(TValueId val) {
#if DEBUG_UNUSED_VALUES
    CL_DEBUG("SymHeapCore: value #" << val << " became internally unused");
#else
    (void) val;
#endif
}

void SymHeapCore::Private::releaseValueOf(TObjId obj) {
    // this method is strictly private, range checks should be already done
    const TValueId val = this->objects[obj].value;
    if (val <= 0)
        return;

    Value::TUsedBy &uses = this->values.at(val).usedBy;
#if SE_SELF_TEST
    if (1 != uses.erase(obj))
        // *** offset detected ***
        SE_TRAP;
#else
    uses.erase(obj);
#endif

    if (uses.empty())
        this->valueDestructor(val);
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
            SE_TRAP;

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

    const Private::Value::TUsedBy &usedBy = d->values[val].usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
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
    SymHeapCore::objSetValue(obj, origin.value);

    // we've just created an object, let's notify posterity
    this->notifyResize(/* valOnly */ false);
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

    // we've just created an object, let's notify posterity
    this->notifyResize(/* valOnly */ false);
    return obj;
}

TValueId SymHeapCore::valCreate(EUnknownValue code, TObjId target) {
    // check range (we allow OBJ_INVALID here for custom values)
    SE_BREAK_IF(this->lastObjId() < target);

    // acquire value ID
    const size_t last = std::max(d->objects.size(), d->values.size());
    const TValueId val = static_cast<TValueId>(last);
    d->values.resize(val + 1);

    Private::Value &ref = d->values[val];
    ref.code = code;
    if (UV_KNOWN == code || UV_ABSTRACT == code)
        // ignore target for unknown values, they should be not followed anyhow
        ref.target = target;

    // we've just created a new value, let's notify posterity
    this->notifyResize(/* valOnly */ true);
    return val;
}

void SymHeapCore::valSetUnknown(TValueId val, EUnknownValue code) {
#if SE_SELF_TEST
    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        default:
            // please check if the caller is aware of what he's doing
            SE_TRAP;
    }
#endif

    Private::Value &ref = d->values[val];
    SE_BREAK_IF(ref.target <= 0);

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
    // check range
    SE_BREAK_IF(this->lastObjId()   < obj || obj < 0);
    SE_BREAK_IF(this->lastValueId() < val || val == VAL_INVALID);

    d->releaseValueOf(obj);
    d->objects[obj].value = val;
    if (val < 0)
        return;

    Private::Value &ref = d->values.at(val);
    ref.usedBy.insert(obj);
}

void SymHeapCore::pack() {
    const bool hasNeq  = !d->neqDb.empty();
    const bool hasEqIf = !d->eqIfDb.empty();
    if (!hasNeq && !hasEqIf)
        // no predicates found, we're done
        return;

    if (hasEqIf) {
        CL_WARN("SymHeapCore::pack() encountered an EqIf predicate, chances are"
                " that it will cause some problems elsewhere...");
    }

    const unsigned cnt = d->values.size();
    for (unsigned i = 1; i < cnt; ++i) {
        const TValueId val = static_cast<TValueId>(i);
        const Private::Value &ref = d->values[val];
        if (!ref.usedBy.empty())
            // value used, keep going...
            continue;

        if (hasNeq)
            d->neqDb.killByValue(val);

        if (hasEqIf)
            d->eqIfDb.killByValue(val);
    }
}

void SymHeapCore::objDestroy(TObjId obj, TObjId kind) {
#if SE_SELF_TEST
    // check range (we allow to destroy OBJ_RETURN)
    SE_BREAK_IF(this->lastObjId() < obj || obj < 0);

    switch (kind) {
        case OBJ_DELETED:
        case OBJ_LOST:
            break;

        default:
            SE_TRAP;
    }
#endif

    const TValueId addr = d->objects[obj].address;
    if (0 < addr)
        d->values.at(addr).target = kind;

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
    SE_BREAK_IF(this->lastValueId() < val || val < 0);

    return d->values[val].code;
}

TValueId SymHeapCore::valDuplicateUnknown(TValueId val) {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return VAL_INVALID;

    const Private::Value &ref = d->values[val];
    const TValueId valNew = this->valCreate(ref.code, ref.target);

    // we've just created a new value, let's notify posterity
    this->notifyResize(/* valOnly */ true);
    return valNew;
}

/// change value of all variables with value val to (fresh) newval
void SymHeapCore::valReplace(TValueId val, TValueId newVal) {
    // collect objects having the value val
    TContObj rlist;
    this->usedBy(rlist, val);

    // go through the list and replace the value by newval
    BOOST_FOREACH(const TObjId obj, rlist) {
        this->objSetValue(obj, newVal);
    }

    // kill Neq predicate among the pair of values (if any)
    SymHeapCore::neqOp(NEQ_DEL, val, newVal);

    // reflect the change in NeqDb
    TContValue neqs;
    d->neqDb.gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValueId neq, neqs) {
        d->neqDb.del(val, neq);
        d->neqDb.add(newVal, neq);
    }
#if SE_SELF_TEST
    // make sure we didn't create any dangling predicates
    TContValue related;
    this->gatherRelatedValues(related, val);
    if (!related.empty())
        SE_TRAP;
#endif
}

void SymHeapCore::addAlias(TValueId v1, TValueId v2) {
    d->aliasDb.add(v1, v2);
}

// template method
bool SymHeapCore::valReplaceUnknownImpl(TValueId val, TValueId replaceBy) {
    // collect objects having the value 'val'
    TContObj rlist;
    this->usedBy(rlist, val);

    // go through the list and replace the value by 'replaceBy'
    BOOST_FOREACH(const TObjId obj, rlist) {
        this->objSetValue(obj, replaceBy);
    }

    // this implementation is so easy that it really can't fail
    return true;
}

void SymHeapCore::valReplaceUnknown(TValueId val, TValueId replaceBy) {
    typedef std::pair<TValueId /* val */, TValueId /* replaceBy */> TItem;
    TItem item(val, replaceBy);

    WorkList<TItem> wl(item);
    while (wl.next(item)) {
        boost::tie(val, replaceBy) = item;

#if SE_SELF_TEST
        // ensure there hasn't been any inequality defined among the pair
        if (d->neqDb.areNeq(val, replaceBy)) {
            CL_ERROR("inconsistency detected among values #" << val
                    << " and #" << replaceBy);
            SE_TRAP;
        }
#endif

        // make it possible to override the implementation (template method)
        if (!this->valReplaceUnknownImpl(val, replaceBy)) {
            CL_WARN("overridden implementation valReplaceUnknownImpl() failed"
                    ", has to over-approximate...");
            continue;
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
                    SE_TRAP;
            }

            if (areEqual)
                wl.schedule(TItem(valGt, valLt));
            else
                this->neqOp(NEQ_ADD, valLt, valGt);
        }
    }
}

TValueId SymHeapCore::valCreateByOffset(TOffVal ov) {
    // first look if such a value already exists
    TValueId val = d->offsetDb.lookup(ov);
    if (0 < val)
        return val;

    // create a new unknown value and associate it with the offset
    val = this->valCreate(UV_UNKNOWN, /* no valid target */ OBJ_INVALID);
    d->offsetDb.add(ov, val);

    // we've just created a new value, let's notify posterity
    this->notifyResize(/* valOnly */ true);
    return val;
}

TValueId SymHeapCore::valGetByOffset(TOffVal ov) const {
    return d->offsetDb.lookup(ov);
}

void SymHeapCore::gatherOffValues(TOffValCont &dst, TValueId ref) const {
    dst = d->offsetDb.getOffValues(ref);
}

void SymHeapCore::gatherValAliasing(TContValue &dst, TValueId ref) const {
    const AliasDb::TLine &line = d->aliasDb.getByValue(ref);
    std::copy(line.begin(), line.end(), std::back_inserter(dst));
}

void SymHeapCore::neqOp(ENeqOp op, TValueId valA, TValueId valB) {
    switch (op) {
        case NEQ_NOP:
            break;

        case NEQ_ADD:
            d->neqDb.add(valA, valB);
            break;

        case NEQ_DEL:
            d->neqDb.del(valA, valB);
            break;
    }
}

namespace {
    void moveKnownValueToLeft(const SymHeapCore &sh,
                              TValueId &valA, TValueId &valB)
    {
        sortValues(valA, valB);

        if (UV_ABSTRACT == sh.valGetUnknown(valA)) {
            // UV_ABSTRACT is treated as _unkown_ value here, it has to be valB
            const TValueId tmp = valA;
            valA = valB;
            valB = tmp;
        }
    }
}

void SymHeapCore::addEqIf(TValueId valCond, TValueId valA, TValueId valB,
                          bool neg)
{
#if SE_SELF_TEST
    SE_BREAK_IF(VAL_INVALID == valA || VAL_INVALID == valB);
    SE_BREAK_IF(this->lastValueId() < valCond || valCond <= 0);

    // valCond must be an unknown value
    const Private::Value &refCond = d->values[valCond];
    SE_BREAK_IF(UV_KNOWN == refCond.code || UV_ABSTRACT == refCond.code);
#endif

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, valA, valB);

#if SE_SELF_TEST
    // valB must be an unknown value
    SE_BREAK_IF(this->lastValueId() < valB || valB <= 0);

    // valB must be an unknown value (we count UV_ABSTRACT as unknown here)
    const Private::Value &refB = d->values[valB];
    SE_BREAK_IF(UV_KNOWN == refB.code);
#endif

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

    if (valA == valB || d->aliasDb.lookup(valA, valB)) {
        // identical value IDs (or explicit aliasing) ... the prove is done
        *result = true;
        return true;
    }

    if (d->offsetDb.lookup(valA, valB)) {
        // there is an offset defined among the values, they can't be equal
        *result = false;
        return true;
    }

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, valA, valB);

    // non-heap comparison of bool values
    if (proveEqBool(result, valA, valB))
        return true;

    // we presume (0 <= valA) and (0 < valB) at this point
    SE_BREAK_IF(this->lastValueId() < valB || valB < 0);
    const Private::Value &refB = d->values[valB];

    // now look at the kind of valB
    const EUnknownValue code = refB.code;
    if (UV_KNOWN == code) {
        // it should be safe to just compare the IDs in this case
        // NOTE: it's not that easy when UV_ABSTRACT is involved

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
    // TODO: should we care about off-values here?
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
#if SE_SELF_TEST
    SE_TRAP;
#endif
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
        dst.neqOp(NEQ_ADD, valLt, valGt);
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

bool SymHeapCore::matchPreds(const SymHeapCore &ref, const TValMap &valMap)
    const
{
    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb.cont_) {
        TValueId valLt = item.first;
        TValueId valGt = item.second;
        if (!valMapLookup(valMap, &valLt) || !valMapLookup(valMap, &valGt))
            // seems like a dangling predicate, which we are not interested in
            continue;

        if (!ref.d->neqDb.areNeq(valLt, valGt))
            // Neq predicate not matched
            return false;
    }

    // go through EqIfDb
    BOOST_FOREACH(EqIfDb::TMap::const_reference item, d->eqIfDb.cont_) {
        const EqIfDb::TSet &line = item.second;
        BOOST_FOREACH(const EqIfDb::TPred &pred, line) {
            TValueId valCond, valLt, valGt; bool neg;
            boost::tie(valCond, valLt, valGt, neg) = pred;

            if (!valMapLookup(valMap, &valCond)
                    || !valMapLookup(valMap, &valLt)
                    || !valMapLookup(valMap, &valGt))
                // seems like a dangling predicate, which we are not interested in
                continue;

            const EqIfDb::TMap &peer = ref.d->eqIfDb.cont_;
            EqIfDb::TMap::const_iterator iter = peer.find(valCond);
            if (peer.end() == iter)
                // EqIf predicate not found
                return false;

            BOOST_FOREACH(const EqIfDb::TPred &peerPred, iter->second) {
                if (peerPred.get</* valCond */ 0>() == valCond
                        && peerPred.get</* valLt */ 1>() == valLt
                        && peerPred.get</* valGt */ 2>() == valGt
                        && peerPred.get</* neg */3>() == neg)
                    goto eqif_matched;
            }

            // no matching EqIf predicate found
            return false;
        }

eqif_matched:
        (void) 0;
    }

    return true;
}

// /////////////////////////////////////////////////////////////////////////////
// CVar lookup container
class CVarMap {
    private:
        typedef std::map<CVar, TObjId>              TCont;
        TCont                                       cont_;

    public:
        void insert(CVar cVar, TObjId obj) {
#if SE_SELF_TEST
            const unsigned last = cont_.size();
#endif
            cont_[cVar] = obj;

            // check for mapping redefinition
            SE_BREAK_IF(last == cont_.size());
        }

        void remove(CVar cVar) {
#if SE_SELF_TEST
            if (1 != cont_.erase(cVar))
                // *** offset detected ***
                SE_TRAP;
#else
            cont_.erase(cVar);
#endif
        }

        TObjId find(const CVar &cVar) {
            // regular lookup
            TCont::iterator iter = cont_.find(cVar);
            const bool found = (cont_.end() != iter);
            if (!cVar.inst) {
                // gl variable explicitly requested
                return (found)
                    ? iter->second
                    // avoid a compile-time warning with DEBUG_SYMID_FORCE_INT
                    : static_cast<TObjId>(OBJ_INVALID);
            }

            // automatic fallback to gl variable
            CVar gl = cVar;
            gl.inst = /* global variable */ 0;
            TCont::iterator iterGl = cont_.find(gl);
            const bool foundGl = (cont_.end() != iterGl);

            if (!found && !foundGl)
                // not found anywhere
                return OBJ_INVALID;

            // check for clash on uid among lc/gl variable
            SE_BREAK_IF(found && foundGl);

            if (found)
                return iter->second;
            else /* if (foundGl) */
                return iterGl->second;
        }

        template <class TDst>
        void getAll(TDst &dst) {
            BOOST_FOREACH(const TCont::value_type &item, cont_) {
                dst.push_back(item.first);
            }
        }

        template <class TFunctor>
        void goThroughObjs(TFunctor &f)
        {
            BOOST_FOREACH(const TCont::value_type &item, cont_) {
                f(item.second);
            }
        }
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapTyped
struct SymHeapTyped::Private {
    struct Object {
        const struct cl_type        *clt;
        size_t                      cbSize;
        CVar                        cVar;
        int                         nthItem; // -1  OR  0 .. parent.item_cnt-1
        TObjId                      parent;
        TContObj                    subObjs;
        bool                        dummy;

        Object():
            clt(0),
            cbSize(0),
            nthItem(-1),
            parent(OBJ_INVALID),
            dummy(false)
        {
        }
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

void SymHeapTyped::notifyResize(bool valOnly) {
    const size_t lastValueId = this->lastValueId();
    if (d->values.size() <= lastValueId)
        d->values.resize(lastValueId + 1);

    if (valOnly)
        // no objects created recently
        return;

    const size_t lastObjId = this->lastObjId();
    if (d->objects.size() <= lastObjId)
        d->objects.resize(lastObjId + 1);
}

TValueId SymHeapTyped::createCompValue(const struct cl_type *clt, TObjId obj) {
    const TValueId val = SymHeapCore::valCreate(UV_KNOWN, OBJ_INVALID);
    SE_BREAK_IF(VAL_INVALID == val);

    Private::Value &ref = d->values[val];
    ref.clt         = clt;
    ref.compObj     = obj;

    return val;
}

void SymHeapTyped::initValClt(TObjId obj) {
    // look for object's address
    const TValueId val = SymHeapCore::placedAt(obj);
    SE_BREAK_IF(VAL_INVALID == val);

    // initialize value's type
    const Private::Object &ref = d->objects[obj];
    d->values.at(val).clt = ref.clt;
}

TObjId SymHeapTyped::createSubVar(const struct cl_type *clt, TObjId parent) {
    const TObjId obj = SymHeapCore::objCreate();
    SE_BREAK_IF(OBJ_INVALID == obj);

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
        SE_BREAK_IF(!clt);

        if (!isComposite(clt))
            continue;

        const int cnt = clt->item_cnt;
        SymHeapCore::objSetValue(obj, this->createCompValue(clt, obj));

        // keeping a reference at this point may cause headaches in case
        // of reallocation
        d->objects[obj].subObjs.resize(cnt);
        for (int i = 0; i < cnt; ++i) {
            const struct cl_type_item *item = clt->items + i;
            const struct cl_type *subClt = item->type;
            const TObjId subObj = this->createSubVar(subClt, obj);
            d->objects[subObj].nthItem = i; // position in struct
            d->objects[obj].subObjs[i] = subObj;

            if (!item->offset) {
                // declare explicit aliasing with parent object's addr
                SymHeapCore::addAlias(this->placedAt(obj),
                                      this->placedAt(subObj));
            }

            push(todo, subObj, subClt);
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
        if (OBJ_INVALID == image)
            image = dst;

        // copy the metadata
        d->objects[dst] = d->objects[src];
        d->objects[dst].parent = item.dstParent;

        // initialize clt of its address
        this->initValClt(dst);

        // update the reference to self in the parent object
        const TObjId parent = item.dstParent;
        if (OBJ_INVALID != parent) {
            Private::Object &refParent = d->objects.at(parent);
            refParent.subObjs[item.nth] = dst;

            if (!subOffsetIn(*this, parent, dst)) {
                // declare explicit aliasing with parent object's addr
                SymHeapCore::addAlias(this->placedAt(dst),
                                      this->placedAt(parent));
            }
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
        // FIXME: should we care about CL_TYPE_UNION here?
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
    SymHeapTyped::notifyResize(/* valOnly */ false);
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

void SymHeapTyped::objSetValue(TObjId obj, TValueId val) {
#if SE_SELF_TEST
    // range check
    SE_BREAK_IF(this->lastObjId() < obj || obj < 0);

    if (!d->objects[obj].subObjs.empty())
        // invalid call of SymHeapTyped::objSetValue(), you want probably go
        // through SymProc::objSetValue()
        SE_TRAP;
#endif
    SymHeapCore::objSetValue(obj, val);
}

const struct cl_type* SymHeapTyped::objType(TObjId obj) const {
    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        // (we allow OBJ_RETURN here)
        return 0;

    return d->objects[obj].clt;
}

const struct cl_type* SymHeapTyped::valType(TValueId val) const {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point a valid obj
        return 0;

    return d->values[val].clt;
}

TValueId SymHeapTyped::valDuplicateUnknown(TValueId val) {
    if (this->lastValueId() < val || val <= 0)
        // value ID is either out of range, or does not point to a valid obj
        return VAL_INVALID;

    // duplicate the value by core
    const TValueId dup = SymHeapCore::valDuplicateUnknown(val);

    // duplicate also the type-info
    d->values.at(dup).clt = this->valType(val);

    return dup;
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
    if (this->lastObjId() < obj || obj < 0)
        // object ID is either out of range, or does not represent a valid obj
        // (we allow OBJ_RETURN here)
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

TObjId SymHeapTyped::objParent(TObjId obj, int *nth) const {
    if (this->lastObjId() < obj || obj <= 0)
        // object ID is either out of range, or does not represent a valid obj
        return OBJ_INVALID;

    const Private::Object &ref = d->objects[obj];
    const TObjId parent = ref.parent;
    if (OBJ_INVALID == parent)
        return OBJ_INVALID;

    if (nth)
        *nth = ref.nthItem;

    return parent;
}

TObjId SymHeapTyped::objCreate(const struct cl_type *clt, CVar cVar) {
    const TObjId obj = SymHeapCore::objCreate();
    if (OBJ_INVALID == obj)
        return OBJ_INVALID;

    Private::Object &ref = d->objects[obj];
    ref.clt     = clt;
    ref.cVar    = cVar;
    if (clt) {
        this->createSubs(obj);
        if (CL_TYPE_STRUCT == clt->code)
            // FIXME: should we care about CL_TYPE_UNION here?
            d->roots.push_back(obj);
    }

    if (/* heap object */ -1 != cVar.uid)
        d->cVarMap.insert(cVar, obj);

    this->initValClt(obj);
    return obj;
}

TObjId SymHeapTyped::objCreateAnon(int cbSize) {
    const TObjId obj = SymHeapCore::objCreate();
    d->objects[obj].cbSize = cbSize;

    return obj;
}

int SymHeapTyped::objSizeOfAnon(TObjId obj) const {
    SE_BREAK_IF(this->lastObjId() < obj || obj <= 0);
    const Private::Object &ref = d->objects[obj];

    // if we know the type, it's not an anonymous object
    SE_BREAK_IF(ref.clt);

    return ref.cbSize;
}

void SymHeapTyped::objDefineType(TObjId obj, const struct cl_type *clt) {
    SE_BREAK_IF(this->lastObjId() < obj || obj < 0);
    Private::Object &ref = d->objects[obj];

    // type redefinition not allowed for now
    SE_BREAK_IF(ref.clt);

    // delayed object's type definition
    ref.clt = clt;
    this->createSubs(obj);
    if (CL_TYPE_STRUCT == clt->code)
        // FIXME: should we care about CL_TYPE_UNION here?
        d->roots.push_back(obj);

    if (OBJ_RETURN == obj)
        // OBJ_RETURN has no address
        return;

    // delayed value's type definition
    this->initValClt(obj);
}

void SymHeapTyped::objDestroy(TObjId obj) {
    SE_BREAK_IF(this->lastObjId() < obj || obj < 0);
    Private::Object &ref = d->objects[obj];

    const CVar cv = ref.cVar;
    if (cv.uid != /* heap object */ -1)
        d->cVarMap.remove(cv);

    SE_BREAK_IF(OBJ_INVALID != this->objParent(obj));
    this->objDestroyPriv(obj);

    if (OBJ_RETURN == obj) {
        // (un)initialize OBJ_RETURN for next wheel
        const TValueId uv = this->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);
        SymHeapCore::objSetValue(OBJ_RETURN, uv);

        Private::Object &ref = d->objects[OBJ_RETURN];
        ref.clt = 0;
        ref.subObjs.clear();
    }
}

TValueId SymHeapTyped::valCreateUnknown(EUnknownValue code,
                                   const struct cl_type *clt)
{
    const TValueId val = SymHeapCore::valCreate(code, OBJ_UNKNOWN);
    if (VAL_INVALID == val)
        return VAL_INVALID;

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

        // initialize heap value
        Private::Value &ref = d->values[val];
        ref.clt         = clt;
        ref.isCustom    = true;
        ref.customData  = cVal;

        // store cVal --> val mapping
        d->cValueMap[cVal] = val;

        return val;
    }

    // custom value already wrapped, we have to reuse it
    const TValueId val = iter->second;

#if SE_SELF_TEST
    // check heap integrity
    const Private::Value &ref = d->values.at(val);
    SE_BREAK_IF(!ref.isCustom);

    // type-info has to match
    SE_BREAK_IF(ref.clt != clt);
#endif

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
        SegBindingFields    bf;
        bool                shared;

        ObjectEx(): kind(OK_CONCRETE), shared(true) { }
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
        // duplicate metadata of an abstract object
        Private::ObjectEx tmp(iter->second);
        d->objMap[objNew] = tmp;

        // set the pointing value's code to UV_ABSTRACT
        const TValueId addrNew = this->placedAt(objNew);
        SymHeapCore::valSetUnknown(addrNew, UV_ABSTRACT);

        // mark the address of 'head' as UV_ABSTRACT
        const TValueId addrHead = segHeadAddr(*this, objNew);
        SymHeapCore::valSetUnknown(addrHead, UV_ABSTRACT);
    }

    return objNew;
}

EObjKind SymHeap::objKind(TObjId obj) const {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    if (d->objMap.end() != iter)
        return iter->second.kind;

    const TObjId root = objRoot(*this, obj);
    if (!hasKey(d->objMap, root))
        return OK_CONCRETE;

    return (segHead(*this, root) == obj)
        ? OK_HEAD
        : OK_PART;
}

const SegBindingFields& SymHeap::objBinding(TObjId obj) const {
    const TObjId root = objRoot(*this, obj);

    Private::TObjMap::iterator iter = d->objMap.find(root);
    SE_BREAK_IF(d->objMap.end() == iter);

    return iter->second.bf;
}

bool SymHeap::objShared(TObjId obj) const {
    const TObjId root = objRoot(*this, obj);

    Private::TObjMap::iterator iter = d->objMap.find(root);
    SE_BREAK_IF(d->objMap.end() == iter);

    return iter->second.shared;
}

void SymHeap::objSetShared(TObjId obj, bool shared) {
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    SE_BREAK_IF(d->objMap.end() == iter);

    iter->second.shared = shared;
}

void SymHeap::objSetAbstract(TObjId obj, EObjKind kind,
                             const SegBindingFields &bf)
{
    SE_BREAK_IF(OK_CONCRETE == kind || hasKey(d->objMap, obj));

    // initialize abstract object
    Private::ObjectEx &ref = d->objMap[obj];
    ref.kind    = kind;
    ref.bf      = bf;

    // mark the value as UV_ABSTRACT
    const TValueId addr = this->placedAt(obj);
    SymHeapCore::valSetUnknown(addr, UV_ABSTRACT);

    // mark the address of 'head' as UV_ABSTRACT
    const TValueId addrHead = segHeadAddr(*this, obj);
    SymHeapCore::valSetUnknown(addrHead, UV_ABSTRACT);
#if SE_SELF_TEST
    // check for self-loops
    const TObjId objBind = subObjByChain(*this, obj, bf.next);
    const TValueId valNext = this->valueOf(objBind);
    SE_BREAK_IF(addr == valNext || addrHead == valNext);
#endif
}

void SymHeap::objSetConcrete(TObjId obj) {
    CL_DEBUG("SymHeap::objSetConcrete() is taking place...");
    Private::TObjMap::iterator iter = d->objMap.find(obj);
    SE_BREAK_IF(d->objMap.end() == iter);

    // mark the address of 'head' as UV_KNOWN
    const TValueId addrHead = segHeadAddr(*this, obj);
    SymHeapCore::valSetUnknown(addrHead, UV_KNOWN);

    // mark the value as UV_KNOWN
    const TValueId addr = this->placedAt(obj);
    SymHeapCore::valSetUnknown(addr, UV_KNOWN);

    // just remove the object ID from the map
    d->objMap.erase(iter);
}

bool SymHeap::valReplaceUnknownImpl(TValueId val, TValueId replaceBy) {
    const EUnknownValue code = this->valGetUnknown(val);
    switch (code) {
        case UV_KNOWN:
            SE_TRAP;
            return false;

        case UV_ABSTRACT:
            return spliceOutListSegment(*this, val, replaceBy);

        default:
            return SymHeapTyped::valReplaceUnknownImpl(val, replaceBy);
    }
}

void SymHeap::dlSegCrossNeqOp(ENeqOp op, TValueId headAddr1) {
    const TObjId head1 = this->pointsTo(headAddr1);
    const TObjId seg1 = objRoot(*this, head1);
    const TObjId seg2 = dlSegPeer(*this, seg1);
    const TValueId headAddr2 = segHeadAddr(*this, seg2);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(*this, seg1);
    const TObjId next2 = nextPtrFromSeg(*this, seg2);

    // read the values (addresses of the surround)
    const TValueId val1 = this->valueOf(next1);
    const TValueId val2 = this->valueOf(next2);

    // add/del Neq predicates
    SymHeapCore::neqOp(op, val1, headAddr2);
    SymHeapCore::neqOp(op, val2, headAddr1);

    if (NEQ_DEL == op)
        // removing the 1+ flag implies removal of the 2+ flag
        SymHeapCore::neqOp(NEQ_DEL, headAddr1, headAddr2);
}

void SymHeap::neqOp(ENeqOp op, TValueId valA, TValueId valB) {
    if (NEQ_ADD == op && haveDlSegAt(*this, valA, valB)) {
        // adding the 2+ flag implies adding of the 1+ flag
        this->dlSegCrossNeqOp(op, valA);
    }
    else {
        if (haveSeg(*this, valA, valB, OK_DLS)) {
            this->dlSegCrossNeqOp(op, valA);
            return;
        }

        if (haveSeg(*this, valB, valA, OK_DLS)) {
            this->dlSegCrossNeqOp(op, valB);
            return;
        }
    }

    SymHeapTyped::neqOp(op, valA, valB);
}

// TODO: check if the implementation is ready for Linux lists
bool SymHeap::proveEq(bool *result, TValueId valA, TValueId valB) const {
    if (SymHeapTyped::proveEq(result, valA, valB))
        return true;

    // having the values always in the same order leads to simpler code
    sortValues(valA, valB);
    if (VAL_NULL != valA)
        return false;

    EUnknownValue code = this->valGetUnknown(valB);
    if (UV_ABSTRACT != code)
        // we are interested only in abstract objects here
        return false;

    TObjId obj = this->pointsTo(valB);
    if (OK_DLS == this->objKind(obj))
        // jump to peer in case of DLS
        obj = dlSegPeer(*this, obj);

    const TObjId next = nextPtrFromSeg(*this, obj);
    const TValueId valNext = this->valueOf(next);
    if (VAL_NULL == valNext)
        // we already know that there is no Neq(valB, VAL_NULL) defined, from
        // the call of SymHeapTyped::proveEq() above
        return false;

    code = this->valGetUnknown(valNext);
    switch (code) {
        case UV_ABSTRACT:
            if (valA != VAL_NULL
                    || !segNotEmpty(*this, this->pointsTo(valNext)))
            {
                CL_WARN("SymHeap::proveEq() "
                        "does not see through a chain of segments");
                return false;
            }
            // fall through!

        case UV_KNOWN:
            // prove done
            *result = false;
            return true;

        default:
            return false;
    }
}

void SymHeap::objDestroy(TObjId obj) {
    SymHeapTyped::objDestroy(obj);
    d->objMap.erase(obj);
}
