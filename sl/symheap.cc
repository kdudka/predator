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
#include "syments.hh"
#include "sympred.hh"
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

void enableProtectedMode(bool enable)
{
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
                const TValMap           &vMap,
                const bool              nonZeroOnly)
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
        typedef std::map<CVar, TObjId>              TCont;
        TCont                                       cont_;

    public:
        void insert(CVar cVar, TObjId val) {
            // check for mapping redefinition
            CL_BREAK_IF(hasKey(cont_, cVar));

            // define mapping
            cont_[cVar] = val;
        }

        void remove(CVar cVar) {
            if (1 != cont_.erase(cVar))
                CL_BREAK_IF("offset detected in CVarMap::remove()");
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
};


// /////////////////////////////////////////////////////////////////////////////
// implementation of CustomValue
CustomValue::~CustomValue()
{
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

CustomValue& CustomValue::operator=(const CustomValue &ref)
{
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

int CustomValue::uid() const
{
    CL_BREAK_IF(CV_FNC != code_);
    return data_.uid;
}

IR::Range& CustomValue::rng()
{
    CL_BREAK_IF(CV_INT_RANGE != code_);
    return data_.rng;
}

double CustomValue::fpn() const
{
    CL_BREAK_IF(CV_REAL != code_);
    return data_.fpn;
}

const std::string& CustomValue::str() const
{
    CL_BREAK_IF(CV_STRING != code_);
    CL_BREAK_IF(!data_.str);
    return *data_.str;
}

/// eliminates the warning 'comparing floating point with == or != is unsafe'
inline bool areEqual(const double a, const double b)
{
    return (a <= b)
        && (b <= a);
}

bool operator==(const CustomValue &a, const CustomValue &b)
{
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
typedef std::set<TObjId>                                TObjSet;
typedef std::set<TFldId>                                TFldIdSet;
typedef std::map<TOffset, TValId>                       TOffMap;
typedef IntervalArena<TOffset, TFldId>                  TArena;
typedef TArena::key_type                                TMemChunk;
typedef TArena::value_type                              TMemItem;
typedef std::map<CallInst, TObjList>                    TAnonStackMap;
typedef std::map<ETargetSpecifier, TValId>              TAddrByTS;

inline TMemItem createArenaItem(
        const TOffset               off,
        const TSizeOf               size,
        const TFldId                fld)
{
    const TMemChunk chunk(off, off + size);
    return TMemItem(chunk, fld);
}

inline bool arenaLookup(
        TFldIdSet                   *dst,
        const TArena                &arena,
        const TMemChunk             &chunk,
        const TFldId                fld)
{
    CL_BREAK_IF(!dst->empty());
    arena.intersects(*dst, chunk);

    if (FLD_INVALID != fld)
        // remove the reference object itself
        dst->erase(fld);

    // finally check if there was anything else
    return !dst->empty();
}

inline void arenaLookForExactMatch(
        TFldIdSet                   *dst,
        const TArena                &arena,
        const TMemChunk             &chunk)
{
    arena.exactMatch(*dst, chunk);
}

// create a right-open interval
inline TMemChunk createChunk(const TOffset off, const TObjType clt)
{
    CL_BREAK_IF(!clt || clt->code == CL_TYPE_VOID);
    return TMemChunk(off, off + clt->size);
}

enum EBlockKind {
    BK_INVALID,
    BK_FIELD,
    BK_COMPOSITE,
    BK_UNIFORM
};

typedef std::map<TFldId, EBlockKind>                    TLiveObjs;

inline EBlockKind bkFromClt(const TObjType clt)
{
    return (isComposite(clt, /* includingArray */ false))
        ? BK_COMPOSITE
        : BK_FIELD;
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
    TObjId                      obj;
    TOffset                     off;
    TSizeOf                     size;
    TValId                      value;

    BlockEntity(
            const EBlockKind        code_,
            const TObjId            obj_,
            const TOffset           off_,
            const TSizeOf           size_,
            const TValId            value_):
        code(code_),
        obj(obj_),
        off(off_),
        size(size_),
        value(value_)
    {
    }

    virtual BlockEntity* clone() const {
        return new BlockEntity(*this);
    }
};

struct FieldOfObj: public BlockEntity {
    TObjType                    clt;
    int                         extRefCnt;

    FieldOfObj(TObjId obj_, TOffset off_, TObjType clt_):
        BlockEntity(bkFromClt(clt_), obj_, off_, clt_->size, VAL_INVALID),
        clt(clt_),
        extRefCnt(0)
    {
    }

    virtual FieldOfObj* clone() const {
        return new FieldOfObj(*this);
    }
};

struct BaseValue: public AbstractHeapEntity {
    EValueTarget                    code;
    EValueOrigin                    origin;
    TValId                          valRoot;
    TValId                          anchor;
    TOffset /* FIXME: misleading */ offRoot;
    TFldIdSet                       usedBy;

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
    TFldId                          compObj;

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

struct Region: public AbstractHeapEntity {
    EStorageClass                   code;
    CVar                            cVar;
    TSizeRange                      size;
    TLiveObjs                       liveFields;
    TFldIdSet                       usedByGl;
    TArena                          arena;
    TObjType                        lastKnownClt;
    bool                            isValid;
    TProtoLevel                     protoLevel;
    TAddrByTS                       addrByTS;

    Region(EStorageClass code_):
        code(code_),
        size(IR::rngFromNum(0)),
        lastKnownClt(0),
        isValid(true),
        protoLevel(/* not a prototype */ 0)
    {
    }

    virtual Region* clone() const {
        return new Region(*this);
    }
};

struct BaseAddress: public AnchorValue {
    TObjId                          obj;
    ETargetSpecifier                ts;

    BaseAddress(const TObjId obj_, const ETargetSpecifier ts_):
        AnchorValue(VT_OBJECT, VO_ASSIGNED),
        obj(obj_),
        ts(ts_)
    {
    }

    virtual BaseAddress* clone() const {
        return new BaseAddress(*this);
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
struct TObjSetWrapper: public TObjSet {
    RefCounter refCnt;
};

// FIXME: std::map is not a good candidate for base class
struct TAnonStackMapWrapper: public TAnonStackMap {
    RefCounter refCnt;
};

/// @note there is no virtual destructor for now (may be needed in the future)
class IMatchPolicy {
    public:
        /// return true if we should keep searching
        virtual bool matchBlock(TFldId, const FieldOfObj *data, bool alive) = 0;

        /// return the field ID which matches the best, FLD_INVALID for no match
        virtual TFldId bestMatch() const = 0;
};

struct SymHeapCore::Private {
    Private(Trace::Node *);
    Private(const Private &);
    ~Private();

    Trace::NodeHandle               traceHandle;
    EntStore<AbstractHeapEntity>    ents;
    TObjSetWrapper                 *liveObjs;
    TAnonStackMapWrapper           *anonStackMap;
    CVarMap                        *cVarMap;
    CustomValueMapper              *cValueMap;
    CoincidenceDb                  *coinDb;
    NeqDb                          *neqDb;

    inline TFldId assignId(BlockEntity *);
    inline TValId assignId(BaseValue *);
    inline TObjId assignId(Region *);

    TValId valCreate(EValueTarget code, EValueOrigin origin);
    TValId valDup(TValId);
    bool valsEqual(TValId, TValId);

    TFldId fldCreate(TObjId obj, TOffset off, TObjType clt);
    TValId fldInit(TFldId fld);
    void fldDestroy(TFldId, bool removeVal, bool detach);

    TFldId copySingleLiveBlock(
            const TObjId            objDst,
            Region                 *objDataDst,
            const TFldId            fldSrc,
            const EBlockKind        code,
            const TOffset           shift = 0,
            const TSizeOf           sizeLimit = 0);

    bool /* wasPtr */ releaseValueOf(TFldId fld, TValId val);
    void registerValueOf(TFldId fld, TValId val);
    void splitBlockByObject(TFldId block, TFldId fld);
    bool writeCharToString(TValId *pValDst, const TValId, const TOffset);
    bool reinterpretSingleObj(FieldOfObj *dstData, const BlockEntity *srcData);
    void reinterpretObjData(TFldId old, TFldId fld, TValSet *killedPtrs = 0);
    void setValueOf(TFldId of, TValId val, TValSet *killedPtrs = 0);

    // runs only in debug build
    bool chkValueDeps(const TValId);

    // runs only in debug build
    bool chkArenaConsistency(
            const Region           *rootData,
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

    TFldId writeUniformBlock(
            const TObjId            obj,
            const UniformBlock     &ub,
            TValSet                *killedPtrs);

    bool findZeroInBlock(
            TOffset                *offDst,
            bool                   *provenPrefix,
            const TOffset           offSrc,
            const                   TFldId fld);

    bool findZeroAtOff(
            IR::Range              *offDst,
            const TOffset           offSrc,
            const TValId            root);

    void bindValues(TValId v1, TValId v2, TValId valSum);

    TValId shiftCustomValue(TValId val, TOffset shift);

    TValId wrapIntVal(const IR::TInt);

    void replaceRngByInt(const InternalCustomValue *valData);

    void trimCustomValue(TValId val, const IR::Range &win);

    TFldId fldLookup(
            const TObjId            obj,
            const TOffset           off,
            const TObjType          clt,
            IMatchPolicy           *policy);

    private:
        // intentionally not implemented
        Private& operator=(const Private &);
};

inline TValId SymHeapCore::Private::assignId(BaseValue *valData)
{
    const TValId val = this->ents.assignId<TValId>(valData);
    valData->valRoot = val;
    valData->anchor  = val;
    return val;
}

inline TFldId SymHeapCore::Private::assignId(BlockEntity *hbData)
{
    return this->ents.assignId<TFldId>(hbData);
}

inline TObjId SymHeapCore::Private::assignId(Region *regData)
{
    return this->ents.assignId<TObjId>(regData);
}

bool /* wasPtr */ SymHeapCore::Private::releaseValueOf(TFldId fld, TValId val)
{
    if (val <= 0)
        // we do not track uses of special values
        return /* wasPtr */ false;

    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    TFldIdSet &usedBy = valData->usedBy;
    if (1 != usedBy.erase(fld))
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
    const BaseAddress *rootData;
    this->ents.getEntRO(&rootData, root);

    // jump to region
    Region *regData;
    this->ents.getEntRW(&regData, rootData->obj);

    if (1 != regData->usedByGl.erase(fld))
        CL_BREAK_IF("SymHeapCore::Private::releaseValueOf(): offset detected");

    return /* wasPtr */ true;
}

void SymHeapCore::Private::registerValueOf(TFldId fld, TValId val)
{
    if (val <= 0)
        return;

    // update usedBy
    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    valData->usedBy.insert(fld);

    const EValueTarget code = valData->code;
    if (!isAnyDataArea(code))
        return;

    // jump to root
    const TValId root = valData->valRoot;
    const BaseAddress *rootData;
    this->ents.getEntRO(&rootData, root);

    // update usedByGl
    Region *regData;
    this->ents.getEntRW(&regData, rootData->obj);
    regData->usedByGl.insert(fld);
}

// runs only in debug build
bool SymHeapCore::Private::chkValueDeps(const TValId val)
{
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
        const Region               *rootData,
        const bool                  allowOverlap)
{
    if (::bypassSelfChecks)
        return true;

    if (!rootData->isValid)
        // invalid objects ... just check there are no outgoing has-value edges
        return rootData->liveFields.empty();

    std::set<TOffset> offs;
    BOOST_FOREACH(TLiveObjs::const_reference item, rootData->liveFields) {
        const BlockEntity *blData;
        this->ents.getEntRO(&blData, /* fld */ item.first);
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

    TLiveObjs all(rootData->liveFields);
    const TArena &arena = rootData->arena;
    const TMemChunk chunk(0, rootData->size.hi);

    TFldIdSet overlaps;
    if (arenaLookup(&overlaps, arena, chunk, FLD_INVALID)) {
        BOOST_FOREACH(const TFldId fld, overlaps)
            all.erase(fld);
    }

    if (all.empty())
        return true;

    CL_WARN("live object not mapped in arena: #" << all.begin()->first);
    return false;
}

void SymHeapCore::Private::splitBlockByObject(
        TFldId                      block,
        TFldId                      fld)
{
    BlockEntity *blData;
    this->ents.getEntRW(&blData, block);

    const BlockEntity *hbData;
    this->ents.getEntRO(&hbData, fld);

    const EBlockKind code = hbData->code;
    switch (code) {
        case BK_FIELD:
            if (this->valsEqual(blData->value, hbData->value))
                // preserve non-conflicting uniform blocks
                return;

        default:
            break;
    }

    // resolve object
    const TObjId obj = blData->obj;
    CL_BREAK_IF(obj != hbData->obj);
    Region *rootData;
    this->ents.getEntRW(&rootData, obj);

    // check up to now arena consistency
    CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* mayOverlap */ true));

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
        if (!rootData->liveFields.erase(block))
            CL_BREAK_IF("attempt to kill an already dead uniform block");

        rootData->arena -= createArenaItem(blOff, blSize, block);
        this->ents.releaseEnt(block);
        return;
    }

    if (0 < blBegToObjBeg && 0 < objEndToBlEnd) {
        // the object is strictly in the middle of the block (needs split)
        BlockEntity *blDataOther = blData->clone();
        const TFldId blOther = this->assignId(blDataOther);

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

        rootData->liveFields[blOther] = BK_UNIFORM;
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
        const FieldOfObj           *fldData,
        const BlockEntity          *blData)
{
    const TOffset beg1 = fldData->off;
    const TOffset beg2 = blData->off;
    if (beg1 < beg2)
        // the object starts above the block
        return false;

    const TOffset end1 = beg1 + fldData->clt->size;
    const TOffset end2 = beg2 + blData->size;
    return (end1 <= end2);
}

inline bool isChar(const TObjType clt)
{
    return (CL_TYPE_INT == clt->code)
        && (1 == clt->size);
}

inline bool isString(const TObjType clt)
{
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
        FieldOfObj                 *dstData,
        const BlockEntity          *srcData)
{
    CL_BREAK_IF(srcData->obj != dstData->obj);

    const EBlockKind code = srcData->code;
    if (BK_FIELD != code)
        // TODO: hook various reinterpretation drivers here
        return false;

    const FieldOfObj *fldData = DCAST<const FieldOfObj *>(srcData);
    const TValId valSrc = fldData->value;
    if (VAL_INVALID == valSrc)
        // invalid source
        return false;

    const TObjType cltSrc = fldData->clt;
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
        TFldId                      old,
        TFldId                      fld,
        TValSet                    *killedPtrs)
{
    BlockEntity *blData;
    this->ents.getEntRW(&blData, old);

    EBlockKind code = blData->code;
    switch (code) {
        case BK_FIELD:
            break;

        case BK_COMPOSITE:
            // do not invalidate those place-holding values of composite objects
            return;

        case BK_UNIFORM:
            this->splitBlockByObject(/* block */ old, fld);
            return;

        case BK_INVALID:
        default:
            CL_BREAK_IF("invalid call of reinterpretObjData()");
            return;
    }

    CL_DEBUG("reinterpretObjData() is taking place...");
    FieldOfObj *oldData = DCAST<FieldOfObj *>(blData);
    const TValId valOld = oldData->value;
    if (/* wasPtr */ this->releaseValueOf(old, valOld) && killedPtrs)
        killedPtrs->insert(valOld);

    // resolve object data
    const TObjId obj = oldData->obj;
    Region *rootData;
    this->ents.getEntRW(&rootData, obj);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* mayOverlap */ true));

    this->ents.getEntRW(&blData, fld);
    code = blData->code;

    switch (code) {
        case BK_UNIFORM:
            if (isCoveredByBlock(oldData, blData)) {
                // object fully covered by the overlapping uniform block
                oldData->value = this->valDup(blData->value);
                goto data_restored;
            }
            // fall through!

        case BK_FIELD:
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
    if (rootData->liveFields.erase(old))
        CL_DEBUG("reinterpretObjData() kills a live object");

    if (!oldData->extRefCnt) {
        CL_DEBUG("reinterpretObjData() destroys a dead object");
        this->fldDestroy(old, /* removeVal */ false, /* detach */ true);
        return;
    }

data_restored:
    // register the newly assigned value of the _old_ object
    this->registerValueOf(old, oldData->value);
}

void SymHeapCore::Private::setValueOf(
        TFldId                      fld,
        TValId                      val,
        TValSet                    *killedPtrs)
{
    // release old value
    FieldOfObj *fldData;
    this->ents.getEntRW(&fldData, fld);

    const TValId valOld = fldData->value;
    if (valOld == val)
        // we are asked to write a value which is already there, skip it!
        return;

    if (/* wasPtr */ this->releaseValueOf(fld, valOld) && killedPtrs)
        killedPtrs->insert(valOld);

    // store new value
    fldData->value = val;
    this->registerValueOf(fld, val);

    // read object data
    const TObjId obj = fldData->obj;
    Region *rootData;
    this->ents.getEntRW(&rootData, obj);

    // (re)insert self into the arena if not there
    TArena &arena = rootData->arena;
    const TOffset off = fldData->off;
    const TObjType clt = fldData->clt;
    arena += createArenaItem(off, clt->size, fld);

    // invalidate contents of the objects we are overwriting
    TFldIdSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), fld)) {
        BOOST_FOREACH(const TFldId old, overlaps)
            this->reinterpretObjData(old, fld, killedPtrs);
    }

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
}

TFldId SymHeapCore::Private::fldCreate(
        TObjId                      obj,
        TOffset                     off,
        TObjType                    clt)
{
    // acquire object ID
    FieldOfObj *fldData = new FieldOfObj(obj, off, clt);
    const TFldId fld = this->assignId(fldData);

    // read object data
    Region *rootData;
    this->ents.getEntRW(&rootData, obj);

    // map the region occupied by the object
    rootData->arena += createArenaItem(off, clt->size, fld);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));
    return fld;
}

