/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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
#include <cl/storage.hh>

#include "symabstract.hh"
#include "symdump.hh"
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

template <class TMap>
typename TMap::mapped_type& roMapLookup(
        TMap                                &roMap,
        const typename TMap::key_type       id)
{
    CL_BREAK_IF(id < 0);
    typename TMap::iterator iter = roMap.find(id);
    CL_BREAK_IF(roMap.end() == iter);
    return iter->second;
}

// /////////////////////////////////////////////////////////////////////////////
// Neq predicates store
class NeqDb {
    private:
        typedef std::pair<TValId /* valLt */, TValId /* valGt */> TItem;
        typedef std::set<TItem> TCont;
        TCont cont_;

    public:
        bool areNeq(TValId valLt, TValId valGt) const {
            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            return hasKey(cont_, item);
        }
        void add(TValId valLt, TValId valGt) {
            CL_BREAK_IF(valLt == valGt);

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.insert(item);
        }
        void del(TValId valLt, TValId valGt) {
            CL_BREAK_IF(valLt == valGt);

            sortValues(valLt, valGt);
            TItem item(valLt, valGt);
            cont_.erase(item);
        }

        bool empty() const {
            return cont_.empty();
        }

        template <class TDst>
        void gatherRelatedValues(TDst &dst, TValId val) const {
            // FIXME: suboptimal due to performance
            BOOST_FOREACH(const TItem &item, cont_) {
                if (item.first == val)
                    dst.push_back(item.second);
                else if (item.second == val)
                    dst.push_back(item.first);
            }
        }

        friend void SymHeapCore::copyRelevantPreds(SymHeapCore &dst,
                                                   const SymHeapCore::TValMap &)
                                                   const;

        friend bool SymHeapCore::matchPreds(const SymHeapCore &,
                                            const SymHeapCore::TValMap &)
                                            const;
};

// /////////////////////////////////////////////////////////////////////////////
// CVar lookup container
class CVarMap {
    private:
        typedef std::map<CVar, TObjId>              TCont;
        TCont                                       cont_;

    public:
        void insert(CVar cVar, TObjId obj) {
#ifndef NDEBUG
            const unsigned last = cont_.size();
#endif
            cont_[cVar] = obj;

            // check for mapping redefinition
            CL_BREAK_IF(last == cont_.size());
        }

