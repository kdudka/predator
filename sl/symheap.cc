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
#include "symneq.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>
#include <map>
#include <set>

#include <boost/icl/interval_map.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifdef NDEBUG
    // aggressive optimization
#   define DCAST static_cast
#else
#   define DCAST dynamic_cast
#endif

namespace icl = boost::icl;

template <class TCont> typename TCont::value_type::second_type&
assignInvalidIfNotFound(
        TCont                                           &cont,
        const typename TCont::value_type::first_type    &item)
{
    if (!hasKey(cont, item))
        // -1 means "invalid", e.g. VAL_INVALID in case [T = map<???, TValId>]
        cont[item] = static_cast<typename TCont::value_type::second_type>(-1);

    return cont[item];
}

// /////////////////////////////////////////////////////////////////////////////
// Neq predicates store
class FriendlyNeqDb: public NeqDb {
    public:
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

        friend void SymHeapCore::copyRelevantPreds(
                SymHeapCore             &dst,
                const TValMap           &vMap)
            const;

        friend bool SymHeapCore::matchPreds(
                const SymHeapCore       &src,
                const TValMap           &vMap)
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
            // check for mapping redefinition
            CL_BREAK_IF(hasKey(cont_, cVar));

            // define mapping
            cont_[cVar] = val;
        }

        void remove(CVar cVar) {
            if (1 != cont_.erase(cVar))
                CL_BREAK_IF("offset detected in CVarMap::remove()");
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
};

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapCore
typedef std::set<TObjId>                                TObjSet;
typedef std::map<TOffset, TValId>                       TOffMap;
typedef std::map<TObjType, TObjId>                      TObjByType;
typedef std::map<TOffset, TObjByType>                   TGrid;
typedef icl::interval_map<unsigned, TObjSet>            TArena;
typedef TArena::key_type                                TMemChunk;
typedef TArena::value_type                              TMemItem;
typedef std::map<TObjId, bool /* isPtr */>              TLiveObjs;

inline TMemChunk createChunk(const TOffset off, const TObjType clt) {
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
    return TMemChunk(off, off + clt->size);
}

inline TObjSet createObjSet(const TObjId obj) {
    TObjSet single;
    single.insert(obj);
    return single;
}

inline TMemItem createArenaItem(
        const TOffset               off,
        const TObjType              clt,
        const TObjId                obj)
{
    return TMemItem(
            createChunk(off, clt),
            createObjSet(obj));
}

inline bool arenaLookup(
        TObjSet                     *dst,
        const TArena                &arena,
        const TOffset               off,
        const TObjType              clt,
        const TObjId                obj)
{
    TArena::const_iterator it = arena.find(createChunk(off, clt));
    if (arena.end() == it)
        // not found
        return false;

    // remove the reference object itself
    *dst = it->second;
    dst->erase(obj);

    // finally check if there was anything else
    return !dst->empty();
}

struct IHeapEntity {
    virtual ~IHeapEntity() { }
    virtual IHeapEntity* clone() const = 0;
};

struct HeapObject: public IHeapEntity {
    TValId                      root;
    TOffset                     off;
    TObjType                    clt;
    TValId                      value;

    HeapObject(TValId root_, TOffset off_, TObjType clt_):
        root(root_),
        off(off_),
        clt(clt_),
        value(VAL_INVALID)
    {
    }

    virtual IHeapEntity* clone() const {
        return new HeapObject(*this);
    }
};

struct BaseValue: public IHeapEntity {
    EValueTarget                    code;
    EValueOrigin                    origin;
    TOffset                         offRoot;
    TObjSet                         usedBy;

    BaseValue(EValueTarget code_, EValueOrigin origin_):
        code(code_),
        origin(origin_),
        offRoot(0)
    {
    }

    virtual BaseValue* clone() const {
        return new BaseValue(*this);
    }
};

struct CompValue: public BaseValue {
    TObjId                          compObj;

    CompValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_)
    {
    }

    virtual BaseValue* clone() const {
        return new CompValue(*this);
    }
};

struct InternalCustomValue: public BaseValue {
    CustomValue                     customData;

    InternalCustomValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_)
    {
    }

    virtual BaseValue* clone() const {
        return new InternalCustomValue(*this);
    }
};

struct OffValue: public BaseValue {
    TValId                          root;

    OffValue(
            EValueTarget            code_,
            EValueOrigin            origin_,
            TValId                  root_,
            TOffset                 offRoot_):
        BaseValue(code_, origin_),
        root(root_)
    {
        offRoot = offRoot_;
    }

    virtual BaseValue* clone() const {
        return new OffValue(*this);
    }
};

struct RootValue: public BaseValue {
    TOffMap                         offMap;
    TValId                          addr;
    TObjType                        lastKnownClt;
    unsigned                        cbSize;
    CVar                            cVar;
    bool                            initializedToZero;
    bool                            isProto;
    TLiveObjs                       liveObjs;
    TObjSet                         usedByGl;
    TGrid                           grid;
    TArena                          arena;

    RootValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_),
        addr(VAL_NULL),
        lastKnownClt(0),
        cbSize(0),
        initializedToZero(false),
        isProto(false)
    {
    }

    virtual BaseValue* clone() const {
        return new RootValue(*this);
    }
};

struct CustomValueMapper {
    private:
        typedef std::map<int, TValId>                           TCustomByInt;
        typedef std::map<long, TValId>                          TCustomByLong;
        typedef std::map<std::string, TValId>                   TCustomByString;

        TCustomByInt        fncMap;
        TCustomByLong       numMap;
        TCustomByString     strMap;

    public:
        TValId& lookup(const CustomValue &item) {
            const ECustomValue code = item.code;
            switch (code) {
                case CV_INVALID:
                default:
                    CL_BREAK_IF("invalid call of CustomValueMapper::lookup()");

                case CV_FNC:
                    return assignInvalidIfNotFound(fncMap, item.data.uid);

                case CV_INT:
                    return assignInvalidIfNotFound(numMap, item.data.num);

                case CV_STRING:
                    return assignInvalidIfNotFound(strMap, item.data.str);
            }
        }
};

struct SymHeapCore::Private {
    // allocate a placeholder for VAL_NULL
    Private();

    // clone heap entities, they are now allocated separately
    Private(const Private &);

    // delete heap entities, they are now allocated separately
    ~Private();

    CVarMap                         cVarMap;
    CustomValueMapper               cValueMap;
    std::vector<IHeapEntity *>      ents;
    std::set<TValId>                liveRoots;
    FriendlyNeqDb                   neqDb;

    template <typename T> T lastId() const;

    inline bool valOutOfRange(TValId);
    inline bool objOutOfRange(TObjId);

    HeapObject* objData(const TObjId);
    BaseValue* valData(const TValId);
    RootValue* rootData(const TValId);

    inline TValId valRoot(const TValId, const IHeapEntity *);
    inline TValId valRoot(const TValId);

    TValId valCreate(EValueTarget code, EValueOrigin origin);
    TValId valDup(TValId);

    TObjId objCreate(TValId root, TOffset off, TObjType clt);
    TValId objInit(TObjId obj);
    TValId dupRoot(TValId root);
    void destroyRoot(TValId obj);

    bool /* wasPtr */ releaseValueOf(TObjId obj, TValId val);
    void registerValueOf(TObjId obj, TValId val);
    void reinterpretObjData(TObjId old, TObjId obj);
    void setValueOf(TObjId of, TValId val);

    bool gridLookup(TObjByType **pRow, const TValId);
    TObjId lazyCreatePtr(TStorRef stor, TValId at);

    void neqOpWrap(SymHeap::ENeqOp, TValId, TValId);

    private:
        // intentionally not implemented
        Private& operator=(const Private &);
};

template <typename T> T SymHeapCore::Private::lastId() const {
    return static_cast<T>(this->ents.size() - 1);
}

inline bool SymHeapCore::Private::valOutOfRange(TValId val) {
    return (val <= 0)
        || (this->lastId<TValId>() < val);
}

inline bool SymHeapCore::Private::objOutOfRange(TObjId obj) {
    return (obj < 0)
        || (this->lastId<TObjId>() < obj);
}

inline HeapObject* SymHeapCore::Private::objData(const TObjId obj) {
    CL_BREAK_IF(objOutOfRange(obj));
    IHeapEntity *ent = this->ents[obj];

    // check the base pointer first, chances are the object was already deleted
    CL_BREAK_IF(!ent);

    return DCAST<HeapObject *>(ent);
}

inline BaseValue* SymHeapCore::Private::valData(const TValId val) {
    CL_BREAK_IF(valOutOfRange(val));
    IHeapEntity *ent = this->ents[val];
    return DCAST<BaseValue *>(ent);
}

inline RootValue* SymHeapCore::Private::rootData(const TValId val) {
    CL_BREAK_IF(valOutOfRange(val));
    IHeapEntity *ent = this->ents[val];
    return DCAST<RootValue *>(ent);
}

inline TValId SymHeapCore::Private::valRoot(
        const TValId                val,
        const IHeapEntity          *ent)
{
    const BaseValue *valData = DCAST<const BaseValue *>(ent);
    if (!valData->offRoot)
        return val;

    const TValId valRoot = DCAST<const OffValue *>(valData)->root;
    CL_BREAK_IF(valOutOfRange(valRoot));
    return valRoot;
}

inline TValId SymHeapCore::Private::valRoot(const TValId val) {
    return this->valRoot(val, this->valData(val));
}