void SymHeapCore::Private::fldDestroy(TFldId fld, bool removeVal, bool detach)
{
    BlockEntity *blData;
    this->ents.getEntRW(&blData, fld);

    const EBlockKind code = blData->code;
    if (removeVal && BK_UNIFORM != code) {
        // release value of the object
        TValId &val = blData->value;
        this->releaseValueOf(fld, val);
        val = VAL_INVALID;
    }

    if (detach) {
        // properly remove the object from grid and arena
        Region *rootData;
        this->ents.getEntRW(&rootData, blData->obj);
        CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* overlap */true));

        // remove the object from arena unless we are destroying everything
        const TOffset off = blData->off;
        const TSizeOf size = blData->size;
        rootData->arena -= createArenaItem(off, size, fld);

        CL_BREAK_IF(hasKey(rootData->liveFields, fld));
        CL_BREAK_IF(!this->chkArenaConsistency(rootData, /* overlap */true));
    }

    if (BK_UNIFORM != code && 0 < DCAST<FieldOfObj *>(blData)->extRefCnt)
        // preserve an externally referenced object
        return;

    // release the corresponding FieldOfObj instance
    this->ents.releaseEnt(fld);
}

TValId SymHeapCore::Private::valCreate(
        EValueTarget                code,
        EValueOrigin                origin)
{
    TValId val = VAL_INVALID;

    switch (code) {
        case VT_INVALID:
        case VT_OBJECT:
        case VT_RANGE:
            CL_BREAK_IF("invalid call of SymHeapCore::Private::valCreate()");
            // fall through!

        case VT_UNKNOWN:
            val = this->assignId(new BaseValue(code, origin));
            break;

        case VT_COMPOSITE:
            val = this->assignId(new CompValue(code, origin));
            break;

        case VT_CUSTOM:
            val = this->assignId(new InternalCustomValue(code, origin));
            break;
    }

    return val;
}

