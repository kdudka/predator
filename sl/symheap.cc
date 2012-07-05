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

#include "intarena.hh"
#include "prototype.hh"
#include "symabstract.hh"
#include "syments.hh"
#include "sympred.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "util.hh"
#include "worklist.hh"

#ifndef NDEBUG
    // just for debugging purposes
#   include "symcmp.hh"
#endif

#include <algorithm>
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

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

static bool bypassSelfChecks;

void enableProtectedMode(bool enable) {
    ::bypassSelfChecks = !enable;
}

// /////////////////////////////////////////////////////////////////////////////
// Neq predicates store
class NeqDb: public SymPairSet<TValId, /* IREFLEXIVE */ true> {
    public:
        RefCounter refCnt;

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
// CoincidenceDb lookup container
class CoincidenceDb: public SymPairMap</* v1, v2 */ TValId, TValId /* sum */> {
    public:
        RefCounter refCnt;

    public:
        template <class TDst>
        void gatherRelatedValues(TDst &dst, TValId val) const {
            // FIXME: suboptimal due to performance
            BOOST_FOREACH(TMap::const_reference ref, db_) {
                const TItem &item = ref.first;
                if (item.first == val)
                    dst.push_back(item.second);
                else if (item.second == val)
                    dst.push_back(item.first);
            }
        }
};

// /////////////////////////////////////////////////////////////////////////////
// CVar lookup container
class CVarMap {
    public:
        RefCounter refCnt;

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
// implementation of CustomValue
CustomValue::~CustomValue() {
    if (CV_STRING != code_)
        return;

    CL_BREAK_IF(!data_.str);
    delete data_.str;
}

CustomValue::CustomValue(const CustomValue &ref):
    code_(ref.code_),
    data_(ref.data_)
{
    if (CV_STRING == code_)
        data_.str = new std::string(*ref.data_.str);
}

CustomValue& CustomValue::operator=(const CustomValue &ref) {
    if (&ref == this)
        return *this;

    if (CV_STRING == code_) {
        CL_BREAK_IF(!data_.str);
        delete data_.str;
    }

    code_ = ref.code_;
    data_ = ref.data_;

    if (CV_STRING == code_)
        data_.str = new std::string(*ref.data_.str);

    return *this;
}

int CustomValue::uid() const {
    CL_BREAK_IF(CV_FNC != code_);
    return data_.uid;
}

IR::Range& CustomValue::rng() {
    CL_BREAK_IF(CV_INT_RANGE != code_);
    return data_.rng;
}

double CustomValue::fpn() const {
    CL_BREAK_IF(CV_REAL != code_);
    return data_.fpn;
}

const std::string& CustomValue::str() const {
    CL_BREAK_IF(CV_STRING != code_);
    CL_BREAK_IF(!data_.str);
    return *data_.str;
}

/// eliminates the warning 'comparing floating point with == or != is unsafe'
inline bool areEqual(const double a, const double b) {
    return (a <= b)
        && (b <= a);
}

bool operator==(const CustomValue &a, const CustomValue &b) {
    const ECustomValue code = a.code_;
    if (b.code_ != code)
        return false;

    switch (code) {
        case CV_INVALID:
            return true;

        case CV_FNC:
            return (a.data_.uid == b.data_.uid);

        case CV_REAL:
            // just for convenience, no need to compare CV_REAL for equality
            return areEqual(a.data_.fpn, b.data_.fpn);

        case CV_STRING:
            CL_BREAK_IF(!a.data_.str || !b.data_.str);
            return !a.data_.str->compare(*b.data_.str);

        case CV_INT_RANGE:
            return (a.data_.rng == b.data_.rng);
    }

    CL_BREAK_IF("CustomValue::operator==() got something special");
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeapCore
typedef std::set<TObjId>                                TObjIdSet;
typedef std::map<TOffset, TValId>                       TOffMap;
typedef IntervalArena<TOffset, TObjId>                  TArena;
typedef TArena::key_type                                TMemChunk;
typedef TArena::value_type                              TMemItem;

inline TMemItem createArenaItem(
        const TOffset               off,
        const TSizeOf               size,
        const TObjId                obj)
{
    const TMemChunk chunk(off, off + size);
    return TMemItem(chunk, obj);
}

inline bool arenaLookup(
        TObjIdSet                   *dst,
        const TArena                &arena,
        const TMemChunk             &chunk,
        const TObjId                obj)
{
    CL_BREAK_IF(!dst->empty());
    arena.intersects(*dst, chunk);

    if (OBJ_INVALID != obj)
        // remove the reference object itself
        dst->erase(obj);

    // finally check if there was anything else
    return !dst->empty();
}

inline void arenaLookForExactMatch(
        TObjIdSet                   *dst,
        const TArena                &arena,
        const TMemChunk             &chunk)
{
    arena.exactMatch(*dst, chunk);
}

// create a right-open interval
inline TMemChunk createChunk(const TOffset off, const TObjType clt) {
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
    return TMemChunk(off, off + clt->size);
}

enum EBlockKind {
    BK_INVALID,
    BK_DATA_PTR,
    BK_DATA_OBJ,
    BK_COMPOSITE,
    BK_UNIFORM
};

typedef std::map<TObjId, EBlockKind>                    TLiveObjs;

inline EBlockKind bkFromClt(const TObjType clt) {
    if (isComposite(clt, /* includingArray */ false))
        return BK_COMPOSITE;

    return (isDataPtr(clt))
        ? BK_DATA_PTR
        : BK_DATA_OBJ;
}

class AbstractHeapEntity {
    public:
        virtual AbstractHeapEntity* clone() const = 0;

    protected:
        virtual ~AbstractHeapEntity() { }
        friend class EntStore<AbstractHeapEntity>;
        friend struct RefCntLibBase;
        friend struct RefCntLib<RCO_VIRTUAL>;

    private:
        RefCounter refCnt;

        // intentionally not implemented
        AbstractHeapEntity& operator=(const AbstractHeapEntity &);
};

struct BlockEntity: public AbstractHeapEntity {
    EBlockKind                  code;
    TValId                      root;
    TOffset                     off;
    TSizeOf                     size;
    TValId                      value;

    BlockEntity(
            const EBlockKind        code_,
            const TValId            root_,
            const TOffset           off_,
            const TSizeOf           size_,
            const TValId            value_):
        code(code_),
        root(root_),
        off(off_),
        size(size_),
        value(value_)
    {
    }

    virtual BlockEntity* clone() const {
        return new BlockEntity(*this);
    }
};

struct HeapObject: public BlockEntity {
    TObjType                    clt;
    int                         extRefCnt;

    HeapObject(TValId root_, TOffset off_, TObjType clt_):
        BlockEntity(bkFromClt(clt_), root_, off_, clt_->size, VAL_INVALID),
        clt(clt_),
        extRefCnt(0)
    {
    }

    virtual HeapObject* clone() const {
        return new HeapObject(*this);
    }
};

struct BaseValue: public AbstractHeapEntity {
    EValueTarget                    code;
    EValueOrigin                    origin;
    TValId                          valRoot;
    TValId                          anchor;
    TOffset /* FIXME: misleading */ offRoot;
    TObjIdSet                       usedBy;

    // cppcheck-suppress uninitMemberVar
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

/// maintains a list of dependent values
struct ReferableValue: public BaseValue {
    TValList                        dependentValues;

    // unless clone() is properly overridden, the constructor cannot be public
    protected:
    ReferableValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_)
    {
    }
};

struct AnchorValue: public ReferableValue {
    TOffMap                         offMap;

    // unless clone() is properly overridden, the constructor cannot be public
    protected:
    AnchorValue(EValueTarget code_, EValueOrigin origin_):
        ReferableValue(code_, origin_)
    {
    }
};

struct RangeValue: public AnchorValue {
    IR::Range                       range;

    RangeValue(const IR::Range &range_):
        AnchorValue(VT_RANGE, VO_ASSIGNED),
        range(range_)
    {
    }

    virtual RangeValue* clone() const {
        return new RangeValue(*this);
    }
};

struct CompValue: public BaseValue {
    TObjId                          compObj;

    // cppcheck-suppress uninitMemberVar
    CompValue(EValueTarget code_, EValueOrigin origin_):
        BaseValue(code_, origin_)
    {
    }

    virtual BaseValue* clone() const {
        return new CompValue(*this);
    }
};

struct InternalCustomValue: public ReferableValue {
    CustomValue                     customData;

    InternalCustomValue(EValueTarget code_, EValueOrigin origin_):
        ReferableValue(code_, origin_)
    {
    }

    virtual InternalCustomValue* clone() const {
        return new InternalCustomValue(*this);
    }
};

struct RootValue: public AnchorValue {
    CVar                            cVar;
    TSizeRange                      size;
    TLiveObjs                       liveObjs;
    TObjIdSet                       usedByGl;
    TArena                          arena;
    TObjType                        lastKnownClt;
    TProtoLevel                     protoLevel;

    RootValue(EValueTarget code_, EValueOrigin origin_):
        AnchorValue(code_, origin_),
        size(IR::rngFromNum(0)),
        lastKnownClt(0),
        protoLevel(/* not a prototype */ 0)
    {
    }

    virtual RootValue* clone() const {
        return new RootValue(*this);
    }
};

// cppcheck-suppress noConstructor
class CustomValueMapper {
    private:
        typedef std::map<int /* uid */, TValId>                 TCustomByUid;
        typedef std::map<IR::TInt, TValId>                      TCustomByNum;
        typedef std::map<double, TValId>                        TCustomByReal;
        typedef std::map<std::string, TValId>                   TCustomByString;

        TCustomByUid        fncMap;
        TCustomByNum        numMap;
        TCustomByReal       fpnMap;
        TCustomByString     strMap;
        TValId              inval_;

    public:
        RefCounter          refCnt;

    public:
        TValId& lookup(const CustomValue &item) {
            const ECustomValue code = item.code();
            switch (code) {
                case CV_INVALID:
                default:
                    CL_BREAK_IF("invalid call of CustomValueMapper::lookup()");
                    return inval_ = VAL_INVALID;

                case CV_FNC:
                    return assignInvalidIfNotFound(fncMap, item.uid());

                case CV_INT_RANGE:
                    CL_BREAK_IF(!isSingular(item.rng()));
                    return assignInvalidIfNotFound(numMap, item.rng().lo);

                case CV_REAL:
                    return assignInvalidIfNotFound(fpnMap, item.fpn());

                case CV_STRING:
                    return assignInvalidIfNotFound(strMap, item.str());
            }
        }
};

// FIXME: std::set is not a good candidate for base class
struct TValSetWrapper: public TValSet {
    RefCounter refCnt;
};

struct SymHeapCore::Private {
    Private(Trace::Node *);
    Private(const Private &);
    ~Private();

    Trace::NodeHandle               traceHandle;
    EntStore<AbstractHeapEntity>    ents;
    TValSetWrapper                 *liveRoots;
    CVarMap                        *cVarMap;
    CustomValueMapper              *cValueMap;
    CoincidenceDb                  *coinDb;
    NeqDb                          *neqDb;

    inline TObjId assignId(BlockEntity *);
    inline TValId assignId(BaseValue *);

    TValId valCreate(EValueTarget code, EValueOrigin origin);
    TValId valDup(TValId);
    bool valsEqual(TValId, TValId);

    TObjId objCreate(TValId root, TOffset off, TObjType clt);
    TValId objInit(TObjId obj);
    void objDestroy(TObjId, bool removeVal, bool detach);

    TObjId copySingleLiveBlock(
            const TValId            rootDst,
            RootValue              *rootDataDst,
            const TObjId            objSrc,
            const EBlockKind        code,
            const TOffset           shift = 0,
            const TSizeOf           sizeLimit = 0);

    TValId dupRoot(TValId root);
    void destroyRoot(TValId obj);

    bool /* wasPtr */ releaseValueOf(TObjId obj, TValId val);
    void registerValueOf(TObjId obj, TValId val);
    void splitBlockByObject(TObjId block, TObjId obj);
    bool writeCharToString(TValId *pValDst, const TValId, const TOffset);
    bool reinterpretSingleObj(HeapObject *dstData, const BlockEntity *srcData);
    void reinterpretObjData(TObjId old, TObjId obj, TValSet *killedPtrs = 0);
    void setValueOf(TObjId of, TValId val, TValSet *killedPtrs = 0);

    // runs only in debug build
    bool chkValueDeps(const TValId);

    // runs only in debug build
    bool chkArenaConsistency(
            const RootValue        *rootData,
            const bool              allowOverlap = false);

    void shiftBlockAt(
            const TValId            dstRoot,
            const TOffset           off,
            const TSizeOf           size,
            const TValSet          *killedPtrs);

    void transferBlock(
            const TValId            dstRoot,
            const TValId            srcRoot, 
            const TOffset           dstOff,
            const TOffset           srcOff,
            const TSizeOf           size);

    TObjId writeUniformBlock(
            const TValId            addr,
            const TValId            tplValue,
            const TSizeOf           size,
            TValSet                *killedPtrs);

    bool findZeroInBlock(
            TOffset                *offDst,
            bool                   *provenPrefix,
            const TOffset           offSrc,
            const                   TObjId obj);

    bool findZeroAtOff(
            IR::Range              *offDst,
            const TOffset           offSrc,
            const TValId            root);

    void bindValues(TValId v1, TValId v2, TValId valSum);

    TValId shiftCustomValue(TValId val, TOffset shift);

    TValId wrapIntVal(const IR::TInt);

    void replaceRngByInt(const InternalCustomValue *valData);

    void trimCustomValue(TValId val, const IR::Range &win);

    private:
        // intentionally not implemented
        Private& operator=(const Private &);
};

inline TValId SymHeapCore::Private::assignId(BaseValue *valData) {
    const TValId val = this->ents.assignId<TValId>(valData);
    valData->valRoot = val;
    valData->anchor  = val;
    return val;
}

inline TObjId SymHeapCore::Private::assignId(BlockEntity *hbData) {
    return this->ents.assignId<TObjId>(hbData);
}

bool /* wasPtr */ SymHeapCore::Private::releaseValueOf(TObjId obj, TValId val) {
    if (val <= 0)
        // we do not track uses of special values
        return /* wasPtr */ false;

    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    TObjIdSet &usedBy = valData->usedBy;
    if (1 != usedBy.erase(obj))
        CL_BREAK_IF("SymHeapCore::Private::releaseValueOf(): offset detected");

    if (usedBy.empty()) {
        // kill all related Neq predicates
        TValList neqs;
        this->neqDb->gatherRelatedValues(neqs, val);
        BOOST_FOREACH(const TValId valNeq, neqs) {
            CL_DEBUG("releaseValueOf() kills an orphan Neq predicate");
            this->neqDb->del(valNeq, val);
        }
    }

    const EValueTarget code = valData->code;
    if (!isAnyDataArea(code))
        return /* wasPtr */ false;

    // jump to root
    const TValId root = valData->valRoot;
    this->ents.getEntRW(&valData, root);

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

    const EValueTarget code = valData->code;
    if (!isAnyDataArea(code))
        return;

    // update usedByGl
    const TValId root = valData->valRoot;
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    rootData->usedByGl.insert(obj);
}

// runs only in debug build
bool SymHeapCore::Private::chkValueDeps(const TValId val) {
    const BaseValue *valData;
    this->ents.getEntRO(&valData, val);
    if (VT_CUSTOM != valData->code)
        // we are interested only in CV_INT_RANGE here
        return true;

    const InternalCustomValue *customData =
        dynamic_cast<const InternalCustomValue *>(valData);

    if (CV_INT_RANGE != customData->customData.code())
        // we are interested only in CV_INT_RANGE here
        return true;

    // jump to anchor
    const TValId anchor = customData->anchor;
    const InternalCustomValue *anchorData;
    this->ents.getEntRO(&anchorData, anchor);

    // compute range width of the anchor
    const IR::Range &rngAnchor = anchorData->customData.rng();
    const bool loUnbounded = (IR::IntMin == rngAnchor.lo);
    const bool hiUnbounded = (IR::IntMax == rngAnchor.hi);

    const IR::TUInt width = (loUnbounded || hiUnbounded)
        ? 0U
        : widthOf(rngAnchor);

    const TValList &deps = anchorData->dependentValues;
    BOOST_FOREACH(const TValId depVal, deps) {
        // get the range of the dependent value
        const InternalCustomValue *depData;
        this->ents.getEntRO(&depData, depVal);
        const IR::Range &rng = depData->customData.rng();

        if (loUnbounded != (IR::IntMin == rng.lo))
            goto fail;
        if (hiUnbounded != (IR::IntMax == rng.hi))
            goto fail;

        if (loUnbounded || hiUnbounded)
            continue;

        if (rng.lo - rngAnchor.lo != depData->offRoot)
            goto fail;
        if (rng.hi - rngAnchor.hi != depData->offRoot)
            goto fail;

        if (width == widthOf(rng))
            continue;
fail:
        CL_BREAK_IF("broken CV_INT_RANGE dependency");
        return false;
    }

    return true;
}

// runs only in debug build
bool SymHeapCore::Private::chkArenaConsistency(
        const RootValue            *rootData,
        const bool                  allowOverlap)
{
    if (::bypassSelfChecks)
        return true;

    TLiveObjs all(rootData->liveObjs);
    if (isGone(rootData->code)) {
        CL_BREAK_IF(IR::rngFromNum(IR::Int0) != rootData->size);
        CL_BREAK_IF(!rootData->liveObjs.empty());

        // we can check nothing for VT_DELETED/VT_LOST, we do not know the size
        return true;
    }

    std::set<TOffset> offs;
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const BlockEntity *blData;
        this->ents.getEntRO(&blData, /* obj */ item.first);
        CL_BREAK_IF(!blData->size);

        if (allowOverlap)
            continue;

        const EBlockKind code = item.second;
        if (BK_UNIFORM != code)
            continue;

        // we catch this in gatherUniformBlocks(), but that is already too late
        const TOffset off = blData->off;
        if (!insertOnce(offs, off))
            CL_BREAK_IF("two uniform blocks share the same offset");
    }

    const TArena &arena = rootData->arena;
    const TMemChunk chunk(0, rootData->size.hi);

    TObjIdSet overlaps;
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
    BlockEntity *blData;
    this->ents.getEntRW(&blData, block);

    const BlockEntity *hbData;
    this->ents.getEntRO(&hbData, obj);

    const EBlockKind code = hbData->code;
    switch (code) {
        case BK_DATA_PTR:
        case BK_DATA_OBJ:
            if (this->valsEqual(blData->value, hbData->value))
                // preserve non-conflicting uniform blocks
                return;

        default:
            break;
    }

    // dig root
    const TValId root = blData->root;
    CL_BREAK_IF(root != hbData->root);

    // check up to now arena consistency
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* allowOverlap */ true));