bool /* wasPtr */ SymHeapCore::Private::releaseValueOf(TObjId obj, TValId val) {
    if (val <= 0)
        // we do not track uses of special values
        return /* wasPtr */ false;

    BaseValue *valData = this->valData(val);
    if (1 != valData->usedBy.erase(obj))
        CL_BREAK_IF("SymHeapCore::Private::releaseValueOf(): offset detected");

    const TValId root = this->valRoot(val, valData);
    valData = this->valData(root);
    if (!isPossibleToDeref(valData->code))
        return /* wasPtr */ false;

    RootValue *rootData = DCAST<RootValue *>(valData);
    if (1 != rootData->usedByGl.erase(obj))
        CL_BREAK_IF("SymHeapCore::Private::releaseValueOf(): offset detected");

    return /* wasPtr */ true;
}

void SymHeapCore::Private::registerValueOf(TObjId obj, TValId val) {
    if (val <= 0)
        return;

    // update usedBy
    BaseValue *valData = this->valData(val);
    valData->usedBy.insert(obj);
    if (!isPossibleToDeref(valData->code))
        return;

    // update usedByGl
    const TValId root = this->valRoot(val, valData);
    RootValue *rootData = this->rootData(root);
    rootData->usedByGl.insert(obj);
}

void SymHeapCore::Private::reinterpretObjData(TObjId old, TObjId obj) {
    HeapObject *oldData = this->objData(old);
    if (isComposite(oldData->clt))
        // do not invalidate those place-holding values of composite objects
        return;

    CL_DEBUG("reinterpretObjData() is taking place...");
    HeapObject *objData = this->objData(obj);
    // TODO: hook various reinterpretation drivers here
    (void) objData;

    if (/* wasPtr */ this->releaseValueOf(old, oldData->value))
        // FIXME: this needs to be somehow solved at a higher-level
        CL_WARN("reinterpretObjData() ignores possible memory leakage");

    // assign a fresh VO_REINTERPRET value
    const TValId val = this->valCreate(VT_UNKNOWN, VO_REINTERPRET);
    oldData->value = val;
    this->registerValueOf(old, val);

    // mark the object as dead
    const TValId root = oldData->root;
    RootValue *rootData = this->rootData(root);
    if (rootData->liveObjs.erase(old))
        CL_DEBUG("reinterpretObjData() kills a live object");
}

void SymHeapCore::Private::setValueOf(TObjId obj, TValId val) {
    // release old value
    HeapObject *objData = DCAST<HeapObject *>(this->ents[obj]);
    this->releaseValueOf(obj, objData->value);

    // store new value
    objData->value = val;
    this->registerValueOf(obj, val);

    // resolve root
    const TValId root = objData->root;
    RootValue *rootData = this->rootData(root);
    const TArena &arena = rootData->arena;
    const TOffset off = objData->off;
    const TObjType clt = objData->clt;

    // invalidate contents of the objects we are overwriting
    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, off, clt, obj)) {
        BOOST_FOREACH(const TObjId old, overlaps)
            this->reinterpretObjData(old, obj);
    }
}

TObjId SymHeapCore::Private::objCreate(TValId root, TOffset off, TObjType clt) {
    // acquire object ID
    HeapObject *objData = new HeapObject(root, off, clt);
    this->ents.push_back(objData);
    const TObjId obj = this->lastId<TObjId>();

    // register the object by the owning root value
    RootValue *rootData = this->rootData(root);
    TObjByType &row = rootData->grid[off];
    CL_BREAK_IF(hasKey(row, clt));
    row[clt] = obj;

    // map the region occupied by the object
    rootData->arena += createArenaItem(off, clt, obj);
    return obj;
}

TValId SymHeapCore::Private::valCreate(
        EValueTarget                code,
        EValueOrigin                origin)
{
    switch (code) {
        case VT_INVALID:
        case VT_UNKNOWN:
            this->ents.push_back(new BaseValue(code, origin));
            break;

        case VT_COMPOSITE:
            this->ents.push_back(new CompValue(code, origin));
            break;

        case VT_CUSTOM:
            this->ents.push_back(new InternalCustomValue(code, origin));
            break;

        case VT_ABSTRACT:
            CL_BREAK_IF("invalid call of SymHeapCore::Private::valCreate()");
            // fall through!

        case VT_ON_HEAP:
        case VT_ON_STACK:
        case VT_STATIC:
        case VT_DELETED:
        case VT_LOST:
            this->ents.push_back(new RootValue(code, origin));
            break;
    }

    return this->lastId<TValId>();
}

TValId SymHeapCore::Private::valDup(TValId val) {
    // deep copy the value
    const BaseValue *tpl = this->valData(val);
    this->ents.push_back(/* FIXME: subtle */ tpl->clone());

    // wipe BaseValue::usedBy
    const TValId dup = this->lastId<TValId>();
    BaseValue *dupData = this->valData(dup);
    dupData->usedBy.clear();

    return dup;
}

SymHeapCore::Private::Private() {
    // allocate a placeholder for VAL_NULL
    this->ents.push_back(static_cast<IHeapEntity *>(0));
}