TValId SymHeapCore::Private::valDup(TValId val)
{
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
bool SymHeapCore::Private::valsEqual(TValId v1, TValId v2)
{
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
    const BaseAddress *rootValDataSrc;
    this->ents.getEntRO(&rootValDataSrc, srcRoot);

    // resolve src region
    const Region *rootDataSrc;
    this->ents.getEntRO(&rootDataSrc, rootValDataSrc->obj);

    // check up to now arena consistency
    CL_BREAK_IF(!this->chkArenaConsistency(rootDataSrc));

    const TArena &arena = rootDataSrc->arena;
    const TOffset winEnd = winBeg + winSize;
    const TMemChunk chunk (winBeg, winEnd);

    TFldIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, FLD_INVALID))
        // no data to copy in here
        return;

    const BaseAddress *rootValDataDst;
    this->ents.getEntRO(&rootValDataDst, dstRoot);
    const TOffset shift = dstOff - winBeg;

    // resolve dst region
    Region *objDataDst;
    const TObjId objDst = rootValDataDst->obj;
    this->ents.getEntRW(&objDataDst, objDst);

    // go through overlaps and copy the live ones
    BOOST_FOREACH(const TFldId objSrc, overlaps) {
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

        if (!hasKey(rootDataSrc->liveFields, objSrc))
            // dead object anyway
            continue;

        // copy a single live block
        this->copySingleLiveBlock(objDst, objDataDst, objSrc, code,
                realShift, sizeLimit);
    }
}

SymHeapCore::Private::Private(Trace::Node *trace):
    traceHandle (trace),
    liveObjs    (new TObjSetWrapper),
    anonStackMap(new TAnonStackMapWrapper),
    cVarMap     (new CVarMap),
    cValueMap   (new CustomValueMapper),
    coinDb      (new CoincidenceDb),
    neqDb       (new NeqDb)
{
}

SymHeapCore::Private::Private(const SymHeapCore::Private &ref):
    traceHandle (new Trace::CloneNode(ref.traceHandle.node())),
    ents        (ref.ents),
    liveObjs    (ref.liveObjs),
    anonStackMap(ref.anonStackMap),
    cVarMap     (ref.cVarMap),
    cValueMap   (ref.cValueMap),
    coinDb      (ref.coinDb),
    neqDb       (ref.neqDb)
{
    RefCntLib<RCO_NON_VIRT>::enter(this->liveObjs);
    RefCntLib<RCO_NON_VIRT>::enter(this->anonStackMap);
    RefCntLib<RCO_NON_VIRT>::enter(this->cVarMap);
    RefCntLib<RCO_NON_VIRT>::enter(this->cValueMap);
    RefCntLib<RCO_NON_VIRT>::enter(this->coinDb);
    RefCntLib<RCO_NON_VIRT>::enter(this->neqDb);
}

SymHeapCore::Private::~Private()
{
    RefCntLib<RCO_NON_VIRT>::leave(this->liveObjs);
    RefCntLib<RCO_NON_VIRT>::leave(this->anonStackMap);
    RefCntLib<RCO_NON_VIRT>::leave(this->cVarMap);
    RefCntLib<RCO_NON_VIRT>::leave(this->cValueMap);
    RefCntLib<RCO_NON_VIRT>::leave(this->coinDb);
    RefCntLib<RCO_NON_VIRT>::leave(this->neqDb);
}

TValId SymHeapCore::Private::fldInit(TFldId fld)
{
    FieldOfObj *fldData;
    this->ents.getEntRW(&fldData, fld);
    CL_BREAK_IF(!fldData->extRefCnt);

    // read object data
    const TObjId obj = fldData->obj;
    Region *rootData;
    this->ents.getEntRW(&rootData, obj);
    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    const TArena &arena = rootData->arena;
    const TOffset off = fldData->off;
    const TObjType clt = fldData->clt;

    // first check for data reinterpretation
    TFldIdSet overlaps;
    if (arenaLookup(&overlaps, arena, createChunk(off, clt), fld)) {
        BOOST_FOREACH(const TFldId other, overlaps) {
            const BlockEntity *blockData;
            this->ents.getEntRO(&blockData, other);

            const EBlockKind code = blockData->code;
            if (BK_UNIFORM != code && !hasKey(rootData->liveFields, other))
                continue;

            // reinterpret _self_ by another live object or uniform block
            this->reinterpretObjData(/* old */ fld, other);
            CL_BREAK_IF(!this->chkArenaConsistency(rootData));
            return fldData->value;
        }
    }

    // assign a fresh unknown value
    const TValId val = this->valCreate(VT_UNKNOWN, VO_UNKNOWN);
    fldData->value = val;

    // mark the object as live
    rootData->liveFields[fld] = BK_FIELD;

    CL_BREAK_IF(!this->chkArenaConsistency(rootData));

    // store backward reference
    BaseValue *valData;
    this->ents.getEntRW(&valData, val);
    valData->usedBy.insert(fld);
    return val;
}

TValId SymHeapCore::valueOf(TFldId fld)
{
    // handle special cases first
    switch (fld) {
        case FLD_UNKNOWN:
            // not implemented
        case FLD_INVALID:
            return VAL_INVALID;

        case FLD_DEREF_FAILED:
            return d->valCreate(VT_UNKNOWN, VO_DEREF_FAILED);

        default:
            break;
    }

    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);

    TValId val = fldData->value;
    if (VAL_INVALID != val)
        // the field has a value
        return val;

    const TObjType clt = fldData->clt;
    if (isComposite(clt)) {
        // deleayed creation of a composite value
        val = d->valCreate(VT_COMPOSITE, VO_INVALID);
        CompValue *compData;
        d->ents.getEntRW(&compData, val);
        compData->compObj = fld;

        // store the value
        FieldOfObj *fldDataRW;
        d->ents.getEntRW(&fldDataRW, fld);
        fldDataRW->value = val;

        // store backward reference
        compData->usedBy.insert(fld);
        return val;
    }

    // delayed field initialization
    return d->fldInit(fld);
}