    // dig offsets and sizes
    const TOffset blOff = blData->off;
    const TOffset objOff = hbData->off;
    const TSizeOf blSize = blData->size;
    const TSizeOf objSize = hbData->size;

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
        BlockEntity *blDataOther = blData->clone();
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

        rootData->liveObjs[blOther] = BK_UNIFORM;
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
        const BlockEntity          *blData)
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

inline bool isChar(const TObjType clt) {
    return (CL_TYPE_INT == clt->code)
        && (1 == clt->size);
}

inline bool isString(const TObjType clt) {
    return (CL_TYPE_ARRAY == clt->code)
        && isChar(targetTypeOfArray(clt));
}

bool SymHeapCore::Private::writeCharToString(
        TValId                     *pValDst,
        const TValId                valToWrite,
        const TOffset               pos)
{
    const TValId valDst = *pValDst;
    if (VAL_INVALID == valDst)
        // there is no string to write to
        return false;

    const BaseValue *dstData;
    this->ents.getEntRO(&dstData, valDst);
    if (VT_CUSTOM != dstData->code)
        // there is no string to write to
        return false;

    // extract the string that is going to be modified
    const InternalCustomValue *stringData =
        DCAST<const InternalCustomValue *>(dstData);
    std::string str(stringData->customData.str());
    CL_BREAK_IF(static_cast<TOffset>(str.size()) < pos || pos < 0);

    // modify the string accordingly as long as the result is still a string
    if (VAL_NULL == valToWrite)
        str.resize(pos);
    else {
        const BaseValue *valData;
        this->ents.getEntRO(&valData, valToWrite);
        if (VT_CUSTOM != valData->code)
            return false;

        const InternalCustomValue *numData =
            DCAST<const InternalCustomValue *>(valData);

        const IR::Range rng = numData->customData.rng();
        if (!isSingular(rng))
            return false;

        const IR::TInt num = rng.lo;
        str[pos] = num;
    }

    // update the mapping of the string being assigned
    CL_DEBUG("CV_STRING replaced as a consequence of data reinterpretation");
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->cValueMap);
    const CustomValue cvStr(str.c_str());
    TValId &valStr = this->cValueMap->lookup(cvStr);

    if (VAL_INVALID == valStr) {
        // CV_STRING not found, wrap it as a new heap value
        valStr = this->valCreate(VT_CUSTOM, VO_ASSIGNED);
        InternalCustomValue *dstData;
        this->ents.getEntRW(&dstData, valStr);
        dstData->customData = cvStr;
    }

    *pValDst = valStr;
    return true;
}

bool SymHeapCore::Private::reinterpretSingleObj(
        HeapObject                 *dstData,
        const BlockEntity          *srcData)
{
    CL_BREAK_IF(srcData->root != dstData->root);

    const EBlockKind code = srcData->code;
    switch (code) {
        case BK_DATA_OBJ:
            break;

        default:
            // TODO: hook various reinterpretation drivers here
            return false;
    }

    const HeapObject *objData = DCAST<const HeapObject *>(srcData);
    const TValId valSrc = objData->value;
    if (VAL_INVALID == valSrc)
        // invalid source
        return false;

    const TObjType cltSrc = objData->clt;
    const TObjType cltDst = dstData->clt;

    if (isString(cltSrc) && isChar(cltDst)) {
        // read char from a zero-terminated string
        const InternalCustomValue *valData;
        this->ents.getEntRO(&valData, valSrc);

        const TOffset off = dstData->off - srcData->off;
        const std::string &str = valData->customData.str();
        CL_BREAK_IF(static_cast<TOffset>(str.size()) < off || off < 0);

        // byte-level access to zero-terminated strings
        const IR::TInt num = str[off];
        dstData->value = this->wrapIntVal(num);
        return true;
    }

    if (isChar(cltSrc) && isString(cltDst)) {
        // write char to a zero-terminated string
        const TOffset off = srcData->off - dstData->off;
        return this->writeCharToString(&dstData->value, valSrc, off);
    }

    // TODO: hook various reinterpretation drivers here
    return false;
}

