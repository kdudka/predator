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
#include "syments.hh"
#include "symneq.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#ifdef USE_BOOST_ICL
#   include <boost/icl/interval_map.hpp>
#else
#   include "intarena.hh"
#endif

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

#ifdef USE_BOOST_ICL
typedef boost::icl::interval_map<unsigned, TObjSet>     TArena;
#else
typedef IntervalArena<int, TObjId>                      TArena;
#endif

typedef TArena::key_type                                TMemChunk;
typedef TArena::value_type                              TMemItem;

inline TMemItem createArenaItem(
        const TOffset               off,
        const unsigned              size,
        const TObjId                obj)
{
    const TMemChunk chunk(off, off + size);
#ifdef USE_BOOST_ICL
    TObjSet singleObj;
    singleObj.insert(obj);
    return TMemItem(chunk, singleObj);
#else
    return TMemItem(chunk, obj);
#endif
}

inline bool arenaLookup(
        TObjSet                     *dst,
        const TArena                &arena,
        const TMemChunk             &chunk,
        const TObjId                obj)
{
#if USE_BOOST_ICL
    TArena::const_iterator it = arena.find(chunk);
    if (arena.end() == it)
        // not found
        return false;

    // FIXME: Doing this is ultimately stupid.  Are we using icl in a wrong way?
    TArena tmp(arena);
    tmp &= chunk;
    for (it = tmp.begin(); tmp.end() != it; ++it) {
        const TObjSet &objs = it->second;
        std::copy(objs.begin(), objs.end(), std::inserter(*dst, dst->begin()));
    }
#else
    arena.intersects(*dst, chunk);
#endif

    if (OBJ_INVALID != obj)
        // remove the reference object itself
        dst->erase(obj);

    // finally check if there was anything else
    return !dst->empty();
}

inline void arenaLookForExactMatch(
        TObjSet                     *dst,
        const TArena                &arena,
        const TMemChunk             &chunk)
{
#if USE_BOOST_ICL
    // FIXME: Doing this is ultimately stupid.  Are we using icl in a wrong way?
    TArena tmp(arena);
    tmp &= chunk;
    BOOST_FOREACH(TArena::const_reference ref, tmp) {
        const TMemChunk now(ref.first);
        if (now != chunk)
            continue;

        const TObjSet &objs = ref.second;
        std::copy(objs.begin(), objs.end(), std::inserter(*dst, dst->begin()));
    }
#else
    arena.exactMatch(*dst, chunk);
#endif
}

inline TMemChunk createChunk(const TOffset off, const TObjType clt) {
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);

    // I believe a right-open interval is going to be used by default, see
    // boost_1_46_1/libs/icl/doc/html/boost_icl/interface.html for details
    return TMemChunk(off, off + clt->size);
}

enum ELiveObj {
    LO_INVALID,
    LO_BLOCK,
    LO_DATA_PTR,
    LO_DATA
};

typedef std::map<TObjId, ELiveObj>                      TLiveObjs;

enum EBlockKind {
    BK_INVALID,
    BK_OBJECT,
    BK_COMPOSITE,
    BK_UNIFORM
};

struct HeapBlock: public AbstractHeapEntity {
    EBlockKind                  code;
    TValId                      root;
    TOffset                     off;
    TValId                      value;

    HeapBlock(EBlockKind code_, TValId root_, TOffset off_, TValId val_):
        code(code_),
        root(root_),
        off(off_),
        value(val_)
    {
    }

    virtual int getSize() const = 0;
};

struct InternalUniformBlock: public HeapBlock {
    unsigned                    size;

    InternalUniformBlock(
            const TValId            root_,
            const TOffset           off_,
            const TValId            tplValue_,
            const unsigned          size_):
        HeapBlock(BK_UNIFORM, root_, off_, tplValue_),
        size(size_)
    {
    }

    virtual InternalUniformBlock* clone() const {
        return new InternalUniformBlock(*this);
    }

    virtual int getSize() const {
        return size;
    }
};

struct HeapObject: public HeapBlock {
    TObjType                    clt;
    bool                        hasExtRef;

    HeapObject(TValId root_, TOffset off_, TObjType clt_, bool hasExtRef_):
        HeapBlock(isComposite(clt_)
                ? BK_COMPOSITE
                : BK_OBJECT,
                root_, off_, VAL_INVALID),
        clt(clt_),
        hasExtRef(hasExtRef_)
    {
    }

    virtual HeapObject* clone() const {
        return new HeapObject(*this);
    }

    virtual int getSize() const {
        return clt->size;
    }
};

struct BaseValue: public AbstractHeapEntity {
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
    bool                            isProto;
    TLiveObjs                       liveObjs;
    TObjSet                         usedByGl;
    TArena                          arena;

    RootValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_),
        addr(VAL_NULL),
        lastKnownClt(0),
        cbSize(0),
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
    // allocate a root-value for VAL_NULL
    Private();

    // clone heap entities, they are now allocated separately
    Private(const Private &);

    CVarMap                         cVarMap;
    CustomValueMapper               cValueMap;
    EntStore                        ents;
    std::set<TValId>                liveRoots;
    FriendlyNeqDb                   neqDb;

    TValId assignId(BaseValue *e) {
        return this->ents.assignId<TValId>(e);
    }

    TObjId assignId(HeapBlock *e) {
        return this->ents.assignId<TObjId>(e);
    }

    inline TValId valRoot(const TValId, const AbstractHeapEntity *);
    inline TValId valRoot(const TValId);

    TValId valCreate(EValueTarget code, EValueOrigin origin);
    TValId valDup(TValId);
    bool valsEqual(TValId, TValId);

    TObjId objCreate(TValId root, TOffset off, TObjType clt, bool hasExtRef);
    TObjId objExport(TObjId obj, bool *pExcl = 0);
    TValId objInit(TObjId obj);
    void objDestroy(TObjId, bool removeVal, bool detach);

    TObjId copySingleLiveBlock(
            const TValId            rootDst,
            RootValue              *rootDataDst,
            const TObjId            objSrc,
            const ELiveObj          code,
            const TOffset           shift = 0);

    TValId dupRoot(TValId root);
    void destroyRoot(TValId obj);

    bool /* wasPtr */ releaseValueOf(TObjId obj, TValId val);
    void registerValueOf(TObjId obj, TValId val);
    void splitBlockByObject(TObjId block, TObjId obj);
    void reinterpretObjData(TObjId old, TObjId obj, TValSet *killedPtrs = 0);
    void setValueOf(TObjId of, TValId val, TValSet *killedPtrs = 0);

    void neqOpWrap(SymHeap::ENeqOp, TValId, TValId);

    // runs only in debug build
    bool chkArenaConsistency(const RootValue *);

    void shiftBlockAt(
            const TValId            dstRoot,
            const TOffset           off,
            const TOffset           size,
            const TValSet          *killedPtrs);

    void transferBlock(
            const TValId            dstRoot,
            const TValId            srcRoot, 
            const TOffset           dstOff,
            const TOffset           srcOff,
            const TOffset           size);

    TObjId writeUniformBlock(
            const TValId            addr,
            const TValId            tplValue,
            const unsigned          size,
            TValSet                 *killedPtrs);

    private:
        // intentionally not implemented
        Private& operator=(const Private &);
};