SymHeapCore::Private::Private(const SymHeapCore::Private &ref):
    cVarMap     (ref.cVarMap),
    cValueMap   (ref.cValueMap),
    ents        (ref.ents),
    liveRoots   (ref.liveRoots),
    neqDb       (ref.neqDb)
{
    // deep copy of all heap entities
    BOOST_FOREACH(IHeapEntity *&ent, this->ents)
        if (ent)
            ent = ent->clone();
}

SymHeapCore::Private::~Private() {
    BOOST_FOREACH(const IHeapEntity *ent, this->ents)
        delete ent;
}

EValueOrigin originByCode(const EValueTarget code) {
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
            CL_BREAK_IF("invalid call of originByCode");
            return VO_INVALID;
    }
}

TValId SymHeapCore::Private::objInit(TObjId obj) {
    HeapObject *objData = this->objData(obj);

    // resolve root
    const TValId root = objData->root;
    RootValue *rootData = this->rootData(root);
    const TArena &arena = rootData->arena;
    const TOffset off = objData->off;
    const TObjType clt = objData->clt;

    // first check for data reinterpretation
    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, off, clt, obj)) {
        BOOST_FOREACH(const TObjId other, overlaps) {
            if (!hasKey(rootData->liveObjs, other))
                // the other object is already dead anyway
                continue;

            // reinterpret _self_ by another live object
            this->reinterpretObjData(/* old */ obj, other);
            return objData->value;
        }
    }

    // deleayed creation of an uninitialized value
    if (rootData->initializedToZero) {
        objData->value = VAL_NULL;
        return VAL_NULL;
    }

    // assign a fresh unknown value
    const EValueTarget code = rootData->code;
    const EValueOrigin origin = originByCode(code);
    const TValId val = this->valCreate(VT_UNKNOWN, origin);
#if SE_TRACK_UNINITIALIZED
    objData->value = val;
#else
    return val;
#endif

    // mark the object as live
    rootData->liveObjs[obj] = /* isPtr */ isDataPtr(clt);

    // store backward reference
    this->valData(val)->usedBy.insert(obj);
    return val;
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
            return d->valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        default:
            break;
    }

    HeapObject *objData = d->objData(obj);
    TValId &val = objData->value;
    if (VAL_INVALID != val)
        // the object has a value
        return val;

    const TObjType clt = objData->clt;
    if (isComposite(clt)) {
        // deleayed creation of a composite value
        val = d->valCreate(VT_COMPOSITE, VO_INVALID);
        CompValue *valData = DCAST<CompValue *>(d->ents[val]);
        valData->compObj = obj;

        // store backward reference
        d->valData(val)->usedBy.insert(obj);
        return val;
    }

    // deleayed object initialization
    return d->objInit(obj);
}