void SymHeapCore::Private::reinterpretObjData(
        TObjId                      old,
        TObjId                      obj,
        TValSet                    *killedPtrs)
{
    BlockEntity *blData;
    this->ents.getEntRW(&blData, old);

    EBlockKind code = blData->code;
    switch (code) {
        case BK_DATA_PTR:
        case BK_DATA_OBJ:
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
    CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* allowOverlap */ true));

    this->ents.getEntRW(&blData, obj);
    code = blData->code;

    switch (code) {
        case BK_UNIFORM:
            if (isCoveredByBlock(oldData, blData)) {
                // object fully covered by the overlapping uniform block
                oldData->value = this->valDup(blData->value);
                goto data_restored;
            }
            // fall through!

        case BK_DATA_PTR:
        case BK_DATA_OBJ:
            if (this->reinterpretSingleObj(oldData, blData))
                goto data_restored;

            oldData->value = this->valCreate(VT_UNKNOWN, VO_REINTERPRET);
            break;

        case BK_COMPOSITE:
        case BK_INVALID:
        default:
            CL_BREAK_IF("invalid call of reinterpretObjData()");
            return;
    }

    // mark the object as dead
    if (rootData->liveObjs.erase(old))
        CL_DEBUG("reinterpretObjData() kills a live object");

    if (!oldData->extRefCnt) {
        CL_DEBUG("reinterpretObjData() destroys a dead object");
        this->objDestroy(old, /* removeVal */ false, /* detach */ true);
        return;
    }

data_restored:
    // register the newly assigned value of the _old_ object
    this->registerValueOf(old, oldData->value);
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
    TObjIdSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), obj)) {
        BOOST_FOREACH(const TObjId old, overlaps)
            this->reinterpretObjData(old, obj, killedPtrs);
    }

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
}

TObjId SymHeapCore::Private::objCreate(
        TValId                      root,
        TOffset                     off,
        TObjType                    clt)
{
    // acquire object ID
    HeapObject *objData = new HeapObject(root, off, clt);
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
    BlockEntity *blData;
    this->ents.getEntRW(&blData, obj);

    const EBlockKind code = blData->code;
    if (removeVal && BK_UNIFORM != code) {
        // release value of the object
        TValId &val = blData->value;
        this->releaseValueOf(obj, val);
        val = VAL_INVALID;
    }

    if (detach) {
        // properly remove the object from grid and arena
        RootValue *rootData;
        this->ents.getEntRW(&rootData, blData->root);
        CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* mayOverlap */true));

        // remove the object from arena unless we are destroying everything
        const TOffset off = blData->off;
        const TSizeOf size = blData->size;
        rootData->arena -= createArenaItem(off, size, obj);

        CL_BREAK_IF(hasKey(rootData->liveObjs, obj));
        CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* mayOverlap */true));
    }

    if (BK_UNIFORM != code && 0 < DCAST<HeapObject *>(blData)->extRefCnt)
        // preserve an externally referenced object
        return;

    // release the corresponding HeapObject instance
    this->ents.releaseEnt(obj);
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

        case VT_RANGE:
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
        const TSizeOf               size,
        const TValSet              *killedPtrs)
{
    CL_ERROR("shiftBlockAt() is not implemented yet");
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
        const TSizeOf               winSize)
{
    // check up to now arena consistency
    const RootValue *rootDataSrc;
    this->ents.getEntRO(&rootDataSrc, srcRoot);
    CL_BREAK_IF(!this->chkArenaConsistency(rootDataSrc));

    const TArena &arena = rootDataSrc->arena;
    const TOffset winEnd = winBeg + winSize;
    const TMemChunk chunk (winBeg, winEnd);

    TObjIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, OBJ_INVALID))
        // no data to copy in here
        return;

    RootValue *rootDataDst;
    this->ents.getEntRW(&rootDataDst, dstRoot);
    const TOffset shift = dstOff - winBeg;

    // go through overlaps and copy the live ones
    BOOST_FOREACH(const TObjId objSrc, overlaps) {
        const BlockEntity *hbDataSrc;
        this->ents.getEntRO(&hbDataSrc, objSrc);

        const EBlockKind code = hbDataSrc->code;
        const TOffset beg = hbDataSrc->off;
        const TOffset end = beg + hbDataSrc->size;

        TOffset realShift = shift;
        TSizeOf sizeLimit = /* disabled */ 0;

        const bool begExceeds = (beg < winBeg);
        if (begExceeds || winEnd < end) {
            if (BK_UNIFORM != code)
                // regular object that exceeds the window, do not copy this one
                continue;

            if (begExceeds)
                realShift -= beg - winBeg;

            sizeLimit = hbDataSrc->size + beg + winEnd - winBeg - end;
            CL_BREAK_IF(hbDataSrc->size <= sizeLimit || sizeLimit <= 0);
        }

        if (!hasKey(rootDataSrc->liveObjs, objSrc))
            // dead object anyway
            continue;

        // copy a single live block
        this->copySingleLiveBlock(dstRoot, rootDataDst, objSrc, code,
                realShift, sizeLimit);
    }
}


SymHeapCore::Private::Private(Trace::Node *trace):
    traceHandle (trace),
    liveRoots   (new TValSetWrapper),
    cVarMap     (new CVarMap),
    cValueMap   (new CustomValueMapper),
    coinDb      (new CoincidenceDb),
    neqDb       (new NeqDb)
{
    // allocate a root-value for VAL_NULL
    this->assignId(new RootValue(VT_INVALID, VO_INVALID));
}

SymHeapCore::Private::Private(const SymHeapCore::Private &ref):
    traceHandle (new Trace::CloneNode(ref.traceHandle.node())),
    ents        (ref.ents),
    liveRoots   (ref.liveRoots),
    cVarMap     (ref.cVarMap),
    cValueMap   (ref.cValueMap),
    coinDb      (ref.coinDb),
    neqDb       (ref.neqDb)
{
    RefCntLib<RCO_NON_VIRT>::enter(this->liveRoots);
    RefCntLib<RCO_NON_VIRT>::enter(this->cVarMap);
    RefCntLib<RCO_NON_VIRT>::enter(this->cValueMap);
    RefCntLib<RCO_NON_VIRT>::enter(this->coinDb);
    RefCntLib<RCO_NON_VIRT>::enter(this->neqDb);
}

SymHeapCore::Private::~Private() {
    RefCntLib<RCO_NON_VIRT>::leave(this->liveRoots);
    RefCntLib<RCO_NON_VIRT>::leave(this->cVarMap);
    RefCntLib<RCO_NON_VIRT>::leave(this->cValueMap);
    RefCntLib<RCO_NON_VIRT>::leave(this->coinDb);
    RefCntLib<RCO_NON_VIRT>::leave(this->neqDb);
}

TValId SymHeapCore::Private::objInit(TObjId obj) {
    HeapObject *objData;
    this->ents.getEntRW(&objData, obj);
    CL_BREAK_IF(!objData->extRefCnt);

    // resolve root
    const TValId root = objData->root;
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    const TArena &arena = rootData->arena;
    const TOffset off = objData->off;
    const TObjType clt = objData->clt;

    // first check for data reinterpretation
    TObjIdSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), obj)) {
        BOOST_FOREACH(const TObjId other, overlaps) {
            const BlockEntity *blockData;
            this->ents.getEntRO(&blockData, other);

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
        rootData->liveObjs[obj] = BK_DATA_PTR;
#if SE_TRACK_NON_POINTER_VALUES
    else
        rootData->liveObjs[obj] = BK_DATA_OBJ;
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

    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);

    TValId val = objData->value;
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

        // store the value
        HeapObject *objDataRW;
        d->ents.getEntRW(&objDataRW, obj);
        objDataRW->value = val;

        // store backward reference
        compData->usedBy.insert(obj);
        return val;
    }

    // delayed object initialization
    return d->objInit(obj);
}

void SymHeapCore::usedBy(ObjList &dst, TValId val, bool liveOnly) const {
    if (VAL_NULL == val)
        // we do not track uses of special values
        return;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    const TObjIdSet &usedBy = valData->usedBy;
    if (!liveOnly) {
        // dump everything
        BOOST_FOREACH(const TObjId obj, usedBy)
            dst.push_back(ObjHandle(*const_cast<SymHeapCore *>(this), obj));

        return;
    }

    BOOST_FOREACH(const TObjId obj, usedBy) {
        // get object data
        const HeapObject *objData;
        d->ents.getEntRO(&objData, obj);

        // get root data
        const TValId root = objData->root;
        const RootValue *rootData;
        d->ents.getEntRO(&rootData, root);

        // check if the object is alive
        if (hasKey(rootData->liveObjs, obj))
            dst.push_back(ObjHandle(*const_cast<SymHeapCore *>(this), obj));
    }
}

unsigned SymHeapCore::usedByCount(TValId val) const {
    if (VAL_NULL == val)
        return 0;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->usedBy.size();
}

void SymHeapCore::pointedBy(ObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    CL_BREAK_IF(rootData->offRoot);
    CL_BREAK_IF(!isPossibleToDeref(rootData->code));

    const TObjIdSet &usedBy = rootData->usedByGl;
    BOOST_FOREACH(const TObjId obj, usedBy)
        dst.push_back(ObjHandle(*const_cast<SymHeapCore *>(this), obj));
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

    const TValId root = valData->valRoot;
    if (VAL_NULL == root) {
        CL_BREAK_IF("VAL_NULL is not supposed to be cloned");
        return val;
    }

    if (VT_RANGE == code) {
        CL_DEBUG("support for VT_RANGE in valClone() is experimental");
        const IR::Range range = this->valOffsetRange(val);
        return this->valByRange(valData->valRoot, range);
    }

    if (!isPossibleToDeref(code))
        // duplicate an unknown value
        return d->valDup(val);

    // duplicate a root object
    const TValId dupAt = d->dupRoot(root);

    // take the offset into consideration
    return this->valByOffset(dupAt, valData->offRoot);
}