inline TValId SymHeapCore::Private::valRoot(
        const TValId                val,
        const AbstractHeapEntity   *ent)
{
    const BaseValue *valData = DCAST<const BaseValue *>(ent);
    if (!valData->offRoot)
        return val;

    const TValId valRoot = DCAST<const OffValue *>(valData)->root;
    CL_BREAK_IF(this->ents.outOfRange(valRoot));
    return valRoot;
}

inline TValId SymHeapCore::Private::valRoot(const TValId val) {
    const BaseValue *valData;
    this->ents.getEntRO(&valData, val);
    return this->valRoot(val, valData);
}

bool /* wasPtr */ SymHeapCore::Private::releaseValueOf(TObjId obj, TValId val) {
    if (val <= 0)
        // we do not track uses of special values
        return /* wasPtr */ false;

    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    if (1 != valData->usedBy.erase(obj))
        CL_BREAK_IF("SymHeapCore::Private::releaseValueOf(): offset detected");

    const TValId root = this->valRoot(val, valData);
    this->ents.getEntRW(&valData, root);
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
    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    valData->usedBy.insert(obj);
    if (!isPossibleToDeref(valData->code))
        return;

    // update usedByGl
    const TValId root = this->valRoot(val, valData);
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    rootData->usedByGl.insert(obj);
}

// runs only in debug build
bool SymHeapCore::Private::chkArenaConsistency(const RootValue *rootData) {
    TLiveObjs all(rootData->liveObjs);
    if (isGone(rootData->code)) {
        CL_BREAK_IF(rootData->cbSize);
        CL_BREAK_IF(!rootData->liveObjs.empty());

        // we can check nothing for VT_DELETED/VT_LOST, we do not know the size
        return true;
    }

    const TArena &arena = rootData->arena;
    const TMemChunk chunk(0, rootData->cbSize);

    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, chunk, OBJ_INVALID)) {
        BOOST_FOREACH(const TObjId obj, overlaps)
            all.erase(obj);
    }

    if (all.empty())
        return true;

    CL_WARN("live object not mapped in arena: #" << all.begin()->first);
    return false;
}

void SymHeapCore::Private::splitBlockByObject(
        TObjId                      block,
        TObjId                      obj)
{
    InternalUniformBlock *blData;
    this->ents.getEntRW(&blData, block);

    const HeapBlock *hbData;
    this->ents.getEntRO(&hbData, obj);

    const EBlockKind code = hbData->code;
    if (BK_OBJECT == code && this->valsEqual(blData->value, hbData->value))
        // preserve non-conflicting uniform blocks
        return;

    // dig root
    const TValId root = blData->root;
    CL_BREAK_IF(root != hbData->root);

    // check up to now arean consistency
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // dig offsets and sizes
    const TOffset blOff = blData->off;
    const TOffset objOff = hbData->off;
    const unsigned blSize = blData->size;
    const unsigned objSize = hbData->getSize();

    // check overlapping
    const TOffset blBegToObjBeg = objOff - blOff;
    const TOffset objEndToBlEnd = blSize - objSize - blBegToObjBeg;

    if (blBegToObjBeg <= 0 && objEndToBlEnd <= 0) {
        // block completely overlapped by the object, throw it away
        if (!rootData->liveObjs.erase(block))
            CL_BREAK_IF("attempt to kill an already dead uniform block");

        rootData->arena -= createArenaItem(blOff, blSize, block);
        this->ents.releaseEnt(block);
        return;
    }

    if (0 < blBegToObjBeg && 0 < objEndToBlEnd) {
        // the object is strictly in the middle of the block (needs split)
        InternalUniformBlock *blDataOther = blData->clone();
        const TObjId blOther = this->assignId(blDataOther);

        // update metadata
        blData->size = blBegToObjBeg;
        blDataOther->size = objEndToBlEnd;
        blDataOther->off = objOff + objSize;

        // unmap part of the original block
        rootData->arena -= createArenaItem(
                blOff + blBegToObjBeg,
                objSize + objEndToBlEnd,
                block);

        // map the new block
        rootData->arena += createArenaItem(
                objOff + objSize,
                objEndToBlEnd,
                blOther);

        rootData->liveObjs[blOther] = LO_BLOCK;
        return;
    }

    // check direction
    const TOffset diff = blOff - objOff;
    const bool shiftBeg = (0 <= diff);
    const TOffset beg = (shiftBeg)
        ? /* shift begin of the block */ blOff
        : /* trim end of the block    */ objOff;

    // compute size of the overlapping region
    const TOffset trim = (shiftBeg)
        ? (objSize - diff)
        : (blSize + /* negative */ diff);

    // throw away the overlapping part of the block
    blData->size -= trim;
    if (shiftBeg)
        blData->off += trim;

    // unmap the overlapping part
    CL_BREAK_IF(trim <= 0 || !blData->size);
    rootData->arena -= createArenaItem(beg, trim, block);
}

bool isCoveredByBlock(
        const HeapObject           *objData,
        const InternalUniformBlock *blData)
{
    const TOffset beg1 = objData->off;
    const TOffset beg2 = blData->off;
    if (beg1 < beg2)
        // the object starts above the block
        return false;

    const TOffset end1 = beg1 + objData->clt->size;
    const TOffset end2 = beg2 + blData->size;
    return (end1 <= end2);
}

void SymHeapCore::Private::reinterpretObjData(
        TObjId                      old,
        TObjId                      obj,
        TValSet                    *killedPtrs)
{
    HeapBlock *blData;
    this->ents.getEntRW(&blData, old);

    EBlockKind code = blData->code;
    switch (code) {
        case BK_OBJECT:
            break;

        case BK_COMPOSITE:
            // do not invalidate those place-holding values of composite objects
            return;

        case BK_UNIFORM:
            this->splitBlockByObject(/* block */ old, obj);
            return;

        case BK_INVALID:
        default:
            CL_BREAK_IF("invalid call of reinterpretObjData()");
            return;
    }

    CL_DEBUG("reinterpretObjData() is taking place...");
    HeapObject *oldData = DCAST<HeapObject *>(blData);
    const TValId valOld = oldData->value;
    if (/* wasPtr */ this->releaseValueOf(old, valOld) && killedPtrs)
        killedPtrs->insert(valOld);

    // dig root
    const TValId root = oldData->root;
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // mark the object as dead
    if (rootData->liveObjs.erase(old))
        CL_DEBUG("reinterpretObjData() kills a live object");

    if (!oldData->hasExtRef) {
        CL_DEBUG("reinterpretObjData() destroys a dead object");
        this->objDestroy(old, /* removeVal */ false, /* detach */ true);
        return;
    }

    CL_DEBUG("an object being reinterpreted is still referenced from outside");
    this->ents.getEntRW(&blData, obj);
    code = blData->code;

    InternalUniformBlock *uniData;
    TValId val;

    switch (code) {
        case BK_UNIFORM:
            uniData = DCAST<InternalUniformBlock *>(blData);
            if (isCoveredByBlock(oldData, uniData)) {
                // object fully covered by the overlapping uniform block
                val = this->valDup(uniData->value);
                break;
            }
            // fall through!

        case BK_OBJECT:
            // TODO: hook various reinterpretation drivers here
            val = this->valCreate(VT_UNKNOWN, VO_REINTERPRET);
            break;

        case BK_COMPOSITE:
        case BK_INVALID:
        default:
            CL_BREAK_IF("invalid call of reinterpretObjData()");
            return;
    }

    // assign the value to the _old_ object
    oldData->value = val;
    this->registerValueOf(old, val);
}