void SymHeapCore::usedBy(TObjList &dst, TValId val) const {
    if (VAL_NULL == val)
        return;

    const BaseValue *valData = d->valData(val);
    const TObjSet &usedBy = valData->usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

unsigned SymHeapCore::usedByCount(TValId val) const {
    if (VAL_NULL == val)
        return 0;

    const BaseValue *valData = d->valData(val);
    return valData->usedBy.size();
}

void SymHeapCore::pointedBy(TObjList &dst, TValId root) const {
    const RootValue *rootData = d->rootData(root);
    CL_BREAK_IF(rootData->offRoot);
    CL_BREAK_IF(!isPossibleToDeref(rootData->code));

    const TObjSet &usedBy = rootData->usedByGl;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

unsigned SymHeapCore::pointedByCount(TValId root) const {
    const RootValue *rootData = d->rootData(root);
    CL_BREAK_IF(rootData->offRoot);
    CL_BREAK_IF(!isPossibleToDeref(rootData->code));
    return rootData->usedBy.size();
}

unsigned SymHeapCore::lastId() const {
    return d->lastId<unsigned>();
}

TValId SymHeapCore::valClone(TValId val) {
    const BaseValue *valData = d->valData(val);
    const EValueTarget code = valData->code;
    if (VT_CUSTOM == code) {
        CL_BREAK_IF("custom values are not supposed to be cloned");
        return val;
    }

    if (isProgramVar(code)) {
        CL_BREAK_IF("program variables are not supposed to be cloned");
        return val;
    }

    if (!isPossibleToDeref(code))
        // duplicate an unknown value
        return d->valDup(val);

    // duplicate a root object
    const TValId root = d->valRoot(val, valData);
    const TValId dupAt = d->dupRoot(root);

    // take the offset into consideration
    return this->valByOffset(dupAt, valData->offRoot);
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

TValId SymHeapCore::Private::dupRoot(TValId rootAt) {
    CL_DEBUG("SymHeapCore::Private::dupRoot() is taking place...");
    const RootValue *rootDataSrc = this->rootData(rootAt);

    // assign an address to the clone
    const EValueTarget code = rootDataSrc->code;
    const TValId imageAt = this->valCreate(code, VO_ASSIGNED);
    RootValue *rootDataDst = this->rootData(imageAt);
    rootDataDst->addr = imageAt;

    // duplicate root metadata
    rootDataDst->cVar               = rootDataSrc->cVar;
    rootDataDst->isProto            = rootDataSrc->isProto;
    rootDataDst->initializedToZero  = rootDataSrc->initializedToZero;
    rootDataDst->cbSize             = rootDataSrc->cbSize;
    rootDataDst->lastKnownClt       = rootDataSrc->lastKnownClt;

    this->liveRoots.insert(imageAt);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootDataSrc->liveObjs) {
        const HeapObject *objDataSrc = this->objData(/* src */ item.first);

        // duplicate a single object
        const TOffset off = objDataSrc->off;
        const TObjType clt = objDataSrc->clt;
        const TObjId dst = this->objCreate(imageAt, off, clt);
        this->setValueOf(dst, objDataSrc->value);

        // prevserve live ptr/data object
        rootDataDst->liveObjs[dst] = /* isPtr */ item.second;
    }

    return imageAt;
}

void SymHeapCore::gatherLivePointers(TObjList &dst, TValId root) const {
    const RootValue *rootData = d->rootData(root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        if (/* isPtr */ item.second)
            dst.push_back(/* obj */ item.first);
    }
}

void SymHeapCore::gatherLiveObjects(TObjList &dst, TValId root) const {
    const RootValue *rootData = d->rootData(root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs)
        dst.push_back(/* obj */ item.first);
}

SymHeapCore::SymHeapCore(TStorRef stor):
    stor_(stor),
    d(new Private)
{
    CL_BREAK_IF(!&stor_);

    // initialize VAL_ADDR_OF_RET
    const TValId addrRet = d->valCreate(VT_ON_STACK, VO_ASSIGNED);
    CL_BREAK_IF(VAL_ADDR_OF_RET != addrRet);
    d->rootData(addrRet)->addr = addrRet;
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
    // we allow to set values of atomic types only
    const HeapObject *objData = d->objData(obj);
    const TObjType clt = objData->clt;
    CL_BREAK_IF(isComposite(clt));

    // mark the destination object as live
    const TValId root = objData->root;
    RootValue *rootData = d->rootData(root);
    rootData->liveObjs[obj] = /* isPtr */ isDataPtr(clt);

    // now set the value
    d->setValueOf(obj, val);
}

TObjType SymHeapCore::objType(TObjId obj) const {
    if (obj < 0)
        return 0;

    const HeapObject *objData = d->objData(obj);
    return objData->clt;
}

TValId SymHeapCore::valByOffset(TValId at, TOffset off) {
    if (!off || at <= 0)
        return at;

    // subtract the root
    const BaseValue *valData = d->valData(at);
    const TValId valRoot = d->valRoot(at, valData);
    off += valData->offRoot;
    if (!off)
        return valRoot;

    const EValueTarget code = valData->code;
    if (VT_UNKNOWN == code)
        // do not track off-value for invalid targets
        return d->valDup(at);

    // off-value lookup
    RootValue *rootData = d->rootData(valRoot);
    TOffMap &offMap = rootData->offMap;
    TOffMap::const_iterator it = offMap.find(off);
    if (offMap.end() != it)
        return it->second;

    // create a new off-value
    d->ents.push_back(new OffValue(code, valData->origin, valRoot, off));
    const TValId val = d->lastId<TValId>();

    // store the mapping for next wheel
    offMap[off] = val;
    return val;
}

EValueOrigin SymHeapCore::valOrigin(TValId val) const {
    switch (val) {
        case VAL_INVALID:
            return VO_INVALID;

        case VAL_NULL /* = VAL_FALSE */:
        case VAL_TRUE:
            return VO_ASSIGNED;

        default:
            break;
    }

    const BaseValue *valData = d->valData(val);
    return valData->origin;
}

EValueTarget SymHeapCore::valTarget(TValId val) const {
    if (val <= 0)
        return VT_INVALID;

    if (this->hasAbstractTarget(val))
        // the overridden implementation claims the target is abstract
        return VT_ABSTRACT;

    const BaseValue *valData = d->valData(val);
    return valData->code;
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

    const BaseValue *valData = d->valData(val);
    return valData->offRoot;
}

void SymHeapCore::valReplace(TValId val, TValId replaceBy) {
    const BaseValue *valData = d->valData(val);

    // we intentionally do not use a reference here (tight loop otherwise)
    TObjSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TObjId obj, usedBy)
        this->objSetValue(obj, replaceBy);

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
    if (obj < 0)
        return VAL_INVALID;

    // jump to root
    const HeapObject *objData = d->objData(obj);
    const RootValue *rootData = d->rootData(objData->root);
    const TValId addr = rootData->addr;
    CL_BREAK_IF(addr <= 0);

    // then subtract the offset
    SymHeapCore &writable = /* FIXME */ const_cast<SymHeapCore &>(*this);
    return writable.valByOffset(addr, objData->off);
}

bool SymHeapCore::Private::gridLookup(TObjByType **pRow, const TValId val) {
    if (val <= 0)
        return false;

    const BaseValue *valData = this->valData(val);
    const EValueTarget code = valData->code;
    if (!isPossibleToDeref(code))
        return false;

    // jump to root
    const TValId valRoot = this->valRoot(val, valData);
    RootValue *rootData = this->rootData(valRoot);

    // grid lookup
    TGrid &grid = rootData->grid;
    const TOffset off = valData->offRoot;
    *pRow = &grid[off];
    return true;
}

template <class TPred>
class CltFinder {
    private:
        const TObjType          cltRoot_;
        const TOffset           offToSeek_;
        TObjType                cltFound_;
        TPred                  &pred_;

    public:
        CltFinder(TObjType cltRoot, TOffset offToSeek, TPred &pred):
            cltRoot_(cltRoot),
            offToSeek_(offToSeek),
            cltFound_(/* just to silence GCC with -O2 */ 0),
            pred_(pred)
        {
        }

        TObjType cltFound() const { return cltFound_; }

        bool operator()(const TFieldIdxChain &ic, const struct cl_type_item *it)
        {
            const TObjType clt = it->type;
            if (!pred_(clt))
                return /* continue */ true;

            const TOffset off = offsetByIdxChain(cltRoot_, ic);
            if (offToSeek_ != off)
                return /* continue */ true;

            // matched!
            cltFound_ = clt;
            return false;
        }
};

template <class TPred>
TObjType guideCltFinder(
        const TObjType          cltRoot,
        const TOffset           off,
        TPred                  &pred)
{
    CltFinder<TPred> visitor(cltRoot, off, pred);

    // first check the root itself
    const TFieldIdxChain ic;
    struct cl_type_item item;
    item.type = cltRoot;
    if (!visitor(ic, &item))
        return visitor.cltFound();

    // traverse the type-info
    if (!traverseTypeIc(cltRoot, visitor, /* digOnlyComposite */ true))
        return visitor.cltFound();

    // not found
    return 0;
}

TObjId SymHeapCore::Private::lazyCreatePtr(TStorRef stor, TValId at) {
    // check offset
    const BaseValue *valData = this->valData(at);
    const TOffset off = valData->offRoot;

    // jump to root
    const TValId root = this->valRoot(at, valData);
    /* const */ RootValue *rootData = this->rootData(root);

    // check root type-info
    const TObjType cltRoot = rootData->lastKnownClt;

    TObjType clt = 0;
    if (cltRoot)
        // try the best match of type-info for the pointer
        clt = guideCltFinder(cltRoot, off, isDataPtr);

    if (!clt)
        // try to use a generic data pointer
        clt = stor.types.genericDataPtr();

    if (!clt) {
        CL_BREAK_IF("critical lack of type-info");
        return OBJ_INVALID;
    }

    if (!cltRoot && !off)
        rootData->lastKnownClt = clt;

    // create a new pointer
    return this->objCreate(root, off, clt);
}

TObjId SymHeapCore::ptrAt(TValId at) {
    TObjByType *row;
    if (!d->gridLookup(&row, at))
        return OBJ_INVALID;

    // seek a _data_ pointer at the given row
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
        const TObjType clt = item.first;
        if (isDataPtr(clt))
            // matched
            return item.second;
    }

    if (this->valSizeOfTarget(at) < stor_.types.dataPtrSizeof())
        // TODO: return a more specific error code (out of range)
        return OBJ_UNKNOWN;

    return d->lazyCreatePtr(stor_, at);
}