TObjId SymHeapCore::Private::copySingleLiveBlock(
        const TValId                rootDst,
        RootValue                  *rootDataDst,
        const TObjId                objSrc,
        const EBlockKind            code,
        const TOffset               shift,
        const TSizeOf               sizeLimit)
{
    TObjId dst;

    if (BK_UNIFORM == code) {
        // duplicate a uniform block
        const BlockEntity *blSrc;
        this->ents.getEntRO(&blSrc, objSrc);
        BlockEntity *blDst = blSrc->clone();
        dst = this->assignId(blDst);
        blDst->root = rootDst;

        // shift the block and limit the size if asked to do so
        blDst->off += shift;
        if (sizeLimit)
            blDst->size = sizeLimit;

        // map the cloned block
        rootDataDst->arena += createArenaItem(blDst->off, blDst->size, dst);
    }
    else {
        // duplicate a regular object
        CL_BREAK_IF(BK_DATA_PTR != code && BK_DATA_OBJ != code);
        CL_BREAK_IF(sizeLimit);

        const HeapObject *objDataSrc;
        this->ents.getEntRO(&objDataSrc, objSrc);

        const TOffset off = objDataSrc->off + shift;
        const TObjType clt = objDataSrc->clt;
        dst = this->objCreate(rootDst, off, clt);
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

    // duplicate root metadata
    rootDataDst->cVar               = rootDataSrc->cVar;
    rootDataDst->size               = rootDataSrc->size;
    rootDataDst->lastKnownClt       = rootDataSrc->lastKnownClt;
    rootDataDst->protoLevel         = rootDataSrc->protoLevel;

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->liveRoots);
    this->liveRoots->insert(imageAt);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootDataSrc->liveObjs)
        this->copySingleLiveBlock(imageAt, rootDataDst,
                /* src  */ item.first,
                /* code */ item.second);

    CL_BREAK_IF(!this->chkArenaConsistency(rootDataDst));
    return imageAt;
}

void SymHeapCore::gatherLivePointers(ObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const EBlockKind code = item.second;
        if (BK_DATA_PTR != code)
            continue;

        const TObjId obj = item.first;
        dst.push_back(ObjHandle(*const_cast<SymHeapCore *>(this), obj));
    }
}

void SymHeapCore::gatherUniformBlocks(TUniBlockMap &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const EBlockKind code = item.second;
        if (BK_UNIFORM != code)
            continue;

        const BlockEntity *blData;
        d->ents.getEntRO(&blData, /* obj */ item.first);
        const TOffset off = blData->off;
        CL_BREAK_IF(hasKey(dst, off) && !::bypassSelfChecks);
        UniformBlock &block = dst[off];

        // export uniform block
        block.off       = off;
        block.size      = blData->size;
        block.tplValue  = blData->value;
    }
}

void SymHeapCore::gatherLiveObjects(ObjList &dst, TValId root) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);

    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveObjs) {
        const EBlockKind code = item.second;

        switch (code) {
            case BK_UNIFORM:
                continue;

            case BK_DATA_PTR:
            case BK_DATA_OBJ:
                break;

            case BK_INVALID:
            default:
                CL_BREAK_IF("gatherLiveObjects sees something special");
        }

        const TObjId obj = item.first;
        dst.push_back(ObjHandle(*const_cast<SymHeapCore *>(this), obj));
    }
}

bool SymHeapCore::findCoveringUniBlocks(
        TUniBlockMap               *pCovered,
        const TValId                root,
        UniformBlock                block)
    const
{
    CL_BREAK_IF(!pCovered->empty());

    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    CL_BREAK_IF(!d->chkArenaConsistency(rootData));

    const TArena &arena = rootData->arena;
    const TOffset beg = block.off;
    const TOffset end = beg + block.size;
    const TMemChunk chunk(beg, end);

    TObjIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, OBJ_INVALID))
        // not found
        return false;

    // use a temporary arena to test the coverage
    TArena coverage;
    coverage += TMemItem(chunk, /* XXX: misleading */ OBJ_UNKNOWN);

    // go through overlaps and subtract the chunks that are covered
    BOOST_FOREACH(const TObjId obj, overlaps) {
        const BlockEntity *blData;
        d->ents.getEntRO(&blData, obj);

        const EBlockKind code = blData->code;
        if (BK_UNIFORM != code && VAL_NULL != blData->value)
            continue;

        if (!areValProtosEqual(*this, *this, blData->value, block.tplValue))
            // incompatible value prototype
            continue;

        // this block entity can be used to build up the coverage, subtract it
        const TOffset blBeg = blData->off;
        const TSizeOf blSize = blData->size;
        coverage -= createArenaItem(blBeg, blSize, OBJ_UNKNOWN);
    }

    TObjIdSet uncovered;
    if (!arenaLookup(&uncovered, coverage, chunk, OBJ_INVALID)) {
        // full coverage has been found
        (*pCovered)[beg] = block;
        return true;
    }

    TArena::TKeySet gaps;
    coverage.reverseLookup(gaps, OBJ_UNKNOWN);
    if (gaps.empty())
        // there is really nothing we could pick for coverage
        return false;

    // TODO: rewrite the algorithm so that we do not compute complement twice
    coverage.clear();
    coverage += TMemItem(chunk, /* XXX: misleading */ OBJ_UNKNOWN);
    BOOST_FOREACH(TArena::TKeySet::const_reference item, gaps)
        coverage -= TMemItem(item, OBJ_UNKNOWN);

    // return partial coverage (if any)
    TArena::TKeySet covChunks;
    coverage.reverseLookup(covChunks, OBJ_UNKNOWN);
    BOOST_FOREACH(TArena::TKeySet::const_reference item, covChunks) {
        const TOffset off = item.first;
        block.off = off;
        block.size = /* end */ item.second - off;
        (*pCovered)[off] = block;
    }

    return false;
}

SymHeapCore::SymHeapCore(TStorRef stor, Trace::Node *trace):
    stor_(stor),
    d(new Private(trace))
{
    CL_BREAK_IF(!&stor_);

    // initialize VAL_ADDR_OF_RET
    const TValId addrRet = d->valCreate(VT_ON_STACK, VO_ASSIGNED);
    CL_BREAK_IF(VAL_ADDR_OF_RET != addrRet);
    (void) addrRet;
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

// cppcheck-suppress operatorEqToSelf
SymHeapCore& SymHeapCore::operator=(const SymHeapCore &ref) {
    CL_BREAK_IF(&ref == this);
    CL_BREAK_IF(&stor_ != &ref.stor_);

    delete d;
    d = new Private(*ref.d);
    return *this;
}

void SymHeapCore::swap(SymHeapCore &ref) {
    CL_BREAK_IF(&stor_ != &ref.stor_);
    swapValues(this->d, ref.d);
}

Trace::Node* SymHeapCore::traceNode() const {
    return d->traceHandle.node();
}

void SymHeapCore::traceUpdate(Trace::Node *node) {
    d->traceHandle.reset(node);
}

void SymHeapCore::objSetValue(TObjId obj, TValId val, TValSet *killedPtrs) {
    // we allow to set values of atomic types only
    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);

    // make sure that the value is not a pointer to a structure object
    const TObjType clt = objData->clt;
    CL_BREAK_IF(VT_COMPOSITE == this->valTarget(val));

    // check whether the root entity that owns this object ID is still valid
    CL_BREAK_IF(!isPossibleToDeref(this->valTarget(objData->root)));

    // mark the destination object as live
    const TValId root = objData->root;
    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);
    rootData->liveObjs[obj] = bkFromClt(clt);

    // now set the value
    d->setValueOf(obj, val, killedPtrs);
}

TObjId SymHeapCore::Private::writeUniformBlock(
        const TValId                addr,
        const TValId                tplVal,
        const TSizeOf               size,
        TValSet                     *killedPtrs)
{
    const BaseValue *valData;
    this->ents.getEntRO(&valData, addr);

    const TValId root = valData->valRoot;
    const TOffset beg = valData->offRoot;
    const TOffset end = beg + size;

    // acquire object ID
    BlockEntity *blData = new BlockEntity(BK_UNIFORM, root, beg, size, tplVal);
    const TObjId obj = this->assignId(blData);

    // check up to now arena consistency
    RootValue *rootData;
    this->ents.getEntRW(&rootData, root);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // mark the block as live
    rootData->liveObjs[obj] = BK_UNIFORM;

    TArena &arena = rootData->arena;
    arena += createArenaItem(beg, size, obj);
    const TMemChunk chunk(beg, end);

    // invalidate contents of the objects we are overwriting
    TObjIdSet overlaps;
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
        const TSizeOf               size,
        TValSet                     *killedPtrs)
{
    CL_BREAK_IF(this->valSizeOfTarget(addr).lo < size);
    d->writeUniformBlock(addr, tplValue, size, killedPtrs);
}

void SymHeapCore::copyBlockOfRawMemory(
        const TValId                dst,
        const TValId                src,
        const TSizeOf               size,
        TValSet                     *killedPtrs)
{
    // this should have been checked by the caller
    CL_BREAK_IF(this->valSizeOfTarget(dst).lo < size);
    CL_BREAK_IF(this->valSizeOfTarget(src).lo < size);

    const BaseValue *dstData;
    const BaseValue *srcData;

    d->ents.getEntRO(&dstData, dst);
    d->ents.getEntRO(&srcData, src);

    CL_BREAK_IF(!isPossibleToDeref(dstData->code));
    CL_BREAK_IF(!isPossibleToDeref(srcData->code));
    CL_BREAK_IF(!size);

    const TOffset dstOff = dstData->offRoot;
    const TOffset srcOff = srcData->offRoot;
    const TValId dstRoot = dstData->valRoot;
    const TValId srcRoot = srcData->valRoot;

    if (dstRoot == srcRoot) {
        // movement within a single root entity
        const TOffset diff = dstOff - srcOff;
        d->shiftBlockAt(dstRoot, diff, size, killedPtrs);
        return;
    }

    // nuke the content we are going to overwrite
    const TObjId blKiller = d->writeUniformBlock(dst, /* misleading */ VAL_NULL,
                                                 size, killedPtrs);

    // check up to now arena consistency
    RootValue *rootDataDst;
    d->ents.getEntRW(&rootDataDst, dstRoot);
    CL_BREAK_IF(!d->chkArenaConsistency(rootDataDst));

    // remove the dummy block we used just to trigger the data reinterpretation
    rootDataDst->liveObjs.erase(blKiller);
    rootDataDst->arena -= createArenaItem(dstOff, size, blKiller);
    d->ents.releaseEnt(blKiller);
    CL_BREAK_IF(!d->chkArenaConsistency(rootDataDst));

    // now we need to transfer data between two distinct root entities
    d->transferBlock(dstRoot, srcRoot, dstOff, srcOff, size);
    CL_BREAK_IF(!d->chkArenaConsistency(rootDataDst));
}

bool SymHeapCore::Private::findZeroInBlock(
        TOffset                *offDst,
        bool                   *provenPrefix,
        const TOffset           offSrc,
        const                   TObjId obj)
{
    const BlockEntity *blData;
    this->ents.getEntRO(&blData, obj);

    const EBlockKind code = blData->code;
    switch (code) {
        case BK_DATA_OBJ:
            break;

        case BK_DATA_PTR:
        case BK_UNIFORM:
            if (VAL_NULL != blData->value)
                return false;

            // a uniform block full of zeros
            *offDst = blData->off;
            return true;

        default:
            CL_BREAK_IF("findZeroInBlock() got something special");
            return false;
    }

    const HeapObject *objData = DCAST<const HeapObject *>(blData);
    if (CL_TYPE_ARRAY == objData->clt->code) {
        // assume zero-terminated string
        const InternalCustomValue *valData;
        this->ents.getEntRO(&valData, objData->value);

        // check whether the prefix is proven to be non-zero
        const TOffset off = blData->off;
        *provenPrefix = (off <= offSrc);

        // the length of the string is equal to the offset of its trailing zero
        const std::string &str = valData->customData.str();
        *offDst = off + str.size();
        return true;
    }

    CL_BREAK_IF("please implement");
    return false;
}