void SymHeapCore::Private::setValueOf(
        TObjId                      obj,
        TValId                      val,
        TValSet                    *killedPtrs)
{
    // release old value
    HeapObject *objData;
    this->ents.getEntRW(&objData, obj);

    const TValId valOld = objData->value;
    if (/* wasPtr */ this->releaseValueOf(obj, valOld) && killedPtrs)
        killedPtrs->insert(valOld);

    // store new value
    objData->value = val;
    this->registerValueOf(obj, val);

    // resolve root
    const TValId root = objData->root;
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);

    // (re)insert self into the arena if not there
    TArena &arena = rootData->arena;
    const TOffset off = objData->off;
    const TObjType clt = objData->clt;
    arena += createArenaItem(off, clt->size, obj);

    // invalidate contents of the objects we are overwriting
    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), obj)) {
        BOOST_FOREACH(const TObjId old, overlaps)
            this->reinterpretObjData(old, obj, killedPtrs);
    }

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
}

TObjId SymHeapCore::Private::objCreate(
        TValId                      root,
        TOffset                     off,
        TObjType                    clt,
        bool                        hasExtRef)
{
    // acquire object ID
    HeapObject *objData = new HeapObject(root, off, clt, hasExtRef);
    const TObjId obj = this->assignId(objData);

    // register the object by the owning root value
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);

    // map the region occupied by the object
    rootData->arena += createArenaItem(off, clt->size, obj);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
    return obj;
}

void SymHeapCore::Private::objDestroy(TObjId obj, bool removeVal, bool detach) {
    HeapObject *objData;
    this->ents.getEntRW(&objData, obj);

    if (removeVal) {
        // release value of the object
        const TValId val = objData->value;
        this->releaseValueOf(obj, val);
    }

    if (detach) {
        // properly remove the object from grid and arena
        RootValue *rootData;
        this->ents.getEntRW(&rootData, objData->root);
        CL_BREAK_IF(!this->chkArenaConsistency(rootData));

        // remove the object from arena unless we are destroying everything
        const TOffset off = objData->off;
        const TObjType clt = objData->clt;
        rootData->arena -= createArenaItem(off, clt->size, obj);

        CL_BREAK_IF(hasKey(rootData->liveObjs, obj));
        CL_BREAK_IF(!this->chkArenaConsistency(rootData));
    }

    // release the corresponding HeapObject instance
    this->ents.releaseEnt(obj);
}

TObjId SymHeapCore::Private::objExport(TObjId obj, bool *pExcl) {
    HeapObject *data;
    this->ents.getEntRW(&data, obj);
    if (pExcl)
        *pExcl = !data->hasExtRef;

    data->hasExtRef = true;
    return obj;
}

TValId SymHeapCore::Private::valCreate(
        EValueTarget                code,
        EValueOrigin                origin)
{
    TValId val = VAL_INVALID;

    switch (code) {
        case VT_INVALID:
        case VT_UNKNOWN:
            val = this->assignId(new BaseValue(code, origin));
            break;

        case VT_COMPOSITE:
            val = this->assignId(new CompValue(code, origin));
            break;

        case VT_CUSTOM:
            val = this->assignId(new InternalCustomValue(code, origin));
            break;

        case VT_ABSTRACT:
            CL_BREAK_IF("invalid call of SymHeapCore::Private::valCreate()");
            // fall through!

        case VT_ON_HEAP:
        case VT_ON_STACK:
        case VT_STATIC:
        case VT_DELETED:
        case VT_LOST:
            val = this->assignId(new RootValue(code, origin));
            break;
    }

    return val;
}

TValId SymHeapCore::Private::valDup(TValId val) {
    if (val <= 0)
        // do not clone special values
        return val;

    // deep copy the value
    const BaseValue *tpl;
    this->ents.getEntRO(&tpl, val);
    BaseValue *dupData = /* FIXME: subtle */ tpl->clone();

    const TValId dup = this->assignId(dupData);

    // wipe BaseValue::usedBy
    dupData->usedBy.clear();

    return dup;
}

// FIXME: copy/pasted in symutil.hh
bool SymHeapCore::Private::valsEqual(TValId v1, TValId v2) {
    if (v1 == v2)
        // matches trivially
        return true;

    if (v1 <= 0 || v2 <= 0)
        // special values have to match
        return false;

    const BaseValue *valData1;
    const BaseValue *valData2;

    this->ents.getEntRO(&valData1, v1);
    this->ents.getEntRO(&valData2, v2);

    const EValueTarget code1 = valData1->code;
    const EValueTarget code2 = valData2->code;

    if (VT_UNKNOWN != code1 || VT_UNKNOWN != code2)
        // for now, we handle only unknown values here
        return false;

    CL_BREAK_IF(valData1->offRoot || valData2->offRoot);

    // just compare kinds of unknown values
    return (valData1->origin == valData2->origin);
}

void SymHeapCore::Private::shiftBlockAt(
        const TValId                dstRoot,
        const TOffset               off,
        const TOffset               size,
        const TValSet              *killedPtrs)
{
    CL_BREAK_IF("please implement");
    (void) dstRoot;
    (void) off;
    (void) size;
    (void) killedPtrs;
}

void SymHeapCore::Private::transferBlock(
        const TValId                dstRoot,
        const TValId                srcRoot, 
        const TOffset               dstOff,
        const TOffset               winBeg,
        const TOffset               winSize)
{
    const RootValue *rootDataSrc;
    this->ents.getEntRO(&rootDataSrc, srcRoot);
    const TArena &arena = rootDataSrc->arena;
    const TOffset winEnd = winBeg + winSize;
    const TMemChunk chunk (winBeg, winEnd);

    TObjSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, OBJ_INVALID))
        // no data to copy in here
        return;

    RootValue *rootDataDst;
    this->ents.getEntRW(&rootDataDst, dstRoot);
    const TOffset shift = dstOff - winBeg;

    // go through overlaps and copy the live ones
    BOOST_FOREACH(const TObjId objSrc, overlaps) {
        const HeapBlock *hbDataSrc;
        this->ents.getEntRO(&hbDataSrc, objSrc);

        const TOffset beg = hbDataSrc->off;
        if (beg < winBeg)
            // the object starts above the window, do not copy this one
            continue;

        const TOffset end = beg + hbDataSrc->getSize();
        if (winEnd < end)
            // the object ends beyond the window, do not copy this one
            continue;

        const TLiveObjs &liveSrc = rootDataSrc->liveObjs;
        const TLiveObjs::const_iterator it = liveSrc.find(objSrc);
        if (liveSrc.end() == it)
            // dead object anyway
            continue;

        // copy a single live block
        const ELiveObj code = it->second;
        this->copySingleLiveBlock(dstRoot, rootDataDst, objSrc, code, shift);
    }
}


