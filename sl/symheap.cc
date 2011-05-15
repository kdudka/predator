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

#ifdef NDEBUG
    // aggressive optimization
#   define DCAST static_cast
#else
#   define DCAST dynamic_cast
#endif

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
        typedef std::map<CVar, TValId>              TCont;
        TCont                                       cont_;

    public:
        void insert(CVar cVar, TValId val) {
#ifndef NDEBUG
            const unsigned last = cont_.size();
#endif
            cont_[cVar] = val;

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

        TValId find(const CVar &cVar) {
            // regular lookup
            TCont::iterator iter = cont_.find(cVar);
            const bool found = (cont_.end() != iter);
            if (!cVar.inst) {
                // gl variable explicitly requested
                return (found)
                    ? iter->second
                    : VAL_INVALID;
            }

            // automatic fallback to gl variable
            CVar gl = cVar;
            gl.inst = /* global variable */ 0;
            TCont::iterator iterGl = cont_.find(gl);
            const bool foundGl = (cont_.end() != iterGl);

            if (!found && !foundGl)
                // not found anywhere
                return VAL_INVALID;

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
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapCore
typedef std::set<TObjId>                                TUsedBy;
typedef std::map<TOffset, TValId>                       TOffMap;
typedef std::map<TObjType, TObjId>                      TObjByType;
typedef std::map<TOffset, TObjByType>                   TGrid;
typedef std::map<TObjId, bool /* isPtr */>              TLiveObjs;
typedef std::map<int, TValId>                           TCValueMap;

// FIXME: really good idea to define this as a value type?
struct BareValue {
    EValueTarget                    code;
    EValueOrigin                    origin;
    TOffset                         offRoot;
    TUsedBy                         usedBy;

    BareValue(EValueTarget code_, EValueOrigin origin_):
        code(code_),
        origin(origin_),
        offRoot(0)
    {
    }

    virtual ~BareValue() { }
    virtual BareValue* clone() const {
        return new BareValue(*this);
    }
};

struct CompValue: public BareValue {
    TObjId                          compObj;

    CompValue(EValueTarget code_, EValueOrigin origin_):
        BareValue(code_, origin_)
    {
    }

    virtual BareValue* clone() const {
        return new CompValue(*this);
    }
};

struct CustomValue: public BareValue {
    int                             customData;

    CustomValue(EValueTarget code_, EValueOrigin origin_):
        BareValue(code_, origin_)
    {
    }

    virtual BareValue* clone() const {
        return new CustomValue(*this);
    }
};

struct OffValue: public BareValue {
    TValId                          root;

    OffValue(
            EValueTarget            code_,
            EValueOrigin            origin_,
            TValId                  root_,
            TOffset                 offRoot_):
        BareValue(code_, origin_),
        root(root_)
    {
        offRoot = offRoot_;
    }

    virtual BareValue* clone() const {
        return new OffValue(*this);
    }
};

struct RootValue: public BareValue {
    TObjId                          target;
    TOffMap                         offMap;

    RootValue(EValueTarget code_, EValueOrigin origin_):
        BareValue(code_, origin_),
        target(OBJ_INVALID)
    {
    }

    virtual BareValue* clone() const {
        return new RootValue(*this);
    }
};

struct SymHeapCore::Private {
    // allocate VAL_NULL (TODO: avoid allocation of VAL_NULL)
    Private();

    // clone values, they are now allocated separately
    Private(const Private &);

    // delete values, they are now allocated separately
    ~Private();

    struct Object {
        TValId                      value;
        TValId                      root;
        TOffset                     off;
        TObjType                    clt;

        Object():
            value(VAL_INVALID),
            root(VAL_INVALID),
            off(0),
            clt(0)
        {
        }
    };

    struct Root {
        TValId                      addr;
        unsigned                    cbSize;
        CVar                        cVar;
        TObjList          /* XXX */ allObjs;
        TLiveObjs                   liveObjs;
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

    // TODO: drop this
    typedef std::map<TObjId, Root>  TRootMap;

    CVarMap                         cVarMap;
    TCValueMap                      cValueMap;
    std::vector<Object>             objects;
    std::vector<BareValue *>        values;
    TRootMap                        roots;
    NeqDb                           neqDb;

    inline TValId lastValId();
    inline TObjId lastObjId();
    template <typename T> T lastId();
    template <typename T> T nextId();

    inline bool valOutOfRange(TValId);
    inline bool objOutOfRange(TObjId);

    RootValue* valData(const TValId);

    inline TValId valRoot(const TValId, const BareValue *);
    inline TValId valRoot(const TValId);

    inline TObjId objRoot(const TObjId);

    TValId valCreate(EValueTarget code, EValueOrigin origin);
    TValId valCreateOff(EValueTarget code, EValueOrigin origin, TValId root,
            TOffset);

    TValId valDup(TValId);
    TObjId objCreate();
    TValId objDup(TObjId root);
    void objDestroy(TObjId obj);

    void releaseValueOf(TObjId obj);
    void setValueOf(TObjId of, TValId val);

    void subsCreate(TObjId obj);
    void subsDestroy(TObjId obj);

    TObjId rootLookup(TValId val);
    bool gridLookup(TObjId *pFailCode, const TObjByType **pRow, const TValId);
    void neqOpWrap(SymHeap::ENeqOp, TValId, TValId);

    private:
        // intentionally not implemented
        Private& operator=(const Private &);
};

inline TValId SymHeapCore::Private::lastValId() {
    return static_cast<TValId>(this->values.size() - 1);
}

inline TObjId SymHeapCore::Private::lastObjId() {
    return static_cast<TObjId>(this->objects.size() - 1);
}

template <typename T> T SymHeapCore::Private::lastId() {
    const unsigned last = std::max(this->objects.size(), this->values.size());
    return static_cast<T>(last);
}

template <typename T> T SymHeapCore::Private::nextId() {
    const unsigned last = this->lastId<unsigned>();
    return static_cast<T>(last + 1);
}

inline bool SymHeapCore::Private::valOutOfRange(TValId val) {
    return (val < 0)
        || (this->lastValId() < val);
}

inline bool SymHeapCore::Private::objOutOfRange(TObjId obj) {
    return (obj < 0)
        || (this->lastObjId() < obj);
}

inline RootValue* SymHeapCore::Private::valData(const TValId val) {
    CL_BREAK_IF(valOutOfRange(val));
    BareValue *base = this->values[val];
    return DCAST<RootValue *>(base);
}

inline TValId SymHeapCore::Private::valRoot(
        const TValId                val,
        const BareValue            *valData)
{
    if (!valData->offRoot)
        return val;

    const TValId valRoot = DCAST<const OffValue *>(valData)->root;
    CL_BREAK_IF(VAL_NULL == valRoot || valOutOfRange(valRoot));
    return valRoot;
}

inline TValId SymHeapCore::Private::valRoot(const TValId val) {
    CL_BREAK_IF(VAL_NULL == val);
    CL_BREAK_IF(valOutOfRange(val));
    return this->valRoot(val, this->values[val]);
}

inline TObjId SymHeapCore::Private::objRoot(const TObjId obj) {
    if (OBJ_RETURN == obj)
        return OBJ_RETURN;

    CL_BREAK_IF(objOutOfRange(obj));
    const Object &objData = this->objects[obj];

    const TValId valRoot = objData.root;
    CL_BREAK_IF(valOutOfRange(valRoot));
    return DCAST<RootValue *>(this->values[valRoot])->target;
}

void SymHeapCore::Private::releaseValueOf(TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    const TValId val = this->objects[obj].value;
    if (val <= 0)
        // we do not track uses of special values
        return;

    CL_BREAK_IF(valOutOfRange(val));
    TUsedBy &uses = this->values[val]->usedBy;
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

    this->values[val]->usedBy.insert(obj);

    const TObjId root = this->rootLookup(val);
    if (root < 0)
        return;

    Private::Root &rootData = roMapLookup(this->roots, root);
    rootData.usedBy.insert(obj);
}

TObjId SymHeapCore::Private::objCreate() {
    // acquire object ID
    const TObjId obj = this->nextId<TObjId>();
    this->objects.resize(obj + 1);
    return obj;
}

TValId SymHeapCore::Private::valCreate(
        EValueTarget                code,
        EValueOrigin                origin)
{
    // acquire value ID
    const TValId val = this->nextId<TValId>();
    this->values.resize(1 + static_cast<unsigned>(val));

    switch (code) {
        case VT_INVALID:
        case VT_UNKNOWN:
        case VT_DELETED:
        case VT_LOST:
            this->values[val] = new BareValue(code, origin);
            break;

        case VT_COMPOSITE:
            this->values[val] = new CompValue(code, origin);
            break;

        case VT_CUSTOM:
            this->values[val] = new CustomValue(code, origin);
            break;

        case VT_ABSTRACT:
            CL_BREAK_IF("invalid call of SymHeapCore::Private::valCreate()");
            // fall through!

        case VT_ON_HEAP:
        case VT_ON_STACK:
        case VT_STATIC:
            this->values[val] = new RootValue(code, origin);
            break;
    }

    return val;
}

TValId SymHeapCore::Private::valCreateOff(
        EValueTarget                code,
        EValueOrigin                origin,
        TValId                      root,
        TOffset                     off)
{
    // acquire value ID
    const TValId val = this->nextId<TValId>();
    this->values.resize(1 + static_cast<unsigned>(val));

    this->values[val] = new OffValue(code, origin, root, off);
    return val;
}

TValId SymHeapCore::Private::valDup(TValId val) {
    CL_BREAK_IF(valOutOfRange(val));

    // acquire value ID
    const TValId dup = this->nextId<TValId>();
    this->values.resize(1 + static_cast<unsigned>(dup));

    // deep copy the value
    const BareValue *tpl = this->values[val];
    BareValue *dupData = /* FIXME: subtle */ tpl->clone();
    this->values[dup] = dupData;

    // wipe BareValue::usedBy
    dupData->usedBy.clear();
    return dup;
}

SymHeapCore::Private::Private() {
    // a placeholder for VAL_NULL (TODO: remove this)
    this->values.push_back(new RootValue(VT_INVALID, VO_INVALID));
}

SymHeapCore::Private::Private(const SymHeapCore::Private &ref):
    cVarMap     (ref.cVarMap),
    cValueMap   (ref.cValueMap),
    objects     (ref.objects),
    roots       (ref.roots),
    neqDb       (ref.neqDb)
{
    // allocate space for pointers
    const unsigned cnt = ref.values.size();
    this->values.resize(ref.values.size());

    // deep copy of all values
    for (unsigned i = 0; i < cnt; ++i) {
        const BareValue *tpl = ref.values[i];
        this->values[i] = (tpl)
            ? tpl->clone()
            : 0;
    }
}

SymHeapCore::Private::~Private() {
    BOOST_FOREACH(const BareValue *valData, this->values)
        delete valData;
}

template <class TRootMap>
EValueTarget digTarget(TStorRef stor, TRootMap &rootMap, TObjId root) {
    if (root < 0)
        return VT_INVALID;

    if (OBJ_RETURN == root)
        return VT_ON_STACK;

    const typename TRootMap::mapped_type &rootData = roMapLookup(rootMap, root);
    const int uid = rootData.cVar.uid;
    if (-1 == uid)
        return VT_ON_HEAP;

    return (isOnStack(stor.vars[uid]))
        ? VT_ON_STACK
        : VT_STATIC;
}

template <class TRootMap>
EValueOrigin digOrigin(TStorRef stor, TRootMap &rootMap, TObjId root) {
    const EValueTarget code = digTarget(stor, rootMap, root);
    switch (code) {
        case VT_INVALID:
            return VO_INVALID;

        case VT_ON_HEAP:
            return VO_HEAP;

        case VT_ON_STACK:
            return VO_STACK;

        case VT_STATIC:
            return VO_STATIC;

        default:
            CL_BREAK_IF("digTarget() malfunction");
            return VO_INVALID;
    }
}

// FIXME: should this be declared non-const?
TValId SymHeapCore::valueOf(TObjId obj) const {
    // handle special cases first
    switch (obj) {
        case OBJ_UNKNOWN:
            // not implemented
        case OBJ_INVALID:
            return VAL_INVALID;

        case OBJ_DEREF_FAILED:
            return VAL_DEREF_FAILED;

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
        val = d->valCreate(VT_COMPOSITE, VO_INVALID);
        CompValue *valData = DCAST<CompValue *>(d->values[val]);
        valData->compObj = obj;
    }
    else {
        // deleayed creation of an uninitialized value
        const TObjId root = d->objRoot(obj);
        const EValueOrigin vo = digOrigin(stor_, d->roots, root);
        val = d->valCreate(VT_UNKNOWN, vo);
    }

    // store backward reference
    d->values[val]->usedBy.insert(obj);
    return val;
}

void SymHeapCore::usedBy(TObjList &dst, TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        return;

    const TUsedBy &usedBy = d->values[val]->usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

void SymHeapCore::pointedBy(TObjList &dst, TValId root) const {
    CL_BREAK_IF(d->valOutOfRange(root));
    CL_BREAK_IF(d->values[root]->offRoot);

    RootValue *valData = d->valData(root);
    const Private::Root &rootData = roMapLookup(d->roots, valData->target);
    const TUsedBy &usedBy = rootData.usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

/// return how many objects use the value
unsigned SymHeapCore::usedByCount(TValId val) const {
    if (VAL_NULL == val || d->valOutOfRange(val))
        return 0;

    const TUsedBy &usedBy = d->values[val]->usedBy;
    return usedBy.size();
}

unsigned SymHeapCore::lastId() const {
    return d->lastId<unsigned>();
}

TValId SymHeapCore::valClone(TValId val) {
    CL_BREAK_IF(VAL_NULL == val || d->valOutOfRange(val));
    const BareValue *valData = d->values[val];

    // check unknown value code
    const EValueTarget code = valData->code;
    if (!isPossibleToDeref(code))
        return d->valDup(val);

    // duplicate the root object
    const TObjId root = d->rootLookup(val);
    const TValId dupAt = d->objDup(root);

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
    TGrid &grid = this->roots[root].grid;
    grid[/* off */ 0][clt] = obj;

    typedef std::pair<TObjId, TObjType> TPair;
    typedef std::stack<TPair> TStack;
    TStack todo;

    const TValId rootAt = this->roots[root].addr;
    CL_BREAK_IF(rootAt <= 0);

    // we use explicit stack to avoid recursion
    push(todo, obj, clt);
    while (!todo.empty()) {
        boost::tie(obj, clt) = todo.top();
        todo.pop();
        CL_BREAK_IF(!clt);

        if (!isComposite(clt))
            continue;

        const int cnt = clt->item_cnt;
        for (int i = 0; i < cnt; ++i) {
            const struct cl_type_item *item = clt->items + i;
            TObjType subClt = item->type;
            const TObjId subObj = this->objCreate();
            this->objects[subObj].clt  = subClt;
            this->objects[subObj].root = rootAt;

            const TOffset off = item->offset;
            const TOffset offTotal = off + this->objects[obj].off;
            this->objects[subObj].off = offTotal;
            grid[offTotal][subClt] = subObj;

            this->roots[root].allObjs.push_back(subObj);

            push(todo, subObj, subClt);
        }
    }
}

TValId SymHeapCore::Private::objDup(TObjId root) {
    CL_DEBUG("SymHeapCore::Private::objDup() is taking place...");
    CL_BREAK_IF(objOutOfRange(root));
    CL_BREAK_IF(this->objects[root].off);

    // duplicate object metadata
    const TObjId image = this->objCreate();
    const Private::Object &origin = this->objects[root];
    this->setValueOf(image, origin.value);
    this->objects[image] = origin;

    // check address of the given root
    const Private::Root &rootDataSrc = roMapLookup(this->roots, root);
    const TValId rootAt = rootDataSrc.addr;
    CL_BREAK_IF(rootAt <= 0);
    const EValueTarget code = this->values[rootAt]->code;

    // assign an address to the clone
    const TValId imageAt = this->valCreate(code, VO_ASSIGNED);
    this->valData(imageAt)->target = image;
    Private::Root &rootDataDst = this->roots[image];
    rootDataDst.addr = imageAt;
    this->objects[image].root = imageAt;

    // duplicate root metadata
    const TObjType cltRoot = origin.clt;
    rootDataDst.grid[/* off */0][cltRoot] = image;
    rootDataDst.cVar    = rootDataSrc.cVar;
    rootDataDst.isProto = rootDataSrc.isProto;

    if (!isComposite(cltRoot))
        return imageAt;

    // assume composite object
    this->setValueOf(image, VAL_INVALID);

    BOOST_FOREACH(const TObjId src, rootDataSrc.allObjs) {
        // duplicate a single object
        const TObjId dst = this->objCreate();
        const Private::Object &origin = this->objects[src];
        this->setValueOf(dst, origin.value);

        // copy the metadata
        this->objects[dst] = origin;
        const TLiveObjs &liveSrc = rootDataSrc.liveObjs;
        TLiveObjs::const_iterator it = liveSrc.find(src);
        if (liveSrc.end() != it) {
            TLiveObjs &liveDst = rootDataDst.liveObjs;
            liveDst[dst] = it->second;
        }

        // recover root
        this->objects[dst].root = imageAt;
        rootDataDst.allObjs.push_back(dst);

        // recover grid
        const TOffset off = this->objects[dst].off;
        const TObjType clt = this->objects[dst].clt;
        TGrid &grid = rootDataDst.grid;
        grid[off][clt] = dst;
    }

    return imageAt;
}

void SymHeapCore::gatherLivePointers(TObjList &dst, TValId atAddr) const {
    const TObjId root = const_cast<SymHeapCore *>(this)->objAt(atAddr);
    const Private::Root &rootData = roMapLookup(d->roots, root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData.liveObjs) {
        if (/* isPtr */ item.second)
            dst.push_back(/* obj */ item.first);
    }
}

void SymHeapCore::gatherLiveObjects(TObjList &dst, TValId atAddr) const {
    const TObjId root = const_cast<SymHeapCore *>(this)->objAt(atAddr);
    const Private::Root &rootData = roMapLookup(d->roots, root);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootData.liveObjs)
        dst.push_back(/* obj */ item.first);
}

void SymHeapCore::Private::subsDestroy(TObjId root) {
    this->releaseValueOf(root);
    this->objects[root].value = VAL_INVALID;
    Private::Root &rootData = roMapLookup(this->roots, root);

    BOOST_FOREACH(const TObjId obj, rootData.allObjs) {
        this->releaseValueOf(obj);
        this->objects[obj].value = VAL_INVALID;
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

    // XXX
    d->objects.resize(/* OBJ_RETURN */ 1);
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

    // seek root
    const TObjId root = d->objRoot(obj);
    Private::Root &rootData = roMapLookup(d->roots, root);

    // we allow to set values of atomic types only
    const TObjType clt = d->objects[obj].clt;
    CL_BREAK_IF(isComposite(clt));

    // mark the destination object as live
    const bool isPtr = isDataPtr(clt);
    rootData.liveObjs[obj] = isPtr;

    // now set the value
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
    off += d->values[at]->offRoot;
    if (!off)
        return valRoot;

    const EValueTarget vt = d->values[at]->code;
    if (VT_UNKNOWN == vt || isGone(vt))
        // do not track off-value for invalid targets
        return d->valDup(at);

    // off-value lookup
    const RootValue *valData = d->valData(valRoot);
    const TOffMap &offMap = valData->offMap;
    TOffMap::const_iterator it = offMap.find(off);
    if (offMap.end() != it)
        return it->second;

    // create a new off-value
    const EValueTarget code = this->valTarget(valRoot);
    const TValId val = d->valCreateOff(code, valData->origin, valRoot, off);
    d->valData(valRoot)->offMap[off] = val;
    return val;
}

// TODO: remove this
bool handleSpecialTargets(EValueTarget *pCode, const TObjId target) {
    switch (target) {
        case OBJ_RETURN:
            // this happens in case a composite value is returned from a
            // function;  the expected output of test-0090 prior to this commit
            // was wrongly assuming that this area is dynamically allocated
            *pCode = VT_ON_STACK;
            return true;

        case OBJ_UNKNOWN:
            // either unknown value, or off-value
            return false;

        default:
            CL_BREAK_IF(target <= 0);
            return false;
    }
}

EValueOrigin SymHeapCore::valOrigin(TValId val) const {
    switch (val) {
        case VAL_INVALID:
            return VO_INVALID;

        case VAL_DEREF_FAILED:
            return VO_DEREF_FAILED;

        case VAL_NULL /* = VAL_FALSE */:
        case VAL_TRUE:
            return VO_ASSIGNED;

        default:
            CL_BREAK_IF(d->valOutOfRange(val));
            return d->values[val]->origin;
    }
}

// TODO: rewrite
EValueTarget SymHeapCore::valTarget(TValId val) const {
    if (val <= 0)
        return VT_INVALID;

    if (this->hasAbstractTarget(val))
        return VT_ABSTRACT;

    CL_BREAK_IF(d->valOutOfRange(val));
    const TOffset off = d->values[val]->offRoot;
    if (off < 0)
        // this value ended up above the root
        return VT_UNKNOWN;

    EValueTarget code = d->values[val]->code;
    switch (code) {
        case VT_CUSTOM:
        case VT_COMPOSITE:
        case VT_DELETED:
        case VT_LOST:
        case VT_UNKNOWN:
            return code;

        default:
            break;
    }

    const TValId valRoot = d->valRoot(val);
    code = d->values[valRoot]->code;
    switch (code) {
        case VT_DELETED:
        case VT_LOST:
            return code;

        default:
            break;
    }

    TObjId target = d->valData(valRoot)->target;
    if (handleSpecialTargets(&code, target))
        return code;

    const Private::Root &rootData = d->roots[target];
    const int uid = rootData.cVar.uid;
    if (-1 == uid)
        return VT_ON_HEAP;

    if (isOnStack(stor_.vars[uid]))
        return VT_ON_STACK;
    else
        return VT_STATIC;
}

bool isUninitialized(EValueOrigin code) {
    switch (code) {
        case VO_STATIC:
            // this may be subject for discussion
        case VO_HEAP:
        case VO_STACK:
            return true;

        default:
            return false;
    }
}

bool isAbstract(EValueTarget code) {
    return (VT_ABSTRACT == code);
}

bool isKnownObject(EValueTarget code) {
    switch (code) {
        case VT_STATIC:
        case VT_ON_HEAP:
        case VT_ON_STACK:
            return true;

        default:
            return false;
    }
}

bool isGone(EValueTarget code) {
    switch (code) {
        case VT_DELETED:
        case VT_LOST:
            return true;

        default:
            return false;
    }
}

bool isOnHeap(EValueTarget code) {
    switch (code) {
        case VT_ON_HEAP:
        case VT_ABSTRACT:
            return true;

        default:
            return false;
    }
}

bool isProgramVar(EValueTarget code) {
    switch (code) {
        case VT_STATIC:
        case VT_ON_STACK:
            return true;

        default:
            return false;
    }
}

bool isPossibleToDeref(EValueTarget code) {
    return isOnHeap(code)
        || isProgramVar(code);
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
    return d->values[val]->offRoot;
}

/// change value of all variables with value val to (fresh) newval
void SymHeapCore::valReplace(TValId val, TValId replaceBy) {
    CL_BREAK_IF(val <= 0);

    // collect objects having the value val
    TObjList rlist;
    this->usedBy(rlist, val);

    // go through the list and replace the value by newval
    BOOST_FOREACH(const TObjId obj, rlist) {
        this->objSetValue(obj, replaceBy);
    }

    // kill Neq predicate among the pair of values (if any)
    SymHeapCore::neqOp(NEQ_DEL, val, replaceBy);

    // reflect the change in NeqDb
    TValList neqs;
    d->neqDb.gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValId valNeq, neqs) {
        SymHeapCore::neqOp(NEQ_DEL, valNeq, val);
        SymHeapCore::neqOp(NEQ_ADD, valNeq, replaceBy);
    }
}

void SymHeapCore::Private::neqOpWrap(ENeqOp op, TValId valA, TValId valB) {
    switch (op) {
        case NEQ_NOP:
            return;

        case NEQ_ADD:
            this->neqDb.add(valA, valB);
            return;

        case NEQ_DEL:
            this->neqDb.del(valA, valB);
            return;
    }
}

void SymHeapCore::neqOp(ENeqOp op, TValId valA, TValId valB) {
    d->neqOpWrap(op, valA, valB);

    const TOffset off = this->valOffset(valA);
    if (!off || off != this->valOffset(valB))
        return;

    // if both values have the same non-zero offset, connect also the roots
    valA = this->valRoot(valA);
    valB = this->valRoot(valB);
    d->neqOpWrap(op, valA, valB);
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

    const TObjId root = d->objRoot(obj);
    CL_BREAK_IF(root < 0);

    Private::Root &rootData = roMapLookup(d->roots, root);
    TValId &addr = rootData.addr;
    if (OBJ_RETURN == root && VAL_NULL == addr) {
        // deleayed address creation
        addr = d->valCreate(VT_ON_STACK, VO_ASSIGNED);
        d->valData(addr)->target = OBJ_RETURN;
        d->objects[OBJ_RETURN].root = addr;
    }

    CL_BREAK_IF(addr <= 0);
    SymHeapCore &self = /* FIXME */ const_cast<SymHeapCore &>(*this);
    return self.valByOffset(rootData.addr, d->objects[obj].off);
}

TObjId SymHeapCore::Private::rootLookup(TValId val) {
    if (VAL_NULL == val || this->valOutOfRange(val))
        return OBJ_INVALID;

    const EValueTarget code = this->values[val]->code;
    switch (code) {
        case VT_UNKNOWN:
            return /* XXX */ OBJ_UNKNOWN;

        default:
            if (!isPossibleToDeref(code))
                return OBJ_INVALID;
    }

    // root lookup
    const TValId valRoot = this->valRoot(val);
    return this->valData(valRoot)->target;
}

bool SymHeapCore::Private::gridLookup(
        TObjId                     *pFailCode,
        const TObjByType          **pRow,
        const TValId                val)
{
    const TObjId root = this->rootLookup(val);
    if (root < 0) {
        *pFailCode = root;
        return false;
    }

    // grid lookup
    const Private::Root &rootData = roMapLookup(this->roots, root);
    const TGrid &grid = rootData.grid;
    const TOffset off = this->values[val]->offRoot;
    TGrid::const_iterator it = grid.find(off);
    if (grid.end() == it) {
        *pFailCode = OBJ_UNKNOWN;
        return false;
    }

    const TObjByType *row = &it->second;
    CL_BREAK_IF(row->empty());
    *pRow = row;
    return true;
}

TObjId SymHeapCore::ptrAt(TValId at) {
    TObjId failCode;
    const TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    // seek a _data_ pointer at the given row
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
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

    const EValueTarget vt = d->values[at]->code;
    switch (vt) {
        case VT_COMPOSITE:
        case VT_CUSTOM:
            return OBJ_INVALID;

        default:
            break;
    }

    if (CL_TYPE_VOID == code && !d->values[at]->offRoot
            && isPossibleToDeref(vt))
        return /* XXX */ d->valData(at)->target;

    TObjId failCode;
    const TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    // seek the bigest object at the given row
    int maxSize = 0;
    TObjId max = OBJ_UNKNOWN;
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
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
    if (clt
            && CL_TYPE_PTR == clt->code
            && CL_TYPE_FNC != targetTypeOfPtr(clt)->code)
        // look for a pointer to data
        return this->ptrAt(at);

    TObjId failCode;
    const TObjByType *row;
    if (!d->gridLookup(&failCode, &row, at))
        return failCode;

    TObjByType::const_iterator it = row->find(clt);
    if (row->end() != it)
        // exact match
        return it->second;

    if (!clt)
        // no type-free object found
        return OBJ_UNKNOWN;

    // try semantic match
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
        const TObjType cltItem = item.first;
        if (cltItem && *cltItem == *clt)
            return item.second;
    }

    // not found
    return OBJ_UNKNOWN;
}

CVar SymHeapCore::cVarByRoot(TValId valRoot) const {
    // the following breakpoint checks everything
    CL_BREAK_IF(valRoot <= 0 || valRoot != d->valRoot(valRoot));

    const TObjId root = d->valData(valRoot)->target;
    const Private::Root &rootData = roMapLookup(d->roots, root);

    const CVar &cVar = rootData.cVar;
    CL_BREAK_IF(-1 == cVar.uid);
    return cVar;
}

TValId SymHeapCore::addrOfVar(CVar cv) {
    TValId addr = d->cVarMap.find(cv);
    if (0 < addr)
        return addr;

    // lazy creation of a program variable
    const CodeStorage::Var &var = stor_.vars[cv.uid];
    TObjType clt = var.type;
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
#if DEBUG_SE_STACK_FRAME
    const struct cl_loc *loc = 0;
    std::string varString = varTostring(stor_, cv.uid, &loc);
    CL_DEBUG_MSG(loc, "FFF SymHeapCore::objByCVar() creates var " << varString);
#endif
    // create the corresponding heap object
    const TObjId root = d->objCreate();
    d->objects[root].clt = clt;
    d->roots[root].cVar = cv;

    // assign an address
    const EValueTarget code = isOnStack(var) ? VT_ON_STACK : VT_STATIC;
    addr = d->valCreate(code, VO_ASSIGNED);
    d->valData(addr)->target = root;
    d->roots[root].addr = addr;
    d->objects[root].root = addr;

    // create the structure
    d->subsCreate(root);

    // store the address for next wheel
    d->cVarMap.insert(cv, addr);
    return addr;
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

TObjId SymHeapCore::valGetComposite(TValId val) const {
    CL_BREAK_IF(d->valOutOfRange(val));
    CL_BREAK_IF(VT_COMPOSITE != d->values[val]->code);

    const CompValue *valData = DCAST<CompValue *>(d->values[val]);
    return valData->compObj;
}

TValId SymHeapCore::heapAlloc(int cbSize) {
    const TObjId obj = d->objCreate();

    Private::Root &rootData = d->roots[obj];
    rootData.cbSize = cbSize;
    rootData.grid[/* off */ 0][/* clt */ 0] = obj;

    // assign an address
    const TValId addr = d->valCreate(VT_ON_HEAP, VO_ASSIGNED);
    d->valData(addr)->target = obj;
    d->roots[obj].addr = addr;
    d->objects[obj].root = addr;

    return addr;
}

bool SymHeapCore::valDestroyTarget(TValId val) {
    if (VAL_NULL == val || this->valOffset(val))
        return false;

    const TObjId target = d->valData(val)->target;
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
        //CL_BREAK_IF("SymHeapCore::objDefineType() sees a dangling value");
        d->releaseValueOf(obj);
    }

    // delayed object's type definition
    objData.clt = clt;
    if (OBJ_RETURN == obj)
        // XXX
        this->placedAt(OBJ_RETURN);

    d->subsCreate(obj);
}

void SymHeapCore::Private::objDestroy(TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    CL_BREAK_IF(this->objects[obj].off);
    Private::Root &rootData = roMapLookup(this->roots, obj);

    // remove the corresponding program variable (if any)
    EValueTarget code = VT_DELETED;
    const CVar cv = rootData.cVar;
    if (cv.uid != /* heap object */ -1) {
        this->cVarMap.remove(cv);
        code = VT_LOST;
    }

    // invalidate the address
    const TValId addr = rootData.addr;
    CL_BREAK_IF(valOutOfRange(addr));
    this->valData(addr)->target = OBJ_INVALID;
    this->valData(addr)->code   = code;

    // destroy the object
    this->subsDestroy(obj);
    if (OBJ_RETURN == obj) {
        // reinitialize OBJ_RETURN
        this->objects[OBJ_RETURN] = Private::Object();
        this->roots[OBJ_RETURN] = Private::Root();
    }
}

TValId SymHeapCore::valCreate(EValueTarget code, EValueOrigin origin) {
    return d->valCreate(code, origin);
}

TValId SymHeapCore::valCreateCustom(int cVal) {
    TCValueMap::iterator iter = d->cValueMap.find(cVal);
    if (d->cValueMap.end() == iter) {
        // cVal not found, create a new wrapper for it
        const TValId val = d->valCreate(VT_CUSTOM, VO_ASSIGNED);

        // initialize heap value
        CustomValue *valData = DCAST<CustomValue *>(d->values[val]);
        valData->customData  = cVal;

        // store cVal --> val mapping
        d->cValueMap[cVal] = val;

        return val;
    }

    // custom value already wrapped, we have to reuse it
    return iter->second;
}

int SymHeapCore::valGetCustom(TValId val) const
{
    CL_BREAK_IF(d->valOutOfRange(val));
    const BareValue *base = d->values[val];

    CL_BREAK_IF(VT_CUSTOM != base->code);
    const CustomValue *valData = DCAST<const CustomValue *>(base);
    return valData->customData;
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
    SymHeap &ref = DCAST<SymHeap &>(baseRef);
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

bool SymHeap::hasAbstractTarget(TValId val) const {
    return (OK_CONCRETE != this->valTargetKind(val));
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
    const EValueTarget code1 = this->valTarget(v1);
    const EValueTarget code2 = this->valTarget(v2);
    CL_BREAK_IF(isKnownObject(code2));

    if (VT_ABSTRACT != code1 && VT_ABSTRACT != code2) {
        // no abstract objects involved
        SymHeapCore::valMerge(v1, v2);
        return;
    }

    if (VT_ABSTRACT == code1 && spliceOutListSegment(*this, v1, v2))
        // splice-out succeeded ... ls(v1, v2)
        return;

    if (VT_ABSTRACT == code2 && spliceOutListSegment(*this, v2, v1))
        // splice-out succeeded ... ls(v2, v1)
        return;

    CL_DEBUG("failed to splice-out list segment, has to over-approximate");
}

void SymHeap::dlSegCrossNeqOp(ENeqOp op, TValId seg1) {
    // seek roots
    seg1 = this->valRoot(seg1);
    TValId seg2 = dlSegPeer(*this, seg1);

    // read the values (addresses of the surround)
    const TValId val1 = this->valueOf(nextPtrFromSeg(*this, seg1));
    const TValId val2 = this->valueOf(nextPtrFromSeg(*this, seg2));

    // add/del Neq predicates
    SymHeapCore::neqOp(op, val1, segHeadAt(*this, seg2));
    SymHeapCore::neqOp(op, val2, segHeadAt(*this, seg1));

    if (NEQ_DEL == op)
        // removing the 1+ flag implies removal of the 2+ flag
        SymHeapCore::neqOp(NEQ_DEL, seg1, seg2);
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
    CL_BREAK_IF(d->valOutOfRange(valB));
    const EValueTarget code = this->valTarget(valB);
    if (isKnownObject(code))
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
    if (root1 == root2) {
        CL_BREAK_IF("not tested");
        return true;
    }

    const TOffset off = this->valOffset(valA);
    if (!off)
        // roots differ
        return false;

    if (off != this->valOffset(valB))
        // offsets differ
        return false;

    // check for Neq between the roots
    return d->neqDb.areNeq(root1, root2);
}

bool SymHeap::proveNeq(TValId ref, TValId val) const {
    if (SymHeapCore::proveNeq(ref, val))
        return true;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, ref, val);
    const EValueTarget code = this->valTarget(ref);
    if (VAL_NULL != ref
            && !isKnownObject(code)
            && /* XXX */ !isGone(code)
            && /* XXX */ VT_CUSTOM != code)
        return false;

    std::set<TValId> haveSeen;

    while (0 < val && insertOnce(haveSeen, val)) {
        const EValueTarget code = this->valTarget(val);
        switch (code) {
            case VT_ON_STACK:
            case VT_ON_HEAP:
            case VT_STATIC:
            case VT_DELETED:
            case VT_LOST:
            case VT_CUSTOM:
                // concrete object reached --> prove done
                return (val != ref);

            case VT_ABSTRACT:
                break;

            default:
                // we can't prove much for unknown values
                return false;
        }

        if (SymHeapCore::proveNeq(ref, val))
            // prove done
            return true;

        SymHeap &writable = *const_cast<SymHeap *>(this);

        TValId seg = this->valRoot(val);
        if (OK_DLS == this->valTargetKind(val))
            seg = dlSegPeer(writable, seg);

        if (seg < 0)
            // no valid object here
            return false;

        const TValId valNext = this->valueOf(nextPtrFromSeg(writable, seg));
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