bool SymHeapCore::Private::findZeroAtOff(
        IR::Range              *offDst,
        const TOffset           offSrc,
        const TValId            root)
{
    const RootValue *rootData;
    this->ents.getEntRO(&rootData, root);

    const TArena &arena = rootData->arena;
    const TSizeOf limit = rootData->size.hi;
    const TMemChunk chunk(offSrc, limit);

    TObjIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, OBJ_INVALID))
        // no blocks that would serve as a trailing zero
        return false;

    bool provenPrefix = false;;

    // go through all intersections and find the zero that is closest to offSrc
    TOffset first = limit;
    BOOST_FOREACH(const TObjId obj, overlaps) {
        TOffset beg;
        if (!this->findZeroInBlock(&beg, &provenPrefix, offSrc, obj))
            // failed to imply zero in this block entity
            continue;

        if (first < beg)
            // we have already found a zero closer to offSrc
            continue;

        if (beg <= offSrc) {
            // if the nullified block begins before offSrc, trim the offset
            first = offSrc;
            break;
        }

        // update the best match
        first = beg;

        if (provenPrefix)
            // we already know there are no zeros in the prefix
            break;
    }

    if (limit == first)
        // found nothing
        return false;

    offDst->hi = first;
    offDst->lo = (provenPrefix)
        ? first
        : offSrc;

    return true;
}

TObjType SymHeapCore::objType(TObjId obj) const {
    if (obj < 0)
        return 0;

    const HeapObject *objData;
    d->ents.getEntRO(&objData, obj);
    return objData->clt;
}

TValId SymHeapCore::Private::shiftCustomValue(TValId ref, TOffset shift) {
    CL_BREAK_IF(!this->chkValueDeps(ref));

    const InternalCustomValue *customDataRef;
    this->ents.getEntRO(&customDataRef, ref);

    // prepare a custom value template and compute the shifted range
    const IR::Range rngRef = rngFromCustom(customDataRef->customData);
    const CustomValue cv(rngRef + IR::rngFromNum(shift));

    // create a new CV_INT_RANGE custom value (do not recycle existing)
    const TValId val = this->valCreate(VT_CUSTOM, VO_ASSIGNED);
    InternalCustomValue *customData;
    this->ents.getEntRW(&customData, val);
    customData->anchor      = customDataRef->anchor;
    customData->offRoot     = customDataRef->offRoot + shift;
    customData->customData  = cv;

    // register this value as a dependent value by the anchor
    ReferableValue *refData;
    this->ents.getEntRW(&refData, customData->anchor);
    refData->dependentValues.push_back(val);

    CL_BREAK_IF(!this->chkValueDeps(val));
    return val;
}

TValId SymHeapCore::Private::wrapIntVal(const IR::TInt num) {
    if (IR::Int0 == num)
        return VAL_NULL;

    if (IR::Int1 == num)
        return VAL_TRUE;

    // CV_INT values are supposed to be reused if they exist already
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->cValueMap);
    const CustomValue cvRng(IR::rngFromNum(num));
    TValId &valInt = this->cValueMap->lookup(cvRng);

    if (VAL_INVALID == valInt) {
        // CV_INT_RANGE not found, wrap it as a new heap value
        valInt = this->valCreate(VT_CUSTOM, VO_ASSIGNED);
        InternalCustomValue *intData;
        this->ents.getEntRW(&intData, valInt);
        intData->customData = cvRng;
    }

    return valInt;
}

void SymHeapCore::Private::replaceRngByInt(const InternalCustomValue *valData) {
    CL_DEBUG("replaceRngByInt() is taking place...");

    // we already expect a scalar at this point
    const CustomValue &cvRng = valData->customData;
    const IR::Range &rng = rngFromCustom(cvRng);
    CL_BREAK_IF(!isSingular(rng));

    const TValId replaceBy = this->wrapIntVal(rng.lo);

    // we intentionally do not use a reference here (tight loop otherwise)
    TObjIdSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TObjId obj, usedBy)
        this->setValueOf(obj, replaceBy);
}

void SymHeapCore::Private::trimCustomValue(TValId val, const IR::Range &win) {
    CL_BREAK_IF(!this->chkValueDeps(val));

    const InternalCustomValue *valData;
    this->ents.getEntRO(&valData, val);

    // extract the original integral ragne
    const IR::Range &refRange = valData->customData.rng();
    CL_BREAK_IF(isSingular(refRange));

    // compute the difference between the original and desired ranges
    if (win.lo < refRange.lo || refRange.hi < win.hi) {
        CL_BREAK_IF("attempt to use trimCustomValue() to enlarge the interval");
        return;
    }

    // jump to anchor
    const TValId anchor = valData->anchor;
    InternalCustomValue *anchorData;
    this->ents.getEntRW(&anchorData, anchor);

    // update range of the anchor
    const TOffset off = valData->offRoot;
    IR::Range &rngAnchor = anchorData->customData.rng();
    rngAnchor = win - IR::rngFromNum(off);

    if (isSingular(rngAnchor))
        // CV_INT_RANGE reduced to CV_INT
        this->replaceRngByInt(anchorData);

    // go through all dependent values including the anchor itself
    const TValList deps(anchorData->dependentValues);
    BOOST_FOREACH(const TValId depVal, deps) {
        InternalCustomValue *depData;
        this->ents.getEntRW(&depData, depVal);

        // update the dependent value
        IR::Range &rngDep = depData->customData.rng();
        rngDep = rngAnchor +  IR::rngFromNum(off);

        if (isSingular(rngDep))
            // CV_INT_RANGE reduced to CV_INT
            this->replaceRngByInt(depData);
    }

    CL_BREAK_IF(!this->chkValueDeps(val));
}

TValId SymHeapCore::valByOffset(TValId at, TOffset off) {
    if (!off || at < 0)
        return at;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);
    const TValId valRoot = valData->valRoot;
    const EValueTarget code = valData->code;

    TValId anchor = valRoot;
    if (VT_RANGE == code)
        anchor = valData->anchor;

    // subtract the root
    off += valData->offRoot;
    if (!off)
        return anchor;

    if (VT_UNKNOWN == code)
        // do not track off-value for invalid targets
        return d->valDup(at);

    if (VT_CUSTOM == code) {
        CL_BREAK_IF("invalid call of valByOffset(), use valShift() instead");
        return VAL_INVALID;
    }

    // off-value lookup
    const AnchorValue *anchorData;
    d->ents.getEntRO(&anchorData, anchor);
    const TOffMap &offMap = anchorData->offMap;
    TOffMap::const_iterator it = offMap.find(off);
    if (offMap.end() != it)
        return it->second;

    // create a new off-value
    BaseValue *offVal = new BaseValue(code, valData->origin);
    const TValId val = d->assignId(offVal);

    // offVal->valRoot needs to be set after the call of Private::assignId()
    offVal->valRoot = valRoot;
    offVal->anchor  = anchor;
    offVal->offRoot = off;

    // store the mapping for next wheel
    AnchorValue *anchorDataRW;
    d->ents.getEntRW(&anchorDataRW, anchor);
    anchorDataRW->offMap[off] = val;
    return val;
}

TValId SymHeapCore::valByRange(TValId at, IR::Range range) {
    if (isSingular(range)) {
        CL_DEBUG("valByRange() got a singular range, passing to valByOffset()");
        return this->valByOffset(at, range.lo);
    }

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);
    if (VAL_NULL == at || isGone(valData->code))
        return d->valCreate(VT_UNKNOWN, VO_UNKNOWN);

    CL_BREAK_IF(!isPossibleToDeref(valData->code));

    // include the relative offset of the starting point
    const TValId valRoot = valData->valRoot;
    const TOffset offset = valData->offRoot;
    range += IR::rngFromNum(offset);

    if (isAligned(range)) {
        CL_BREAK_IF("TODO: deal better with alignment");
        range.alignment = IR::Int1;
    }

    // create a new range value
    RangeValue *rangeData = new RangeValue(range);
    const TValId val = d->assignId(rangeData);

    // offVal->valRoot needs to be set after the call of Private::assignId()
    rangeData->valRoot  = valRoot;
    rangeData->anchor   = val;

    // register the VT_RANGE value by the owning root entity
    RootValue *rootData;
    d->ents.getEntRW(&rootData, valRoot);
    rootData->dependentValues.push_back(val);

    return val;
}

TValId SymHeapCore::valShift(TValId valToShift, TValId shiftBy) {
    if (valToShift < 0)
        // do not shift special values
        return valToShift;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, valToShift);

    IR::Range rng;
    if (!rngFromVal(&rng, *this, shiftBy)) {
        CL_BREAK_IF("valShift() needs at least integral range as shiftBy");
        return VAL_INVALID;
    }

    const EValueTarget code = valData->code;

    if (isSingular(rng)) {
        // the value is going the be shifted by a known integer (not a range)
        const IR::TInt off = rng.lo;

        if (VT_CUSTOM == code)
            return d->shiftCustomValue(valToShift, off);
        else
            return this->valByOffset(valToShift, off);
    }

    if (isPossibleToDeref(code))
        return this->valByRange(valToShift, rng);

    if (VT_RANGE != code) {
        CL_BREAK_IF("unhandled call of SymHeapCore::valShift()");
        return d->valCreate(VT_UNKNOWN, VO_UNKNOWN);
    }

    // split valToShift to anchor/offset
    const TValId anchor1 = valData->anchor;
    const TOffset off1   = valData->offRoot;

    // split shiftBy to anchor/offset
    const BaseValue *shiftData;
    d->ents.getEntRO(&shiftData, shiftBy);
    const TValId anchor2 = shiftData->anchor;
    const TOffset off2   = shiftData->offRoot;

    // summarize the total offset
    const TOffset offTotal = off1 + off2;

    // lookup on anchors, then shift by the total offset if succeeded
    TValId valResult;
    if (d->coinDb->chk(&valResult, anchor1, anchor2))
        return this->valByOffset(valResult, off1 + off2);

    // compute the resulting range and wrap it as a heap value
    const IR::Range rngResult = rng + this->valOffsetRange(valToShift);
    valResult = this->valByRange(valData->valRoot, rngResult);

    // store the mapping for next wheel
    const TValId valSum = this->valByOffset(valResult, -offTotal);
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->coinDb);
    d->coinDb->add(anchor1, anchor2, valSum);

    // NOTE: valResult is what the caller asks for (valSum is what we track)
    return valResult;
}