SymHeapCore::Private::Private() {
    // allocate a root-value for VAL_NULL
    this->assignId(new RootValue(VT_INVALID, VO_INVALID));
}

SymHeapCore::Private::Private(const SymHeapCore::Private &ref):
    cVarMap     (ref.cVarMap),
    cValueMap   (ref.cValueMap),
    ents        (ref.ents),
    liveRoots   (ref.liveRoots),
    neqDb       (ref.neqDb)
{
}

TValId SymHeapCore::Private::objInit(TObjId obj) {
    HeapObject *objData;
    this->ents.getEntRW(&objData, obj);
    CL_BREAK_IF(!objData->hasExtRef);

    // resolve root
    const TValId root = objData->root;
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    const TArena &arena = rootData->arena;
    const TOffset off = objData->off;
    const TObjType clt = objData->clt;

    // first check for data reinterpretation
    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), obj)) {
        BOOST_FOREACH(const TObjId other, overlaps) {
            HeapBlock *blockData;
            this->ents.getEntRW(&blockData, other);

            const EBlockKind code = blockData->code;
            if (BK_UNIFORM != code && !hasKey(rootData->liveObjs, other))
                continue;

            // reinterpret _self_ by another live object or uniform block
            this->reinterpretObjData(/* old */ obj, other);
            CL_BREAK_IF(!this->chkArenaConsistency(rootData));
            return objData->value;
        }
    }

    // assign a fresh unknown value
    const TValId val = this->valCreate(VT_UNKNOWN, VO_UNKNOWN);
    objData->value = val;

    // mark the object as live
    if (isDataPtr(clt))
        rootData->liveObjs[obj] = LO_DATA_PTR;
#if SE_TRACK_NON_POINTER_VALUES
    else
        rootData->liveObjs[obj] = LO_DATA;
#endif

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // store backward reference
    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    valData->usedBy.insert(obj);
    return val;
}

TValId SymHeapCore::valueOf(TObjId obj) {
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

    HeapObject *objData;
    d->ents.getEntRW(&objData, obj);

    TValId &val = objData->value;
    if (VAL_INVALID != val)
        // the object has a value
        return val;

    const TObjType clt = objData->clt;
    if (isComposite(clt)) {
        // deleayed creation of a composite value
        val = d->valCreate(VT_COMPOSITE, VO_INVALID);
        CompValue *compData;
        d->ents.getEntRW(&compData, val);
        compData->compObj = obj;

        // store backward reference
        BaseValue *valData;
        d->ents.getEntRW(&valData, val);
        valData->usedBy.insert(obj);
        return val;
    }

    // deleayed object initialization
    return d->objInit(obj);
}

void SymHeapCore::usedBy(TObjList &dst, TValId val) const {
    if (VAL_NULL == val)
        return;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    const TObjSet &usedBy = valData->usedBy;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

unsigned SymHeapCore::usedByCount(TValId val) const {
    if (VAL_NULL == val)
        return 0;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->usedBy.size();
}

void SymHeapCore::pointedBy(TObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    CL_BREAK_IF(rootData->offRoot);
    CL_BREAK_IF(!isPossibleToDeref(rootData->code));

    const TObjSet &usedBy = rootData->usedByGl;
    std::copy(usedBy.begin(), usedBy.end(), std::back_inserter(dst));
}

unsigned SymHeapCore::pointedByCount(TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    return rootData->usedByGl.size();
}

unsigned SymHeapCore::lastId() const {
    return d->ents.lastId<unsigned>();
}

TValId SymHeapCore::valClone(TValId val) {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    const EValueTarget code = valData->code;
    if (VT_CUSTOM == code) {
        CL_BREAK_IF("custom values are not supposed to be cloned");
        return val;
    }

    if (isProgramVar(code)) {
        CL_BREAK_IF("program variables are not supposed to be cloned");
        return val;
    }

    const TValId root = d->valRoot(val, valData);
    if (VAL_NULL == root) {
        CL_BREAK_IF("VAL_NULL is not supposed to be cloned");
        return val;
    }

    if (!isPossibleToDeref(code))
        // duplicate an unknown value
        return d->valDup(val);

    // duplicate a root object
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

TObjId SymHeapCore::Private::copySingleLiveBlock(
        const TValId                rootDst,
        RootValue                  *rootDataDst,
        const TObjId                objSrc,
        const ELiveObj              code,
        const TOffset               shift)
{
    TObjId dst;

    if (LO_BLOCK == code) {
        // duplicate a uniform block
        InternalUniformBlock *blSrc;
        this->ents.getEntRW(&blSrc, objSrc);
        InternalUniformBlock *blDst = blSrc->clone();
        dst = this->assignId(blDst);
        blDst->root = rootDst;

        // shift the block if asked to do so
        blDst->off += shift;

        // map the cloned block
        rootDataDst->arena += createArenaItem(blDst->off, blDst->size, dst);
    }
    else {
        // duplicate a regular object
        CL_BREAK_IF(LO_DATA_PTR != code && LO_DATA != code);

        const HeapObject *objDataSrc;
        this->ents.getEntRO(&objDataSrc, objSrc);

        const TOffset off = objDataSrc->off + shift;
        const TObjType clt = objDataSrc->clt;
        dst = this->objCreate(rootDst, off, clt, /* hasExtRef */ false);
        this->setValueOf(dst, objDataSrc->value);
    }

    // prevserve live object code
    rootDataDst->liveObjs[dst] = code;
    return dst;
}

TValId SymHeapCore::Private::dupRoot(TValId rootAt) {
    CL_DEBUG("SymHeapCore::Private::dupRoot() is taking place...");
    const RootValue *rootDataSrc;
    this->ents.getEntRO(&rootDataSrc, rootAt);
    CL_BREAK_IF(!this->chkArenaConsistency(rootDataSrc));

    // assign an address to the clone
    const EValueTarget code = rootDataSrc->code;
    const TValId imageAt = this->valCreate(code, VO_ASSIGNED);
    RootValue *rootDataDst;
    this->ents.getEntRW(&rootDataDst, imageAt);
    rootDataDst->addr = imageAt;

    // duplicate root metadata
    rootDataDst->cVar               = rootDataSrc->cVar;
    rootDataDst->isProto            = rootDataSrc->isProto;
    rootDataDst->cbSize             = rootDataSrc->cbSize;
    rootDataDst->lastKnownClt       = rootDataSrc->lastKnownClt;

    this->liveRoots.insert(imageAt);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootDataSrc->liveObjs)
        this->copySingleLiveBlock(imageAt, rootDataDst,
                /* src  */ item.first,
                /* code */ item.second);

    CL_BREAK_IF(!this->chkArenaConsistency(rootDataDst));
    return imageAt;
}

void SymHeapCore::gatherLivePointers(TObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const ELiveObj code = item.second;
        if (LO_DATA_PTR == code)
            dst.push_back(d->objExport(/* obj */ item.first));
    }
}