TObjId SymHeapCore::objAt(TValId at, TObjCode code) {
    TObjByType *row;
    if (!d->gridLookup(&row, at))
        return OBJ_INVALID;

    // seek the bigest object at the given row
    int maxSize = 0;
    TObjId max = OBJ_UNKNOWN;
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
        const TObjType cltItem = item.first;
        CL_BREAK_IF(!cltItem);

        const TObjId obj = item.second;
        CL_BREAK_IF(d->objOutOfRange(obj));

        if (CL_TYPE_VOID != code && (cltItem->code != code))
            // not the type we are looking for
            continue;

        const int size = cltItem->size;
        if (size < maxSize)
            continue;

        if ((size == maxSize) && !isComposite(cltItem))
            // if two types have the same size, prefer the composite one
            continue;

        // update the maximum
        maxSize = size;
        max = obj;
    }

    if (OBJ_UNKNOWN != max)
        return max;

    const BaseValue *valData = d->valData(at);
    const TValId root = d->valRoot(at, valData);
    const TOffset off = valData->offRoot;
    if (off)
        // TODO
        return OBJ_UNKNOWN;

    RootValue *rootData = d->rootData(root);
    const TObjType clt = rootData->lastKnownClt;
    if (CL_TYPE_VOID != code && code != clt->code)
        // TODO
        return OBJ_UNKNOWN;

    return d->objCreate(root, off, clt);
}