void SymHeapCore::valRestrictRange(TValId val, IR::Range win) {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    const EValueTarget code = valData->code;
    switch (code) {
        case VT_RANGE:
            break;

        case VT_CUSTOM:
            d->trimCustomValue(val, win);
            return;

        case VT_UNKNOWN:
            if (!isSingular(win)) {
                const CustomValue cv(win);
                this->valReplace(val, this->valWrapCustom(cv));
                return;
            }
            // fall through!

        default:
            CL_BREAK_IF("invalid call of valRestrictRange()");
            return;
    }

    const TValId anchor = valData->anchor;
    const TOffset shift = valData->offRoot;
    CL_BREAK_IF((!!shift) == (anchor == val));

    RangeValue *rangeData;
    d->ents.getEntRW(&rangeData, anchor);
    IR::Range &range = rangeData->range;

    // translate the given window to our root coords
    win -= IR::rngFromNum(shift);

    // first check that the caller uses the SymHeapCore API correctly
    CL_BREAK_IF(win == range);
    CL_BREAK_IF(win.lo < range.lo);
    CL_BREAK_IF(range.hi < win.hi);

    // restrict the offset range now!
    range = win;
    if (!isSingular(range))
        return;

    // the range has been restricted to a single off-value, trow it away!
    CL_DEBUG("valRestrictRange() throws away a singular offset range...");
    const TValId valRoot = rangeData->valRoot;
    const TOffset offRoot = range.lo;
    const TValId valSubst = this->valByOffset(valRoot, offRoot);
    this->valReplace(anchor, valSubst);

    BOOST_FOREACH(TOffMap::const_reference item, rangeData->offMap) {
        const TOffset offRel = item.first;
        const TValId valOld = item.second;

        const TOffset offTotal = offRoot + offRel;
        const TValId valNew = this->valByOffset(valRoot, offTotal);
        this->valReplace(valOld, valNew);
    }
}

void SymHeapCore::Private::bindValues(TValId v1, TValId v2, TValId valSum) {
    const BaseValue *valData1, *valData2;
    this->ents.getEntRO(&valData1, v1);
    this->ents.getEntRO(&valData2, v2);

    const TOffset off1 = valData1->offRoot;
    const TOffset off2 = valData2->offRoot;
    if (off1 || off2) {
        CL_BREAK_IF("bindValues() does not support offsets yet");
        return;
    }

    const TValId anchor1 = valData1->anchor;
    const TValId anchor2 = valData2->anchor;

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->coinDb);
    this->coinDb->add(anchor1, anchor2, valSum);
}

TValId SymHeapCore::diffPointers(const TValId v1, const TValId v2) {
    const TValId root1 = this->valRoot(v1);
    const TValId root2 = this->valRoot(v2);
    if (root1 != root2)
        return d->valCreate(VT_UNKNOWN, VO_UNKNOWN);

    // get offset ranges for both pointers
    const IR::Range off1 = this->valOffsetRange(v1);
    const IR::Range off2 = this->valOffsetRange(v2);

    // TODO: check for an already existing coincidence to improve the precision

    // compute the difference and wrap it as a heap value
    const CustomValue cv(off1 - off2);
    const TValId valDiff = this->valWrapCustom(cv);
    if (isSingular(rngFromCustom(cv)))
        // good luck, the difference is a scalar
        return valDiff;

    d->bindValues(v2, valDiff, /* sum */ v1);
    return valDiff;
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

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    const EValueTarget code = valData->code;
    if (VT_RANGE == code)
        // VT_RANGE takes precedence over VT_ABSTRACT
        return VT_RANGE;

    if (this->hasAbstractTarget(val))
        // the overridden implementation claims the target is abstract
        return VT_ABSTRACT;

    // just return the native code we track in BaseValue
    return code;
}

bool isUninitialized(EValueOrigin code) {
    switch (code) {
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

bool isAnyDataArea(EValueTarget code) {
    return isPossibleToDeref(code)
        || (VT_RANGE == code);
}

TValId SymHeapCore::valRoot(TValId val) const {
    if (val <= 0)
        return val;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->valRoot;
}

TOffset SymHeapCore::valOffset(TValId val) const {
    if (val <= 0)
        return 0;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    const EValueTarget code = valData->code;
    switch (code) {
        case VT_RANGE:
            CL_BREAK_IF("valOffset() called on VT_RANGE, which is not supported");
            // fall through!

        case VT_CUSTOM:
            // FIXME: valData->offRoot is used internally for unrelated purposes
            return 0;

        default:
            return valData->offRoot;
    }
}

IR::Range SymHeapCore::valOffsetRange(TValId val) const {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    if (VT_RANGE != valData->code)
        // this is going to be a singular range
        return IR::rngFromNum(valData->offRoot);

    const TValId anchor = valData->anchor;
    if (anchor == val) {
        // we got the VT_RANGE anchor directly
        const RangeValue *rangeData = DCAST<const RangeValue *>(valData);
        return rangeData->range;
    }

    // we need to resolve an off-value to VT_RANGE anchor
    const RangeValue *rangeData;
    d->ents.getEntRO(&rangeData, anchor);

    // check the offset we need to shift the anchor by
    const TOffset off = valData->offRoot;
    CL_BREAK_IF(!off);

    // shift the range (if not already saturated) and return the result
    IR::Range range = rangeData->range;
    range += IR::rngFromNum(off);

    if (isAligned(range)) {
        CL_BREAK_IF("TODO: deal better with alignment");
        range.alignment = IR::Int1;
    }

    return range;
}

void SymHeapCore::valReplace(TValId val, TValId replaceBy) {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    // kill all related Neq predicates
    TValList neqs;
    d->neqDb->gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValId valNeq, neqs) {
        CL_BREAK_IF(valNeq == replaceBy);
        SymHeapCore::neqOp(NEQ_DEL, valNeq, val);
    }

    // we intentionally do not use a reference here (tight loop otherwise)
    TObjIdSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TObjId obj, usedBy) {
        // this used to happen with with test-0037 running in OOM mode [fixed]
        CL_BREAK_IF(isGone(this->valTarget(this->placedAt(obj))));

        this->objSetValue(obj, replaceBy);
    }
}

void SymHeapCore::neqOp(ENeqOp op, TValId v1, TValId v2) {
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->neqDb);

    const EValueTarget code1 = this->valTarget(v1);
    const EValueTarget code2 = this->valTarget(v2);

    if (VT_UNKNOWN != code1 && VT_UNKNOWN != code2) {
        CL_BREAK_IF(NEQ_ADD != op);
        CL_DEBUG("SymHeap::neqOp() refuses to add an extraordinary Neq predicate");
        return;
    }

    switch (op) {
        case NEQ_NOP:
            CL_BREAK_IF("invalid call of SymHeapCore::neqOp()");
            return;

        case NEQ_ADD:
            d->neqDb->add(v1, v2);
            return;

        case NEQ_DEL:
            d->neqDb->del(v1, v2);
            return;
    }
}

void SymHeapCore::gatherRelatedValues(TValList &dst, TValId val) const {
    d->neqDb->gatherRelatedValues(dst, val);
    d->coinDb->gatherRelatedValues(dst, val);
}

void SymHeapCore::copyRelevantPreds(SymHeapCore &dst, const TValMap &valMap)
    const
{
    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb->cont_) {
        TValId valLt = item.first;
        TValId valGt = item.second;

        if (!translateValId(&valLt, dst, *this, valMap))
            // not relevant
            continue;

        if (!translateValId(&valGt, dst, *this, valMap))
            // not relevant
            continue;

        // create the image now!
        dst.neqOp(NEQ_ADD, valLt, valGt);
    }

    // go through CoincidenceDb
    const CoincidenceDb &coinDb = *d->coinDb;
    BOOST_FOREACH(CoincidenceDb::const_reference &ref, coinDb) {
        TValId valLt = ref/* key */.first/* lt */.first;
        TValId valGt = ref/* key */.first/* gt */.second;

        if (!translateValId(&valLt, dst, *this, valMap))
            // not relevant
            continue;

        if (!translateValId(&valGt, dst, *this, valMap))
            // not relevant
            continue;

        // create the image now!
        RefCntLib<RCO_NON_VIRT>::requireExclusivity(dst.d->coinDb);
        dst.d->coinDb->add(valLt, valGt, /* sum */ ref.second);
    }
}

bool SymHeapCore::matchPreds(const SymHeapCore &ref, const TValMap &valMap)
    const
{
    SymHeapCore &src = const_cast<SymHeapCore &>(*this);
    SymHeapCore &dst = const_cast<SymHeapCore &>(ref);

    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb->cont_) {
        TValId valLt = item.first;
        TValId valGt = item.second;

        if (!translateValId(&valLt, dst, src, valMap))
            // failed to translate value ID, better to give up
            return false;

        if (!translateValId(&valGt, dst, src, valMap))
            // failed to translate value ID, better to give up
            return false;

        if (!ref.d->neqDb->chk(valLt, valGt))
            // Neq predicate not matched
            return false;
    }

    // go through CoincidenceDb
    const CoincidenceDb &coinDb = *d->coinDb;
    BOOST_FOREACH(CoincidenceDb::const_reference &ref, coinDb) {
        TValId valLt = ref/* key */.first/* lt */.first;
        TValId valGt = ref/* key */.first/* gt */.second;

        if (!translateValId(&valLt, dst, *this, valMap))
            // failed to translate value ID, better to give up
            return false;

        if (!translateValId(&valGt, dst, *this, valMap))
            // failed to translate value ID, better to give up
            return false;

        TValId sum;
        if (!dst.d->coinDb->chk(&sum, valLt, valGt))
            // coincidence not matched
            return false;

        SymHeapCore &writable = *const_cast<SymHeapCore *>(this);
        if (!translateValId(&sum, writable, dst, valMap))
            // failed to translate value ID, better to give up
            return false;

        if (sum != /* sum */ ref.second)
            // target value ID not matched
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
    const TValId root = objData->root;

    // then subtract the offset
    return this->valByOffset(root, objData->off);
}

TObjId SymHeapCore::ptrAt(TValId at) {
    if (at <= 0)
        return OBJ_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

    const EValueTarget code = valData->code;
    CL_BREAK_IF(VT_RANGE == code);
    if (!isPossibleToDeref(code))
        return OBJ_INVALID;

    // jump to root
    const TValId valRoot = valData->valRoot;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);

    // generic pointer, (void *) if available
    const TObjType clt = stor_.types.genericDataPtr();
    if (!clt || clt->code != CL_TYPE_PTR) {
        CL_BREAK_IF("Code Listener failed to capture a type of data pointer");
        return OBJ_INVALID;
    }

    const TSizeOf size = clt->size;
    CL_BREAK_IF(size <= 0);

    // arena lookup
    TObjIdSet candidates;
    const TArena &arena = rootData->arena;
    const TOffset off = valData->offRoot;
    const TMemChunk chunk(off, off + size);
    arenaLookForExactMatch(&candidates, arena, chunk);

    // seek a _data_ pointer in the given interval
    BOOST_FOREACH(const TObjId obj, candidates) {
        const BlockEntity *blData;
        d->ents.getEntRO(&blData, obj);
        const EBlockKind code = blData->code;
        if (BK_DATA_PTR != code && BK_DATA_OBJ != code)
            continue;

        const HeapObject *objData = DCAST<const HeapObject *>(blData);
        const TObjType clt = objData->clt;
        if (isDataPtr(clt))
            return obj;
    }

    // check whether we have enough space allocated for the pointer
    const TSizeRange avail = this->valSizeOfTarget(at);
    if (avail.lo < clt->size) {
        CL_BREAK_IF("ptrAt() called out of bounds");
        return OBJ_UNKNOWN;
    }

    // resolve root
    const TValId root = valData->valRoot;

    // create the pointer
    return d->objCreate(root, off, clt);
}