void SymHeapCore::usedBy(FldList &dst, TValId val, bool liveOnly) const
{
    if (VAL_NULL == val)
        // we do not track uses of special values
        return;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    const TFldIdSet &usedBy = valData->usedBy;
    if (!liveOnly) {
        // dump everything
        BOOST_FOREACH(const TFldId fld, usedBy)
            dst.push_back(FldHandle(*const_cast<SymHeapCore *>(this), fld));

        return;
    }

    BOOST_FOREACH(const TFldId fld, usedBy) {
        // get field data
        const FieldOfObj *fldData;
        d->ents.getEntRO(&fldData, fld);

        // get object data
        const Region *rootData;
        d->ents.getEntRO(&rootData, fldData->obj);

        // check if the object is alive
        if (hasKey(rootData->liveFields, fld))
            dst.push_back(FldHandle(*const_cast<SymHeapCore *>(this), fld));
    }
}

unsigned SymHeapCore::usedByCount(TValId val) const
{
    if (VAL_NULL == val)
        return 0;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->usedBy.size();
}

void SymHeapCore::pointedBy(FldList &dst, TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);

    const TFldIdSet &usedBy = regData->usedByGl;
    BOOST_FOREACH(const TFldId fld, usedBy)
        dst.push_back(FldHandle(*const_cast<SymHeapCore *>(this), fld));
}

unsigned SymHeapCore::pointedByCount(TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->usedByGl.size();
}

unsigned SymHeapCore::lastId() const
{
    return d->ents.lastId<unsigned>();
}

TFldId SymHeapCore::Private::copySingleLiveBlock(
        const TObjId                objDst,
        Region                     *objDataDst,
        const TFldId                fldSrc,
        const EBlockKind            code,
        const TOffset               shift,
        const TSizeOf               sizeLimit)
{
    TFldId dst;

    if (BK_UNIFORM == code) {
        // duplicate a uniform block
        const BlockEntity *blSrc;
        this->ents.getEntRO(&blSrc, fldSrc);
        BlockEntity *blDst = blSrc->clone();
        dst = this->assignId(blDst);
        blDst->obj = objDst;

        // shift the block and limit the size if asked to do so
        blDst->off += shift;
        if (sizeLimit)
            blDst->size = sizeLimit;

        // map the cloned block
        objDataDst->arena += createArenaItem(blDst->off, blDst->size, dst);
    }
    else {
        // duplicate a regular object
        CL_BREAK_IF(BK_FIELD != code);
        CL_BREAK_IF(sizeLimit);

        const FieldOfObj *fldDataSrc;
        this->ents.getEntRO(&fldDataSrc, fldSrc);

        const TOffset off = fldDataSrc->off + shift;
        const TObjType clt = fldDataSrc->clt;
        dst = this->fldCreate(objDst, off, clt);
        this->setValueOf(dst, fldDataSrc->value);
    }

    // prevserve live object code
    objDataDst->liveFields[dst] = code;
    return dst;
}

TObjId SymHeapCore::objClone(TObjId obj)
{
    CL_DEBUG("SymHeapCore::objClone() is taking place...");

    // resolve the src region
    const Region *objDataSrc;
    d->ents.getEntRO(&objDataSrc, obj);
    CL_BREAK_IF(!d->chkArenaConsistency(objDataSrc));

    // create the cloned object
    const TObjId dup = d->assignId(new Region(objDataSrc->code));
    Region *objDataDst;
    d->ents.getEntRW(&objDataDst, dup);

    // duplicate root metadata
    objDataDst->cVar                = objDataSrc->cVar;
    objDataDst->size                = objDataSrc->size;
    objDataDst->lastKnownClt        = objDataSrc->lastKnownClt;
    objDataDst->isValid             = objDataSrc->isValid;
    objDataDst->protoLevel          = objDataSrc->protoLevel;

    if (objDataDst->isValid) {
        RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveObjs);
        d->liveObjs->insert(dup);
    }

    BOOST_FOREACH(TLiveObjs::const_reference item, objDataSrc->liveFields)
        d->copySingleLiveBlock(dup, objDataDst,
                /* src  */ item.first,
                /* code */ item.second);

    CL_BREAK_IF(!d->chkArenaConsistency(objDataDst));
    return dup;
}

void SymHeapCore::gatherUniformBlocks(TUniBlockMap &dst, TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);

    BOOST_FOREACH(TLiveObjs::const_reference item, regData->liveFields) {
        const EBlockKind code = item.second;
        if (BK_UNIFORM != code)
            continue;

        const BlockEntity *blData;
        d->ents.getEntRO(&blData, /* fld */ item.first);
        const TOffset off = blData->off;
        CL_BREAK_IF(hasKey(dst, off) && !::bypassSelfChecks);
        UniformBlock &block = dst[off];

        // export uniform block
        block.off       = off;
        block.size      = blData->size;
        block.tplValue  = blData->value;
    }
}

void SymHeapCore::gatherLiveFields(FldList &dst, TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);

    BOOST_FOREACH(TLiveObjs::const_reference item, regData->liveFields) {
        const EBlockKind code = item.second;

        switch (code) {
            case BK_UNIFORM:
                continue;

            case BK_FIELD:
                break;

            case BK_INVALID:
            default:
                CL_BREAK_IF("gatherLiveFields sees something special");
        }

        const TFldId fld = item.first;
        dst.push_back(FldHandle(*const_cast<SymHeapCore *>(this), fld));
    }
}

bool SymHeapCore::findCoveringUniBlocks(
        TUniBlockMap               *pCovered,
        const TObjId                obj,
        UniformBlock                block)
    const
{
    CL_BREAK_IF(!pCovered->empty());

    const Region *regData;
    d->ents.getEntRO(&regData, obj);

    const TArena &arena = regData->arena;
    const TOffset beg = block.off;
    const TOffset end = beg + block.size;
    const TMemChunk chunk(beg, end);

    TFldIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, FLD_INVALID))
        // not found
        return false;

    // use a temporary arena to test the coverage
    TArena coverage;
    coverage += TMemItem(chunk, /* XXX: misleading */ FLD_UNKNOWN);

    // go through overlaps and subtract the chunks that are covered
    BOOST_FOREACH(const TFldId fld, overlaps) {
        const BlockEntity *blData;
        d->ents.getEntRO(&blData, fld);

        const EBlockKind code = blData->code;
        if (BK_UNIFORM != code && VAL_NULL != blData->value)
            continue;

        if (!areValProtosEqual(*this, *this, blData->value, block.tplValue))
            // incompatible value prototype
            continue;

        // this block entity can be used to build up the coverage, subtract it
        const TOffset blBeg = blData->off;
        const TSizeOf blSize = blData->size;
        coverage -= createArenaItem(blBeg, blSize, FLD_UNKNOWN);
    }

    TFldIdSet uncovered;
    if (!arenaLookup(&uncovered, coverage, chunk, FLD_INVALID)) {
        // full coverage has been found
        (*pCovered)[beg] = block;
        return true;
    }

    TArena::TKeySet gaps;
    coverage.reverseLookup(gaps, FLD_UNKNOWN);
    if (gaps.empty())
        // there is really nothing we could pick for coverage
        return false;

    // TODO: rewrite the algorithm so that we do not compute complement twice
    coverage.clear();
    coverage += TMemItem(chunk, /* XXX: misleading */ FLD_UNKNOWN);
    BOOST_FOREACH(TArena::TKeySet::const_reference item, gaps)
        coverage -= TMemItem(item, FLD_UNKNOWN);

    // return partial coverage (if any)
    TArena::TKeySet covChunks;
    coverage.reverseLookup(covChunks, FLD_UNKNOWN);
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

    // allocate the VAL_NULL base address
    const TValId valNull = d->assignId(new BaseAddress(OBJ_NULL, TS_REGION));
    CL_BREAK_IF(VAL_NULL != valNull);

    // allocate the VAL_TRUE value
    const TValId valTrue = d->wrapIntVal(IR::Int1);
    CL_BREAK_IF(VAL_TRUE != valTrue);
    (void) valTrue;

    // allocate the OBJ_NULL region and mark it as invalid
    Region *objNullData = new Region(SC_INVALID);
    objNullData->addrByTS[TS_REGION] = valNull;
    objNullData->isValid = false;
    const TObjId objNull = d->assignId(objNullData);
    CL_BREAK_IF(OBJ_NULL != objNull);
    (void) objNull;

    // allocate OBJ_RETURN
    const TObjId objRet = d->assignId(new Region(SC_ON_STACK));
    CL_BREAK_IF(OBJ_RETURN != objRet);
    (void) objRet;
}