TObjId SymHeapCore::objAt(TValId at, TObjType clt) {
    CL_BREAK_IF(!clt);
    if (isDataPtr(clt))
        // look for a pointer to data
        return this->ptrAt(at);

    TObjByType *row;
    if (!d->gridLookup(&row, at))
        return OBJ_INVALID;

    TObjByType::const_iterator it = row->find(clt);
    if (row->end() != it)
        // exact match
        return it->second;

    // try semantic match
    BOOST_FOREACH(const TObjByType::const_reference item, *row) {
        const TObjType cltItem = item.first;
        CL_BREAK_IF(!cltItem);
        if (*cltItem == *clt)
            return item.second;
    }

    if (this->valSizeOfTarget(at) < clt->size)
        // TODO: return a more specific error code (out of range)
        return OBJ_UNKNOWN;

    // create the object
    const BaseValue *valData = d->valData(at);
    const TValId root = d->valRoot(at, valData);
    const TOffset off = valData->offRoot;
    return d->objCreate(root, off, clt);
}

CVar SymHeapCore::cVarByRoot(TValId valRoot) const {
    const RootValue *rootData = d->rootData(valRoot);
    return rootData->cVar;
}

TValId SymHeapCore::addrOfVar(CVar cv) {
    TValId addr = d->cVarMap.find(cv);
    if (0 < addr)
        return addr;

    // lazy creation of a program variable
    const CodeStorage::Var &var = stor_.vars[cv.uid];
    if (!isOnStack(var))
        cv.inst = /* gl var */ 0;

    TObjType clt = var.type;
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
#if DEBUG_SE_STACK_FRAME
    const struct cl_loc *loc = 0;
    std::string varString = varToString(stor_, cv.uid, &loc);
    CL_DEBUG_MSG(loc, "FFF SymHeapCore::addrOfVar() creates var " << varString);
#endif

    // assign an address
    const EValueTarget code = isOnStack(var) ? VT_ON_STACK : VT_STATIC;
    addr = d->valCreate(code, VO_ASSIGNED);
    d->liveRoots.insert(addr);

    RootValue *rootData = d->rootData(addr);
    rootData->cVar = cv;
    rootData->addr = addr;
    rootData->lastKnownClt = clt;
    rootData->cbSize = clt->size;
#if SE_ASSUME_FRESH_STATIC_DATA
    if (VT_STATIC == code)
        rootData->initializedToZero = true;
#endif

    // store the address for next wheel
    d->cVarMap.insert(cv, addr);
    return addr;
}

static bool dummyFilter(EValueTarget) {
    return true;
}

void SymHeapCore::gatherRootObjects(TValList &dst, bool (*filter)(EValueTarget))
    const
{
    if (!filter)
        filter = dummyFilter;

    BOOST_FOREACH(const TValId at, d->liveRoots)
        if (filter(this->valTarget(at)))
            dst.push_back(at);
}

TObjId SymHeapCore::valGetComposite(TValId val) const {
    const BaseValue *valData = d->valData(val);
    CL_BREAK_IF(VT_COMPOSITE != valData->code);

    const CompValue *compData = DCAST<const CompValue *>(valData);
    return compData->compObj;
}

TValId SymHeapCore::heapAlloc(int cbSize, bool nullify) {
    // assign an address
    const TValId addr = d->valCreate(VT_ON_HEAP, VO_ASSIGNED);

    // initialize meta-data
    RootValue *rootData = d->rootData(addr);
    rootData->addr = addr;
    rootData->cbSize = cbSize;
    rootData->initializedToZero = nullify;

    return addr;
}

bool SymHeapCore::untouchedContentsIsNullified(TValId root) const {
    const RootValue *rootData = d->rootData(root);
    return rootData->initializedToZero;
}

bool SymHeapCore::valDestroyTarget(TValId val) {
    if (VAL_NULL == val)
        // VAL_NULL has no target
        return false;

    const BaseValue *valData = d->valData(val);
    if (valData->offRoot || !isPossibleToDeref(valData->code))
        // such a target is not supposed to be destroyed
        return false;

    d->destroyRoot(val);
    return true;
}

int SymHeapCore::valSizeOfTarget(TValId val) const {
    const BaseValue *valData = d->valData(val);
    const EValueTarget code = valData->code;
    if (isGone(code))
        return 0;

    CL_BREAK_IF(!isPossibleToDeref(valData->code));
    const TValId root = d->valRoot(val, valData);
    const RootValue *rootData = d->rootData(root);

    const int rootSize = rootData->cbSize;
    const TOffset off = valData->offRoot;
    return rootSize - off;
}

void SymHeapCore::valSetLastKnownTypeOfTarget(TValId root, TObjType clt) {
    RootValue *rootData = d->rootData(root);

    if (VAL_ADDR_OF_RET == root) {
        // destroy any stale target of VAL_ADDR_OF_RET
        d->destroyRoot(root);

        // allocate a new root value at VAL_ADDR_OF_RET
        rootData->code = VT_ON_STACK;
    }

    // convert a type-free object into a type-aware object
    rootData->lastKnownClt = clt;
    rootData->cbSize = clt->size;
    d->liveRoots.insert(root);
}