        void remove(CVar cVar) {
#ifndef NDEBUG
            if (1 != cont_.erase(cVar))
                // *** offset detected ***
                CL_TRAP;
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
                    : OBJ_INVALID;
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
            CL_BREAK_IF(found && foundGl);

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
// implementation of SymHeapCore
struct SymHeapCore::Private {
    struct Object {
        TValId                      value;
        TObjType                    clt;
        int                         nthItem; // -1  OR  0 .. parent.item_cnt-1
        TObjId                      root;
        TObjId                      parent;
        TObjList                    subObjs;
        TOffset                     off;

        Object():
            value(VAL_INVALID),
            clt(0),
            nthItem(-1),
            root(OBJ_INVALID),
            parent(OBJ_INVALID),
            off(0)
        {
        }
    };

    typedef std::set<TObjId>                            TUsedBy;
    typedef std::map<TOffset, TValId>                   TOffMap;

    // symheap-ng
    typedef std::map<TObjType, TObjId>                  TObjByType;
    typedef std::map<TOffset, TObjByType>               TGrid;

    struct Root {
        TValId                      addr;
        size_t                      cbSize;
        CVar                        cVar;
        std::set<TObjId>            livePtrs;
        std::set<TObjId>            liveData;
        bool                        isProto;
        TUsedBy                     usedBy;

        // symheap-ng
        TGrid                       grid;

        Root():
            addr(VAL_NULL),
            cbSize(0),
            isProto(false)
        {
        }
    };

    struct Value {
        EUnknownValue               code;
        TObjId                      target;
        TUsedBy                     usedBy;
        TValId                      valRoot;
        TOffset                     offRoot;
        TObjId                      compObj;
        bool                        isCustom;
        int                         customData;
        TOffMap                     offMap;


        Value():
            code(UV_KNOWN),
            target(OBJ_INVALID),
            valRoot(VAL_INVALID),
            offRoot(0),
            compObj(OBJ_INVALID),
            isCustom(false)
        {
        }
    };

    CVarMap                         cVarMap;

    typedef std::map<int, TValId>   TCValueMap;
    TCValueMap                      cValueMap;

    std::vector<Object>             objects;
    std::vector<Value>              values;

    typedef std::map<TObjId, Root>  TRootMap;
    TRootMap                        roots;

    NeqDb                           neqDb;

    inline TValId lastValId();
    inline TObjId lastObjId();
    template <typename T> T lastId();

    inline bool valOutOfRange(TValId);
    inline bool objOutOfRange(TObjId);

    inline TValId valRoot(const TValId, const Value &);
    inline TValId valRoot(const TValId);

    inline TObjId objRoot(const TObjId, const Object &);
    inline TObjId objRoot(const TObjId);

    TValId valCreate(EUnknownValue code, TObjId target);
    TObjId objCreate();
    TObjId objCreate(TObjType clt, CVar cVar);
    TObjId objDup(TObjId root);
    void objDestroy(TObjId obj);

    void releaseValueOf(TObjId obj);
    void setValueOf(TObjId of, TValId val);

    void subsCreate(TObjId obj);
    void subsDestroy(TObjId obj);

    TObjId rootLookup(TValId val);
    bool gridLookup(TObjId *pFailCode, const TObjByType **pRow, const TValId);
};

inline TValId SymHeapCore::Private::lastValId() {
    return static_cast<TValId>(this->values.size());
}

inline TObjId SymHeapCore::Private::lastObjId() {
    return static_cast<TObjId>(this->objects.size());
}

template <typename T> T SymHeapCore::Private::lastId() {
    const size_t last = std::max(this->objects.size(), this->values.size());
    return static_cast<T>(last);
}

inline bool SymHeapCore::Private::valOutOfRange(TValId val) {
    return (val < 0)
        || (this->lastValId() <= val);
}

inline bool SymHeapCore::Private::objOutOfRange(TObjId obj) {
    return (obj < 0)
        || (this->lastObjId() <= obj);
}

/// optimized variant, use it in case you already have &valData
inline TValId SymHeapCore::Private::valRoot(const TValId val, const Value &valData) {
    const TValId valRoot = valData.valRoot;
    if (VAL_INVALID == valRoot)
        return val;

    CL_BREAK_IF(VAL_NULL == valRoot);
    CL_BREAK_IF(valOutOfRange(valRoot));
    return valRoot;
}

inline TValId SymHeapCore::Private::valRoot(const TValId val) {
    CL_BREAK_IF(VAL_NULL == val);
    CL_BREAK_IF(valOutOfRange(val));

    const Value &valData = this->values[val];
    return this->valRoot(val, valData);
}

/// optimized variant, use it in case you already have &objData
inline TObjId SymHeapCore::Private::objRoot(const TObjId obj, const Object &objData) {
    const TObjId objRoot = objData.root;
    if (OBJ_INVALID == objRoot)
        return obj;

    CL_BREAK_IF(objOutOfRange(objRoot));
    return objRoot;
}

inline TObjId SymHeapCore::Private::objRoot(const TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    const Object &objData = this->objects[obj];
    return this->objRoot(obj, objData);
}

void SymHeapCore::Private::releaseValueOf(TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    const TValId val = this->objects[obj].value;
    if (val <= 0)
        // we do not track uses of special values
        return;

    CL_BREAK_IF(valOutOfRange(val));
    TUsedBy &uses = this->values[val].usedBy;
#ifndef NDEBUG
    if (1 != uses.erase(obj))
        // *** offset detected ***
        CL_TRAP;
#else
    uses.erase(obj);
#endif

    const TObjId root = this->rootLookup(val);
    if (root < 0)
        return;

    Private::Root &rootData = roMapLookup(this->roots, root);
#ifndef NDEBUG
    if (1 != rootData.usedBy.erase(obj))
        // *** offset detected ***
        CL_TRAP;
#else
    rootData.usedBy.erase(obj);
#endif
}

void SymHeapCore::Private::setValueOf(TObjId obj, TValId val) {
    this->releaseValueOf(obj);
    this->objects[obj].value = val;
    if (val <= 0)
        return;

    CL_BREAK_IF(valOutOfRange(val));
    Private::Value &valData = this->values[val];
    valData.usedBy.insert(obj);

    const TObjId root = this->rootLookup(val);
    if (root < 0)
        return;

    Private::Root &rootData = roMapLookup(this->roots, root);
    rootData.usedBy.insert(obj);
}

TObjId SymHeapCore::Private::objCreate() {
    // acquire object ID
    const TObjId obj = this->lastId<TObjId>();
    this->objects.resize(obj + 1);
    return obj;
}

TValId SymHeapCore::Private::valCreate(EUnknownValue code, TObjId target) {
    // acquire value ID
    const TValId val = this->lastId<TValId>();
    this->values.resize(val + 1);

    Private::Value &valData = this->values[val];
    valData.code        = code;
    valData.target      = target;

    return val;
}

// FIXME: should this be declared non-const?
TValId SymHeapCore::valueOf(TObjId obj) const {
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
            CL_TRAP;

        default:
            break;
    }

    CL_BREAK_IF(d->objOutOfRange(obj));
    Private::Object &objData = d->objects[obj];

    TValId &val = objData.value;
    if (VAL_INVALID != val)
        // the object has a value
        return val;

    if (isComposite(objData.clt)) {
        // deleayed creation of a composite value
        val = d->valCreate(UV_KNOWN, OBJ_INVALID);
        d->values[val].compObj = obj;
    }
    else
        // deleayed creation of an uninitialized value
        val = d->valCreate(UV_UNINITIALIZED, OBJ_UNKNOWN);

    // store backward reference
    d->values[val].usedBy.insert(obj);
    return val;
}

void SymHeapCore::usedBy(TObjList &dst, TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        return;

    const Private::TUsedBy &usedBy = d->values[val].usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

void SymHeapCore::pointedBy(TObjList &dst, TValId root) const {
    CL_BREAK_IF(d->valOutOfRange(root));

    const Private::Value &valData = d->values[root];
    CL_BREAK_IF(VAL_INVALID != valData.valRoot);

    const Private::Root &rootData = roMapLookup(d->roots, valData.target);
    const Private::TUsedBy &usedBy = rootData.usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

/// return how many objects use the value
unsigned SymHeapCore::usedByCount(TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        return 0;

    const Private::TUsedBy &usedBy = d->values[val].usedBy;
    return usedBy.size();
}

unsigned SymHeapCore::lastId() const {
    return d->lastId<unsigned>();
}

TValId SymHeapCore::valCreateDangling(TObjId kind) {
    CL_BREAK_IF(OBJ_DELETED != kind && OBJ_LOST != kind);
    return d->valCreate(UV_KNOWN, kind);
}

TValId SymHeapCore::valClone(TValId val) {
    CL_BREAK_IF(VAL_NULL == val);
    CL_BREAK_IF(d->valOutOfRange(val));
    const Private::Value &valData = d->values[val];

    // check unknown value code
    const EUnknownValue code = valData.code;
    switch (code) {
        case UV_UNKNOWN:
        case UV_UNINITIALIZED:
        case UV_DONT_CARE:
            return d->valCreate(valData.code, valData.target);

        case UV_KNOWN:
        case UV_ABSTRACT:
            break;
    }

    // are we duplicating an object that does not exist?
    const TObjId root = d->rootLookup(val);
    switch (root) {
        case OBJ_DELETED:
        case OBJ_LOST:
            return this->valCreateDangling(root);

        default:
            CL_BREAK_IF(root <= 0);
    }

    // duplicate the root object
    const TObjId dup = d->objDup(root);
    const TValId dupAt = this->placedAt(dup);

    // take the offset into consideration
    const TOffset off = this->valOffset(val);
    return this->valByOffset(dupAt, off);
}

void SymHeapCore::valMerge(TValId val, TValId replaceBy) {
    CL_BREAK_IF(this->proveNeq(val, replaceBy));
    moveKnownValueToLeft(*this, replaceBy, val);
    this->valReplace(val, replaceBy);
}

template <class TValMap>
bool valMapLookup(const TValMap &valMap, TValId *pVal) {
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

void SymHeapCore::Private::subsCreate(TObjId obj) {
    const TObjId root = obj;

    // initialize grid's root clt
    TObjType clt = this->objects[obj].clt;
    Private::TGrid &grid = this->roots[root].grid;
    grid[/* off */ 0][clt] = obj;

    typedef std::pair<TObjId, TObjType> TPair;
    typedef std::stack<TPair> TStack;
    TStack todo;

    // we use explicit stack to avoid recursion
    push(todo, obj, clt);
    while (!todo.empty()) {
        boost::tie(obj, clt) = todo.top();
        todo.pop();
        CL_BREAK_IF(!clt);

        if (!isComposite(clt))
            continue;

        const int cnt = clt->item_cnt;

        // keeping a reference at this point may cause headaches in case
        // of reallocation
        this->objects[obj].subObjs.resize(cnt);
        for (int i = 0; i < cnt; ++i) {
            const struct cl_type_item *item = clt->items + i;
            TObjType subClt = item->type;
            const TObjId subObj = this->objCreate();
            this->objects[subObj].clt           = subClt;
            this->objects[subObj].parent        = obj;
            this->objects[subObj].root          = root;
            this->objects[subObj].nthItem       = i; // position in struct
            this->objects[obj].subObjs[i]       = subObj;

            const TOffset off = item->offset;
            const TOffset offTotal = off + this->objects[obj].off;
            this->objects[subObj].off = offTotal;
            grid[offTotal][subClt] = subObj;

            push(todo, subObj, subClt);
        }
    }
}

struct ObjDupStackItem {
    TObjId  srcObj;
    TObjId  dstParent;
    int     nth;
};
TObjId SymHeapCore::Private::objDup(TObjId root) {
    CL_DEBUG("SymHeapCore::Private::objDup() is taking place...");
    CL_BREAK_IF(objOutOfRange(root));
    CL_BREAK_IF(OBJ_INVALID != this->objects[root].root);

    TObjId image = OBJ_INVALID;

    ObjDupStackItem item;
    item.srcObj = root;
    item.dstParent = OBJ_INVALID;

    std::stack<ObjDupStackItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        // duplicate a single object
        const TObjId src = item.srcObj;
        const TObjId dst = this->objCreate();
        const Private::Object &origin = this->objects.at(src);
        this->setValueOf(dst, origin.value);

        // copy the metadata
        this->objects[dst] = this->objects[src];
        this->objects[dst].parent = item.dstParent;
        if (OBJ_INVALID == image) {
            // root object
            image = dst;
            this->objects[dst].root = OBJ_INVALID;

            const TObjType cltRoot = this->objects[root].clt;
            this->roots[image].grid[/* off */0][cltRoot] = image;
            this->roots[image].cVar    = this->roots[root].cVar;
            this->roots[image].isProto = this->roots[root].isProto;
        }
        else if (hasKey(this->roots[root].livePtrs, src))
            this->roots[image].livePtrs.insert(dst);
        else if (hasKey(this->roots[root].liveData, src))
            this->roots[image].liveData.insert(dst);

        // update the reference to self in the parent object
        const TObjId parent = item.dstParent;
        if (OBJ_INVALID != parent) {
            // recover root
            this->objects[dst].root = image;

            CL_BREAK_IF(objOutOfRange(parent));
            this->objects[parent].subObjs[item.nth] = dst;

            // recover grid
            const TOffset off = this->objects[dst].off;
            const TObjType clt = this->objects[dst].clt;
            Private::TGrid &grid = this->roots[image].grid;
            grid[off][clt] = dst;
        }

        const TObjList subObjs(this->objects[src].subObjs);
        if (subObjs.empty())
            continue;

        // assume composite object
        this->setValueOf(dst, VAL_INVALID);

        // traverse composite types recursively
        for (unsigned i = 0; i < subObjs.size(); ++i) {
            item.srcObj     = subObjs[i];
            item.dstParent  = dst;
            item.nth        = i;
            todo.push(item);
        }
    }

    return image;
}

void SymHeapCore::gatherLivePointers(TObjList &dst, TValId atAddr) const {
    const TObjId root = const_cast<SymHeapCore *>(this)->objAt(atAddr);
    const Private::Root &rootData = roMapLookup(d->roots, root);
    std::copy(rootData.livePtrs.begin(), rootData.livePtrs.end(),
            std::back_inserter(dst));
}

void SymHeapCore::gatherLiveObjects(TObjList &dst, TValId atAddr) const {
    const TObjId root = const_cast<SymHeapCore *>(this)->objAt(atAddr);
    const Private::Root &rootData = roMapLookup(d->roots, root);

    std::copy(rootData.livePtrs.begin(), rootData.livePtrs.end(),
            std::back_inserter(dst));
    std::copy(rootData.liveData.begin(), rootData.liveData.end(),
            std::back_inserter(dst));
}

void SymHeapCore::Private::subsDestroy(TObjId root) {
    typedef std::stack<TObjId> TStack;
    TStack todo;

    // we are using explicit stack to avoid recursion
    todo.push(root);
    while (!todo.empty()) {
        const TObjId obj = todo.top();
        todo.pop();

        // schedule all subvars for removal
        CL_BREAK_IF(objOutOfRange(obj));
        const Private::Object &objData = this->objects[obj];
        BOOST_FOREACH(TObjId subObj, objData.subObjs) {
            todo.push(subObj);
        }

        // remove current
        this->releaseValueOf(obj);
        this->objects[obj].value   = VAL_INVALID;
    }

    // remove self from roots
    if (/* XXX */ OBJ_RETURN != root)
        this->roots.erase(root);
}

SymHeapCore::SymHeapCore(TStorRef stor):
    stor_(stor),
    d(new Private)
{
    CL_BREAK_IF(!&stor_);

    // define specials
    d->objects.resize(/* OBJ_RETURN */ 1);
    d->values.resize(/* VAL_NULL */ 1);

    // XXX
    d->roots[OBJ_RETURN];
}

SymHeapCore::SymHeapCore(const SymHeapCore &ref):
    stor_(ref.stor_),
    d(new Private(*ref.d))
{
    CL_BREAK_IF(!&stor_);
}

SymHeapCore::~SymHeapCore() {
    delete d;
}

SymHeapCore& SymHeapCore::operator=(const SymHeapCore &ref) {
    CL_BREAK_IF(&stor_ != &ref.stor_);

    delete d;
    d = new Private(*ref.d);
    return *this;
}

void SymHeapCore::swap(SymHeapCore &ref) {
    CL_BREAK_IF(&stor_ != &ref.stor_);
    swapValues(this->d, ref.d);
}

void SymHeapCore::objSetValue(TObjId obj, TValId val) {
    CL_BREAK_IF(d->objOutOfRange(obj));
    CL_BREAK_IF(!d->objects[obj].subObjs.empty());

    // seek root
    Private::Object &objData = d->objects[obj];
    const TObjId root = d->objRoot(obj, objData);
    Private::Root &rootData = roMapLookup(d->roots, root);

    if (isDataPtr(objData.clt))
        rootData.livePtrs.insert(obj);
    else
        rootData.liveData.insert(obj);

    d->setValueOf(obj, val);
}

TObjType SymHeapCore::objType(TObjId obj) const {
    if (d->objOutOfRange(obj))
        return 0;

    return d->objects[obj].clt;
}

TValId SymHeapCore::valByOffset(TValId at, TOffset off) {
    if (!off || at <= 0)
        return at;

    // subtract the root
    const TValId valRoot = d->valRoot(at);
    off += d->values[at].offRoot;
    if (!off)
        return valRoot;

    // off-value lookup
    const Private::TOffMap &offMap = d->values[valRoot].offMap;
    Private::TOffMap::const_iterator it = offMap.find(off);
    if (offMap.end() != it)
        return it->second;

    // TODO: check if there is a valid target
    EUnknownValue code = this->valGetUnknown(valRoot);
    if (off < 0)
        // we ended up above the root
        code = UV_UNKNOWN;

    // create a new off-value
    const TValId val = d->valCreate(code, OBJ_UNKNOWN);
    d->values[val].valRoot = valRoot;
    d->values[val].offRoot = off;

    // NOTE: do not use the 'offMap' reference at this point anymore, chances
    // are that the 'd->values' vector was reallocated meanwhile
    d->values[valRoot].offMap[off] = val;
    return val;
}

EValueTarget SymHeapCore::valTarget(TValId val) const {
    if (val <= 0)
        return VT_INVALID;

    CL_BREAK_IF(d->valOutOfRange(val));
    const Private::Value &valData = d->values[val];
    if (valData.isCustom)
        return VT_CUSTOM;

    TObjId target = valData.target;
    switch (target) {
        case OBJ_DELETED:
            return VT_DELETED;

        case OBJ_LOST:
            return VT_LOST;

        case OBJ_UNKNOWN:
            // either unknown value, or off-value
            break;

        case OBJ_RETURN:
            // this happens in case a composite value is returned from a
            // function;  the expected output of test-0090 prior to this commit
            // was wrongly assuming that this area is dynamically allocated
            return VT_ON_STACK;

        default:
            CL_BREAK_IF(target <= 0);
    }

    const EUnknownValue code = this->valGetUnknown(val);
    switch (code) {
        case UV_KNOWN:
            break;

        case UV_ABSTRACT:
            return VT_ABSTRACT;

        case UV_UNINITIALIZED:
        case UV_UNKNOWN:
        case UV_DONT_CARE:
            return VT_UNKNOWN;
    }

    const TValId valRoot = d->valRoot(val);
    target = d->values[valRoot].target;

    CL_BREAK_IF(target < 0);
    const Private::Root &rootData = d->roots[target];
    const int uid = rootData.cVar.uid;
    if (-1 == uid)
        return VT_ON_HEAP;

    if (isOnStack(stor_.vars[uid]))
        return VT_ON_STACK;
    else
        return VT_STATIC;
}

bool SymHeapCore::isAbstract(EValueTarget code) {
    return (VT_ABSTRACT == code);
}

bool SymHeapCore::isOnHeap(EValueTarget code) {
    switch (code) {
        case VT_ON_HEAP:
        case VT_ABSTRACT:
            return true;

        default:
            return false;
    }
}

bool SymHeapCore::isProgramVar(EValueTarget code) {
    switch (code) {
        case VT_STATIC:
        case VT_ON_STACK:
            return true;

        default:
            return false;
    }
}

TValId SymHeapCore::valRoot(TValId val) const {
    if (val <= 0)
        return val;

    return d->valRoot(val);
}

TOffset SymHeapCore::valOffset(TValId val) const {
    if (val <= 0)
        return 0;

    CL_BREAK_IF(d->valOutOfRange(val));
    const Private::Value &valData = d->values[val];
    return valData.offRoot;
}

/// change value of all variables with value val to (fresh) newval
void SymHeapCore::valReplace(TValId val, TValId newVal) {
    // collect objects having the value val
    TObjList rlist;
    this->usedBy(rlist, val);

    // go through the list and replace the value by newval
    BOOST_FOREACH(const TObjId obj, rlist) {
        this->objSetValue(obj, newVal);
    }

    // kill Neq predicate among the pair of values (if any)
    SymHeapCore::neqOp(NEQ_DEL, val, newVal);

    // reflect the change in NeqDb
    TValList neqs;
    d->neqDb.gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValId neq, neqs) {
        d->neqDb.del(val, neq);
        d->neqDb.add(newVal, neq);
    }
#ifndef NDEBUG
    // make sure we didn't create any dangling predicates
    TValList related;
    this->gatherRelatedValues(related, val);
    CL_BREAK_IF(!related.empty());
#endif
}

void SymHeapCore::neqOp(ENeqOp op, TValId valA, TValId valB) {
    switch (op) {
        case NEQ_NOP:
            return;

        case NEQ_ADD:
            d->neqDb.add(valA, valB);
            return;

        case NEQ_DEL:
            d->neqDb.del(valA, valB);
            return;
    }
}

void SymHeapCore::gatherRelatedValues(TValList &dst, TValId val) const {
    // TODO: should we care about off-values here?
    d->neqDb.gatherRelatedValues(dst, val);
}

void SymHeapCore::copyRelevantPreds(SymHeapCore &dst, const TValMap &valMap)
    const
{
    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb.cont_) {
        TValId valLt = item.first;
        TValId valGt = item.second;

        if (!valMapLookup(valMap, &valLt) || !valMapLookup(valMap, &valGt))
            // not relevant
            continue;

        // create the image now!
        dst.neqOp(NEQ_ADD, valLt, valGt);
    }
}

bool SymHeapCore::matchPreds(const SymHeapCore &ref, const TValMap &valMap)
    const
{
    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb.cont_) {
        TValId valLt = item.first;
        TValId valGt = item.second;
        if (!valMapLookup(valMap, &valLt) || !valMapLookup(valMap, &valGt))
            // seems like a dangling predicate, which we are not interested in
            continue;

        if (!ref.d->neqDb.areNeq(valLt, valGt))
            // Neq predicate not matched
            return false;
    }