// TODO: simplify the code
TObjId SymHeapCore::objAt(TValId at, TObjType clt) {
    if (at <= 0)
        return OBJ_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

    const EValueTarget code = valData->code;
    CL_BREAK_IF(VT_RANGE == code);
    if (!isPossibleToDeref(code))
        return OBJ_INVALID;

    CL_BREAK_IF(!clt || !clt->size);
    const TSizeOf size = clt->size;

    // jump to root
    const TValId valRoot = valData->valRoot;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);

    // arena lookup
    TObjIdSet candidates;
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
        const BlockEntity *blData;
        d->ents.getEntRO(&blData, obj);
        const EBlockKind code = blData->code;
        switch (code) {
            case BK_DATA_PTR:
            case BK_DATA_OBJ:
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
                return obj;

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
        return bestMatch;

    const TSizeRange avail = this->valSizeOfTarget(at);
    if (avail.lo < clt->size)
        // out of bounds
        return OBJ_UNKNOWN;

    // create the object
    const TValId root = valData->valRoot;
    return d->objCreate(root, off, clt);
}

void SymHeapCore::objEnter(TObjId obj) {
    HeapObject *objData;
    d->ents.getEntRW(&objData, obj);
    CL_BREAK_IF(objData->extRefCnt < 0);
    ++(objData->extRefCnt);
}

void SymHeapCore::objLeave(TObjId obj) {
    HeapObject *objData;
    d->ents.getEntRW(&objData, obj);
    CL_BREAK_IF(objData->extRefCnt < 1);
    if (--(objData->extRefCnt))
        // still externally referenced
        return;

#if SH_DELAYED_OBJECTS_DESTRUCTION
    return;
#endif

    if (isComposite(objData->clt, /* includingArray */ false)
            && VAL_INVALID != objData->value)
    {
        CL_DEBUG("SymHeapCore::objLeave() preserves a composite object");
        return;
    }

    const TValId root = objData->root;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    if (!hasKey(rootData->liveObjs, obj)) {
        CL_DEBUG("SymHeapCore::objLeave() destroys a dead object");
        d->objDestroy(obj, /* removeVal */ true, /* detach */ true);
    }

    // TODO: pack the representation if possible
}

CVar SymHeapCore::cVarByRoot(TValId valRoot) const {
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, valRoot);
    return rootData->cVar;
}

TValId SymHeapCore::addrOfVar(CVar cv, bool createIfNeeded) {
    TValId addr = d->cVarMap->find(cv);
    if (0 < addr)
        return addr;

    if (!createIfNeeded)
        // the variable does not exist and we are not asked to create the var
        return VAL_INVALID;

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

    RootValue *rootData;
    d->ents.getEntRW(&rootData, addr);
    rootData->cVar = cv;
    rootData->lastKnownClt = clt;

    // read size from the type-info
    const TSizeOf size = clt->size;
    rootData->size = IR::rngFromNum(size);

    // mark the root as live
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveRoots);
    d->liveRoots->insert(addr);

    // store the address for next wheel
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->cVarMap);
    d->cVarMap->insert(cv, addr);
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

    const TValSetWrapper &roots = *d->liveRoots;
    BOOST_FOREACH(const TValId at, roots)
        if (filter(this->valTarget(at)))
            dst.push_back(at);
}

TObjId SymHeapCore::valGetComposite(TValId val) const {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    CL_BREAK_IF(VT_COMPOSITE != valData->code);

    const CompValue *compData = DCAST<const CompValue *>(valData);
    return compData->compObj;
}

TValId SymHeapCore::heapAlloc(const TSizeRange &size) {
    CL_BREAK_IF(size.lo <= IR::Int0);

    // assign an address
    const TValId addr = d->valCreate(VT_ON_HEAP, VO_ASSIGNED);

    // mark the root as live
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveRoots);
    d->liveRoots->insert(addr);

    // initialize meta-data
    RootValue *rootData;
    d->ents.getEntRW(&rootData, addr);
    rootData->size = size;

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

TSizeRange SymHeapCore::valSizeOfTarget(TValId val) const {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    if (!isPossibleToDeref(valData->code))
        // no writable target around here
        return IR::rngFromNum(IR::Int0);

    if (valData->offRoot < 0)
        // we are above the root, so we cannot safely write anything
        return IR::rngFromNum(IR::Int0);

    const EValueTarget code = valData->code;
    if (isGone(code))
        return IR::rngFromNum(IR::Int0);

    CL_BREAK_IF(!isPossibleToDeref(valData->code));
    const TValId root = valData->valRoot;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);

    IR::Range size = rootData->size;
    size -= IR::rngFromNum(/* off */ valData->offRoot);
    return size;
}

TSizeRange SymHeapCore::valSizeOfString(TValId addr) const {
    const BaseValue *valData;
    d->ents.getEntRO(&valData, addr);

    const EValueTarget code = valData->code;
    if (VT_CUSTOM == code) {
        const InternalCustomValue *customData =
            DCAST<const InternalCustomValue *>(valData);

        const CustomValue &cv = customData->customData;
        if (CV_STRING != cv.code())
            return /* error */ IR::rngFromNum(IR::Int0);

        // string literal
        const unsigned len = cv.str().size() + /* trailing zero */ 1;
        return IR::rngFromNum(len);
    }

    if (!isPossibleToDeref(code))
        return /* error */ IR::rngFromNum(IR::Int0);

    // resolve root and offset
    const TValId root = valData->valRoot;
    const TOffset off = valData->offRoot;

    // seek the first zero byte at the given offset
    IR::Range rng;
    rng.alignment = /* FIXME: this needs a better API */ IR::Int1;
    if (!d->findZeroAtOff(&rng, off, root))
        // possibly unterminated string
        return /* error */ IR::rngFromNum(IR::Int0);

    // Private::findZeroAtOff() returns an absolute offset, but we need relative
    CL_BREAK_IF(rng.lo < off);
    rng += /* trailing zero */ IR::rngFromNum(IR::Int1 - off);
    return rng;
}

void SymHeapCore::valSetLastKnownTypeOfTarget(TValId root, TObjType clt) {
    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);

    if (VAL_ADDR_OF_RET == root) {
        // destroy any stale target of VAL_ADDR_OF_RET
        d->destroyRoot(root);

        // allocate a new root value at VAL_ADDR_OF_RET
        rootData->code = VT_ON_STACK;
        rootData->size = IR::rngFromNum(clt->size);
    }

    // convert a type-free object into a type-aware object
    rootData->lastKnownClt = clt;
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
        RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->cVarMap);
        this->cVarMap->remove(cv);
        code = VT_LOST;
    }

    // start with the root itself as anchor
    std::vector<AnchorValue *> refs(1, rootData);

    // collect all VT_RANGE anchors
    BOOST_FOREACH(const TValId rVal, rootData->dependentValues) {
        AnchorValue *anchorData;
        this->ents.getEntRW(&anchorData, rVal);
        refs.push_back(anchorData);
    }

    BOOST_FOREACH(AnchorValue *anchorData, refs) {
        // mark the anchor value as deleted/lost
        anchorData->code = code;

        // mark all associated off-values as deleted/lost
        BOOST_FOREACH(TOffMap::const_reference item, anchorData->offMap) {
            const TValId val = item.second;
            BaseValue *valData;
            this->ents.getEntRW(&valData, val);
            valData->code = code;
        }
    }

    // release the root
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(this->liveRoots);
    this->liveRoots->erase(root);

    const TSizeRange size = rootData->size;
    if (IR::Int0 < size.hi) {
        // look for inner objects
        const TMemChunk chunk(0, size.hi);
        TObjIdSet allObjs;
        if (arenaLookup(&allObjs, rootData->arena, chunk, OBJ_INVALID)) {
            // destroy all inner objects
            BOOST_FOREACH(const TObjId obj, allObjs)
                this->objDestroy(obj, /* removeVal */ true, /* detach */ false);
        }
    }

    // wipe rootData
    rootData->size = IR::rngFromNum(IR::Int0);
    rootData->lastKnownClt = 0;
    rootData->liveObjs.clear();
    rootData->arena.clear();
}

TValId SymHeapCore::valCreate(EValueTarget code, EValueOrigin origin) {
    switch (code) {
        case VT_UNKNOWN:
            // this is the most common case

        case VT_DELETED:
        case VT_LOST:
            // these are used by symcut
            break;

        default:
            CL_BREAK_IF("invalid call of SymHeapCore::valCreate()");

            // just to avoid an unnecessary SIGSEGV in the production build
            code = VT_UNKNOWN;
    }

    return d->valCreate(code, origin);
}

TValId SymHeapCore::valWrapCustom(CustomValue cVal) {
    const ECustomValue code = cVal.code();

    if (CV_INT_RANGE == code) {
        const IR::Range &rng = cVal.rng();
        if (isSingular(rng))
            // recycle scalar values
            return d->wrapIntVal(rng.lo);

        // CV_INT_RANGE with a valid range (do not recycle these)
        const TValId val = d->valCreate(VT_CUSTOM, VO_ASSIGNED);
        InternalCustomValue *valData;
        d->ents.getEntRW(&valData, val);
        valData->customData = cVal;
        return val;
    }

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->cValueMap);
    TValId &val = d->cValueMap->lookup(cVal);
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

    const CustomValue &cv = valData->customData;
    const ECustomValue code = cv.code();

    if (CV_INT_RANGE == code) {
        const IR::Range &rng = cv.rng();
        if (!isSingular(rng))
            return cv;

        // VAL_FALSE/VAL_TRUE are not supposed to be wrapped as custom values
        CL_BREAK_IF(IR::Int0 == rng.lo || IR::Int1 == rng.lo);
    }

    // check the consistency of backward mapping
    CL_BREAK_IF(val != d->cValueMap->lookup(valData->customData));

    return cv;
}

TProtoLevel SymHeapCore::valTargetProtoLevel(TValId val) const {
    if (val <= 0)
        // not a prototype for sure
        return 0;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    if (!isPossibleToDeref(valData->code))
        // not a prototype for sure
        return 0;

    // seek root
    const TValId root = valData->valRoot;
    const RootValue *rootData;
    d->ents.getEntRO(&rootData, root);
    return rootData->protoLevel;
}