TObjType SymHeapCore::valLastKnownTypeOfTarget(TValId root) const {
    CL_BREAK_IF(d->valData(root)->offRoot);
    const RootValue *rootData = d->rootData(root);
    return rootData->lastKnownClt;
}

void SymHeapCore::Private::destroyRoot(TValId root) {
    RootValue *rootData = this->rootData(root);

    EValueTarget code = VT_DELETED;
    const CVar cv = rootData->cVar;
    if (cv.uid != /* heap object */ -1) {
        // remove the corresponding program variable
        this->cVarMap.remove(cv);
        code = VT_LOST;
    }

    // mark the root value as deleted/lost
    rootData->code = code;

    // mark all associated off-values as deleted/lost
    BOOST_FOREACH(TOffMap::const_reference item, rootData->offMap) {
        const TValId val = item.second;
        BaseValue *valData = this->valData(val);
        CL_BREAK_IF(!dynamic_cast<OffValue *>(valData));

        valData->code = code;
    }

    // release the root
    this->liveRoots.erase(root);

    // go through objects
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const TObjId obj = item.first;
        HeapObject *objData = this->objData(obj);
        objData->clt = 0;

        // release value of the object
        const TValId val = objData->value;
        this->releaseValueOf(obj, val);

        // release the corresponding HeapObject instance
        delete objData;
        this->ents[obj] = 0;
    }

    // wipe rootData
    rootData->lastKnownClt = 0;
    rootData->liveObjs.clear();
    rootData->grid.clear();
    rootData->arena.clear();
}

TValId SymHeapCore::valCreate(EValueTarget code, EValueOrigin origin) {
    return d->valCreate(code, origin);
}

TValId SymHeapCore::valWrapCustom(const CustomValue &cVal) {
    const ECustomValue code = cVal.code;
    if (CV_INT == code) {
        // short-circuit for special integral values
        const long num = cVal.data.num;
        switch (num) {
            case 0:
                return VAL_NULL;

            case 1:
                return VAL_TRUE;

            default:
                break;
        }
    }

    TValId &val = d->cValueMap.lookup(cVal);
    if (VAL_INVALID != val)
        // custom value already wrapped, we have to reuse it
        return val;

    // cVal not found, wrap it as a new heap value
    val = d->valCreate(VT_CUSTOM, VO_ASSIGNED);
    InternalCustomValue *valData = DCAST<InternalCustomValue *>(d->ents[val]);
    valData->customData = cVal;
    return val;
}

const CustomValue& SymHeapCore::valUnwrapCustom(TValId val) const
{
    const InternalCustomValue *valData =
        DCAST<InternalCustomValue *>(d->ents[val]);

    // check the consistency of backward mapping
    CL_BREAK_IF(val != d->cValueMap.lookup(valData->customData));

    return valData->customData;
}

bool SymHeapCore::valTargetIsProto(TValId val) const {
    if (val <= 0)
        // not a prototype for sure
        return false;

    const BaseValue *valData = d->valData(val);
    if (!isPossibleToDeref(valData->code))
        // not a prototype for sure
        return false;

    // seek root
    const TValId root = d->valRoot(val, valData);
    const RootValue *rootData = d->rootData(root);
    return rootData->isProto;
}

void SymHeapCore::valTargetSetProto(TValId val, bool isProto) {
    const TValId root = d->valRoot(val);
    RootValue *rootData = d->rootData(root);
    rootData->isProto = isProto;
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

    if (OK_MAY_EXIST == kind)
        // there is no 'prev' offset in OK_MAY_EXIST
        aoData.off.prev = off.next;
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
        SymHeapCore::valReplace(v2, v1);
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
    if (haveMayExistAt(*this, valA, valB)) {
        // replace OK_MAY_EXIST at valA by OK_CONCRETE
        this->valTargetSetConcrete(valA);
        return;
    }

    if (haveMayExistAt(*this, valB, valA)) {
        // replace OK_MAY_EXIST at valB by OK_CONCRETE
        this->valTargetSetConcrete(valB);
        return;
    }

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
    if (VT_CUSTOM == code || isKnownObject(code))
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
    if (this->hasAbstractTarget(ref) && this->hasAbstractTarget(val)) {
        const TValId seg = this->valRoot(val);
        if (objMinLength(*this, seg))
            // move the non-empty one to left
            swapValues(ref, val);
    }

    const EValueTarget code = this->valTarget(ref);
    if (isAbstract(code)) {
        // both values are abstract
        const TValId root1 = this->valRoot(ref);
        const TValId root2 = this->valRoot(val);
        if (root2 == segPeer(*this, root1))
            // we already know this is not a DLS 2+
            return false;

        if (!objMinLength(*this, root1))
            // both targets are possibly empty, giving up
            return false;
    }

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