    return true;
}

// FIXME: should this be declared non-const?
TValId SymHeapCore::placedAt(TObjId obj) const {
    if (d->objOutOfRange(obj))
        return VAL_INVALID;

    Private::Object &objData = d->objects[obj];
    const TObjId root = d->objRoot(obj, objData);
    CL_BREAK_IF(root < 0);

    typename Private::TRootMap::iterator it = d->roots.find(root);
    if (d->roots.end() == it)
        // object already deleted?
        return VAL_INVALID;

    Private::Root &rootData = it->second;
    if (VAL_NULL == rootData.addr) {
        // deleayed address creation
        rootData.addr = d->valCreate(UV_KNOWN, root);
        d->values[rootData.addr].offRoot = 0;
    }

    SymHeapCore &self = /* FIXME */ const_cast<SymHeapCore &>(*this);
    return self.valByOffset(rootData.addr, objData.off);
}

TObjId SymHeapCore::Private::rootLookup(TValId val) {
    if (VAL_NULL == val || this->valOutOfRange(val))
        return OBJ_INVALID;

    // root lookup
    const TValId valRoot = this->valRoot(val);
    return this->values[valRoot].target;
}

bool SymHeapCore::Private::gridLookup(
        TObjId                      *pFailCode,
        const Private::TObjByType   **pRow,
        const TValId                val)
{
    const TObjId root = this->rootLookup(val);
    if (root <= 0) {
        *pFailCode = root;
        return false;
    }

    // grid lookup
    const Private::Root &rootData = roMapLookup(this->roots, root);
    const Private::TGrid &grid = rootData.grid;
    const Private::Value &valData = this->values[val];
    Private::TGrid::const_iterator it = grid.find(valData.offRoot);
    if (grid.end() == it) {
        *pFailCode = OBJ_UNKNOWN;
        return false;
    }

    const Private::TObjByType *row = &it->second;
    CL_BREAK_IF(row->empty());
    *pRow = row;
    return true;
}