void SymHeapCore::gatherUniformBlocks(TUniBlockMap &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const ELiveObj code = item.second;
        if (LO_BLOCK != code)
            continue;

        const InternalUniformBlock *blData;
        d->ents.getEntRO(&blData, /* obj */ item.first);
        const TOffset off = blData->off;
        CL_BREAK_IF(hasKey(dst, off));
        UniformBlock &block = dst[off];

        // export uniform block
        block.off       = off;
        block.size      = blData->size;
        block.tplValue  = blData->value;
    }
}

void SymHeapCore::gatherLiveObjects(TObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const ELiveObj code = item.second;

        switch (code) {
            case LO_BLOCK:
                continue;

            case LO_DATA:
            case LO_DATA_PTR:
                dst.push_back(d->objExport(/* obj */ item.first));
                continue;

            case LO_INVALID:
            default:
                CL_BREAK_IF("gatherLiveObjects sees something special");
        }
    }
}

bool SymHeapCore::findCoveringUniBlock(
        UniformBlock                *pDst,
        const TValId                root,
        const TOffset               beg,
        unsigned                    size)
    const
{
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    CL_BREAK_IF(!d->chkArenaConsistency(rootData));

    const TArena &arena = rootData->arena;
    const TOffset end = beg + size;
    const TMemChunk chunk(beg, end);

    TObjSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, OBJ_INVALID))
        // not found
        return false;

    BOOST_FOREACH(const TObjId id, overlaps) {
        const HeapBlock *data;
        d->ents.getEntRO(&data, id);

        const EBlockKind code = data->code;
        if (BK_UNIFORM != code)
            continue;

        const InternalUniformBlock *blData =
            DCAST<const InternalUniformBlock *>(data);

        const TOffset blBeg = blData->off;
        if (beg < blBeg)
            // the template starts above this block
            continue;

        const TOffset size = blData->size;
        const TOffset blEnd = blBeg + size;
        if (blEnd < end)
            // the template ends beyond this block
            continue;

        // covering uniform block matched!
        pDst->off       = blBeg;
        pDst->size      = size;
        pDst->tplValue  = blData->value;
        return true;
    }

    // not found
    return false;
}

SymHeapCore::SymHeapCore(TStorRef stor):
    stor_(stor),
    d(new Private)
{
    CL_BREAK_IF(!&stor_);

    // initialize VAL_ADDR_OF_RET
    const TValId addrRet = d->valCreate(VT_ON_STACK, VO_ASSIGNED);
    CL_BREAK_IF(VAL_ADDR_OF_RET != addrRet);
    RootValue *rootData;
    d->ents.getEntRW(&rootData, addrRet);
    rootData->addr = addrRet;
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

void SymHeapCore::objSetValue(TObjId obj, TValId val, TValSet *killedPtrs) {
    // we allow to set values of atomic types only
    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);

    const TObjType clt = objData->clt;
    CL_BREAK_IF(isComposite(clt, /* includingArray */ false));
    CL_BREAK_IF(isComposite(clt) && objData->off);

    // check whether the root entity that owns this object ID is still valid
    CL_BREAK_IF(!isPossibleToDeref(this->valTarget(objData->root)));

    // mark the destination object as live
    const TValId root = objData->root;
    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);
    rootData->liveObjs[obj] = isDataPtr(clt)
        ? LO_DATA_PTR
        : LO_DATA;

    // now set the value
    d->setValueOf(obj, val, killedPtrs);
}

TObjId SymHeapCore::Private::writeUniformBlock(
        const TValId                addr,
        const TValId                tplValue,
        const unsigned              size,
        TValSet                     *killedPtrs)
{
    const BaseValue *valData;
    this->ents.getEntRO(&valData, addr);

    const TValId root = this->valRoot(addr, valData);
    const TOffset beg = valData->offRoot;
    const TOffset end = beg + size;

    // acquire object ID
    InternalUniformBlock *blockData =
        new InternalUniformBlock(root, beg, tplValue, size);
    const TObjId obj = this->assignId(blockData);

    // check up to now arena consistency
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // mark the block as live
    rootData->liveObjs[obj] = LO_BLOCK;

    TArena &arena = rootData->arena;
    arena += createArenaItem(beg, size, obj);
    const TMemChunk chunk(beg, end);

    // invalidate contents of the objects we are overwriting
    TObjSet overlaps;
    if (arenaLookup(&overlaps, arena, chunk, obj)) {
        BOOST_FOREACH(const TObjId old, overlaps)
            this->reinterpretObjData(old, obj, killedPtrs);
    }

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
    return obj;
}

/// just a trivial wrapper to hide the return value
void SymHeapCore::writeUniformBlock(
        const TValId                addr,
        const TValId                tplValue,
        const unsigned              size,
        TValSet                     *killedPtrs)
{
    CL_BREAK_IF(this->valSizeOfTarget(addr) < static_cast<int>(size));
    d->writeUniformBlock(addr, tplValue, size, killedPtrs);
}

void SymHeapCore::copyBlockOfRawMemory(
        const TValId                dst,
        const TValId                src,
        const unsigned              size,
        TValSet                     *killedPtrs)
{
    // this should have been checked by the caller
    CL_BREAK_IF(this->valSizeOfTarget(dst) < static_cast<int>(size));
    CL_BREAK_IF(this->valSizeOfTarget(src) < static_cast<int>(size));

    const BaseValue *dstData;
    const BaseValue *srcData;

    d->ents.getEntRO(&dstData, dst);
    d->ents.getEntRO(&srcData, src);

    CL_BREAK_IF(!isPossibleToDeref(dstData->code));
    CL_BREAK_IF(!isPossibleToDeref(srcData->code));
    CL_BREAK_IF(!size);

    const TOffset dstOff = dstData->offRoot;
    const TOffset srcOff = srcData->offRoot;
    const TValId dstRoot = d->valRoot(dst, dstData);
    const TValId srcRoot = d->valRoot(src, srcData);

    if (dstRoot == srcRoot) {
        // movement within a single root entity
        const TOffset diff = dstOff - srcOff;
        d->shiftBlockAt(dstRoot, diff, size, killedPtrs);
        return;
    }

    // nuke the content we are going to overwrite
    const TObjId blKiller = d->writeUniformBlock(dst, /* misleading */ VAL_NULL,
                                                 size, killedPtrs);

    // remove the dummy block we used just to trigger the data reinterpretation
    RootValue *rootDataDst;
    d->ents.getEntRW(&rootDataDst, dstRoot);
    rootDataDst->liveObjs.erase(blKiller);
    rootDataDst->arena -= createArenaItem(dstOff, size, blKiller);
    d->ents.releaseEnt(blKiller);

    // now we need to transfer data between two distinct root entities
    d->transferBlock(dstRoot, srcRoot, dstOff, srcOff, size);
}