SymHeapCore::SymHeapCore(const SymHeapCore &ref):
    stor_(ref.stor_),
    d(new Private(*ref.d))
{
    CL_BREAK_IF(!&stor_);
}

SymHeapCore::~SymHeapCore()
{
    delete d;
}

// cppcheck-suppress operatorEqToSelf
SymHeapCore& SymHeapCore::operator=(const SymHeapCore &ref)
{
    CL_BREAK_IF(&ref == this);
    CL_BREAK_IF(&stor_ != &ref.stor_);

    delete d;
    d = new Private(*ref.d);
    return *this;
}

void SymHeapCore::swap(SymHeapCore &ref)
{
    CL_BREAK_IF(&stor_ != &ref.stor_);
    swapValues(this->d, ref.d);
}

Trace::Node* SymHeapCore::traceNode() const
{
    return d->traceHandle.node();
}

void SymHeapCore::traceUpdate(Trace::Node *node)
{
    d->traceHandle.reset(node);
}

void SymHeapCore::setValOfField(TFldId fld, TValId val, TValSet *killedPtrs)
{
    // we allow to set values of atomic types only
    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);

    // make sure that the value is not a pointer to a structure object
    const TObjType clt = fldData->clt;
    CL_BREAK_IF(VT_COMPOSITE == this->valTarget(val));

    // mark the destination object as live
    Region *regData;
    d->ents.getEntRW(&regData, fldData->obj);
    regData->liveFields[fld] = bkFromClt(clt);

    // now set the value
    d->setValueOf(fld, val, killedPtrs);
}

TFldId SymHeapCore::Private::writeUniformBlock(
        const TObjId                obj,
        const UniformBlock         &ub,
        TValSet                    *killedPtrs)
{
    // acquire field ID
    BlockEntity *blData =
        new BlockEntity(BK_UNIFORM, obj, ub.off, ub.size, ub.tplValue);
    const TFldId fld = this->assignId(blData);

    // jump to region
    Region *regData;
    this->ents.getEntRW(&regData, obj);

    // check up to now arena consistency
    CL_BREAK_IF(!this->chkArenaConsistency(regData));

    // mark the block as live
    regData->liveFields[fld] = BK_UNIFORM;

    TArena &arena = regData->arena;
    arena += createArenaItem(ub.off, ub.size, fld);
    const TMemChunk chunk(ub.off, ub.off + ub.size);

    // invalidate contents of the objects we are overwriting
    TFldIdSet overlaps;
    if (arenaLookup(&overlaps, arena, chunk, fld)) {
        BOOST_FOREACH(const TFldId old, overlaps)
            this->reinterpretObjData(old, fld, killedPtrs);
    }

    CL_BREAK_IF(!this->chkArenaConsistency(regData));
    return fld;
}

/// just a trivial wrapper to hide the return value
void SymHeapCore::writeUniformBlock(
        const TObjId                obj,
        const UniformBlock         &ub,
        TValSet                    *killedPtrs)
{
    d->writeUniformBlock(obj, ub, killedPtrs);
}

void SymHeapCore::copyBlockOfRawMemory(
        const TValId                dst,
        const TValId                src,
        const TSizeOf               size,
        TValSet                     *killedPtrs)
{
    // this should have been checked by the caller
    CL_BREAK_IF(valSizeOfTarget(*this, dst).lo < size);
    CL_BREAK_IF(valSizeOfTarget(*this, src).lo < size);

    const BaseValue *dstData;
    const BaseValue *srcData;

    d->ents.getEntRO(&dstData, dst);
    d->ents.getEntRO(&srcData, src);

    CL_BREAK_IF(!isPossibleToDeref(*this, dst));
    CL_BREAK_IF(!isPossibleToDeref(*this, src));
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

    const BaseAddress *rootDataDst;
    d->ents.getEntRO(&rootDataDst, dstRoot);

    // nuke the content we are going to overwrite
    const UniformBlock ubKiller = {
        /* off      */  dstOff,
        /* size     */  size,
        /* tplValue */  VAL_NULL
    };
    const TFldId blKiller =
        d->writeUniformBlock(rootDataDst->obj, ubKiller, killedPtrs);

    // jump to region
    Region *regDataDst;
    d->ents.getEntRW(&regDataDst, rootDataDst->obj);

    // check up to now arena consistency
    CL_BREAK_IF(!d->chkArenaConsistency(regDataDst));

    // remove the dummy block we used just to trigger the data reinterpretation
    regDataDst->liveFields.erase(blKiller);
    regDataDst->arena -= createArenaItem(dstOff, size, blKiller);
    d->ents.releaseEnt(blKiller);
    CL_BREAK_IF(!d->chkArenaConsistency(regDataDst));

    // now we need to transfer data between two distinct root entities
    d->transferBlock(dstRoot, srcRoot, dstOff, srcOff, size);
    CL_BREAK_IF(!d->chkArenaConsistency(regDataDst));
}