TObjId SymHeapCore::ptrAt(TValId at) {
    TObjId failCode;
    const Private::TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    // seek a _data_ pointer at the given row
    BOOST_FOREACH(const Private::TObjByType::const_reference item, *row) {
        const TObjType clt = item.first;
        if (!clt || clt->code != CL_TYPE_PTR)
            // not a pointer
            continue;

        const TObjType cltTarget = targetTypeOfPtr(clt);
        if (CL_TYPE_FNC != cltTarget->code)
            // matched
            return item.second;
    }

    // TODO: check that there is at most once pointer in the row in debug build

    return OBJ_UNKNOWN;
}

TObjId SymHeapCore::objAt(TValId at, TObjCode code) {
    if (VAL_NULL == at || d->valOutOfRange(at))
        return OBJ_INVALID;

    const Private::Value &valData = d->values[at];
    if (CL_TYPE_VOID == code && !valData.offRoot)
        return valData.target;

    TObjId failCode;
    const Private::TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    // seek the bigest object at the given row
    int maxSize = 0;
    TObjId max = OBJ_UNKNOWN;
    BOOST_FOREACH(const Private::TObjByType::const_reference item, *row) {
        const TObjType cltItem = item.first;
        const TObjId obj = item.second;
        CL_BREAK_IF(d->objOutOfRange(obj));

        const bool hasType = !!cltItem;
        if (CL_TYPE_VOID != code && (!hasType || cltItem->code != code))
            // not the type we are looking for
            continue;

        const int size = (hasType)
            ? cltItem->size
            : roMapLookup(d->roots, obj).cbSize;

        if (size < maxSize)
            continue;

        if ((size == maxSize) && !isComposite(cltItem))
            // if two types have the same size, prefer the composite one
            continue;

        // update the maximum
        maxSize = size;
        max = item.second;
    }

    return max;
}