TObjType SymHeapCore::objType(TObjId obj) const {
    if (obj < 0)
        return 0;

    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);
    return objData->clt;
}

TValId SymHeapCore::valByOffset(TValId at, TOffset off) {
    if (!off || at < 0)
        return at;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

    // subtract the root
    const TValId valRoot = d->valRoot(at, valData);
    off += valData->offRoot;
    if (!off)
        return valRoot;

    const EValueTarget code = valData->code;
    if (VT_UNKNOWN == code)
        // do not track off-value for invalid targets
        return d->valDup(at);

    if (VT_CUSTOM == code) {
        CL_BREAK_IF("valByOffset() is NOT supposed to be applied on VT_CUSTOM");
        return at;
    }

    // off-value lookup
    RootValue *rootData;
    d->ents.getEntRW(&rootData, valRoot);
    TOffMap &offMap = rootData->offMap;
    TOffMap::const_iterator it = offMap.find(off);
    if (offMap.end() != it)
        return it->second;

    // create a new off-value
    OffValue *offVal = new OffValue(code, valData->origin, valRoot, off);
    const TValId val = d->assignId(offVal);

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

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->origin;
}

EValueTarget SymHeapCore::valTarget(TValId val) const {
    if (val <= 0)
        return VT_INVALID;

    if (this->hasAbstractTarget(val))
        // the overridden implementation claims the target is abstract
        return VT_ABSTRACT;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
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

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->offRoot;
}

void SymHeapCore::valReplace(TValId val, TValId replaceBy) {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    // kill all related Neq predicates
    TValList neqs;
    d->neqDb.gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValId valNeq, neqs) {
        CL_BREAK_IF(valNeq == replaceBy);
        SymHeapCore::neqOp(NEQ_DEL, valNeq, val);
    }

    // we intentionally do not use a reference here (tight loop otherwise)
    TObjSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TObjId obj, usedBy) {
#ifndef NDEBUG
        if (isGone(this->valTarget(this->placedAt(obj)))) {
            // FIXME: exactly this happens with test-0037 running in OOM mode
            CL_BREAK_IF("valReplace: value in use by deleted object, why?");
            d->releaseValueOf(obj, val);
            continue;
        }
#endif
        this->objSetValue(obj, replaceBy);
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

void SymHeapCore::neqOp(ENeqOp op, TValId v1, TValId v2) {
    const TOffset off1 = this->valOffset(v1);
    const TOffset off2 = this->valOffset(v2);

    const TValId root1 = this->valRoot(v1);
    const TValId root2 = this->valRoot(v2);

    const TOffset diff = off2 - off1;
    if (!diff) {
        // if both values have the same offset, connect the roots
        d->neqOpWrap(op, root1, root2);
        return;
    }

    // if the values have different offsets, associate both roots
    d->neqOpWrap(op, root1, this->valByOffset(root2,  diff));
    d->neqOpWrap(op, root2, this->valByOffset(root1, -diff));
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

TValId SymHeapCore::placedAt(TObjId obj) {
    if (obj < 0)
        return VAL_INVALID;

    // jump to root
    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);

    const RootValue *rootData;
    d->ents.getEntRO(&rootData, objData->root);
    const TValId root = rootData->addr;

    // then subtract the offset
    return this->valByOffset(root, objData->off);
}

TObjId SymHeapCore::ptrAt(TValId at, bool *pExcl) {
    if (at <= 0)
        return OBJ_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

    const EValueTarget code = valData->code;
    if (!isPossibleToDeref(code))
        return OBJ_INVALID;

    // jump to root
    const TValId valRoot = d->valRoot(at, valData);
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);

    // generic pointer, (void *) if available
    const TObjType clt = stor_.types.genericDataPtr();
    CL_BREAK_IF(!clt || clt->code != CL_TYPE_PTR);
    const TOffset size = clt->size;
    CL_BREAK_IF(size <= 0);

    // arena lookup
    TObjSet candidates;
    const TArena &arena = rootData->arena;
    const TOffset off = valData->offRoot;
    const TMemChunk chunk(off, off + size);
    arenaLookForExactMatch(&candidates, arena, chunk);

    // seek a _data_ pointer in the given interval
    BOOST_FOREACH(const TObjId obj, candidates) {
        const HeapBlock *blData;
        d->ents.getEntRO(&blData, obj);
        if (BK_OBJECT != blData->code)
            continue;

        const HeapObject *objData = DCAST<const HeapObject *>(blData);
        const TObjType clt = objData->clt;
        if (isDataPtr(clt))
            return d->objExport(obj, pExcl);
    }

    // check whether we have enough space allocated for the pointer
    if (this->valSizeOfTarget(at) < clt->size) {
        CL_BREAK_IF("ptrAt() called out of bounds");
        return OBJ_UNKNOWN;
    }

    // resolve root
    const TValId root = d->valRoot(at, valData);

    if (pExcl)
        // a newly created object cannot be already externally referenced
        *pExcl = true;

    // create the pointer
    return d->objCreate(root, off, clt, /* hasExtRef */ true);
}

// TODO: simplify the code
TObjId SymHeapCore::objAt(TValId at, TObjType clt, bool *pExcl) {
    if (at <= 0)
        return OBJ_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

    const EValueTarget code = valData->code;
    if (!isPossibleToDeref(code))
        return OBJ_INVALID;

    CL_BREAK_IF(!clt || !clt->size);
    const TOffset size = clt->size;

    // jump to root
    const TValId valRoot = d->valRoot(at, valData);
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);

    // arena lookup
    TObjSet candidates;
    const TArena &arena = rootData->arena;
    const TOffset off = valData->offRoot;
    const TMemChunk chunk(off, off + size);
    arenaLookForExactMatch(&candidates, arena, chunk);

    TObjId bestMatch = OBJ_INVALID;
    bool liveObjFound = false;
    bool cltExactMatch = false;
    bool cltClassMatch = false;

    // go through the objects in the given interval
    BOOST_FOREACH(const TObjId obj, candidates) {
        const HeapBlock *blData;
        d->ents.getEntRO(&blData, obj);
        const EBlockKind code = blData->code;
        switch (code) {
            case BK_OBJECT:
            case BK_COMPOSITE:
                break;

            default:
                continue;
        }

        const bool isLive = hasKey(rootData->liveObjs, obj);
        if (liveObjFound && !isLive)
            continue;

        const HeapObject *objData = DCAST<const HeapObject *>(blData);
        const TObjType cltNow = objData->clt;
        if (cltNow == clt) {
            // exact match
            if (isLive)
                return d->objExport(obj, pExcl);

            CL_BREAK_IF(cltExactMatch);
            cltExactMatch = true;
            goto update_best;
        }

        if (cltExactMatch)
            continue;

        if (*cltNow == *clt) {
            cltClassMatch = true;
            goto update_best;
        }

        if (cltClassMatch)
            continue;

        if (!isDataPtr(cltNow) || !isDataPtr(clt))
            continue;
        // at least both are _data_ pointers at this point, update best match

update_best:
        liveObjFound = isLive;
        bestMatch = obj;
    }

    if (OBJ_INVALID != bestMatch)
        return d->objExport(bestMatch, pExcl);

    if (this->valSizeOfTarget(at) < clt->size)
        // out of bounds
        return OBJ_UNKNOWN;

    if (pExcl)
        // a newly created object cannot be already externally referenced
        *pExcl = true;

    // create the object
    const TValId root = d->valRoot(at, valData);
    return d->objCreate(root, off, clt, /* hasExtRef */ true);
}