bool SymHeapCore::Private::findZeroInBlock(
        TOffset                *offDst,
        bool                   *provenPrefix,
        const TOffset           offSrc,
        const                   TFldId fld)
{
    const BlockEntity *blData;
    this->ents.getEntRO(&blData, fld);

    if (VAL_NULL == blData->value) {
        // a block full of zeros
        *offDst = blData->off;
        return true;
    }

    const EBlockKind code = blData->code;
    if (BK_FIELD != code) {
        if (BK_UNIFORM != code)
            CL_BREAK_IF("findZeroInBlock() got something special");

        return false;
    }

    const FieldOfObj *fldData = DCAST<const FieldOfObj *>(blData);
    if (CL_TYPE_ARRAY == fldData->clt->code) {
        // assume zero-terminated string
        const InternalCustomValue *valData;
        this->ents.getEntRO(&valData, fldData->value);

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
    const BaseAddress *rootValData;
    this->ents.getEntRO(&rootValData, root);

    // jump to region
    const Region *rootData;
    this->ents.getEntRO(&rootData, rootValData->obj);
    CL_BREAK_IF(!rootData->isValid);

    const TArena &arena = rootData->arena;
    const TSizeOf limit = rootData->size.hi;
    const TMemChunk chunk(offSrc, limit);

    TFldIdSet overlaps;
    if (!arenaLookup(&overlaps, arena, chunk, FLD_INVALID))
        // no blocks that would serve as a trailing zero
        return false;

    bool provenPrefix = false;;

    // go through all intersections and find the zero that is closest to offSrc
    TOffset first = limit;
    BOOST_FOREACH(const TFldId fld, overlaps) {
        TOffset beg;
        if (!this->findZeroInBlock(&beg, &provenPrefix, offSrc, fld))
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

TObjType SymHeapCore::fieldType(TFldId fld) const
{
    if (fld < 0)
        return 0;

    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);
    return fldData->clt;
}

TValId SymHeapCore::Private::shiftCustomValue(TValId ref, TOffset shift)
{
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

TValId SymHeapCore::Private::wrapIntVal(const IR::TInt num)
{
    if (IR::Int0 == num)
        return VAL_NULL;

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

void SymHeapCore::Private::replaceRngByInt(const InternalCustomValue *valData)
{
    CL_DEBUG("replaceRngByInt() is taking place...");

    // we already expect a scalar at this point
    const CustomValue &cvRng = valData->customData;
    const IR::Range &rng = rngFromCustom(cvRng);
    CL_BREAK_IF(!isSingular(rng));

    const TValId replaceBy = this->wrapIntVal(rng.lo);

    // we intentionally do not use a reference here (tight loop otherwise)
    TFldIdSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TFldId fld, usedBy)
        this->setValueOf(fld, replaceBy);
}

void SymHeapCore::Private::trimCustomValue(TValId val, const IR::Range &win)
{
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

TValId SymHeapCore::valByOffset(TValId at, TOffset off)
{
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

TValId SymHeapCore::valByRange(TValId at, IR::Range range)
{
    if (isSingular(range))
        return this->valByOffset(at, range.lo);

    const TObjId obj = this->objByAddr(at);
    if (!this->isValid(obj))
        return d->valCreate(VT_UNKNOWN, VO_UNKNOWN);

    const BaseValue *valData;
    d->ents.getEntRO(&valData, at);

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
    BaseAddress *rootData;
    d->ents.getEntRW(&rootData, valRoot);
    rootData->dependentValues.push_back(val);

    return val;
}

TValId SymHeapCore::valShift(TValId valToShift, TValId shiftBy)
{
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

    if (isPossibleToDeref(*this, valToShift))
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

void SymHeapCore::valRestrictRange(TValId val, IR::Range win)
{
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

void SymHeapCore::Private::bindValues(TValId v1, TValId v2, TValId valSum)
{
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

TValId SymHeapCore::diffPointers(const TValId v1, const TValId v2)
{
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

EStorageClass SymHeapCore::objStorClass(TObjId obj) const
{
    if (OBJ_INVALID == obj)
        return SC_INVALID;

    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->code;
}

TValId SymHeapCore::addrOfTarget(TObjId obj, ETargetSpecifier ts, TOffset off)
{
    if (OBJ_INVALID == obj)
        return VAL_INVALID;

    const Region *objDataRO;
    d->ents.getEntRO(&objDataRO, obj);

    const TAddrByTS &addrByTS = objDataRO->addrByTS;
    const TAddrByTS::const_iterator it = addrByTS.find(ts);

    TValId base;
    if (addrByTS.end() == it) {
        // allocate a fresh base address
        BaseAddress *baseAddrData = new BaseAddress(obj, ts);
        base = d->assignId(baseAddrData);

        // register the base address by the target object
        Region *objDataRW;
        d->ents.getEntRW(&objDataRW, obj);
        objDataRW->addrByTS[ts] = base;
    }
    else
        // reuse the existing base address
        base = it->second;

    return this->valByOffset(base, off);
}

EValueOrigin SymHeapCore::valOrigin(TValId val) const
{
    switch (val) {
        case VAL_INVALID:
            return VO_INVALID;

        case VAL_NULL /* = VAL_FALSE */:
            return VO_ASSIGNED;

        default:
            break;
    }

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->origin;
}

EValueTarget SymHeapCore::valTarget(TValId val) const
{
    if (val < VAL_NULL)
        return VT_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->code;
}

ETargetSpecifier SymHeapCore::targetSpec(TValId addr) const
{
    if (addr < VAL_NULL)
        return TS_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, addr);

    if (!isAnyDataArea(valData->code))
        return TS_INVALID;

    const BaseAddress *rootData;
    d->ents.getEntRO(&rootData, valData->valRoot);
    return rootData->ts;
}

void SymHeapCore::rewriteTargetOfBase(TValId root, TObjId objNew)
{
    BaseAddress *rootData;
    d->ents.getEntRW(&rootData, root);
    const TObjId objOld = rootData->obj;

    // rewrite the target object
    CL_BREAK_IF(objOld == objNew);
    rootData->obj = objNew;

    // resolve old/new object data
    Region *regDataOld, *regDataNew;
    d->ents.getEntRW(&regDataOld, objOld);
    d->ents.getEntRW(&regDataNew, objNew);

    // move the address from objOld to objNew
    const ETargetSpecifier ts = rootData->ts;
    CL_BREAK_IF(hasKey(regDataNew->addrByTS, ts));
    if (!regDataOld->addrByTS.erase(ts))
        CL_BREAK_IF("internal error in rewriteTargetOfBase");
    regDataNew->addrByTS[ts] = root;

    // go through fields pointing to objOld
    TFldIdSet unrelatedFlds;
    BOOST_FOREACH(const TFldId fld, regDataOld->usedByGl) {
        // read value of the field
        const FieldOfObj *fldData;
        d->ents.getEntRO(&fldData, fld);
        const TValId val = fldData->value;

        // resolve base address
        const BaseValue *valData;
        d->ents.getEntRO(&valData, val);
        if (valData->valRoot == root)
            // reference moved
            regDataNew->usedByGl.insert(fld);
        else
            unrelatedFlds.insert(fld);
    }

    // write unmoved field IDs
    regDataOld->usedByGl.swap(unrelatedFlds);
}

bool isUninitialized(EValueOrigin code)
{
    switch (code) {
        case VO_HEAP:
        case VO_STACK:
            return true;

        default:
            return false;
    }
}

bool isOnHeap(EStorageClass code)
{
    return (SC_ON_HEAP == code);
}

bool isProgramVar(EStorageClass code)
{
    switch (code) {
        case SC_STATIC:
        case SC_ON_STACK:
            return true;

        default:
            return false;
    }
}

bool isAnyDataArea(EValueTarget code)
{
    switch (code) {
        case VT_OBJECT:
        case VT_RANGE:
            return true;

        default:
            return false;
    }
}

TObjId SymHeapCore::objByAddr(TValId val) const {
    if (val < VAL_NULL)
        return OBJ_INVALID;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    if (!isAnyDataArea(valData->code))
        return OBJ_INVALID;

    const BaseAddress *rootData;
    d->ents.getEntRO(&rootData, valData->valRoot);
    return rootData->obj;
}

TValId SymHeapCore::valRoot(TValId val) const
{
    if (val <= 0)
        return val;

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    return valData->valRoot;
}

TOffset SymHeapCore::valOffset(TValId val) const
{
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

IR::Range SymHeapCore::valOffsetRange(TValId val) const
{
    if (val < VAL_NULL)
        return IR::rngFromNum(IR::Int0);

    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    const EValueTarget code = valData->code;
    switch (code) {
        case VT_RANGE:
            break;

        case VT_CUSTOM:
            // FIXME: valData->offRoot is used internally for unrelated purposes
            return IR::rngFromNum(IR::Int0);

        default:
            // this is going to be a singular range
            return IR::rngFromNum(valData->offRoot);
    }

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

void SymHeapCore::valReplace(TValId val, TValId replaceBy)
{
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);

    // kill all related Neq predicates
    TValList neqs;
    d->neqDb->gatherRelatedValues(neqs, val);
    BOOST_FOREACH(const TValId valNeq, neqs) {
        CL_BREAK_IF(valNeq == replaceBy);
        this->delNeq(valNeq, val);
    }

    // we intentionally do not use a reference here (tight loop otherwise)
    TFldIdSet usedBy = valData->usedBy;
    BOOST_FOREACH(const TFldId fld, usedBy)
        this->setValOfField(fld, replaceBy);
}

void SymHeapCore::addNeq(TValId v1, TValId v2)
{
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->neqDb);

    const EValueTarget code1 = this->valTarget(v1);
    const EValueTarget code2 = this->valTarget(v2);

    if (VT_UNKNOWN != code1 && VT_UNKNOWN != code2) {
        CL_DEBUG("SymHeap::neqOp() refuses to add an extraordinary Neq predicate");
        return;
    }

    d->neqDb->add(v1, v2);
}

void SymHeapCore::delNeq(TValId v1, TValId v2)
{
    CL_BREAK_IF(!this->chkNeq(v1, v2));

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->neqDb);
    d->neqDb->del(v1, v2);
}

void SymHeapCore::gatherRelatedValues(TValList &dst, TValId val) const
{
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
        dst.addNeq(valLt, valGt);
    }

    // go through CoincidenceDb
    const CoincidenceDb &coinDb = *d->coinDb;
    BOOST_FOREACH(CoincidenceDb::const_reference ref, coinDb) {
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

bool SymHeapCore::matchPreds(
        const SymHeapCore           &ref,
        const TValMap               &valMap,
        const bool                   nonZeroOnly)
    const
{
    SymHeapCore &src = const_cast<SymHeapCore &>(*this);
    SymHeapCore &dst = const_cast<SymHeapCore &>(ref);

    // go through NeqDb
    BOOST_FOREACH(const NeqDb::TItem &item, d->neqDb->cont_) {
        TValId valLt = item.first;
        TValId valGt = item.second;

        if (nonZeroOnly && VAL_NULL == valLt)
            continue;

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
    BOOST_FOREACH(CoincidenceDb::const_reference ref, coinDb) {
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

TObjId SymHeapCore::objByField(TFldId fld) const
{
    if (fld < 0)
        return OBJ_INVALID;

    // resolve object
    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);
    return fldData->obj;
}

TOffset SymHeapCore::fieldOffset(TFldId fld) const
{
    if (fld < 0)
        return 0;

    // resolve object
    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);
    return fldData->off;
}

TValId SymHeapCore::placedAt(TFldId fld)
{
    if (fld < 0)
        return VAL_INVALID;

    // resolve object
    const FieldOfObj *fldData;
    d->ents.getEntRO(&fldData, fld);
    const TObjId obj = fldData->obj;

    return this->addrOfTarget(obj, TS_REGION, fldData->off);
}

TFldId SymHeapCore::Private::fldLookup(
        const TObjId                obj,
        const TOffset               off,
        const TObjType              clt,
        IMatchPolicy               *policy)
{
    if (OBJ_INVALID == obj || off < 0)
        return FLD_INVALID;

    const TSizeOf size = clt->size;
    CL_BREAK_IF(size <= 0);

    // jump to region
    const Region *rootData;
    this->ents.getEntRO(&rootData, obj);
    CL_BREAK_IF(!rootData->isValid);

    if (rootData->size.lo < off + size) {
        CL_BREAK_IF("fldLookup() called out of bounds");
        return FLD_UNKNOWN;
    }

    // arena lookup
    TFldIdSet candidates;
    const TArena &arena = rootData->arena;
    const TMemChunk chunk(off, off + size);
    arenaLookForExactMatch(&candidates, arena, chunk);

    // go through fields in the given interval
    BOOST_FOREACH(const TFldId fld, candidates) {
        const BlockEntity *blData;
        this->ents.getEntRO(&blData, fld);

        const EBlockKind code = blData->code;
        switch (code) {
            case BK_FIELD:
            case BK_COMPOSITE:
                break;

            default:
                continue;
        }

        const bool isLive = hasKey(rootData->liveFields, fld);
        const FieldOfObj *fldData = DCAST<const FieldOfObj *>(blData);
        if (!/* continue */policy->matchBlock(fld, fldData, isLive))
            break;
    }

    const TFldId best = policy->bestMatch();
    if (FLD_INVALID != best)
        return best;

    // no match, create a new field
    return this->fldCreate(obj, off, clt);
}

class PtrMatchPolicy: public IMatchPolicy {
    private:
        TFldId bestMatch_;

    public:
        PtrMatchPolicy():
            bestMatch_(FLD_INVALID)
        {
        }

        virtual bool matchBlock(TFldId fld, const FieldOfObj *data, bool) {
            if (!isDataPtr(data->clt))
                return /* continue */ true;

            // pointer found!
            bestMatch_ = fld;
            return false;
        }

        virtual TFldId bestMatch() const {
            return bestMatch_;
        }
};

TFldId SymHeapCore::ptrLookup(TObjId obj, TOffset off)
{
    // generic pointer, (void *) if available
    const TObjType clt = stor_.types.genericDataPtr();
    if (!clt || clt->code != CL_TYPE_PTR) {
        CL_BREAK_IF("Code Listener failed to capture a type of data pointer");
        return FLD_INVALID;
    }

    PtrMatchPolicy policy;
    return d->fldLookup(obj, off, clt, &policy);
}

class FieldMatchPolicy: public IMatchPolicy {
    private:
        TFldId                      bestMatch_;
        const TObjType              cltToMatch_;
        bool                        cltExactMatch_;
        bool                        cltClassMatch_;

    public:
        FieldMatchPolicy(const TObjType clt):
            bestMatch_(FLD_INVALID),
            cltToMatch_(clt),
            cltExactMatch_(false),
            cltClassMatch_(false)
        {
            CL_BREAK_IF(!clt || !clt->size);
        }

        virtual bool matchBlock(TFldId fld, const FieldOfObj *data, bool alive);

        virtual TFldId bestMatch() const {
            return bestMatch_;
        }
};

bool FieldMatchPolicy::matchBlock(
        TFldId                      fld,
        const FieldOfObj           *fldData,
        bool                        isLive)
{
    const TObjType cltNow = fldData->clt;
    if (cltNow == cltToMatch_) {
        // exact match
        cltExactMatch_ = true;
        bestMatch_ = fld;
        return /* continue */ !isLive;
    }
    else if (cltExactMatch_)
        return /* continue */ true;

    if (*cltNow == *cltToMatch_) {
        cltClassMatch_ = true;
        bestMatch_ = fld;
        return /* continue */ true;
    }
    else if (cltClassMatch_)
        return /* continue */ true;

    if (!isDataPtr(cltNow) || !isDataPtr(cltToMatch_))
        return /* continue */ true;

    // at least both are _data_ pointers at this point, update best match
    bestMatch_ = fld;
    return /* continue */ true;
}

TFldId SymHeapCore::fldLookup(TObjId obj, TOffset off, TObjType clt)
{
    FieldMatchPolicy policy(clt);
    return d->fldLookup(obj, off, clt, &policy);
}

void SymHeapCore::fldEnter(TFldId fld)
{
    FieldOfObj *fldData;
    d->ents.getEntRW(&fldData, fld);
    CL_BREAK_IF(fldData->extRefCnt < 0);
    ++(fldData->extRefCnt);
}

void SymHeapCore::fldLeave(TFldId fld)
{
    FieldOfObj *fldData;
    d->ents.getEntRW(&fldData, fld);
    CL_BREAK_IF(fldData->extRefCnt < 1);
    if (--(fldData->extRefCnt))
        // still externally referenced
        return;

#if SH_DELAYED_FIELDS_DESTRUCTION
    return;
#endif

    if (isComposite(fldData->clt, /* includingArray */ false)
            && VAL_INVALID != fldData->value)
    {
        CL_DEBUG("SymHeapCore::fldLeave() preserves a composite field");
        return;
    }

    // jump to region
    const Region *regData;
    d->ents.getEntRO(&regData, fldData->obj);

    if (!hasKey(regData->liveFields, fld)) {
        CL_DEBUG("SymHeapCore::fldLeave() destroys a dead field");
        d->fldDestroy(fld, /* removeVal */ true, /* detach */ true);
    }

    // TODO: pack the representation if possible
}

CVar SymHeapCore::cVarByObject(TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->cVar;
}

TObjId SymHeapCore::regionByVar(CVar cv, bool createIfNeeded)
{
    TObjId obj = d->cVarMap->find(cv);
    if (OBJ_INVALID != obj)
        return obj;

    if (!createIfNeeded)
        // the variable does not exist and we are not asked to create the var
        return OBJ_INVALID;

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

    // create an object
    const EStorageClass code = isOnStack(var) ? SC_ON_STACK : SC_STATIC;
    obj = d->assignId(new Region(code));
    Region *rootData;
    d->ents.getEntRW(&rootData, obj);

    // initialize metadata
    rootData->cVar = cv;
    rootData->lastKnownClt = clt;

    // read size from the type-info
    const TSizeOf size = clt->size;
    rootData->size = IR::rngFromNum(size);

    // mark the root as live
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveObjs);
    d->liveObjs->insert(obj);

    // store the address for next wheel
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->cVarMap);
    d->cVarMap->insert(cv, obj);
    return obj;
}

static bool dummyFilter(EStorageClass)
{
    return true;
}

void SymHeapCore::gatherObjects(TObjList &dst, bool (*filter)(EStorageClass))
    const
{
    if (!filter)
        filter = dummyFilter;

    const TObjSetWrapper &liveObjs = *d->liveObjs;
    BOOST_FOREACH(const TObjId obj, liveObjs) {
        CL_BREAK_IF(!this->isValid(obj));

        const EStorageClass code = this->objStorClass(obj);
        if (!filter(code))
            continue;

        dst.push_back(obj);
    }
}

void SymHeapCore::clearAnonStackObjects(TObjList &dst, const CallInst &of)
{
    CL_BREAK_IF(!dst.empty());

    TAnonStackMap &sMap = *d->anonStackMap;
    const TAnonStackMap::iterator it = sMap.find(of);
    if (sMap.end() == it)
        return;

    // return the list of anonymous stack objects
    dst = it->second;

    // clear the list of anonymous stack objects
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->anonStackMap);
    sMap.erase(it);
}

TFldId SymHeapCore::valGetComposite(TValId val) const
{
    const BaseValue *valData;
    d->ents.getEntRO(&valData, val);
    CL_BREAK_IF(VT_COMPOSITE != valData->code);

    const CompValue *compData = DCAST<const CompValue *>(valData);
    return compData->compObj;
}

TObjId SymHeapCore::stackAlloc(const TSizeRange &size, const CallInst &from)
{
    CL_BREAK_IF(size.lo <= IR::Int0);

    // create an object
    const TObjId reg = d->assignId(new Region(SC_ON_STACK));
    Region *rootData;
    d->ents.getEntRW(&rootData, reg);

    // initialize meta-data
    rootData->size = size;

    // append the object on the list of anonymous stack objects of the call
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->anonStackMap);
    (*d->anonStackMap)[from].push_back(reg);

    return reg;
}

TObjId SymHeapCore::heapAlloc(const TSizeRange &size)
{
    CL_BREAK_IF(size.lo < IR::Int0);

    // create an object
    const TObjId reg = d->assignId(new Region(SC_ON_HEAP));
    Region *rootData;
    d->ents.getEntRW(&rootData, reg);

    // mark the root as live
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveObjs);
    d->liveObjs->insert(reg);

    // initialize meta-data
    rootData->size = size;

    return reg;
}

bool SymHeapCore::isValid(TObjId obj) const {
    if (!d->ents.isValidEnt(obj))
        return false;

    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->isValid;
}

TSizeRange SymHeapCore::objSize(TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->size;
}

TSizeRange SymHeapCore::valSizeOfString(TValId addr) const
{
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

    if (!isPossibleToDeref(*this, addr))
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

void SymHeapCore::objSetEstimatedType(TObjId obj, TObjType clt)
{
    Region *rootData;
    d->ents.getEntRW(&rootData, obj);

    if (OBJ_RETURN == obj) {
        // destroy any stale OBJ_RETURN object
        this->objInvalidate(OBJ_RETURN);

        // reinitalize the OBJ_RETURN region
        rootData->isValid = true;
        rootData->size    = IR::rngFromNum(clt->size);
    }

    // convert a type-free object into a type-aware object
    rootData->lastKnownClt = clt;
}

TObjType SymHeapCore::objEstimatedType(TObjId obj) const
{
    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->lastKnownClt;
}

void SymHeapCore::objInvalidate(TObjId obj)
{
    CL_BREAK_IF(OBJ_RETURN != obj && !this->isValid(obj));

    // mark the region as invalid
    Region *rootData;
    d->ents.getEntRW(&rootData, obj);
    rootData->isValid = false;

    if (OBJ_RETURN == obj)
        rootData->lastKnownClt = 0;

    const CVar cv = rootData->cVar;
    if (cv.uid != /* heap object */ -1) {
        // remove the corresponding program variable
        RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->cVarMap);
        d->cVarMap->remove(cv);
    }

    // release the root
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d->liveObjs);
    d->liveObjs->erase(obj);

    const TSizeRange size = rootData->size;
    if (IR::Int0 < size.hi) {
        // look for inner objects
        const TMemChunk chunk(0, size.hi);
        TFldIdSet allObjs;
        if (arenaLookup(&allObjs, rootData->arena, chunk, FLD_INVALID)) {
            // destroy all inner objects
            BOOST_FOREACH(const TFldId fld, allObjs)
                d->fldDestroy(fld, /* removeVal */ true, /* detach */ false);
        }
    }

    // drop all outgoing has-value edges
    rootData->liveFields.clear();
    rootData->arena.clear();
}

TValId SymHeapCore::valCreate(EValueTarget code, EValueOrigin origin)
{
    switch (code) {
        default:
            CL_BREAK_IF("invalid call of SymHeapCore::valCreate()");
            // fall through!

        case VT_RANGE:
            // VT_RANGE pointing to a freed object is replaced by VT_UNKNOWN
            code = VT_UNKNOWN;
            // fall through!

        case VT_UNKNOWN:
            break;
    }

    return d->valCreate(code, origin);
}

TValId SymHeapCore::valWrapCustom(CustomValue cVal)
{
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

        // VAL_FALSE is not supposed to be wrapped as custom values
        CL_BREAK_IF(IR::Int0 == rng.lo);
    }

    // check the consistency of backward mapping
    CL_BREAK_IF(val != d->cValueMap->lookup(valData->customData));

    return cv;
}

TProtoLevel SymHeapCore::objProtoLevel(TObjId obj) const
{
    if (obj <= 0)
        // not a prototype for sure
        return 0;

    const Region *regData;
    d->ents.getEntRO(&regData, obj);
    return regData->protoLevel;
}

void SymHeapCore::objSetProtoLevel(TObjId obj, TProtoLevel level)
{
    CL_BREAK_IF(OBJ_INVALID == obj);

    Region *regData;
    d->ents.getEntRW(&regData, obj);
    regData->protoLevel = level;
}

bool SymHeapCore::chkNeq(TValId v1, TValId v2) const
{
    return d->neqDb->chk(v1, v2);
}


// /////////////////////////////////////////////////////////////////////////////
// implementation of SymHeap
struct AbstractObject {
    RefCounter                      refCnt;

    EObjKind                        kind;
    BindingOff                      bOff;
    TMinLen                         minLength;

    AbstractObject(EObjKind kind_, BindingOff bOff_):
        kind(kind_),
        bOff(bOff_),
        minLength(0)
    {
    }

    AbstractObject* clone() const {
        return new AbstractObject(*this);
    }
};

struct SymHeap::Private {
    RefCounter                      refCnt;
    EntStore<AbstractObject>        absRoots;
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

SymHeap::~SymHeap()
{
    RefCntLib<RCO_NON_VIRT>::leave(d);
}

SymHeap& SymHeap::operator=(const SymHeap &ref)
{
    SymHeapCore::operator=(ref);

    RefCntLib<RCO_NON_VIRT>::leave(d);

    d = ref.d;
#if defined(__GNUC_MINOR__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 1)
    // work around optimization bug <https://bugzilla.redhat.com/917378>
    asm volatile("" ::: "memory");
#endif
    RefCntLib<RCO_NON_VIRT>::enter(d);

    return *this;
}

void SymHeap::swap(SymHeapCore &baseRef)
{
    // swap base
    SymHeapCore::swap(baseRef);

    // swap self
    SymHeap &ref = DCAST<SymHeap &>(baseRef);
    swapValues(this->d, ref.d);
}

TObjId SymHeap::objClone(TObjId obj)
{
    const TObjId dup = SymHeapCore::objClone(obj);
    if (!d->absRoots.isValidEnt(obj))
        return dup;

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // clone the data
    const AbstractObject *tplData = d->absRoots.getEntRO(obj);
    AbstractObject *dupData = tplData->clone();
    d->absRoots.assignId(dup, dupData);

    return dup;
}

EObjKind SymHeap::objKind(TObjId obj) const
{
    if (!d->absRoots.isValidEnt(obj))
        return OK_REGION;

    const AbstractObject *aData = d->absRoots.getEntRO(obj);
    return aData->kind;
}

const BindingOff& SymHeap::segBinding(TObjId seg) const
{
    const AbstractObject *aData = d->absRoots.getEntRO(seg);
    CL_BREAK_IF(OK_OBJ_OR_NULL == aData->kind);
    return aData->bOff;
}

void SymHeap::objSetAbstract(
        TObjId                      obj,
        EObjKind                    kind,
        const BindingOff           &off)
{
    CL_BREAK_IF(OK_REGION == kind);

    // there is no 'prev' offset in OK_SEE_THROUGH
    CL_BREAK_IF(OK_SEE_THROUGH == kind && off.prev != off.next);

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    if (d->absRoots.isValidEnt(obj)) {
        // the object already exists, just update its properties
        AbstractObject *aData = d->absRoots.getEntRW(obj);
        aData->kind = kind;
        aData->bOff = off;
        return;
    }

    AbstractObject *aData = new AbstractObject(kind, (OK_OBJ_OR_NULL == kind)
            ? BindingOff(OK_OBJ_OR_NULL)
            : off);

    // register a new abstract object
    d->absRoots.assignId(obj, aData);
}

void SymHeap::objSetConcrete(TObjId obj)
{
    CL_DEBUG("SymHeap::objSetConcrete() is taking place...");
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // unregister an abstract object
    d->absRoots.releaseEnt(obj);
}

void SymHeap::objInvalidate(TObjId obj)
{
    SymHeapCore::objInvalidate(obj);
    if (!d->absRoots.isValidEnt(obj))
        return;

    CL_DEBUG("SymHeap::objInvalidate() destroys an abstract object");

    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    // unregister an abstract object
    d->absRoots.releaseEnt(obj);
}

TMinLen SymHeap::segMinLength(TObjId seg) const
{
    const AbstractObject *aData = d->absRoots.getEntRO(seg);

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

void SymHeap::segSetMinLength(TObjId seg, TMinLen len)
{
    RefCntLib<RCO_NON_VIRT>::requireExclusivity(d);

    AbstractObject *aData = d->absRoots.getEntRW(seg);

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
}