void SymHeapCore::valTargetSetProtoLevel(TValId root, TProtoLevel level) {
    CL_BREAK_IF(!isPossibleToDeref(this->valTarget(root)));
    CL_BREAK_IF(this->valOffset(root));
    CL_BREAK_IF(level < 0);

    RootValue *rootData;
    d->ents.getEntRW(&rootData, root);
    rootData->protoLevel = level;
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
    // NOTE: this is only an optimization to avoid calling rngFromVal() twice
    if (VAL_TRUE == valA)
        return (VAL_FALSE == valB);

    // we presume (0 <= valA) and (0 < valB) at this point
    CL_BREAK_IF(d->ents.outOfRange(valB));

    const EValueTarget code = this->valTarget(valB);
    if (VAL_NULL == valA
            && (isKnownObject(code) || isGone(code) || VT_RANGE == code))
        // all addresses of objects have to be non-zero
        return true;

    if (valInsideSafeRange(*this, valA) && valInsideSafeRange(*this, valB))
        // NOTE: we know (valA != valB) at this point, look above
        return true;

    IR::Range rng1, rng2;
    if (rngFromVal(&rng1, *this, valA) && rngFromVal(&rng2, *this, valB)) {
        // both values are integral ranges (
        bool result;
        return (compareIntRanges(&result, CL_BINOP_NE, rng1, rng2) && result);
    }

    // check for a Neq predicate
    if (d->neqDb->chk(valA, valB))
        return true;

    if (valA <= 0 || valB <= 0)
        // no handling of special values here
        return false;

    const TValId root1 = this->valRoot(valA);
    const TValId root2 = this->valRoot(valB);
    if (root1 == root2) {
        // same root, different offsets
        CL_BREAK_IF(matchOffsets(*this, *this, valA, valB));
        return true;
    }

    const TOffset offA = this->valOffset(valA);
    const TOffset offB = this->valOffset(valB);

    const TOffset diff = offB - offA;
    if (!diff)
        // check for Neq between the roots
        return d->neqDb->chk(root1, root2);

    SymHeapCore &writable = /* XXX */ *const_cast<SymHeapCore *>(this);
    return d->neqDb->chk(root1, writable.valByOffset(root2,  diff))
        && d->neqDb->chk(root2, writable.valByOffset(root1, -diff));
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap
struct AbstractRoot {
    RefCounter                      refCnt;

    EObjKind                        kind;
    BindingOff                      bOff;
    TMinLen                         minLength;

    AbstractRoot(EObjKind kind_, BindingOff bOff_):
        kind(kind_),
        bOff(bOff_),
        minLength(0)
    {
    }

    AbstractRoot* clone() const {
        return new AbstractRoot(*this);
    }
};

struct SymHeap::Private {
    RefCounter                      refCnt;
    EntStore<AbstractRoot>          absRoots;
};

SymHeap::SymHeap(TStorRef stor, Trace::Node *trace):
    SymHeapCore(stor, trace),
    d(new Private)
{
}

SymHeap::SymHeap(const SymHeap &ref):
    SymHeapCore(ref),
    d(ref.d)
{
    RefCntLib<RCO_NON_VIRT>::enter(d);
}

SymHeap::~SymHeap() {
    RefCntLib<RCO_NON_VIRT>::leave(d);
}

SymHeap& SymHeap::operator=(const SymHeap &ref) {
    SymHeapCore::operator=(ref);

    RefCntLib<RCO_NON_VIRT>::leave(d);

    d = ref.d;
    RefCntLib<RCO_NON_VIRT>::enter(d);

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
    if (dup <= 0 || VT_RANGE == this->valTarget(val))
        return dup;

    const TValId valRoot = this->valRoot(val);
    if (!d->absRoots.isValidEnt(valRoot))
        return dup;

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // clone the data
    const AbstractRoot *tplData = d->absRoots.getEntRO(valRoot);
    const TValId dupRoot = this->valRoot(dup);
    AbstractRoot *dupData = tplData->clone();
    d->absRoots.assignId(dupRoot, dupData);

    return dup;
}

EObjKind SymHeap::valTargetKind(TValId val) const {
    if (val <= 0)
        return OK_CONCRETE;

    const TValId valRoot = this->valRoot(val);
    if (!d->absRoots.isValidEnt(valRoot))
        return OK_CONCRETE;

    const AbstractRoot *aData = d->absRoots.getEntRO(valRoot);
    return aData->kind;
}

bool SymHeap::hasAbstractTarget(TValId val) const {
    return (OK_CONCRETE != this->valTargetKind(val));
}

const BindingOff& SymHeap::segBinding(TValId root) const {
    CL_BREAK_IF(this->valOffset(root));
    CL_BREAK_IF(!this->hasAbstractTarget(root));
    CL_BREAK_IF(!d->absRoots.isValidEnt(root));

    const AbstractRoot *aData = d->absRoots.getEntRO(root);
    CL_BREAK_IF(OK_OBJ_OR_NULL == aData->kind);
    return aData->bOff;
}

void SymHeap::valTargetSetAbstract(
        TValId                      root,
        EObjKind                    kind,
        const BindingOff            &off)
{
    CL_BREAK_IF(!isPossibleToDeref(this->valTarget(root)));
    CL_BREAK_IF(this->valOffset(root));
    CL_BREAK_IF(OK_CONCRETE == kind);

    // there is no 'prev' offset in OK_SEE_THROUGH
    CL_BREAK_IF(OK_SEE_THROUGH == kind && off.prev != off.next);

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // clone the data
    if (d->absRoots.isValidEnt(root)) {
        CL_BREAK_IF(OK_SLS != kind);

        AbstractRoot *aData = d->absRoots.getEntRW(root);
        CL_BREAK_IF(OK_SEE_THROUGH != aData->kind || off != aData->bOff);

        // OK_SEE_THROUGH -> OK_SLS
        aData->kind = kind;
        return;
    }

    AbstractRoot *aData = new AbstractRoot(kind, (OK_OBJ_OR_NULL == kind)
            ? BindingOff(OK_OBJ_OR_NULL)
            : off);

    // register a new abstract root
    d->absRoots.assignId(root, aData);
}

void SymHeap::valTargetSetConcrete(TValId root) {
    CL_DEBUG("SymHeap::valTargetSetConcrete() is taking place...");
    CL_BREAK_IF(!isPossibleToDeref(this->valTarget(root)));
    CL_BREAK_IF(this->valOffset(root));
    CL_BREAK_IF(!d->absRoots.isValidEnt(root));

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // unregister an abstract object
    // FIXME: suboptimal code of EntStore::releaseEnt() with SH_REUSE_FREE_IDS
    d->absRoots.releaseEnt(root);
}

void SymHeap::segMinLengthOp(ENeqOp op, TValId at, TMinLen len) {
    CL_BREAK_IF(!len);

    if (NEQ_DEL == op) {
        this->segSetMinLength(at, len - 1);
        return;
    }

    CL_BREAK_IF(NEQ_ADD != op);
    const TMinLen current = this->segMinLength(at);
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
        *pDst = sh->valRoot(v1);
        return true;
    }

    if (haveSeg(*sh, v2, v1, kind)) {
        *pDst = sh->valRoot(v2);
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
    if (haveSegBidir(&seg, this, OK_OBJ_OR_NULL, v1, v2)
            || haveSegBidir(&seg, this, OK_SEE_THROUGH, v1, v2)
            || haveSegBidir(&seg, this, OK_SEE_THROUGH_2N, v1, v2))
    {
        // replace OK_SEE_THROUGH/OK_OBJ_OR_NULL by OK_CONCRETE
        decrementProtoLevel(*this, seg);
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

TValId lookThrough(TValSet *pSeen, SymHeap &sh, TValId val) {
    if (VT_RANGE == sh.valTarget(val))
        // not supported yet
        return VAL_INVALID;

    const TOffset off = sh.valOffset(val);

    while (0 < val) {
        const TValId root = sh.valRoot(val);
        if (!insertOnce(*pSeen, root))
            // an already seen root value
            return VAL_INVALID;

        const EValueTarget code = sh.valTarget(val);
        if (!isAbstract(code))
            // a non-abstract object reached
            break;

        const TValId seg = segPeer(sh, root);
        if (sh.segMinLength(seg))
            // non-empty abstract object reached
            break;

        const EObjKind kind = sh.valTargetKind(seg);
        if (OK_OBJ_OR_NULL == kind) {
            // we always end up with VAL_NULL if OK_OBJ_OR_NULL is removed
            val = VAL_NULL;
            continue;
        }

        // jump to next value while taking the 'head' offset into consideration
        const TValId valNext = nextValFromSeg(sh, seg);
        const BindingOff &bOff = sh.segBinding(seg);
        val = sh.valByOffset(valNext, off - bOff.head);
    }

    return val;
}

bool SymHeap::proveNeq(TValId ref, TValId val) const {
    if (SymHeapCore::proveNeq(ref, val))
        // values are non-equal in non-abstract world
        return true;

    TValSet seen;

    // try to look through possibly empty abstract objects
    val = lookThrough(&seen, *const_cast<SymHeap *>(this), val);
    if (VAL_INVALID == val)
        return false;

    // try to look through possibly empty abstract objects
    ref = lookThrough(&seen, *const_cast<SymHeap *>(this), ref);
    if (VAL_INVALID == ref)
        return false;

    if (SymHeapCore::proveNeq(ref, val))
        // values are non-equal in non-abstract world
        return true;

    // having the values always in the same order leads to simpler code
    moveKnownValueToLeft(*this, ref, val);

    const TSizeRange size2 = this->valSizeOfTarget(val);
    if (size2.lo <= IR::Int0)
        // oops, we cannot prove the address is safely allocated, giving up
        return false;

    const TValId root2 = this->valRoot(val);
    const TMinLen len2 = objMinLength(*this, root2);
    if (!len2)
        // one of the targets is possibly empty, giving up
        return false;

    if (VAL_NULL == ref)
        // one of them is VAL_NULL the other one is address of non-empty object
        return true;

    const TSizeRange size1 = this->valSizeOfTarget(ref);
    if (size1.lo <= IR::Int0)
        // oops, we cannot prove the address is safely allocated, giving up
        return false;

    const TValId root1 = this->valRoot(ref);
    const TMinLen len1 = objMinLength(*this, root1);
    if (!len1)
        // both targets are possibly empty, giving up
        return false;

    if (!isAbstract(this->valTarget(ref)))
        // non-empty abstract object vs. concrete object
        return true;

    if (root2 != segPeer(*this, root1))
        // a pair of non-empty abstract objects
        return true;

    // one value points at segment and the other points at its peer
    CL_BREAK_IF(len1 != len2);
    return (1 < len1);
}

void SymHeap::valDestroyTarget(TValId root) {
    SymHeapCore::valDestroyTarget(root);
    if (!d->absRoots.isValidEnt(root))
        return;

    CL_DEBUG("SymHeap::valDestroyTarget() destroys an abstract object");

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // unregister an abstract object
    // FIXME: suboptimal code of EntStore::releaseEnt() with SH_REUSE_FREE_IDS
    d->absRoots.releaseEnt(root);
}

TMinLen SymHeap::segMinLength(TValId seg) const {
    CL_BREAK_IF(this->valOffset(seg));
    CL_BREAK_IF(!d->absRoots.isValidEnt(seg));

    const AbstractRoot *aData = d->absRoots.getEntRO(seg);

    const EObjKind kind = aData->kind;
    if (isMayExistObj(kind))
        return 0;

    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            return aData->minLength;

        default:
            CL_BREAK_IF("invalid call of SymHeap::segMinLength()");
            return 0;
    }
}

void SymHeap::segSetMinLength(TValId seg, TMinLen len) {
    CL_BREAK_IF(this->valOffset(seg));
    CL_BREAK_IF(!d->absRoots.isValidEnt(seg));

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    AbstractRoot *aData = d->absRoots.getEntRW(seg);

    const EObjKind kind = aData->kind;
    if (isMayExistObj(kind)) {
        if (len)
            CL_BREAK_IF("invalid call of SymHeap::segSetMinLength()");

        return;
    }

    switch (kind) {
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

    aData->minLength = len;
    if (OK_DLS != kind)
        return;

    const TValId peer = dlSegPeer(*this, seg);
    if (!d->absRoots.isValidEnt(peer)) {
        CL_BREAK_IF("SymHeap::segMinLength() got an inconsistent DLS");
        return;
    }

    CL_BREAK_IF(peer == seg);

    AbstractRoot *peerData = d->absRoots.getEntRW(peer);
    peerData->minLength = len;
}