TObjId SymHeapCore::objAt(TValId at, TObjType clt) {
    CL_BREAK_IF(!clt);
    if (CL_TYPE_PTR == clt->code && CL_TYPE_FNC != targetTypeOfPtr(clt)->code)
        // look for a pointer to data
        return this->ptrAt(at);

    TObjId failCode;
    const Private::TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    Private::TObjByType::const_iterator it = row->find(clt);
    if (row->end() != it)
        // exact match
        return it->second;

    if (!clt)
        // no type-free object found
        return OBJ_UNKNOWN;

    // try semantic match
    BOOST_FOREACH(const Private::TObjByType::const_reference item, *row) {
        const TObjType cltItem = item.first;
        if (cltItem && *cltItem == *clt)
            return item.second;
    }

    // not found
    return OBJ_UNKNOWN;
}

// FIXME: this should go away
TObjId SymHeapCore::pointsTo(TValId val) const {
    SymHeapCore &self = const_cast<SymHeapCore &>(*this);
    return self.objAt(val);
}

CVar SymHeapCore::cVarByRoot(TValId valRoot) const {
    // the following breakpoint checks everything
    CL_BREAK_IF(valRoot <= 0 || valRoot != d->valRoot(valRoot));

    const TObjId root = d->values[valRoot].target;
    const Private::Root &rootData = roMapLookup(d->roots, root);

    const CVar &cVar = rootData.cVar;
    CL_BREAK_IF(-1 == cVar.uid);
    return cVar;
}