void SymHeapCore::objReleaseId(TObjId obj) {
    HeapObject *objData;
    d->ents.getEntRW(&objData, obj);
    CL_BREAK_IF(!objData->hasExtRef);
    objData->hasExtRef = false;

    if (isComposite(objData->clt, /* includingArray */ false)
            && VAL_INVALID != objData->value)
    {
        CL_DEBUG("SymHeapCore::objReleaseId() preserves a composite object");
        return;
    }

#if !SE_TRACK_UNINITIALIZED
    const TValId root = objData->root;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    if (!hasKey(rootData->liveObjs, obj)) {
        CL_DEBUG("SymHeapCore::objReleaseId() destroys a dead object");
        d->objDestroy(obj, /* removeVal */ true, /* detach */ true);
    }
#endif

    // TODO: pack the representation if possible
}

CVar SymHeapCore::cVarByRoot(TValId valRoot) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);
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

    RootValue *rootData;
    d->ents.getEntRW(&rootData, addr);
    rootData->cVar = cv;
    rootData->addr = addr;
    rootData->lastKnownClt = clt;

    // read size from the type-info
    const unsigned size = clt->size;
    rootData->cbSize = size;

    // initialize to zero?
    bool nullify = var.initialized;
#if SE_ASSUME_FRESH_STATIC_DATA
    nullify |= (VT_STATIC == code);
#endif

    if (nullify) {
        // initialize to zero
        this->writeUniformBlock(addr, VAL_NULL, size);
    }
#if SE_TRACK_UNINITIALIZED
    else if (VT_ON_STACK == code) {
        // uninitialized stack variable
        const TValId tpl = this->valCreate(VT_UNKNOWN, VO_STACK);
        this->writeUniformBlock(addr, tpl, size);
    }
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
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    CL_BREAK_IF(VT_COMPOSITE != valData->code);

    const CompValue *compData = DCAST<const CompValue *>(valData);
    return d->objExport(compData->compObj);
}

TValId SymHeapCore::heapAlloc(int cbSize) {
    CL_BREAK_IF(cbSize <= 0);

    // assign an address
    const TValId addr = d->valCreate(VT_ON_HEAP, VO_ASSIGNED);

    // initialize meta-data
    RootValue *rootData;
    d->ents.getEntRW(&rootData, addr);
    rootData->addr = addr;
    rootData->cbSize = cbSize;

#if SE_TRACK_UNINITIALIZED
    // uninitialized heap block
    const TValId tplValue = this->valCreate(VT_UNKNOWN, VO_HEAP);
    this->writeUniformBlock(addr, tplValue, cbSize);
#endif

    return addr;
}

void SymHeapCore::valDestroyTarget(TValId val) {
    if (VAL_NULL == val) {
        CL_BREAK_IF("SymHeapCore::valDestroyTarget() got VAL_NULL");
        return;
    }

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    if (valData->offRoot || !isPossibleToDeref(valData->code)) {
        CL_BREAK_IF("invalid call of SymHeapCore::valDestroyTarget()");
        return;
    }

    d->destroyRoot(val);
}

int SymHeapCore::valSizeOfTarget(TValId val) const {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    if (valData->offRoot < 0)
        // we are above the root, so we cannot safely write anything
        return 0;

    const EValueTarget code = valData->code;
    if (isGone(code))
        return 0;

    CL_BREAK_IF(!isPossibleToDeref(valData->code));
    const TValId root = d->valRoot(val, valData);
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);

    const int rootSize = rootData->cbSize;
    const TOffset off = valData->offRoot;
    return rootSize - off;
}

void SymHeapCore::valSetLastKnownTypeOfTarget(TValId root, TObjType clt) {
    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);

    if (VAL_ADDR_OF_RET == root) {
        // destroy any stale target of VAL_ADDR_OF_RET
        d->destroyRoot(root);

        // allocate a new root value at VAL_ADDR_OF_RET
        rootData->code = VT_ON_STACK;
        rootData->cbSize = clt->size;
    }

    // convert a type-free object into a type-aware object
    rootData->lastKnownClt = clt;
    d->liveRoots.insert(root);
}

TObjType SymHeapCore::valLastKnownTypeOfTarget(TValId root) const {
    CL_BREAK_IF(this->valOffset(root));
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    return rootData->lastKnownClt;
}

void SymHeapCore::Private::destroyRoot(TValId root) {
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);

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
        BaseValue *valData;
        this->ents.getEntRW(&valData, val);
        CL_BREAK_IF(!dynamic_cast<OffValue *>(valData));

        valData->code = code;
    }

    // release the root
    this->liveRoots.erase(root);

    // destroy all objects inside
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const TObjId obj = item.first;
        const ELiveObj code = item.second;

        if (LO_BLOCK == code) {
            // uniform block
            this->ents.releaseEnt(obj);
            continue;
        }

        this->objDestroy(obj, /* removeVal */ true, /* detach */ false);
    }

    // wipe rootData
    rootData->lastKnownClt = 0;
    rootData->liveObjs.clear();
    rootData->arena.clear();
}

TValId SymHeapCore::valCreate(EValueTarget code, EValueOrigin origin) {
    CL_BREAK_IF(isPossibleToDeref(code));
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
    InternalCustomValue *valData;
    d->ents.getEntRW(&valData, val);
    valData->customData = cVal;
    return val;
}

const CustomValue& SymHeapCore::valUnwrapCustom(TValId val) const
{
    const InternalCustomValue *valData;
    d->ents.getEntRO(&valData, val);

    // check the consistency of backward mapping
    CL_BREAK_IF(val != d->cValueMap.lookup(valData->customData));

    return valData->customData;
}

bool SymHeapCore::valTargetIsProto(TValId val) const {
    if (val <= 0)
        // not a prototype for sure
        return false;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    if (!isPossibleToDeref(valData->code))
        // not a prototype for sure
        return false;

    // seek root
    const TValId root = d->valRoot(val, valData);
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    return rootData->isProto;
}

void SymHeapCore::valTargetSetProto(TValId val, bool isProto) {
    const TValId root = d->valRoot(val);
    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);
    rootData->isProto = isProto;
}

// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap
struct SymHeap::Private {
    struct AbstractObject {
        EObjKind            kind;
        BindingOff          off;
        unsigned            minLength;

        AbstractObject():
            kind(OK_CONCRETE),
            minLength(0)
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

    if (VT_ABSTRACT == code1 && spliceOutAbstractPath(*this, v1, v2))
        // splice-out succeeded ... ls(v1, v2)
        return;

    if (VT_ABSTRACT == code2 && spliceOutAbstractPath(*this, v2, v1))
        // splice-out succeeded ... ls(v2, v1)
        return;

    CL_DEBUG("failed to splice-out list segment, has to over-approximate");
}

void SymHeap::segMinLengthOp(ENeqOp op, TValId at, unsigned len) {
    CL_BREAK_IF(!len);

    if (NEQ_DEL == op) {
        this->segSetMinLength(at, len - 1);
        return;
    }

    CL_BREAK_IF(NEQ_ADD != op);
    const unsigned current = this->segMinLength(at);
    if (len <= current)
        return;

    this->segSetMinLength(at, len);
}

bool haveSegBidir(
        TValId                      *pDst,
        const SymHeap               *sh,
        const EObjKind              kind,
        const TValId                v1,
        const TValId                v2)
{
    if (haveSeg(*sh, v1, v2, kind)) {
        *pDst = v1;
        return true;
    }

    if (haveSeg(*sh, v2, v1, kind)) {
        *pDst = v2;
        return true;
    }

    // found nothing
    return false;
}

void SymHeap::neqOp(ENeqOp op, TValId v1, TValId v2) {
    CL_BREAK_IF(NEQ_ADD != op && NEQ_DEL != op);
    CL_BREAK_IF(v1 <= 0 && v2 <= 0);

    if (!this->hasAbstractTarget(v1) && !this->hasAbstractTarget(v2)) {
        // fallback to the base implementation
        SymHeapCore::neqOp(op, v1, v2);
        return;
    }

    if (VAL_NULL == v1 && !this->valOffset(v2))
        v1 = segNextRootObj(*this, v2);
    if (VAL_NULL == v2 && !this->valOffset(v1))
        v2 = segNextRootObj(*this, v1);

    TValId seg;
    if (haveSegBidir(&seg, this, OK_MAY_EXIST, v1, v2)) {
        // replace OK_MAY_EXIST by OK_CONCRETE
        this->valTargetSetConcrete(seg);
        return;
    }

    if (haveSegBidir(&seg, this, OK_SLS, v1, v2)) {
        this->segMinLengthOp(op, seg, /* SLS 1+ */ 1);
        return;
    }

    if (haveSegBidir(&seg, this, OK_DLS, v1, v2)) {
        this->segMinLengthOp(op, seg, /* DLS 1+ */ 1);
        return;
    }

    if (haveDlSegAt(*this, v1, v2)) {
        this->segMinLengthOp(op, v1, /* DLS 2+ */ 2);
        return;
    }

    CL_BREAK_IF(NEQ_ADD != op);
    CL_DEBUG("SymHeap::neqOp() refuses to add an extraordinary Neq predicate");
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
    CL_BREAK_IF(d->ents.outOfRange(valB));
    if (valInsideSafeRange(*this, valA) && valInsideSafeRange(*this, valB))
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
        // same root, different offsets
        CL_BREAK_IF(this->valOffset(valA) == this->valOffset(valB));
        return true;
    }

    const TOffset offA = this->valOffset(valA);
    const TOffset offB = this->valOffset(valB);

    const TOffset diff = offB - offA;
    if (!diff)
        // check for Neq between the roots
        return d->neqDb.areNeq(root1, root2);

    SymHeapCore &writable = /* XXX */ *const_cast<SymHeapCore *>(this);
    return d->neqDb.areNeq(root1, writable.valByOffset(root2,  diff))
        && d->neqDb.areNeq(root2, writable.valByOffset(root1, -diff));
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

    const EValueTarget refCode = this->valTarget(ref);
    if (isAbstract(refCode)) {
        // both values are abstract
        const TValId root1 = this->valRoot(ref);
        const TValId root2 = this->valRoot(val);
        if (root2 == segPeer(*this, root1)) {
            // one value points at segment and the other points at its peer
            const TOffset off1 = this->valOffset(ref);
            const TOffset off2 = this->valOffset(val);
            return (off1 == off2)
                && (1 < this->segMinLength(root1));
        }

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

        SymHeap &writable = *const_cast<SymHeap *>(this);

        TValId seg = this->valRoot(val);
        if (OK_DLS == this->valTargetKind(val))
            seg = dlSegPeer(writable, seg);

        if (seg < 0)
            // no valid object here
            return false;

        const TValId valNext = nextValFromSeg(writable, seg);
        if (this->segMinLength(seg))
            // non-empty abstract object reached
            return (VAL_NULL == ref)
                || isKnownObject(refCode);

        // jump to next value
        val = valNext;
    }

    return false;
}

void SymHeap::valDestroyTarget(TValId val) {
    const TValId valRoot = this->valRoot(val);
    SymHeapCore::valDestroyTarget(val);

    CL_BREAK_IF(valRoot <= 0);
    if (d->data.erase(valRoot))
        CL_DEBUG("SymHeap::valDestroyTarget() destroys an abstract object");
}

unsigned SymHeap::segMinLength(TValId at) const {
    const TValId seg = this->valRoot(at);
    CL_BREAK_IF(!hasKey(d->data, seg));

    const Private::AbstractObject &aoData = d->data[seg];

    const EObjKind kind = aoData.kind;
    switch (kind) {
        case OK_MAY_EXIST:
            return 0;

        case OK_SLS:
        case OK_DLS:
            return aoData.minLength;

        default:
            CL_BREAK_IF("invalid call of SymHeap::segMinLength()");
            return 0;
    }
}

void SymHeap::segSetMinLength(TValId at, unsigned len) {
    const TValId seg = this->valRoot(at);
    CL_BREAK_IF(!hasKey(d->data, seg));

    Private::AbstractObject &aoData = d->data[seg];

    const EObjKind kind = aoData.kind;
    switch (kind) {
        case OK_MAY_EXIST:
            if (len)
                CL_BREAK_IF("OK_MAY_EXIST is supposed to have zero minLength");
            return;

        case OK_SLS:
#if SE_RESTRICT_SLS_MINLEN
            if ((SE_RESTRICT_SLS_MINLEN) < len)
                len = (SE_RESTRICT_SLS_MINLEN);
#endif
            break;

        case OK_DLS:
#if SE_RESTRICT_DLS_MINLEN
            if ((SE_RESTRICT_DLS_MINLEN) < len)
                len = (SE_RESTRICT_DLS_MINLEN);
#endif
            break;

        default:
            CL_BREAK_IF("invalid call of SymHeap::segMinLength()");
            return;
    }

    aoData.minLength = len;
    if (OK_DLS != kind)
        return;

    const TValId peer = dlSegPeer(*this, seg);
    CL_BREAK_IF(peer == seg);
    d->data[peer].minLength = len;
}