TValId SymHeapCore::addrOfVar(CVar cv) {
    const TObjId obj = d->cVarMap.find(cv);
    if (0 < obj) {
        // already exists, just return its address
        const Private::Root &rootData = roMapLookup(d->roots, obj);
        return rootData.addr;
    }

    // lazy creation of a program variable
    TObjType clt = stor_.vars[cv.uid].clt;
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
#if DEBUG_SE_STACK_FRAME
    const struct cl_loc *loc = 0;
    std::string varString = varTostring(stor_, cv.uid, &loc);
    CL_DEBUG_MSG(loc, "FFF SymHeapCore::objByCVar() creates var " << varString);
#endif
    // create the correspongin heap object
    const TObjId fresh = d->objCreate(clt, cv);

    // assign a heap value to its address
    return this->placedAt(fresh);
}

void SymHeapCore::gatherCVars(TCVarList &dst) const {
    d->cVarMap.getAll(dst);
}

static bool dummyFilter(EValueTarget) {
    return true;
}

void SymHeapCore::gatherRootObjects(TValList &dst, bool (*filter)(EValueTarget))
    const
{
    if (!filter)
        filter = dummyFilter;

    BOOST_FOREACH(Private::TRootMap::const_reference item, d->roots) {
        const TObjId obj = item.first;
        if (/* XXX */ OBJ_RETURN == obj)
            continue;

        const TValId at = this->placedAt(obj);
        if (!filter(this->valTarget(at)))
            continue;

        dst.push_back(at);
    }
}

TObjId SymHeapCore::valGetCompositeObj(TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        return OBJ_INVALID;

    return d->values[val].compObj;
}

// TODO: remove this
TObjId SymHeapCore::subObj(TObjId obj, int nth) const {
    if (d->objOutOfRange(obj))
        return OBJ_INVALID;

    const Private::Object &objData = d->objects[obj];
    const TObjList &subs = objData.subObjs;
    const int cnt = subs.size();

    return (nth < cnt)
        ? subs[nth]
        : OBJ_INVALID;
}

// TODO: remove this
TObjId SymHeapCore::objParent(TObjId obj, int *nth) const {
    if (OBJ_RETURN == obj || d->objOutOfRange(obj))
        return OBJ_INVALID;

    const Private::Object &objData = d->objects[obj];
    const TObjId parent = objData.parent;
    if (OBJ_INVALID == parent)
        return OBJ_INVALID;

    if (nth)
        *nth = objData.nthItem;

    return parent;
}

TObjId SymHeapCore::Private::objCreate(TObjType clt, CVar cVar) {
    const TObjId obj = this->objCreate();
    this->objects[obj].clt = clt;
    this->roots[obj].cVar = cVar;

    if (clt)
        this->subsCreate(obj);

    if (/* heap object */ -1 != cVar.uid)
        this->cVarMap.insert(cVar, obj);

    return obj;
}

TValId SymHeapCore::heapAlloc(int cbSize) {
    const TObjId obj = d->objCreate();

    Private::Root &rootData = d->roots[obj];
    rootData.cbSize = cbSize;
    rootData.grid[/* off */ 0][/* clt */ 0] = obj;

    return this->placedAt(obj);
}

bool SymHeapCore::valDestroyTarget(TValId val) {
    if (VAL_NULL == val || this->valOffset(val))
        return false;

    const TObjId target = d->values[val].target;
    if (target < 0)
        return false;

    d->objDestroy(target);
    return true;
}

int SymHeapCore::valSizeOfTarget(TValId val) const {
    const TObjId root = d->rootLookup(val);
    CL_BREAK_IF(d->objOutOfRange(root));
    const Private::Object &objData = d->objects[root];

    const TObjType clt = objData.clt;
    if (clt)
        // typed object
        return clt->size;

    // RAW object
    const Private::Root &rootData = roMapLookup(d->roots, root);
    return rootData.cbSize;
}

void SymHeapCore::objDefineType(TObjId obj, TObjType clt) {
    if (OBJ_RETURN == obj)
        // cleanup OBJ_RETURN for next wheel
        d->objDestroy(OBJ_RETURN);

    CL_BREAK_IF(d->objOutOfRange(obj));
    Private::Object &objData = d->objects[obj];

    // type reinterpretation not allowed for now
    CL_BREAK_IF(objData.clt);

    // there should be no value as long as it was "a target of a void pointer"
    if (VAL_INVALID != objData.value) {
        // FIXME: this happens with test-0106.c during error recovery
        CL_BREAK_IF("SymHeapCore::objDefineType() sees a dangling value");
        d->releaseValueOf(obj);
    }

    // delayed object's type definition
    objData = Private::Object();
    objData.clt = clt;
    d->subsCreate(obj);
}

void SymHeapCore::Private::objDestroy(TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    CL_BREAK_IF(OBJ_INVALID != this->objects[obj].root);
    Private::Root &rootData = roMapLookup(this->roots, obj);

    // remove the corresponding program variable (if any)
    TObjId kind = OBJ_DELETED;
    const CVar cv = rootData.cVar;
    if (cv.uid != /* heap object */ -1) {
        this->cVarMap.remove(cv);
        kind = OBJ_LOST;
    }

    // invalidate the address
    const TValId addr = rootData.addr;
    if (0 < addr) {
        CL_BREAK_IF(valOutOfRange(addr));
        this->values[addr].target = kind;
    }

    // destroy the object
    this->subsDestroy(obj);
    if (OBJ_RETURN == obj) {
        // reinitialize OBJ_RETURN
        this->objects[OBJ_RETURN] = Private::Object();
        this->roots[OBJ_RETURN] = Private::Root();
    }
}

TValId SymHeapCore::valCreateUnknown(EUnknownValue code) {
    return d->valCreate(code, OBJ_UNKNOWN);
}

EUnknownValue SymHeapCore::valGetUnknown(TValId val) const {
    switch (val) {
        case VAL_NULL: /* == VAL_FALSE */
        case VAL_TRUE:
            return UV_KNOWN;

        case VAL_INVALID:
            // fall through! (basically equal to "out of range")
        default:
            break;
    }

    CL_BREAK_IF(d->valOutOfRange(val));
    const EUnknownValue code = d->values[val].code;
    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        case UV_UNINITIALIZED:
        case UV_UNKNOWN:
        case UV_DONT_CARE:
            return code;
    }

    const TValId valRoot = d->valRoot(val);
    return d->values[valRoot].code;
}

TValId SymHeapCore::valCreateCustom(int cVal) {
    Private::TCValueMap::iterator iter = d->cValueMap.find(cVal);
    if (d->cValueMap.end() == iter) {
        // cVal not found, create a new wrapper for it
        const TValId val = d->valCreate(UV_KNOWN, OBJ_INVALID);
        if (VAL_INVALID == val)
            return VAL_INVALID;

        // initialize heap value
        Private::Value &objData = d->values[val];
        objData.isCustom    = true;
        objData.customData  = cVal;

        // store cVal --> val mapping
        d->cValueMap[cVal] = val;

        return val;
    }

    // custom value already wrapped, we have to reuse it
    return iter->second;
}

int SymHeapCore::valGetCustom(TValId val) const
{
    if (VAL_NULL == val || d->valOutOfRange(val))
        // value ID is either out of range, or does not point to a valid obj
        return -1;

    const Private::Value &valData = d->values[val];
    if (!valData.isCustom)
        // not a custom value
        return -1;

    return valData.customData;
}

bool SymHeapCore::valTargetIsProto(TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        // not a prototype for sure
        return false;

    // seek root
    const TObjId root = d->rootLookup(val);
    if (root < 0)
        return false;

    const Private::Root &rootData = roMapLookup(d->roots, root);
    return rootData.isProto;
}

void SymHeapCore::valTargetSetProto(TValId val, bool isProto) {
    CL_BREAK_IF(VAL_NULL == val || d->valOutOfRange(val));

    // seek root
    const TObjId root = d->rootLookup(val);
    Private::Root &rootData = roMapLookup(d->roots, root);
    rootData.isProto = isProto;
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap
struct SymHeap::Private {
    struct AbstractObject {
        EObjKind            kind;
        BindingOff          off;

        AbstractObject():
            kind(OK_CONCRETE)
        {
        }
    };

    typedef std::map<TValId, AbstractObject> TData;
    TData data;
};

SymHeap::SymHeap(TStorRef stor):
    SymHeapCore(stor),
    d(new Private)
{
}

SymHeap::SymHeap(const SymHeap &ref):
    SymHeapCore(ref),
    d(new Private(*ref.d))
{
}

SymHeap::~SymHeap() {
    delete d;
}

SymHeap& SymHeap::operator=(const SymHeap &ref) {
    SymHeapCore::operator=(ref);
    delete d;
    d = new Private(*ref.d);
    return *this;
}

void SymHeap::swap(SymHeapCore &baseRef) {
    // swap base
    SymHeapCore::swap(baseRef);

    // swap self
    SymHeap &ref = dynamic_cast<SymHeap &>(baseRef);
    swapValues(this->d, ref.d);
}

TValId SymHeap::valClone(TValId val) {
    const TValId dup = SymHeapCore::valClone(val);
    if (dup <= 0)
        return dup;

    const TValId valRoot = this->valRoot(val);
    const TValId dupRoot = this->valRoot(dup);
    CL_BREAK_IF(valRoot <= 0 || dupRoot <= 0);

    Private::TData::iterator iter = d->data.find(valRoot);
    if (d->data.end() != iter)
        // duplicate metadata of an abstract object
        d->data[dupRoot] = iter->second;

    return dup;
}

EObjKind SymHeap::valTargetKind(TValId val) const {
    if (val <= 0)
        // FIXME
        return OK_CONCRETE;

    const TValId valRoot = this->valRoot(val);
    Private::TData::iterator iter = d->data.find(valRoot);
    if (d->data.end() != iter)
        return iter->second.kind;

    return OK_CONCRETE;
}

EUnknownValue SymHeap::valGetUnknown(TValId val) const {
    if (OK_CONCRETE != this->valTargetKind(val))
        // abstract object
        return UV_ABSTRACT;

    return SymHeapCore::valGetUnknown(val);
}

const BindingOff& SymHeap::segBinding(TValId at) const {
    const TValId valRoot = this->valRoot(at);

    Private::TData::iterator iter = d->data.find(valRoot);
    CL_BREAK_IF(d->data.end() == iter);

    return iter->second.off;
}

void SymHeap::valTargetSetAbstract(
        TValId                      at,
        EObjKind                    kind,
        const BindingOff            &off)
{
    const TValId valRoot = this->valRoot(at);

    Private::TData::iterator iter = d->data.find(valRoot);
    if (d->data.end() != iter && OK_SLS == kind) {
        Private::AbstractObject &aoData = iter->second;
        CL_BREAK_IF(OK_MAY_EXIST != aoData.kind || off != aoData.off);

        // OK_MAY_EXIST -> OK_SLS
        aoData.kind = kind;
        return;
    }

    CL_BREAK_IF(OK_CONCRETE == kind || hasKey(d->data, valRoot));

    // initialize abstract object
    Private::AbstractObject &aoData = d->data[valRoot];
    aoData.kind     = kind;
    aoData.off      = off;
}

void SymHeap::valTargetSetConcrete(TValId at) {
    const TValId valRoot = this->valRoot(at);

    CL_DEBUG("SymHeap::objSetConcrete() is taking place...");
    Private::TData::iterator iter = d->data.find(valRoot);
    CL_BREAK_IF(d->data.end() == iter);

    // just remove the object ID from the map
    d->data.erase(iter);
}

void SymHeap::valMerge(TValId v1, TValId v2) {
    // check that at least one value is unknown
    moveKnownValueToLeft(*this, v1, v2);
    const EUnknownValue code1 = this->valGetUnknown(v1);
    const EUnknownValue code2 = this->valGetUnknown(v2);
    CL_BREAK_IF(UV_KNOWN == code2);

    if (UV_ABSTRACT != code1 && UV_ABSTRACT != code2) {
        // no abstract objects involved
        SymHeapCore::valMerge(v1, v2);
        return;
    }

    if (UV_ABSTRACT == code1 && spliceOutListSegment(*this, v1, v2))
        // splice-out succeeded ... ls(v1, v2)
        return;

    if (UV_ABSTRACT == code2 && spliceOutListSegment(*this, v2, v1))
        // splice-out succeeded ... ls(v2, v1)
        return;

    CL_DEBUG("failed to splice-out list segment, has to over-approximate");
}

void SymHeap::dlSegCrossNeqOp(ENeqOp op, TValId headAddr1) {
    const TObjId head1 = this->objAt(headAddr1);
    const TObjId seg1 = objRoot(*this, head1);
    const TObjId seg2 = dlSegPeer(*this, seg1);
    const TValId headAddr2 = segHeadAddr(*this, seg2);

    // dig pointer-to-next objects
    const TObjId next1 = nextPtrFromSeg(*this, seg1);
    const TObjId next2 = nextPtrFromSeg(*this, seg2);

    // read the values (addresses of the surround)
    const TValId val1 = this->valueOf(next1);
    const TValId val2 = this->valueOf(next2);

    // add/del Neq predicates
    SymHeapCore::neqOp(op, val1, headAddr2);
    SymHeapCore::neqOp(op, val2, headAddr1);

    if (NEQ_DEL == op)
        // removing the 1+ flag implies removal of the 2+ flag
        SymHeapCore::neqOp(NEQ_DEL, headAddr1, headAddr2);
}

void SymHeap::neqOp(ENeqOp op, TValId valA, TValId valB) {
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

    SymHeapCore::neqOp(op, valA, valB);
}

bool SymHeapCore::proveNeq(TValId valA, TValId valB) const {
    // check for invalid values
    if (VAL_INVALID == valA || VAL_INVALID == valB)
        return false;

    // check for identical values
    if (valA == valB)
        return false;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, valA, valB);

    // check for known bool values
    if (VAL_TRUE == valA)
        return (VAL_FALSE == valB);

    // we presume (0 <= valA) and (0 < valB) at this point
    CL_BREAK_IF(d->lastValId() < valB || valB < 0);
    const EUnknownValue code = this->valGetUnknown(valB);
    if (UV_KNOWN == code)
        // NOTE: we know (valA != valB) at this point, look above
        return true;

    // check for a Neq predicate
    if (d->neqDb.areNeq(valA, valB))
        return true;

    if (valA <= 0 || valB <= 0)
        // no handling of special values here
        return false;

    const TValId root1 = d->valRoot(valA);
    const TValId root2 = d->valRoot(valB);
    if (root1 != root2)
        // roots differ
        return false;

    CL_BREAK_IF("not tested");
    return true;
}

bool SymHeap::proveNeq(TValId ref, TValId val) const {
    if (SymHeapCore::proveNeq(ref, val))
        return true;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, ref, val);
    if (UV_KNOWN != this->valGetUnknown(ref))
        return false;

    std::set<TValId> haveSeen;

    while (0 < val && insertOnce(haveSeen, val)) {
        const EUnknownValue code = this->valGetUnknown(val);
        switch (code) {
            case UV_KNOWN:
                // concrete object reached --> prove done
                return (val != ref);

            case UV_ABSTRACT:
                break;

            default:
                // we can't prove much for unknown values
                return false;
        }

        if (SymHeapCore::proveNeq(ref, val))
            // prove done
            return true;

        TObjId seg = objRootByVal(*this, val);
        if (OK_DLS == this->valTargetKind(val))
            seg = dlSegPeer(*this, seg);

        if (seg < 0)
            // no valid object here
            return false;

        const TValId valNext = this->valueOf(nextPtrFromSeg(*this, seg));
        if (SymHeapCore::proveNeq(val, valNext))
            // non-empty abstract object reached --> prove done
            return true;

        // jump to next value
        val = valNext;
    }

    return false;
}

bool SymHeap::valDestroyTarget(TValId val) {
    const TValId valRoot = this->valRoot(val);
    if (!SymHeapCore::valDestroyTarget(val))
        return false;

    CL_BREAK_IF(valRoot <= 0);
    if (d->data.erase(valRoot))
        CL_DEBUG("SymHeap::valDestroyTarget() destroys an abstract object");

    return true;
}
