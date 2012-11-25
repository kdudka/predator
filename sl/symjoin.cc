/*
 * Copyright (C) 2010-2012 Kamil Dudka <kdudka@redhat.com>
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
#include "symjoin.hh"

#include <cl/cl_msg.hh>
#include <cl/cldebug.hh>
#include <cl/clutil.hh>

#include "prototype.hh"
#include "symcmp.hh"
#include "symgc.hh"
#include "symplot.hh"
#include "symseg.hh"
#include "symutil.hh"
#include "symtrace.hh"
#include "worklist.hh"
#include "util.hh"

#include <iomanip>
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

static bool debuggingSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debuggingSymJoin)                                                 \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

void debugSymJoin(const bool enable)
{
    if (enable == ::debuggingSymJoin)
        return;

    CL_DEBUG("symjoin: debugSymJoin(" << enable << ") takes effect");
    ::debuggingSymJoin = enable;
}

#define SJ_FLDP(f1, f2) "(f1 = #" << f1 << ", f2 = #" << f2 << ")"
#define SJ_OBJP(o1, o2) "(o1 = #" << o1 << ", o2 = #" << o2 << ")"
#define SJ_VALP(v1, v2) "(v1 = #" << v1 << ", v2 = #" << v2 << ")"

static int cntJoinOps = -1;

namespace {
    void debugPlot(
            const SymHeap       &sh,
            const char          *name,
            const int           dst,
            const int           src,
            const char          *suffix)
    {
        if (!::debuggingSymJoin)
            return;

        std::ostringstream str;
        str << "symjoin-" << FIXW(6) << ::cntJoinOps
            << "-" << name << "-"
            << FIXW(3) << dst << "-"
            << FIXW(3) << src << "-"
            << suffix;

        plotHeap(sh, str.str().c_str());
    }
}

template <class T>
class WorkListWithUndo: public WorkList<T> {
    private:
        typedef WorkList<T> TBase;

    public:
        /// push an @b already @b processed item back to WorkList
        void undo(const T &item) {
            CL_BREAK_IF(!hasKey(TBase::seen_, item));
            TBase::todo_.push(item);
        }
};

struct SchedItem {
    TValId              v1;
    TValId              v2;
    TProtoLevel         ldiff;

    SchedItem():
        v1(VAL_INVALID),
        v2(VAL_INVALID),
        ldiff(0)
    {
    }

    SchedItem(TValId v1_, TValId v2_, TProtoLevel ldiff_):
        v1(v1_),
        v2(v2_),
        ldiff(ldiff_)
    {
    }

    operator TValPair() const {
        return TValPair(v1, v2);
    }
};

// needed by std::set
inline bool operator<(const SchedItem &a, const SchedItem &b)
{
    if (a.v1 < b.v1)
        return true;
    if (b.v1 < a.v1)
        return false;

    if (a.v2 < b.v2)
        return true;
    if (b.v2 < a.v2)
        return false;

    return (a.ldiff < b.ldiff);
}

typedef WorkListWithUndo<SchedItem>                             TWorkList;

typedef TObjMap                                                 TObjMapBidir[2];

/// current state, common for joinSymHeaps(), joinDataReadOnly() and joinData()
struct SymJoinCtx {
    SymHeap                     &dst;
    SymHeap                     &sh1;
    SymHeap                     &sh2;

    // they need to be black-listed for joinAbstractValues()
    TValSet                     sset1;
    TValSet                     sset2;

    FldList                     liveList1;
    FldList                     liveList2;

    TValMapBidir                valMap1;
    TValMapBidir                valMap2;

    TObjMapBidir                objMap1;
    TObjMapBidir                objMap2;

    TWorkList                   wl;
    EJoinStatus                 status;
    bool                        forceThreeWay;
    bool                        allowThreeWay;

    typedef std::map<TObjId /* seg */, TMinLen /* len */>       TSegLengths;
    TSegLengths                 segLengths;
    std::set<TValPair>          sharedNeqs;

    std::set<TValPair>          tieBreaking;
    std::set<TValPair>          alreadyJoined;

    std::set<TObjId /* dst */>  protos;

    // XXX: experimental
    typedef std::map<TValPair /* (v1, v2) */, TValId /* dst */> TMatchLookup;
    TMatchLookup                matchLookup;

    void initValMaps() {
        // VAL_NULL should be always mapped to VAL_NULL
        valMap1[0][VAL_NULL] = VAL_NULL;
        valMap1[1][VAL_NULL] = VAL_NULL;
        valMap2[0][VAL_NULL] = VAL_NULL;
        valMap2[1][VAL_NULL] = VAL_NULL;
    }

    /// constructor used by joinSymHeaps()
    SymJoinCtx(SymHeap &dst_, SymHeap &sh1_, SymHeap &sh2_,
            const bool allowThreeWay_):
        dst(dst_),
        sh1(sh1_),
        sh2(sh2_),
        status(JS_USE_ANY),
        forceThreeWay(false),
        allowThreeWay((1 < (SE_ALLOW_THREE_WAY_JOIN)) && allowThreeWay_)
    {
        initValMaps();
    }

    /// constructor used by joinDataReadOnly()
    SymJoinCtx(SymHeap &tmp_, SymHeap &sh_):
        dst(tmp_),
        sh1(sh_),
        sh2(sh_),
        status(JS_USE_ANY),
        forceThreeWay(false),
        allowThreeWay(0 < (SE_ALLOW_THREE_WAY_JOIN))
    {
        initValMaps();
    }

    /// constructor used by joinData()
    SymJoinCtx(SymHeap &sh_):
        dst(sh_),
        sh1(sh_),
        sh2(sh_),
        status(JS_USE_ANY),
        forceThreeWay(false),
        allowThreeWay(0 < (SE_ALLOW_THREE_WAY_JOIN))
    {
        initValMaps();
    }

    bool joiningData() const {
        return (&sh1 == &sh2);
    }

    bool joiningDataReadWrite() const {
        return (&dst == &sh1)
            && (&dst == &sh2);
    }
};

/// handy when debugging
void dump_ctx(const SymJoinCtx &ctx)
{
    using std::cout;

    // plot heaps
    if (!ctx.joiningDataReadWrite()) {
        cout << "    plotting ctx.sh1...\n";
        plotHeap(ctx.sh1, "dump_ctx");
    }
    if (!ctx.joiningData()) {
        cout << "    plotting ctx.sh2...\n";
        plotHeap(ctx.sh2, "dump_ctx");
    }
    cout << "    plotting ctx.dst...\n";
    plotHeap(ctx.dst, "dump_ctx");

    // print entry-point
    cout << "\ndump_ctx: ";
    if (ctx.joiningDataReadWrite())
        cout << "joinData()\n";
    else if (ctx.joiningData())
        cout << "joinDataReadOnly()\n";
    else
        cout << "joinSymHeaps()\n";

    // summarize mapping
    cout << "    ctx.liveList1      .size() = " << ctx.liveList1.size() << "\n";
    cout << "    ctx.liveList2      .size() = " << ctx.liveList2.size()
        << "\n\n";
    cout << "    ctx.valMap1[0]     .size() = " << (ctx.valMap1[0].size() - 1)
        << "\n";
    cout << "    ctx.valMap2[0]     .size() = " << (ctx.valMap2[0].size() - 1)
        << "\n\n";

    // sumarize aux containers
    cout << "    ctx.segLengths     .size() = " << ctx.segLengths.size()
        << "\n";
    cout << "    ctx.sharedNeqs     .size() = " << ctx.sharedNeqs.size()
        << "\n";
    cout << "    ctx.alreadyJoined  .size() = " << ctx.alreadyJoined.size()
        << "\n";
    cout << "    ctx.protos         .size() = " << ctx.protos.size()
        << "\n\n";

    // print queue stats
    cout << "    ctx.wl          .cntSeen() = " << ctx.wl.cntSeen() << "\n";
    cout << "    ctx.wl          .cntTodo() = " << ctx.wl.cntTodo() << "\n\n";

    // print the current status
    cout << "    ctx.status = " << ctx.status << "\n";
}

/// update ctx.status according to action
bool updateJoinStatus(SymJoinCtx &ctx, const EJoinStatus action)
{
    if (JS_USE_ANY == action)
        return true;

    EJoinStatus &status = ctx.status;
    switch (status) {
        case JS_THREE_WAY:
            break;

        case JS_USE_ANY:
            status = action;
            break;

        default:
            if (action != status)
                status = JS_THREE_WAY;
    }

    return (JS_THREE_WAY != status)
        || ctx.forceThreeWay
        || ctx.allowThreeWay;
}

bool defineObjectMapping(
        SymJoinCtx              &ctx,
        const TObjId            objDst,
        const TObjId            obj1,
        const TObjId            obj2)
{
    const bool hasObj1 = (OBJ_INVALID != obj1);
    const bool hasObj2 = (OBJ_INVALID != obj2);
    CL_BREAK_IF(!hasObj1 && !hasObj2);

    const bool ok1 = !hasObj1 || mapBidir(ctx.objMap1, obj1, objDst);
    const bool ok2 = !hasObj2 || mapBidir(ctx.objMap2, obj2, objDst);

    if (ok1 && ok2)
        return true;

    SJ_DEBUG("<-- object mapping mismatch " << SJ_OBJP(obj1, obj2));
    return false;
}

/**
 * if Neq(v1, vDst) exists in ctx.sh1 and Neq(v2, vDst) exists in ctx.sh2,
 * declare the Neq relation as @b shared, such that it later appears in ctx.dst
 * @note it respects value ID mapping among all symbolic heaps
 */
void gatherSharedPreds(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    // look for shared Neq predicates
    TValList rVals1;
    ctx.sh1.gatherRelatedValues(rVals1, v1);
    BOOST_FOREACH(const TValId rel1, rVals1) {
        if (!ctx.sh1.chkNeq(v1, rel1))
            // not a Neq in sh1
            continue;

        TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
        TValMap::const_iterator it1 = vMap1.find(rel1);
        if (vMap1.end() == it1)
            // related value has not (yet?) any mapping to dst
            continue;

        const TValId relDst = it1->second;
        TValMap &vMap2r = ctx.valMap2[/* rtl */ 1];
        TValMap::const_iterator it2r = vMap2r.find(relDst);
        if (vMap2r.end() == it2r)
            // related value has not (yet?) any mapping back to sh2
            continue;

        const TValId rel2 = it2r->second;
        if (!ctx.sh2.chkNeq(v2, rel2))
            // not a Neq in sh2
            continue;

        // sort Neq values
        TValId valLt = vDst;
        TValId valGt = relDst;
        sortValues(valLt, valGt);

        // insert a shared Neq predicate
        const TValPair neq(valLt, valGt);
        ctx.sharedNeqs.insert(neq);
    }
}

/// define value mapping for the given value triple (v1, v2, vDst)
bool defineValueMapping(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    const bool hasValue1 = (VAL_INVALID != v1);
    const bool hasValue2 = (VAL_INVALID != v2);
    CL_BREAK_IF(!hasValue1 && !hasValue2);

    const bool ok1 = !hasValue1
        || matchPlainValues(ctx.valMap1, ctx.sh1, ctx.dst, v1, vDst);

    const bool ok2 = !hasValue2
        || matchPlainValues(ctx.valMap2, ctx.sh2, ctx.dst, v2, vDst);

    if (!ok1 || !ok2) {
        SJ_DEBUG("<-- value mapping mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (hasValue1 && hasValue2)
        gatherSharedPreds(ctx, v1, v2, vDst);

    return true;
}

bool matchRanges(
        bool                    *pResult,
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              allowUnknownMapping)
{
    const bool isRange1 = (VT_RANGE == ctx.sh1.valTarget(v1));
    const bool isRange2 = (VT_RANGE == ctx.sh2.valTarget(v2));
    if (isRange1 || isRange2) {
        if (allowUnknownMapping)
            return false;

        // the mapping should have already been defined
        const TValPair vp(v1, v2);
        if (hasKey(ctx.matchLookup, vp))
            return false;

        goto fail;
    }

    // no VT_RANGE values involved here
    if (matchOffsets(ctx.sh1, ctx.sh2, v1, v2))
        return false;

fail:
    *pResult = false;
    return true;
}

bool joinRangeValues(
        SymJoinCtx             &ctx,
        const TValId            v1,
        const TValId            v2)
{
    // check whether the values are not matched already
    const TValPair vp(v1, v2);
    CL_BREAK_IF(hasKey(ctx.matchLookup, vp));

    const IR::Range rng1 = ctx.sh1.valOffsetRange(v1);
    const IR::Range rng2 = ctx.sh2.valOffsetRange(v2);

    // resolve root in ctx.dst
    const TValId rootDst = roMapLookup(ctx.valMap1[0], ctx.sh1.valRoot(v1));
    CL_BREAK_IF(rootDst != roMapLookup(ctx.valMap2[0], ctx.sh2.valRoot(v2)));

    // compute the join of ranges
    IR::Range rng = join(rng1, rng2);

    // [experimental] widening on offset ranges
    if (!isSingular(rng1) && !isSingular(rng2)) {
#if (SE_ALLOW_OFF_RANGES & 0x2)
        if (rng.lo == rng1.lo || rng.lo == rng2.lo)
            rng.hi = IR::IntMax;
#endif
#if (SE_ALLOW_OFF_RANGES & 0x4)
        if (rng.hi == rng1.hi || rng.hi == rng2.hi)
            rng.lo = IR::IntMin;
#endif
    }

    if (!isCovered(rng, rng1) && !updateJoinStatus(ctx, JS_USE_SH2))
        return false;
    if (!isCovered(rng, rng2) && !updateJoinStatus(ctx, JS_USE_SH1))
        return false;

    // create a VT_RANGE value in ctx.dst
    const TValId vDst = ctx.dst.valByRange(rootDst, rng);

    // store the mapping (v1, v2) -> vDst
    ctx.matchLookup[vp] = vDst;
    return true;
}

/// read-only (in)consistency check among value pair (v1, v2)
bool checkValueMapping(
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              allowUnknownMapping)
{
    if (!checkNonPosValues(v1, v2))
        return false;

    bool result;
    if (matchRanges(&result, ctx, v1, v2, allowUnknownMapping))
        return result;

    // read-only value lookup
    const TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
    const TValMap &vMap2 = ctx.valMap2[/* ltr */ 0];
    TValMap::const_iterator i1 = vMap1.find(ctx.sh1.valRoot(v1));
    TValMap::const_iterator i2 = vMap2.find(ctx.sh2.valRoot(v2));

    const bool hasMapping1 = (vMap1.end() != i1);
    const bool hasMapping2 = (vMap2.end() != i2);
    if (!hasMapping1 && !hasMapping2)
        // we have not enough info yet
        return allowUnknownMapping;

    const TValId vDst1 = (hasMapping1) ? i1->second : VAL_INVALID;
    const TValId vDst2 = (hasMapping2) ? i2->second : VAL_INVALID;

    if (hasMapping1 && hasMapping2 && (vDst1 == vDst2))
        // mapping already known and known to be consistent
        return true;

    if (allowUnknownMapping) {
        SJ_DEBUG("<-- value mapping mismatch: " << SJ_VALP(v1, v2)
                 "-> " << SJ_VALP(vDst1, vDst2));
    }

    return false;
}

bool checkNullConsistency(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
    const bool isNull1 = (VAL_NULL == v1);
    const bool isNull2 = (VAL_NULL == v2);
    CL_BREAK_IF(isNull1 == isNull2);

    if (v1 < 0 || v2 < 0)
        // VAL_NULL vs. something special
        return false;

    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);

    // special quirk for off-values related to VAL_NULL
    if (isNull1 && VAL_NULL == root2)
        return false;
    if (isNull2 && VAL_NULL == root1)
        return false;

    // check for inconsistency with the up to now mapping of values
    if (isNull1 && hasKey(ctx.valMap2[/* lrt */ 0], root2))
        return false;
    if (isNull2 && hasKey(ctx.valMap1[/* lrt */ 0], root1))
        return false;

    const EValueTarget code = (isNull2)
        ? ctx.sh1.valTarget(v1)
        : ctx.sh2.valTarget(v2);

    if (VT_UNKNOWN == code && !ctx.joiningData())
        // [experimental] reduce state explosion on test-0300
        return !ctx.joiningData();

    return isAnyDataArea(code);
}

bool joinValuesByCode(
        bool                   *pResult,
        SymJoinCtx             &ctx,
        const TValId            v1,
        const TValId            v2);

bool bumpNestingLevel(const FldHandle &fld)
{
    if (!fld.isValidHandle())
        return false;

    // resolve root (the owning object of this field)
    SymHeap &sh = *static_cast<SymHeap *>(fld.sh());
    const TObjId obj = fld.obj();

    if (OK_REGION == sh.objKind(obj))
        // do not bump nesting level on concrete objects
        return false;

    TFldSet ignoreList;
    buildIgnoreList(ignoreList, sh, obj);
    return !hasKey(ignoreList, fld);
}

/// (FLD_INVALID == objDst) means read-only!!!
bool joinFreshObjTripple(
        SymJoinCtx              &ctx,
        const FldHandle         &fld1,
        const FldHandle         &fld2,
        const FldHandle         &fldDst,
        TProtoLevel             ldiff)
{
    const bool segClone = (!fld1.isValidHandle() || !fld2.isValidHandle());
    const bool readOnly = (!fldDst.isValidHandle());
    CL_BREAK_IF(segClone && readOnly);
    CL_BREAK_IF(!fld1.isValidHandle() && !fld2.isValidHandle());

    const TValId v1 = fld1.value();
    const TValId v2 = fld2.value();
    if (VAL_NULL == v1 && VAL_NULL == v2)
        // both values are VAL_NULL, nothing more to join here
        return true;

    if (hasKey(ctx.alreadyJoined, TValPair(v1, v2)))
        // the join has been already successful
        return true;

    if (VAL_NULL == v1 || VAL_NULL == v2) {
        if (segClone)
            // we got only one value and the value is VAL_NULL
            return true;

        if (!checkNullConsistency(ctx, v1, v2))
            // mapping already inconsistent
            return false;

        if (readOnly)
            // act optimistically for now
            return true;
    }

    if (readOnly)
        return checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ true);

    if (segClone) {
        const bool isGt1 = !fld2.isValidHandle();
        const TValMapBidir &vm = (isGt1) ? ctx.valMap1 : ctx.valMap2;
        const SymHeap &shGt = (isGt1) ? ctx.sh1 : ctx.sh2;
        const TValId valGt = (isGt1) ? v1 : v2;
        const TValId root = shGt.valRoot(valGt);
        const EValueTarget code = shGt.valTarget(valGt);

        if (root <= 0 || (VT_RANGE != code && hasKey(vm[/* ltr */ 0], root)))
            return true;

#if SE_ALLOW_THREE_WAY_JOIN < 3
        if (!ctx.joiningData())
            return false;
#endif
    }
    else {
        // special values have to match (NULL not treated as special here)
        if (v1 < 0 || v2 < 0) {
            if (v1 == v2)
                return true;

            SJ_DEBUG("<-- special value mismatch " << SJ_VALP(v1, v2));
            return false;
        }

        if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
            return true;

        bool result;
        if (joinValuesByCode(&result, ctx, v1, v2))
            return result;
    }

    if (bumpNestingLevel(fld1))
        ++ldiff;
    if (bumpNestingLevel(fld2))
        --ldiff;

    const SchedItem item(v1, v2, ldiff);
    if (ctx.wl.schedule(item))
        SJ_DEBUG("+++ " << SJ_VALP(v1, v2)
                << " <- " << SJ_FLDP(fld1.fieldId(), fld2.fieldId())
                << ", ldiff = " << ldiff);

    return true;
}

struct ObjJoinVisitor {
    SymJoinCtx              &ctx;
    const TProtoLevel       ldiff;
    TFldSet                 blackList1;
    TFldSet                 blackList2;
    bool                    noFollow;

    ObjJoinVisitor(SymJoinCtx &ctx_, const TProtoLevel ldiff_):
        ctx(ctx_),
        ldiff(ldiff_),
        noFollow(false)
    {
    }

    bool operator()(const FldHandle item[3]) {
        const FldHandle &fld1   = item[0];
        const FldHandle &fld2   = item[1];
        const FldHandle &fldDst = item[2];

        if (fld1.isValidHandle())
            ctx.liveList1.push_back(fld1);

        if (fld2.isValidHandle())
            ctx.liveList2.push_back(fld2);

        // check black-list
        if (hasKey(blackList1, fld1) || hasKey(blackList2, fld2))
            return /* continue */ true;

        return joinFreshObjTripple(ctx, fld1, fld2, fldDst, ldiff);
    }
};

template <class TDst>
void dlSegBlackListPrevPtr(TDst &dst, SymHeap &sh, TObjId obj)
{
    const EObjKind kind = sh.objKind(obj);
    if (OK_DLS != kind)
        return;

    const PtrHandle prevPtr = prevPtrFromSeg(sh, obj);
    dst.insert(prevPtr);
}

struct SegMatchVisitor {
    SymJoinCtx              &ctx;
    const TProtoLevel       ldiff;
    TFldSet                 blackList1;
    TFldSet                 blackList2;

    public:
        SegMatchVisitor(SymJoinCtx &ctx_, const TProtoLevel ldiff_):
            ctx(ctx_),
            ldiff(ldiff_)
        {
        }

        bool operator()(const FldHandle item[2]) {
            const FldHandle &fld1 = item[0];
            const FldHandle &fld2 = item[1];

            if (hasKey(blackList1, fld1) || hasKey(blackList2, fld2))
                // black-listed
                return true;

            const FldHandle readOnly(FLD_INVALID);
            return joinFreshObjTripple(ctx, fld1, fld2, readOnly, ldiff);
        }
};

bool joinFields(
        SymJoinCtx             &ctx,
        const TObjId            objDst,
        const TObjId            obj1,
        const TObjId            obj2,
        const TProtoLevel       ldiff,
        const BindingOff       *offBlackList = 0)
{
    // TODO: drop this!
    const TValId addr1   = ctx.sh1.addrOfTarget(obj1  , /* XXX */ TS_REGION);
    const TValId addr2   = ctx.sh2.addrOfTarget(obj2  , /* XXX */ TS_REGION);
    const TValId addrDst = ctx.dst.addrOfTarget(objDst, /* XXX */ TS_REGION);
    if (!defineValueMapping(ctx, addr1, addr2, addrDst))
        return false;

    if (!defineObjectMapping(ctx, objDst, obj1, obj2))
        return false;

    // initialize visitor
    ObjJoinVisitor objVisitor(ctx, ldiff);
    dlSegBlackListPrevPtr(objVisitor.blackList1, ctx.sh1, obj1);
    dlSegBlackListPrevPtr(objVisitor.blackList2, ctx.sh2, obj2);

    if (offBlackList) {
        buildIgnoreList(objVisitor.blackList1, ctx.sh1, obj1, *offBlackList);
        buildIgnoreList(objVisitor.blackList2, ctx.sh2, obj2, *offBlackList);
    }
    else if (ctx.joiningData()) {
        if (obj1 == obj2)
            // do not follow shared data
            return true;
        else
            ctx.protos.insert(objDst);
    }

    const TObjId objs[] = {
        obj1,
        obj2,
        objDst
    };

    SymHeap *const heaps[] = {
        &ctx.sh1,
        &ctx.sh2,
        &ctx.dst
    };

    // guide the visitors through them
    return traverseLiveFieldsGeneric<3>(heaps, objs, objVisitor);
}

bool segMatchLookAhead(
        SymJoinCtx              &ctx,
        const SchedItem         &item)
{
    const TObjId obj1 = ctx.sh1.objByAddr(item.v1);
    const TObjId obj2 = ctx.sh2.objByAddr(item.v2);

    const TSizeRange size1 = ctx.sh1.objSize(obj1);
    const TSizeRange size2 = ctx.sh2.objSize(obj2);
    if (size1 != size2)
        // size mismatch
        return false;

    // set up a visitor
    SymHeap *const heaps[] = { &ctx.sh1, &ctx.sh2 };
    TObjId objs[] = { obj1, obj2 };
    SegMatchVisitor visitor(ctx, item.ldiff);

    dlSegBlackListPrevPtr(visitor.blackList1, ctx.sh1, obj1);
    dlSegBlackListPrevPtr(visitor.blackList2, ctx.sh2, obj2);

    return traverseLiveFieldsGeneric<2>(heaps, objs, visitor);
}

bool joinClt(
        const struct cl_type    **pDst,
        const struct cl_type    *clt1,
        const struct cl_type    *clt2)
{
    const struct cl_type *sink;
    if (!pDst)
        pDst = &sink;

    const bool anon1 = !clt1;
    const bool anon2 = !clt2;
    if (anon1 && anon2) {
        *pDst = 0;
        return true;
    }

    if (anon1 != anon2)
        return false;

    CL_BREAK_IF(anon1 || anon2);
    if (*clt1 != *clt2)
        return false;

    *pDst = clt1;
    return true;
}

TObjType joinClt(
        SymJoinCtx              &ctx,
        const TObjType          clt1,
        const TObjType          clt2)
{
    TObjType clt;
    if (joinClt(&clt, clt1, clt2))
        // symmetric join of type-info
        return clt;

    if (clt2)
        updateJoinStatus(ctx, JS_USE_SH1);
    else if (clt1)
        updateJoinStatus(ctx, JS_USE_SH2);
    else
        // not reachable
        CL_BREAK_IF("joinClt() malfunction");

    // type-info is going to be abstracted out
    return 0;
}

bool joinObjKind(
        EObjKind               *pDst,
        const SymJoinCtx       &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const EJoinStatus       action)
{
    CL_BREAK_IF(OBJ_INVALID == obj1 && OBJ_INVALID == obj2);

    const EObjKind kind1 = ctx.sh1.objKind(obj1);
    if (OBJ_INVALID == obj2) {
        *pDst = kind1;
        return true;
    }

    const EObjKind kind2 = ctx.sh2.objKind(obj2);
    if (OBJ_INVALID == obj1) {
        *pDst = kind2;
        return true;
    }

    if (OK_REGION == kind1) {
        CL_BREAK_IF(action == JS_USE_SH1);
        *pDst = kind2;
        return true;
    }

    if (OK_REGION == kind2) {
        CL_BREAK_IF(action == JS_USE_SH2);
        *pDst = kind1;
        return true;
    }

    if (kind1 == kind2) {
        CL_BREAK_IF(action != JS_USE_ANY);
        *pDst = kind1;
        return true;
    }

    switch (action) {
        case JS_USE_SH1:
            *pDst = kind1;
            return true;

        case JS_USE_SH2:
            *pDst = kind2;
            return true;

        default:
            SJ_DEBUG("<-- object kind mismatch " << SJ_OBJP(obj1, obj2));
            return false;
    }
}

bool matchBindingFieldsByValue(
        SymHeap                 &sh,
        const TObjId             obj,
        const BindingOff        &off1,
        const BindingOff        &off2)
{
        const TValId valNextBy1 = valOfPtr(sh, obj, off1.next);
        const TValId valNextBy2 = valOfPtr(sh, obj, off2.next);
        if (valNextBy1 != valNextBy2)
            return false;

        const TValId valPrevBy1 = valOfPtr(sh, obj, off1.prev);
        const TValId valPrevBy2 = valOfPtr(sh, obj, off2.prev);
        return (valPrevBy1 == valPrevBy2);
}

bool joinSegBindingOfMayExist(
        bool                   *pResult,
        BindingOff             *pOff,
        const SymJoinCtx       &ctx,
        const TObjId            obj1,
        const TObjId            obj2)
{
    const EObjKind kind1 = ctx.sh1.objKind(obj1);
    const EObjKind kind2 = ctx.sh2.objKind(obj2);

    const bool isMayExist1 = isMayExistObj(kind1);
    const bool isMayExist2 = isMayExistObj(kind2);
    if (!isMayExist1 && !isMayExist2)
        // no OK_SEE_THROUGH involved
        return false;

    const BindingOff off1 = ctx.sh1.segBinding(obj1);
    const BindingOff off2 = ctx.sh2.segBinding(obj2);
    *pOff = (isMayExist2) ? off1 : off2;

    if (off1 == off2) {
        // the 'next' offset matches trivially
        *pResult = true;
        return true;
    }

    if (OK_OBJ_OR_NULL == kind2) {
        *pResult = (VAL_NULL == valOfPtr(ctx.sh1, obj1, off1.next));
        return true;
    }

    if (OK_OBJ_OR_NULL == kind1) {
        *pResult = (VAL_NULL == valOfPtr(ctx.sh2, obj2, off2.next));
        return true;
    }

    // NOTE: test-0504 utilizes this code path

    if (isMayExist1 && matchBindingFieldsByValue(ctx.sh1, obj1, off1, off2))
            goto match;

    if (isMayExist2 && matchBindingFieldsByValue(ctx.sh2, obj2, off1, off2))
            goto match;

    // giving up
    *pResult = false;
    return true;

match:
    SJ_DEBUG("non-trivial match of 'next' offset of OK_SEE_THROUGH");
    *pResult = true;
    return true;
}

bool joinSegBinding(
        BindingOff             *pOff,
        const SymJoinCtx       &ctx,
        const TObjId            obj1,
        const TObjId            obj2)
{
    const bool isSeg1 = objWithBinding(ctx.sh1, obj1);
    const bool isSeg2 = objWithBinding(ctx.sh2, obj2);
    if (!isSeg1 && !isSeg2)
        // nothing to join here
        return true;

    if (isSeg1 && isSeg2) {
        bool result;
        if (!joinSegBindingOfMayExist(&result, pOff, ctx, obj1, obj2)) {
            // just compare the binding offsets
            const BindingOff off1 = ctx.sh1.segBinding(obj1);
            const BindingOff off2 = ctx.sh2.segBinding(obj2);
            if ((result = (off1 == off2)))
                *pOff = off1;
        }

        if (result)
            return true;

        SJ_DEBUG("<-- segment binding mismatch " << SJ_OBJP(obj1, obj2));
        return false;
    }

    if (isSeg1) {
        *pOff = ctx.sh1.segBinding(obj1);
        return true;
    }

    if (isSeg2) {
        *pOff = ctx.sh2.segBinding(obj2);
        return true;
    }

    // not reachable
    CL_TRAP;
    return false;
}

bool rootNotYetAbstract(SymHeap &sh, const TValSet &sset)
{
    if (sset.empty()) {
        CL_BREAK_IF("rootNotYetAbstract() got an empty set");
        return false;
    }

    const TValId anyRoot = *sset.begin();
    return !isAbstractValue(sh, anyRoot);
}

bool joinNestingLevel(
        TProtoLevel            *pDst,
        const SymJoinCtx       &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const TProtoLevel       ldiff)
{
    TProtoLevel level1 = ctx.sh1.objProtoLevel(obj1);
    TProtoLevel level2 = ctx.sh2.objProtoLevel(obj2);

    if (ctx.joiningData()) {
        // if one of the starting points is not yet abstract, compensate it!

        if (rootNotYetAbstract(ctx.sh1, ctx.sset1))
            ++level1;

        if (rootNotYetAbstract(ctx.sh2, ctx.sset2))
            ++level2;
    }

    *pDst = std::max(level1, level2);

    if (ctx.joiningData() && obj1 == obj2)
        // shared data
        return true;

    if (OBJ_INVALID == obj2 || OBJ_INVALID == obj1)
        // we got only one object, just take its level as it is
        return true;

    // check that the computed ldiff matches the actual one
    const TProtoLevel ldiffExpected = level1 - level2;
    return (ldiffExpected == ldiff);
}

TMinLen joinMinLength(
        SymJoinCtx             &ctx,
        const TObjId            obj1,
        const TObjId            obj2)
{
    if (OBJ_INVALID == obj1 || OBJ_INVALID == obj2) {
        // the status should have been already updated
        CL_BREAK_IF(JS_USE_ANY == ctx.status);

        if (objMinLength(ctx.sh1, obj1) || objMinLength(ctx.sh2, obj2))
            // insertion of non-empty object does not cover both variants
            updateJoinStatus(ctx, JS_THREE_WAY);

        return 0;
    }

    const TMinLen len1 = objMinLength(ctx.sh1, obj1);
    const TMinLen len2 = objMinLength(ctx.sh2, obj2);
    if (len1 < len2) {
        updateJoinStatus(ctx, JS_USE_SH1);
        return len1;
    }

    if (len2 < len1) {
        updateJoinStatus(ctx, JS_USE_SH2);
        return len2;
    }

    // the lengths are equal, pick any
    CL_BREAK_IF(len1 != len2);
    return len1;
}

bool joinObjValidity(
        bool                    *pDst,
        SymJoinCtx              &ctx,
        const TObjId             obj1,
        const TObjId             obj2)
{
    if (OBJ_INVALID == obj1) {
        *pDst = ctx.sh2.isValid(obj2);
        return true;
    }

    if (OBJ_INVALID == obj2) {
        *pDst = ctx.sh1.isValid(obj1);
        return true;
    }

    const bool valid1 = ctx.sh1.isValid(obj1);
    const bool valid2 = ctx.sh2.isValid(obj2);
    if (valid1 != valid2) {
        SJ_DEBUG("<-- object validity mismatch " << SJ_OBJP(obj1, obj2));
        return false;
    }

    *pDst = valid1;
    return true;
}

bool joinObjSize(
        TSizeRange              *pDst,
        SymJoinCtx              &ctx,
        const TObjId             obj1,
        const TObjId             obj2)
{
    if (OBJ_INVALID == obj1) {
        *pDst = ctx.sh2.objSize(obj2);
        return true;
    }

    if (OBJ_INVALID == obj2) {
        *pDst = ctx.sh1.objSize(obj1);
        return true;
    }

    const TSizeRange size1 = ctx.sh1.objSize(obj1);
    const TSizeRange size2 = ctx.sh2.objSize(obj2);
    if (size1 != size2) {
        SJ_DEBUG("<-- object size mismatch " << SJ_OBJP(obj1, obj2));
        return false;
    }

    *pDst = size1;
    return true;
}

void importBlockMap(
        TUniBlockMap            *pMap,
        SymHeap                 &dst,
        const SymHeap           &src)
{
    BOOST_FOREACH(TUniBlockMap::reference item, *pMap) {
        UniformBlock &bl = item.second;
        bl.tplValue = translateValProto(dst, src, bl.tplValue);
    }
}

void joinUniBlocksCore(
        TUniBlockMap            *pMap,
        bool                    *pExtra1,
        bool                    *pExtra2,
        SymJoinCtx              &ctx,
        const TObjId            root1,
        const TObjId            root2)
{
    SymHeap &sh1 = ctx.sh1;
    SymHeap &sh2 = ctx.sh2;

    TUniBlockMap bMap1, bMap2;
    sh1.gatherUniformBlocks(bMap1, root1);
    sh2.gatherUniformBlocks(bMap2, root2);

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap1) {
        UniformBlock bl2(item.second);
        bl2.tplValue = translateValProto(sh2, sh1, bl2.tplValue);

        TUniBlockMap cov2;
        if (!sh2.findCoveringUniBlocks(&cov2, root2, bl2))
            *pExtra1 = true;

        BOOST_FOREACH(TUniBlockMap::const_reference cItem, cov2) {
            UniformBlock blDst(cItem.second);
            blDst.tplValue = translateValProto(ctx.dst, sh2, blDst.tplValue);
            (*pMap)[blDst.off] = blDst;
        }
    }

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap2) {
        UniformBlock bl1(item.second);
        bl1.tplValue = translateValProto(sh1, sh2, bl1.tplValue);

        TUniBlockMap cov1;
        if (!sh1.findCoveringUniBlocks(&cov1, root1, bl1))
            *pExtra2 = true;

        BOOST_FOREACH(TUniBlockMap::const_reference cItem, cov1) {
            UniformBlock blDst(cItem.second);
            const TOffset off = blDst.off;
            if (hasKey(*pMap, off)) {
                // we already have a uniform block at this offset
                const UniformBlock &old = (*pMap)[off];
                CL_BREAK_IF(!areValProtosEqual(ctx.dst, sh1,
                            old.tplValue,
                            bl1.tplValue));

                if (bl1.size < old.size)
                    // the current block is smaller than the original one, skip it!
                    continue;
            }

            blDst.tplValue = translateValProto(ctx.dst, sh1, blDst.tplValue);
            (*pMap)[off] = blDst;
        }
    }
}

bool joinUniBlocks(
        SymJoinCtx              &ctx,
        const TObjId            objDst,
        const TObjId            obj1,
        const TObjId            obj2)
{
    TUniBlockMap bMapDst;
    bool hasExtra1 = false;
    bool hasExtra2 = false;

    if (OBJ_INVALID == obj2) {
        hasExtra2 = true;
        ctx.sh1.gatherUniformBlocks(bMapDst, obj1);
        importBlockMap(&bMapDst, ctx.dst, /* src */ ctx.sh1);
    }
    else if (OBJ_INVALID == obj1) {
        hasExtra1 = true;
        ctx.sh2.gatherUniformBlocks(bMapDst, obj2);
        importBlockMap(&bMapDst, ctx.dst, /* src */ ctx.sh2);
    }
    else
        joinUniBlocksCore(&bMapDst, &hasExtra1, &hasExtra2, ctx, obj1, obj2);

    // update join status accordingly
    if (hasExtra1 && !updateJoinStatus(ctx, JS_USE_SH2))
        return false;
    if (hasExtra2 && !updateJoinStatus(ctx, JS_USE_SH1))
        return false;

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMapDst) {
        const UniformBlock &bl = item.second;
        ctx.dst.writeUniformBlock(objDst, bl);
    }

    return true;
}

static const BindingOff ObjOrNull(OK_OBJ_OR_NULL);

/// (NULL != off) means 'introduce OK_{FLD_OR_NULL,SEE_THROUGH,SEE_THROUGH_2N}'
bool createObject(
        SymJoinCtx             &ctx,
        const struct cl_type   *clt,
        const TObjId            obj1,
        const TObjId            obj2,
        const TProtoLevel       ldiff,
        const EJoinStatus       action,
        const BindingOff       *offMayExist = 0)
{
    bool valid;
    if (!joinObjValidity(&valid, ctx, obj1, obj2))
        return false;

    EObjKind kind;
    if (!joinObjKind(&kind, ctx, obj1, obj2, action))
        return false;

    BindingOff off;
    if (!joinSegBinding(&off, ctx, obj1, obj2))
        return false;

    TProtoLevel protoLevel;
    if (!joinNestingLevel(&protoLevel, ctx, obj1, obj2, ldiff))
        return false;

    if (offMayExist) {
        // we are asked to introduce OK_SEE_THROUGH/OK_OBJ_OR_NULL
        if (OK_REGION != kind && !isMayExistObj(kind))
            CL_BREAK_IF("invalid call of createObject()");

        if (ObjOrNull == *offMayExist)
            kind = OK_OBJ_OR_NULL;
        else {
            off = *offMayExist;
            kind = (off.next == off.prev)
                ? OK_SEE_THROUGH
                : OK_SEE_THROUGH_2N;
        }
    }

    TSizeRange size;
    if (!joinObjSize(&size, ctx, obj1, obj2))
        return false;

    if (!updateJoinStatus(ctx, action))
        return false;

    // create an image in ctx.dst
    const TObjId objDst = ctx.dst.heapAlloc(size);

    if (!joinUniBlocks(ctx, objDst, obj1, obj2))
        // failed to complement uniform blocks
        return false;

    if (clt)
        // preserve estimated type-info of the root
        ctx.dst.objSetEstimatedType(objDst, clt);

    if (!valid)
        // mark the object as invalid
        ctx.dst.objInvalidate(objDst);

    // preserve 'prototype' flag
    ctx.dst.objSetProtoLevel(objDst, protoLevel);

    if (OK_REGION != kind) {
        // abstract object
        ctx.dst.objSetAbstract(objDst, kind, off);

        // compute minimal length of the resulting segment
        ctx.segLengths[objDst] = joinMinLength(ctx, obj1, obj2);
    }

    return joinFields(ctx, objDst, obj1, obj2, ldiff);
}

bool followRootValuesCore(
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const EJoinStatus       action,
        const bool              readOnly)
{
    const TValId root1 = item.v1;
    const TValId root2 = item.v2;
    if (!checkValueMapping(ctx, root1, root2, /* allowUnknownMapping */ true))
        return false;

    const TObjId obj1 = ctx.sh1.objByAddr(root1);
    const TObjId obj2 = ctx.sh2.objByAddr(root2);

    if (hasKey(ctx.valMap1[0], root1) && hasKey(ctx.valMap2[0], root2))
        return true;

    if (readOnly)
        // do not create any object, just check if it was possible
        return segMatchLookAhead(ctx, item);

    if (ctx.joiningDataReadWrite() && root1 == root2)
        // we are on the way from joinData() and hit shared data
        return joinFields(ctx, obj1, obj1, obj2, item.ldiff);

    const TObjType clt1 = ctx.sh1.objEstimatedType(obj1);
    const TObjType clt2 = ctx.sh2.objEstimatedType(obj2);
    const TObjType clt = joinClt(ctx, clt1, clt2);

    return createObject(ctx, clt, obj1, obj2, item.ldiff, action);
}

bool joinReturnAddrs(SymJoinCtx &ctx)
{
    TObjType clt;
    const TObjType clt1 = ctx.sh1.objEstimatedType(OBJ_RETURN);
    const TObjType clt2 = ctx.sh2.objEstimatedType(OBJ_RETURN);
    if (!joinClt(&clt, clt1, clt2))
        // mismatch in type of return value
        return false;

    if (!clt)
        // nothing to join here
        return true;

    ctx.dst.objSetEstimatedType(OBJ_RETURN, clt);
    return joinFields(ctx, OBJ_RETURN, OBJ_RETURN, OBJ_RETURN, /* ldiff */0);
}

bool joinCustomValues(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
    SymHeap &sh1 = ctx.sh1;
    SymHeap &sh2 = ctx.sh2;

    const CustomValue cVal1 = sh1.valUnwrapCustom(v1);
    const CustomValue cVal2 = sh2.valUnwrapCustom(v2);
    if (cVal1 == cVal2) {
        // full match
        const TValId vDst = ctx.dst.valWrapCustom(cVal1);
        return defineValueMapping(ctx, v1, v2, vDst);
    }

    IR::Range rng1, rng2;
    if (!rngFromVal(&rng1, sh1, v1) || !rngFromVal(&rng2, sh2, v2)) {
        // throw custom values away and abstract them by a fresh unknown value
        SJ_DEBUG("throwing away unmatched custom values " << SJ_VALP(v1, v2));
        const TValId vDst = ctx.dst.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        return updateJoinStatus(ctx, JS_THREE_WAY)
            && defineValueMapping(ctx, v1, v2, vDst);
    }

    // compute the resulting range that covers both
    IR::Range rng = join(rng1, rng2);

#if SE_INT_ARITHMETIC_LIMIT
    const IR::TInt max = std::max(std::abs(rng.lo), std::abs(rng.hi));
    if (max <= (SE_INT_ARITHMETIC_LIMIT)) {
        SJ_DEBUG("<-- integral values preserved by SE_INT_ARITHMETIC_LIMIT "
                << SJ_VALP(v1, v2));

        return false;
    }
#endif

#if !(SE_ALLOW_INT_RANGES & 0x1)
    // avoid creation of a CV_INT_RANGE value from two CV_INT values
    if (isSingular(rng1) && isSingular(rng2)) {
        const TValId vDst = ctx.dst.valCreate(VT_UNKNOWN, VO_UNKNOWN);
        if (!defineValueMapping(ctx, v1, v2, vDst))
            return false;

        // force three-way join in order not to loop forever!
        ctx.forceThreeWay = true;
        return updateJoinStatus(ctx, JS_THREE_WAY);
    }
#endif

    // [experimental] widening on intervals
    if (!isSingular(rng1) && !isSingular(rng2)) {
#if (SE_ALLOW_INT_RANGES & 0x2)
        if (rng.lo == rng1.lo || rng.lo == rng2.lo)
            rng.hi = IR::IntMax;
#endif
#if (SE_ALLOW_INT_RANGES & 0x4)
        if (rng.hi == rng1.hi || rng.hi == rng2.hi)
            rng.lo = IR::IntMin;
#endif
    }

    if (!isCovered(rng, rng1) && !updateJoinStatus(ctx, JS_USE_SH2))
        return false;

    if (!isCovered(rng, rng2) && !updateJoinStatus(ctx, JS_USE_SH1))
        return false;

    const CustomValue cv(rng);
    const TValId vDst = ctx.dst.valWrapCustom(cv);
    return defineValueMapping(ctx, v1, v2, vDst);
}

bool followRootValues(
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const EJoinStatus       action,
        const bool              readOnly = false)
{
    if (!followRootValuesCore(ctx, item, action, readOnly))
        return false;

    if (!ctx.joiningData())
        // we are on the way from joinSymHeaps()
        return true;

    const TValId root1 = item.v1;
    const TValId root2 = item.v2;
    if (root1 == root2)
        // shared data
        return true;

    if (readOnly)
        // postpone it till the read-write attempt
        return true;

    const TObjId obj1 = ctx.sh1.objByAddr(root1);
    const TObjId obj2 = ctx.sh2.objByAddr(root2);

    const bool isDls1 = (OK_DLS == ctx.sh1.objKind(obj1))
        && !hasKey(ctx.sset1, root1);

    const bool isDls2 = (OK_DLS == ctx.sh2.objKind(obj2))
        && !hasKey(ctx.sset2, root2);

    if (isDls1 == isDls2)
        return true;

    CL_BREAK_IF(isDls1 && JS_USE_SH1 != action);
    CL_BREAK_IF(isDls2 && JS_USE_SH2 != action);

    const TObjId peer1 = (isDls1) ? dlSegPeer(ctx.sh1, obj1) : OBJ_INVALID;
    const TObjId peer2 = (isDls2) ? dlSegPeer(ctx.sh2, obj2) : OBJ_INVALID;

    const TObjMapBidir &objMap = (isDls1) ? ctx.objMap1 : ctx.objMap2;
    if (hasKey(objMap[0], (isDls1) ? peer1 : peer2))
        // already cloned
        return true;

    /// TODO: drop this!
    const TValId peer1At = ctx.sh1.addrOfTarget(peer1, /* XXX */ TS_REGION);
    const TValId peer2At = ctx.sh2.addrOfTarget(peer2, /* XXX */ TS_REGION);

    // clone peer object
    const TValPair tb(peer1At, peer2At);
    ctx.tieBreaking.insert(tb);

    const struct cl_type *clt = (isDls1)
        ? ctx.sh1.objEstimatedType(peer1)
        : ctx.sh2.objEstimatedType(peer2);

    return createObject(ctx, clt, peer1, peer2, item.ldiff, action);
}

bool followValuePair(
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const bool              readOnly)
{
    const TValId v1 = item.v1;
    const TValId v2 = item.v2;
    if (readOnly)
        // shallow scan only!
        return checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ true);

    bool result;
    if (joinValuesByCode(&result, ctx, v1, v2))
        return result;

    const bool isRange = (VT_RANGE == ctx.sh1.valTarget(v1))
                      || (VT_RANGE == ctx.sh2.valTarget(v2));

    if (!isRange && (ctx.sh1.valOffset(v1) != ctx.sh2.valOffset(v2))) {
        SJ_DEBUG("<-- value offset mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    // follow the roots
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    const SchedItem rootItem(root1, root2, item.ldiff);
    if (!followRootValues(ctx, rootItem, JS_USE_ANY))
        return false;

    // ranges cannot be joint unless the root exists in ctx.dst, join them now!
    if (isRange && !joinRangeValues(ctx, v1, v2))
        return false;

    return true;
}

bool segAlreadyJoined(
        SymJoinCtx              &ctx,
        const TValId            seg1,
        const TValId            seg2,
        const EJoinStatus       action)
{
    TValPair vp;

    switch (action) {
        case JS_USE_SH1:
            vp = TValPair(seg1, VAL_INVALID);
            break;

        case JS_USE_SH2:
            vp = TValPair(VAL_INVALID, seg2);
            break;

        default:
            return false;
    }

    return hasKey(ctx.tieBreaking, vp);
}

bool joinSegmentWithAny(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const EJoinStatus       action,
        bool                    firstTryReadOnly = true)
{
    const TValId root1 = item.v1;
    const TValId root2 = item.v2;
    if (segAlreadyJoined(ctx, root1, root2, action)) {
        // already joined
        *pResult = true;
        return true;
    }

    const bool isValid1 = isPossibleToDeref(ctx.sh1, root1);
    const bool isValid2 = isPossibleToDeref(ctx.sh2, root2);
    if (!isValid1 || !isValid2)
        return false;

    if (firstTryReadOnly && !followRootValues(ctx, item, action, /* RO */ true))
        return false;

    const TObjId obj1 = ctx.sh1.objByAddr(root1);
    const TObjId obj2 = ctx.sh2.objByAddr(root2);

    const bool isDls1 = (OK_DLS == ctx.sh1.objKind(obj1));
    const bool isDls2 = (OK_DLS == ctx.sh2.objKind(obj2));
    const bool haveDls = (isDls1 || isDls2);

    TObjId peer1 = obj1;
    if (isDls1)
        peer1 = dlSegPeer(ctx.sh1, obj1);

    TObjId peer2 = obj2;
    if (isDls2)
        peer2 = dlSegPeer(ctx.sh2, obj2);

    const EObjKind kind = (JS_USE_SH1 == action)
        ? ctx.sh1.objKind(obj1)
        : ctx.sh2.objKind(obj2);

    if (OK_OBJ_OR_NULL != kind) {
        // BindingOff is assumed to be already matching at this point
        const BindingOff off = (JS_USE_SH1 == action)
            ? ctx.sh1.segBinding(peer1)
            : ctx.sh2.segBinding(peer2);

        const TValId valNext1 = valOfPtr(ctx.sh1, peer1, off.next);
        const TValId valNext2 = valOfPtr(ctx.sh2, peer2, off.next);
        if (firstTryReadOnly && !checkValueMapping(ctx, valNext1, valNext2,
                               /* allowUnknownMapping */ true))
            return false;

        if (firstTryReadOnly && haveDls) {
            const TValId valPrev1 = valOfPtr(ctx.sh1, obj1, off.prev);
            const TValId valPrev2 = valOfPtr(ctx.sh2, obj2, off.prev);
            if (!checkValueMapping(ctx, valPrev1, valPrev2,
                                   /* allowUnknownMapping */ true))
                return false;
        }
    }

    // go ahead, try it read-write!
    SJ_DEBUG(">>> joinSegmentWithAny" << SJ_VALP(root1, root2));
    *pResult = followRootValues(ctx, item, action);
    if (!haveDls || !*pResult)
        return true;

    /// TODO: drop this!
    const TValId peer1At = ctx.sh1.addrOfTarget(peer1, /* XXX */ TS_REGION);
    const TValId peer2At = ctx.sh2.addrOfTarget(peer2, /* XXX */ TS_REGION);
    if (segAlreadyJoined(ctx, peer1At, peer2At, action))
        return true;

    const SchedItem peerItem(peer1At, peer2At, item.ldiff);
    *pResult = followRootValues(ctx, peerItem, action);
    return true;
}

/// (NULL != off) means 'introduce OK_{OBJ_OR_NULL,SEE_THROUGH,SEE_THROUGH_2N}'
bool segmentCloneCore(
        SymJoinCtx                  &ctx,
        SymHeap                     &shGt,
        const TValId                valGt,
        const TValMapBidir          &valMapGt,
        const TProtoLevel           ldiff,
        const EJoinStatus           action,
        const BindingOff            *off)
{
    if (!isAnyDataArea(shGt.valTarget(valGt)))
        // not valid target
        return false;

    const TValId addrGt = shGt.valRoot(valGt);
    if (hasKey(valMapGt[0], addrGt))
        // mapping already available for objGt
        return true;

    const TObjType clt = shGt.objEstimatedType(shGt.objByAddr(addrGt));
    SJ_DEBUG("+i+ insertSegmentClone: cloning object at #" << addrGt <<
             ", action = " << action);

    // TODO: drop this!
    const TValId root1 = (JS_USE_SH1 == action) ? addrGt : VAL_INVALID;
    const TValId root2 = (JS_USE_SH2 == action) ? addrGt : VAL_INVALID;
    const SchedItem item(root1, root2, ldiff);
    ctx.tieBreaking.insert(TValPair(item));

    // clone the object
    const TObjId obj1 = ctx.sh1.objByAddr(root1);
    const TObjId obj2 = ctx.sh2.objByAddr(root2);
    if (createObject(ctx, clt, obj1, obj2, ldiff, action, off))
        return true;

    SJ_DEBUG("<-- insertSegmentClone: failed to create object "
             << SJ_VALP(root1, root2));
    return false;
}

void scheduleSegAddr(
        TWorkList               &wl,
        const TValId            seg,
        const TValId            peer,
        const EJoinStatus       action,
        const TProtoLevel       ldiff)
{
    CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);

    const SchedItem segItem(
            (JS_USE_SH1 == action) ? seg : VAL_INVALID,
            (JS_USE_SH2 == action) ? seg : VAL_INVALID,
            ldiff);
    wl.schedule(segItem);

    if (seg == peer)
        return;

    const SchedItem peerItem(
            (JS_USE_SH1 == action) ? peer : VAL_INVALID,
            (JS_USE_SH2 == action) ? peer : VAL_INVALID,
            ldiff);
    wl.schedule(peerItem);
}

bool handleUnknownValues(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    const bool isNull1 = (VAL_NULL == v1);
    const bool isNull2 = (VAL_NULL == v2);
    if (isNull1 != isNull2) {
        const TValPair vp(v1, v2);
        CL_BREAK_IF(hasKey(ctx.matchLookup, vp));
        ctx.matchLookup[vp] = vDst;

        SymHeap &shGt      = (isNull2) ? ctx.sh1 : ctx.sh2;
        const TValId valGt = (isNull2) ? v1 : v2;
        TValMapBidir &vMap = (isNull2) ? ctx.valMap1 : ctx.valMap2;

        return matchPlainValues(vMap, shGt, ctx.dst, valGt, vDst);
    }

    CL_BREAK_IF(isNull1 && isNull2);
    return defineValueMapping(ctx, v1, v2, vDst);
}

bool cloneSpecialValue(
        SymJoinCtx              &ctx,
        SymHeap                 &shGt,
        const TValId            valGt,
        const TValMapBidir      &valMapGt,
        const SchedItem         &itemToClone,
        EValueTarget            code)
{
    const TValPair vp(itemToClone);

    const TValId rootGt = shGt.valRoot(valGt);
    EValueOrigin vo = shGt.valOrigin(valGt);
    TValId vDst;

    switch (code) {
        case VT_RANGE:
            if (hasKey(valMapGt[/* ltr */ 0], rootGt))
                break;

            CL_BREAK_IF("unable to transfer a VT_RANGE value");
            // fall through!

        case VT_CUSTOM:
            code = VT_UNKNOWN;
            vo = VO_UNKNOWN;
            // fall through!

        default:
            vDst = ctx.dst.valCreate(code, vo);
            return handleUnknownValues(ctx, vp.first, vp.second, vDst);
    }

    // VT_RANGE
    const TValId rootDst = roMapLookup(valMapGt[/* ltr */ 0], rootGt);
    const IR::Range range = shGt.valOffsetRange(valGt);
    vDst = ctx.dst.valByRange(rootDst, range);
    if (!handleUnknownValues(ctx, vp.first, vp.second, vDst))
        return false;

    ctx.matchLookup[vp] = vDst;
    return true;
}

/// (NULL != off) means 'introduce OK_{OBJ_OR_NULL,SEE_THROUGH,SEE_THROUGH_2N}'
bool insertSegmentClone(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const EJoinStatus       action,
        const BindingOff        *off = 0)
{
    const TValId v1 = item.v1;
    const TValId v2 = item.v2;
    SJ_DEBUG(">>> insertSegmentClone" << SJ_VALP(v1, v2));

    const bool isGt1 = (JS_USE_SH1 == action);
    const bool isGt2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(isGt1 == isGt2);

    // resolve the existing segment in shGt
    SymHeap &shGt = ((isGt1) ? ctx.sh1 : ctx.sh2);
    const TObjId seg = shGt.objByAddr((isGt1) ? v1 : v2);
    const bool isDls = (OK_DLS == shGt.objKind(seg));
    CL_BREAK_IF(off && isDls);

    TObjId peer = seg;
    if (isDls)
        peer = dlSegPeer(shGt, seg);

    // resolve the 'next' pointer and check its validity
    TValId nextGt;
    if (off)
        nextGt = (ObjOrNull == *off)
            ? VAL_NULL
            : valOfPtr(shGt, seg, off->next);
    else
        nextGt = nextValFromSeg(shGt, peer);

    const TValId nextLt = (isGt2) ? v1 : v2;
    if (!off && !checkValueMapping(ctx,
                (isGt1) ? nextGt : nextLt,
                (isGt2) ? nextGt : nextLt,
                /* allowUnknownMapping */ true))
    {
        SJ_DEBUG("<-- insertSegmentClone: value mismatch "
                 "(nextLt = #" << nextLt << ", nextGt = #" << nextGt << ")");
        return false;
    }

#if SE_ALLOW_THREE_WAY_JOIN < 3
    if (!ctx.joiningData() && objMinLength(shGt, seg))
        // on the way from joinSymHeaps(), some three way joins are destructive
        ctx.allowThreeWay = false;
#endif

    const TValMapBidir &valMapGt = (isGt1)
        ? ctx.valMap1
        : ctx.valMap2;

    // TODO: drop this!
    const TValId segAt = shGt.addrOfTarget(seg, /* XXX */ TS_REGION);
    const TValId peerAt = shGt.addrOfTarget(peer, /* XXX */ TS_REGION);

    scheduleSegAddr(ctx.wl, segAt, peerAt, action, item.ldiff);

    SchedItem cloneItem;
    while (ctx.wl.next(cloneItem)) {
        const TValId valGt = (isGt1) ? cloneItem.v1 : cloneItem.v2;
        const TValId valLt = (isGt2) ? cloneItem.v1 : cloneItem.v2;
        if (VAL_INVALID != valLt) {
            // process the rest of ctx.wl rather in joinPendingValues()
            ctx.wl.undo(cloneItem);
            break;
        }

        if (nextGt == valGt)
            // do not go beyond the segment, just follow its data
            continue;

        if (segAt != valGt)
            // OK_SEE_THROUGH/OK_OBJ_OR_NULL is applicable only on the first fld
            off = 0;

        const EValueTarget code = shGt.valTarget(valGt);
        if (isAnyDataArea(code)) {
            if (segmentCloneCore(ctx, shGt, valGt, valMapGt, cloneItem.ldiff,
                        action, off))
                continue;
        }
        else {
            if (cloneSpecialValue(ctx, shGt, valGt, valMapGt, cloneItem, code))
                continue;
        }

        // clone failed
        *pResult = false;
        return true;
    }

    // schedule the next object in the row (TODO: check if really necessary)
    const TValId valNext1 = (isGt1) ? nextGt : nextLt;
    const TValId valNext2 = (isGt2) ? nextGt : nextLt;
    const SchedItem nextItem(valNext1, valNext2, item.ldiff);
    if (ctx.wl.schedule(nextItem))
        SJ_DEBUG("+++ " << SJ_VALP(v1, v2) << " by insertSegmentClone()");

    *pResult = true;
    return true;
}

void resolveMayExist(
        SymJoinCtx              &ctx,
        bool                    *isAbs1,
        bool                    *isAbs2,
        const TValId            v1,
        const TValId            v2)
{
    if (!*isAbs1 || !*isAbs2)
        // at most one abstract object
        return;

    const TObjId obj1 = ctx.sh1.objByAddr(v1);
    const TObjId obj2 = ctx.sh2.objByAddr(v2);

    const EObjKind kind1 = ctx.sh1.objKind(obj1);
    const EObjKind kind2 = ctx.sh2.objKind(obj2);
    if (kind1 == kind2)
        // kind of abstract object matches in both cases
        return;

    if (isMayExistObj(kind1))
        *isAbs1 = false;

    if (isMayExistObj(kind2))
        *isAbs2 = false;
}

bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        bool                     isAbs1,
        bool                     isAbs2)
{
    const TValId v1 = item.v1;
    const TValId v2 = item.v2;
    resolveMayExist(ctx, &isAbs1, &isAbs2, v1, v2);

    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    const SchedItem rootItem(root1, root2, item.ldiff);

    if (isAbs1 && isAbs2
            && joinSegmentWithAny(pResult, ctx, rootItem, JS_USE_ANY))
        goto done;

    if (!isAbs2 && joinSegmentWithAny(pResult, ctx, rootItem, JS_USE_SH1))
        goto done;

    if (!isAbs1 && joinSegmentWithAny(pResult, ctx, rootItem, JS_USE_SH2))
        goto done;

    if (isAbs1 && insertSegmentClone(pResult, ctx, item, JS_USE_SH1))
        goto done;

    if (isAbs2 && insertSegmentClone(pResult, ctx, item, JS_USE_SH2))
        goto done;

    // we have failed
    *pResult = false;
    return true;

done:
    if (VT_RANGE == ctx.sh1.valTarget(v1) || VT_RANGE == ctx.sh2.valTarget(v2))
        // we came here from a VT_RANGE value, remember to join the entry
        *pResult = joinRangeValues(ctx, v1, v2);

    return true;
}

bool offRangeFallback(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
#if !(SE_ALLOW_OFF_RANGES & 0x1)
    return false;
#endif

    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    if (!checkValueMapping(ctx, root1, root2, /* allowUnknownMapping */ false))
        // not really a suitable candidate for offRangeFallback()
        return false;

    // check we got different offsets
    const TOffset off1 = ctx.sh1.valOffset(v1);
    const TOffset off2 = ctx.sh2.valOffset(v2);
    CL_BREAK_IF(off1 == off2);

    // check whether the values are not matched already
    const TValPair vp(v1, v2);
    CL_BREAK_IF(hasKey(ctx.matchLookup, vp));

    if (!updateJoinStatus(ctx, /* intentionally! */ JS_THREE_WAY))
        return false;

    // resolve root in ctx.dst
    const TValId rootDst = roMapLookup(ctx.valMap1[/* ltr */ 0], root1);
    CL_BREAK_IF(rootDst != roMapLookup(ctx.valMap2[/* ltr */ 0], root2));

    // compute the resulting range
    IR::Range rng;
    rng.lo = std::min(off1, off2);
    rng.hi = std::max(off1, off2);
    rng.alignment = IR::Int1;

    // create a VT_RANGE value in ctx.dst
    const TValId vDst = ctx.dst.valByRange(rootDst, rng);

    // store the mapping (v1, v2) -> vDst
    ctx.matchLookup[vp] = vDst;
    return true;
}

typedef std::vector<TOffset>                        TOffList;

class MayExistVisitor {
    private:
        SymJoinCtx              ctx_;
        const TProtoLevel       ldiff_;
        const EJoinStatus       action_;
        const TValId            valRef_;
        const TValId            root_;
        bool                    lookThrough_;
        TOffList                foundOffsets_;

    public:
        MayExistVisitor(
                SymJoinCtx          &ctx,
                const TProtoLevel   ldiff,
                const EJoinStatus   action,
                const TValId        valRef,
                const TValId        root):
            ctx_(ctx),
            ldiff_(ldiff),
            action_(action),
            valRef_(valRef),
            root_(root),
            lookThrough_(false)
        {
            CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);
        }

        const TOffList& foundOffsets() const {
            return foundOffsets_;
        }

        bool found() const {
            return !foundOffsets_.empty();
        }

        void enableLookThroughMode(bool enable = true) {
            lookThrough_ = enable;
        }

        bool operator()(const FldHandle &fld) {
            if (!isDataPtr(fld.type()))
                // not a pointer
                return /* continue */ true;

            SymHeap &sh = *static_cast<SymHeap *>(fld.sh());
            TValId val = fld.value();

            for (;;) {
                const TValId valRoot = sh.valRoot(val);
                if (valRoot == root_)
                    // refuse referencing the MayExist candidate itself
                    return /* continue */ true;

                const TValId v1 = (JS_USE_SH1 == action_) ? val : valRef_;
                const TValId v2 = (JS_USE_SH2 == action_) ? val : valRef_;
                const SchedItem item(v1, v2, ldiff_);
                if (followValuePair(ctx_, item, /* read-only */ true))
                    // looks like we have a candidate
                    break;

                if (!lookThrough_ || !isAbstractValue(sh, val))
                    return /* continue */ true;

                TObjId seg = sh.objByAddr(valRoot);
                if (sh.segMinLength(seg) || segHeadAt(sh, seg) != val)
                    return /* continue */ true;

                if (OK_DLS == sh.objKind(seg))
                    seg = dlSegPeer(sh, seg);

                val = nextValFromSeg(sh, seg);
            }

            foundOffsets_.push_back(fld.offset());
            return /* continue */ true;
        }
};

bool mayExistDigOffsets(
        BindingOff              *pOff,
        SymHeap                 &sh,
        const TValId             valBy,
        TOffList                 offList)
{
    if (offList.empty())
        // no match
        return false;

    const TObjId obj = sh.objByAddr(valBy);

    typedef std::map<TValId, TOffList>              TOffsByVal;
    TOffsByVal offsByVal;
    BOOST_FOREACH(const TOffset off, offList) {
        const TValId val = valOfPtr(sh, obj, off);
        offsByVal[val].push_back(off);
    }

    int maxCnt = 0;
    TValId maxVal = VAL_INVALID;

    BOOST_FOREACH(TOffsByVal::const_reference item, offsByVal) {
        const int cnt = item.second.size();
        if (cnt < maxCnt)
            continue;

        maxCnt = cnt;
        maxVal = /* val */ item.first;
    }

    // pick the list of offsets with the best sharing
    CL_BREAK_IF(VAL_INVALID == maxVal);
    offList = offsByVal[maxVal];

    switch (maxCnt) {
        case 1:
            // introduce OK_SEE_THROUGH
            pOff->next = pOff->prev = offList.front();
            break;

        case 2:
            // introduce OK_SEE_THROUGH_2N
            pOff->next = offList[0];
            pOff->prev = offList[1];
            break;

        default:
            if (debuggingSymJoin)
                CL_BREAK_IF("please implement");
            return false;
    }

    pOff->head = sh.valOffset(valBy);
    return true;
}

bool mayExistFallback(
        SymJoinCtx              &ctx,
        const SchedItem         &item,
        const EJoinStatus       action)
{
    const bool use1 = (JS_USE_SH1 == action);
    const bool use2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(use1 == use2);

    const TValId v1 = item.v1;
    const TValId v2 = item.v2;

    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);

    const bool hasMapping1 = hasKey(ctx.valMap1[0], root1);
    const bool hasMapping2 = hasKey(ctx.valMap2[0], root2);
    if ((hasMapping1 != hasMapping2) && (hasMapping1 == use1))
        // try it the other way around
        return false;

    SymHeap &sh = (use1) ? ctx.sh1 : ctx.sh2;
    const TValId val = (use1) ? v1 : v2;
    if (!isPossibleToDeref(sh, val))
        // no valid target
        return false;

    const TValId valRoot = (use1) ? root1 : root2;
    const TObjId obj = sh.objByAddr(valRoot);
    if (OK_REGION != sh.objKind(obj))
        // only concrete objects/prototypes are candidates for OK_SEE_THROUGH
        return false;

    BindingOff off;

    const TValId ref = (use2) ? v1 : v2;
    if (VAL_NULL == ref) {
        // introduce OK_OBJ_OR_NULL
        off = ObjOrNull;
    }
    else {
        // look for next pointer(s) of OK_SEE_THROUGH/OK_SEE_THROUGH_2N
        const TObjId obj = sh.objByAddr(valRoot);
        MayExistVisitor visitor(ctx, item.ldiff, action, ref, /* root */ valRoot);
        traverseLivePtrs(sh, obj, visitor);
        if (!visitor.found()) {
            // reference value not matched directly, try to look through in
            // order to allow insert chains of possibly empty abstract objects
            visitor.enableLookThroughMode();
            traverseLivePtrs(sh, obj, visitor);
            if (visitor.found())
                // e.g. test-0124 and test-167 use this code path
                SJ_DEBUG("MayExistVisitor::enableLookThroughMode() in use!");
        }

        if (!mayExistDigOffsets(&off, sh, val, visitor.foundOffsets()))
            // no match
            return false;
    }

    // mayExistFallback() always implies JS_THREE_WAY
    if (!updateJoinStatus(ctx, JS_THREE_WAY))
        return false;

    bool result;
    if (!insertSegmentClone(&result, ctx, item, action, &off))
        result = false;

    return result;
}

EValueOrigin joinOrigin(const EValueOrigin vo1, const EValueOrigin vo2)
{
    if (vo1 == vo2)
        // use any
        return vo2;

    if (VO_DEREF_FAILED == vo1 || VO_DEREF_FAILED == vo2)
        // keep the error recovery as cheap as possible
        return VO_DEREF_FAILED;

    // safe over-approximation
    return VO_UNKNOWN;
}

bool joinValuesByCode(
        bool                   *pResult,
        SymJoinCtx             &ctx,
        const TValId            v1,
        const TValId            v2)
{
    const TObjId obj1 = ctx.sh1.objByAddr(v1);
    const TObjId obj2 = ctx.sh2.objByAddr(v2);

    // check target's validity
    const bool isNull1 = (VAL_NULL == v1);
    const bool isNull2 = (VAL_NULL == v2);
    if (isNull1 || isNull2) {
        if (!checkNullConsistency(ctx, v1, v2)) {
            *pResult = false;
            return true;
        }
    }
    else {
        const bool haveTarget1 = ctx.sh1.isValid(obj1);
        const bool haveTarget2 = ctx.sh2.isValid(obj2);
        if (haveTarget1 != haveTarget2) {
            SJ_DEBUG("<-- target validity mismatch " << SJ_VALP(v1, v2));
            *pResult = false;
            return true;
        }
    }

    const EValueTarget code1 = ctx.sh1.valTarget(v1);
    const EValueTarget code2 = ctx.sh2.valTarget(v2);
    if (VT_RANGE == code1 || VT_RANGE == code2)
        // these have to be handled in followValuePair()
        return false;

    // check for VT_UNKNOWN
    const bool isUnknown1 = (VT_UNKNOWN == code1);
    const bool isUnknown2 = (VT_UNKNOWN == code2);

    if (!isUnknown1 && !isUnknown2) {
        // handle VT_CUSTOM values
        const bool isCustom1 = (VT_CUSTOM == code1);
        const bool isCustom2 = (VT_CUSTOM == code2);
        *pResult = isCustom1 && isCustom2 && joinCustomValues(ctx, v1, v2);
        return     isCustom1 || isCustom2;
    }

    // join the origin
    const EValueOrigin vo1 = ctx.sh1.valOrigin(v1);
    const EValueOrigin vo2 = ctx.sh2.valOrigin(v2);
    const EValueOrigin origin = joinOrigin(vo1, vo2);

    // do not join VT_UNKNOWN with a valid pointer
    if (VO_DEREF_FAILED != origin) {
        const bool haveTarget1 = isPossibleToDeref(ctx.sh1, v1);
        const bool haveTarget2 = isPossibleToDeref(ctx.sh2, v2);
        if (haveTarget1 || haveTarget2) {
            *pResult = false;
            return true;
        }
    }

    // create a new unknown value in ctx.dst
    const TValId vDst = ctx.dst.valCreate(VT_UNKNOWN, origin);
    *pResult = handleUnknownValues(ctx, v1, v2, vDst);

    // we have to use the heap where the unknown value occurs
    if (!isUnknown2)
        return updateJoinStatus(ctx, JS_USE_SH1);
    else if (!isUnknown1)
        return updateJoinStatus(ctx, JS_USE_SH2);
    else
        // use any
        return true;
}

bool joinValuePair(SymJoinCtx &ctx, const SchedItem &item)
{
    const TValId v1 = item.v1;
    const TValId v2 = item.v2;
    if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
        // already joined
        return true;

    bool result;
    if (joinValuesByCode(&result, ctx, v1, v2))
        return result;

    const bool isAbs1 = isAbstractValue(ctx.sh1, v1)
        /* do not treat the starting point as encountered segment */
        && !hasKey(ctx.sset1, ctx.sh1.valRoot(v1));

    const bool isAbs2 = isAbstractValue(ctx.sh2, v2)
        /* do not treat the starting point as encountered segment */
        && !hasKey(ctx.sset2, ctx.sh2.valRoot(v2));

    if ((isAbs1 || isAbs2)
            && joinAbstractValues(&result, ctx, item, isAbs1, isAbs2))
        return result;

    if (followValuePair(ctx, item, /* read-only */ true))
        return followValuePair(ctx, item, /* read-only */ false);

    return offRangeFallback(ctx, v1, v2)
        || mayExistFallback(ctx, item, JS_USE_SH1)
        || mayExistFallback(ctx, item, JS_USE_SH2);
}

bool joinPendingValues(SymJoinCtx &ctx)
{
    SchedItem item;
    while (ctx.wl.next(item)) {
        SJ_DEBUG("--- " << SJ_VALP(item.v1, item.v2));
        if (!joinValuePair(ctx, item))
            return false;

        ctx.alreadyJoined.insert(TValPair(item));
    }

    return true;
}

class JoinVarVisitor {
    public:
        enum EMode {
            JVM_LIVE_OBJS,
            JVM_UNI_BLOCKS
        };

    private:
        SymJoinCtx              &ctx_;
        const EMode             mode_;

    public:
        JoinVarVisitor(SymJoinCtx &ctx, const EMode mode):
            ctx_(ctx),
            mode_(mode)
        {
        }

        bool operator()(const TObjId objs[3]) {
            const TObjId objDst = objs[/* dst */ 0];
            const TObjId obj1   = objs[/* sh1 */ 1];
            const TObjId obj2   = objs[/* sh2 */ 2];

            switch (mode_) {
                case JVM_LIVE_OBJS:
                    return joinFields(ctx_, objDst, obj1, obj2,
                        /* ldiff */ 0);

                case JVM_UNI_BLOCKS:
                    return joinUniBlocks(ctx_, objDst, obj1, obj2);
            }

            CL_BREAK_IF("stack smashing detected");
            return false;
        }
};

bool joinCVars(SymJoinCtx &ctx, const JoinVarVisitor::EMode mode)
{
    SymHeap *const heaps[] = {
        &ctx.dst,
        &ctx.sh1,
        &ctx.sh2
    };

    // go through all program variables
    JoinVarVisitor visitor(ctx, mode);
    return traverseProgramVarsGeneric<
        /* N_DST */ 1,
        /* N_SRC */ 2>
            (heaps, visitor,
             /* allowRecovery */ JoinVarVisitor::JVM_LIVE_OBJS == mode);
}

TValId joinDstValue(
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              validObj1,
        const bool              validObj2)
{
    const TValPair vp(v1, v2);
    SymJoinCtx::TMatchLookup::const_iterator mit = ctx.matchLookup.find(vp);
    if (ctx.matchLookup.end() != mit)
        // XXX: experimental
        return mit->second;

    // translate the roots into 'dst'
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    const TValId valRootDstBy1 = roMapLookup(ctx.valMap1[/* ltr */ 0], root1);
    const TValId valRootDstBy2 = roMapLookup(ctx.valMap2[/* ltr */ 0], root2);

    // translate the offsets into 'dst'
    const IR::Range off1 = ctx.sh1.valOffsetRange(v1);
    const IR::Range off2 = ctx.sh2.valOffsetRange(v2);
    const TValId vDstBy1 = ctx.dst.valByRange(valRootDstBy1, off1);
    const TValId vDstBy2 = ctx.dst.valByRange(valRootDstBy2, off2);
    if (vDstBy1 == vDstBy2)
        // the values are equal --> pick any
        return vDstBy1;

    if (!validObj2)
        return vDstBy1;

    if (!validObj1)
        return vDstBy2;

    // tie breaking
    const TValPair tb1(root1, VAL_INVALID);
    const TValPair tb2(VAL_INVALID, root2);

    const bool use1 = hasKey(ctx.tieBreaking, tb1);
    const bool use2 = hasKey(ctx.tieBreaking, tb2);
    if (use1 && use2)
        return VAL_INVALID;
    else if (use1)
        return vDstBy1;
    else if (use2)
        return vDstBy2;
    else
        return VAL_INVALID;
}

typedef std::pair<FldHandle, FldHandle> THdlPair;

template <class TItem, class TBlackList>
bool setDstValuesCore(
        SymJoinCtx              &ctx,
        const TItem             &rItem,
        const TBlackList        &blackList)
{
    const FldHandle &fldDst = rItem.first;
    CL_BREAK_IF(!fldDst.isValidHandle());
    if (hasKey(blackList, fldDst))
        return true;

    const THdlPair &orig = rItem.second;
    const FldHandle &fld1 = orig.first;
    const FldHandle &fld2 = orig.second;
    CL_BREAK_IF(!fld1.isValidHandle() && !fld2.isValidHandle());

    const TValId v1 = fld1.value();
    const TValId v2 = fld2.value();

    const bool isComp1 = (isComposite(fld1.type()));
    const bool isComp2 = (isComposite(fld2.type()));
    if (isComp1 || isComp2) {
        // do not bother by composite values
        CL_BREAK_IF(fld1.isValidHandle() && !isComp1);
        CL_BREAK_IF(fld2.isValidHandle() && !isComp2);
        return true;
    }

    if (ctx.joiningData() && fld1 == fld2) {
        // shared data
        CL_BREAK_IF(v1 != v2);
        if (ctx.joiningDataReadWrite())
            // read-write mode
            fldDst.setValue(v1);

        return true;
    }

    // compute the resulting value
    const bool validObj1 = (fld1.isValidHandle());
    const bool validObj2 = (fld2.isValidHandle());
    const TValId vDst = joinDstValue(ctx, v1, v2, validObj1, validObj2);
    if (VAL_INVALID == vDst)
        return false;

    // set the value
    fldDst.setValue(vDst);
    return true;
}

bool setDstValues(SymJoinCtx &ctx, const TFldSet *blackList = 0)
{
    SymHeap &dst = ctx.dst;

    typedef std::map<FldHandle /* objDst */, THdlPair> TMap;
    TMap rMap;

    // reverse mapping for ctx.liveList1
    BOOST_FOREACH(const FldHandle &fldSrc, ctx.liveList1) {
        const TObjId objSrc = fldSrc.obj();
        const TObjId objDst = roMapLookup(ctx.objMap1[0], objSrc);
        const FldHandle fldDst = translateFldHandle(dst, objDst, fldSrc);
        if (!hasKey(rMap, fldDst))
            rMap[fldDst].second = FldHandle(FLD_INVALID);

        // fldDst -> fld1
        rMap[fldDst].first = fldSrc;
    }

    // reverse mapping for ctx.liveList2
    BOOST_FOREACH(const FldHandle &fldSrc, ctx.liveList2) {
        const TObjId objSrc = fldSrc.obj();
        const TObjId objDst = roMapLookup(ctx.objMap2[0], objSrc);
        const FldHandle fldDst = translateFldHandle(dst, objDst, fldSrc);
        if (!hasKey(rMap, fldDst))
            rMap[fldDst].first = FldHandle(FLD_INVALID);

        // fldDst -> fld2
        rMap[fldDst].second = fldSrc;
    }

    TFldSet emptyBlackList;
    if (!blackList)
        blackList = &emptyBlackList;

    BOOST_FOREACH(TMap::const_reference rItem, rMap) {
        if (!rItem.first.type())
            // do not set value of anonymous objects
            continue;

        if (!setDstValuesCore(ctx, rItem, *blackList))
            return false;
    }

    return true;
}

// FIXME: the implementation is not going to work well in certain cases
bool isFreshProto(SymJoinCtx &ctx, const TObjId objDst, bool *wasMayExist = 0)
{
    const TObjId obj1 = roMapLookup(ctx.objMap1[/* rtl */ 1], objDst);
    const TObjId obj2 = roMapLookup(ctx.objMap2[/* rtl */ 1], objDst);

    const bool isValid1 = (OBJ_INVALID != obj1);
    const bool isValid2 = (OBJ_INVALID != obj2);
    if (isValid1 == isValid2)
        return false;

    if (wasMayExist) {
        const EObjKind kind = (isValid1)
            ? ctx.sh1.objKind(obj1)
            : ctx.sh2.objKind(obj2);

        *wasMayExist = isMayExistObj(kind);
    }

    return true;
}

struct MayExistLevelUpdater {
    SymJoinCtx         &ctx;
    const TObjId        objDst;
    TFldSet             ignoreList;

    MayExistLevelUpdater(SymJoinCtx &ctx_, const TObjId objDst_):
        ctx(ctx_),
        objDst(objDst_)
    {
        buildIgnoreList(ignoreList, ctx.dst, objDst);
    }

    bool operator()(const FldHandle &sub) const {
        if (hasKey(this->ignoreList, sub))
            return /* continue */ true;

        const TObjId proto = ctx.dst.objByAddr(sub.value());
        if (OBJ_INVALID == proto)
            return /* continue */ true;

        if (objDst == proto)
            // self loop
            return /* continue */ true;

        if (!isFreshProto(ctx, proto))
            // not a newly introduced one
            return /* continue */ true;

        // this object became a prototype, increment its level
        objIncrementProtoLevel(ctx.dst, proto);
        return /* continue */ true;
    }
};

bool updateMayExistLevels(SymJoinCtx &ctx)
{
    TObjList dstRoots;
    ctx.dst.gatherObjects(dstRoots, isOnHeap);
    BOOST_FOREACH(const TObjId objDst, dstRoots) {
        const EObjKind kind = ctx.dst.objKind(objDst);
        if (!isMayExistObj(kind))
            // we are interested only in 0..1 objects here
            continue;

        bool wasMayExist;
        if (!isFreshProto(ctx, objDst, &wasMayExist) || wasMayExist)
            // not a newly introduced one
            continue;

        const MayExistLevelUpdater visitor(ctx, objDst);
        if (!traverseLivePtrs(ctx.dst, objDst, visitor))
            return false;
    }

    return true;
}

bool handleDstPreds(SymJoinCtx &ctx)
{
    // go through all segments and initialize minLength
    BOOST_FOREACH(SymJoinCtx::TSegLengths::const_reference ref, ctx.segLengths)
    {
        const TObjId  seg = ref.first;
        const TMinLen len = ref.second;
        ctx.dst.segSetMinLength(seg, len);
        ctx.dst.segSetMinLength(segPeer(ctx.dst, seg), len);
    }

    // go through shared Neq predicates
    BOOST_FOREACH(const TValPair &neq, ctx.sharedNeqs) {
        TValId valLt, valGt;
        boost::tie(valLt, valGt) = neq;

        const TObjId targetLt = ctx.dst.objByAddr(valLt);
        const TObjId targetGt = ctx.dst.objByAddr(valGt);
        if (hasKey(ctx.segLengths, targetLt)
                || hasKey(ctx.segLengths, targetGt))
            // preserve segment length
            continue;

        // handle generic Neq predicate
        ctx.dst.addNeq(valLt, valGt);
    }

    if (!ctx.joiningData()) {
        // cross-over check of Neq predicates

        if (!ctx.sh1.matchPreds(ctx.dst, ctx.valMap1[/* ltr */ 0])) {
            if (ctx.sh1.matchPreds(ctx.dst, ctx.valMap1[0], /* nonzero */ true))
                ctx.allowThreeWay = false;

            if (!updateJoinStatus(ctx, JS_USE_SH2))
                return false;
        }

        if (!ctx.sh2.matchPreds(ctx.dst, ctx.valMap2[/* ltr */ 0])) {
            if (ctx.sh2.matchPreds(ctx.dst, ctx.valMap2[0], /* nonzero */ true))
                ctx.allowThreeWay = false;

            if (!updateJoinStatus(ctx, JS_USE_SH1))
                return false;
        }
    }

    // TODO: match generic Neq predicates also in prototypes;  for now we
    // consider only minimal segment lengths
    BOOST_FOREACH(const TObjId protoDst, ctx.protos) {
        const TObjId proto1 = roMapLookup(ctx.objMap1[/* rtl */ 1], protoDst);
        const TObjId proto2 = roMapLookup(ctx.objMap2[/* rtl */ 1], protoDst);

        const TMinLen len1   = objMinLength(ctx.sh1, proto1);
        const TMinLen len2   = objMinLength(ctx.sh2, proto2);
        const TMinLen lenDst = objMinLength(ctx.dst, protoDst);

        if ((lenDst < len1) && !updateJoinStatus(ctx, JS_USE_SH2))
            return false;

        if ((lenDst < len2) && !updateJoinStatus(ctx, JS_USE_SH1))
            return false;
    }

    // all OK
    return true;
}

bool validateStatus(const SymJoinCtx &ctx)
{
    if (!ctx.allowThreeWay && ctx.forceThreeWay) {
        CL_DEBUG("!J! forcing three-way join");
        return true;
    }

    if (ctx.allowThreeWay || (JS_THREE_WAY != ctx.status))
        return true;

    CL_DEBUG(">J< cancelling three-way join");
    return false;
}

bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *pDst,
        SymHeap                  sh1,
        SymHeap                  sh2,
        const bool               allowThreeWay)
{
    SJ_DEBUG("--> joinSymHeaps()");
    TStorRef stor = sh1.stor();
    CL_BREAK_IF(&stor != &sh2.stor());

    // update trace
    Trace::waiveCloneOperation(sh1);
    Trace::waiveCloneOperation(sh2);
    *pDst = SymHeap(stor, new Trace::TransientNode("joinSymHeaps()"));

    // initialize symbolic join ctx
    SymJoinCtx ctx(*pDst, sh1, sh2, allowThreeWay);

    CL_BREAK_IF(!protoCheckConsistency(ctx.sh1));
    CL_BREAK_IF(!protoCheckConsistency(ctx.sh2));

    // first try to join return addresses (if in use)
    if (!joinReturnAddrs(ctx))
        goto fail;

    // start with program variables
    if (!joinCVars(ctx, JoinVarVisitor::JVM_LIVE_OBJS))
        goto fail;

    // go through all values in them
    if (!joinPendingValues(ctx))
        goto fail;

    // time to preserve all 'hasValue' edges
    if (!setDstValues(ctx))
        goto fail;

    // join uniform blocks
    if (!joinCVars(ctx, JoinVarVisitor::JVM_UNI_BLOCKS))
        goto fail;

    if (!updateMayExistLevels(ctx))
        goto fail;

    // go through shared Neq predicates and set minimal segment lengths
    if (!handleDstPreds(ctx))
        goto fail;

    // if the result is three-way join, check if it is a good idea
    if (!validateStatus(ctx))
        goto fail;

    // catch possible regression at this point
    CL_BREAK_IF((JS_USE_ANY == ctx.status) != areEqual(sh1, sh2));
    CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh1, ctx.dst));
    CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh2, ctx.dst));

    if (JS_THREE_WAY == ctx.status) {
        // create a new trace graph node for JS_THREE_WAY
        Trace::Node *tr1 = sh1.traceNode();
        Trace::Node *tr2 = sh2.traceNode();
        pDst->traceUpdate(new Trace::JoinNode(tr1, tr2));
    }

    // all OK
    *pStatus = ctx.status;
    SJ_DEBUG("<-- joinSymHeaps() says " << ctx.status);
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    CL_BREAK_IF(!protoCheckConsistency(ctx.dst));
    return true;

fail:
    // if the join failed on heaps that were isomorphic, something went wrong
    CL_BREAK_IF(areEqual(sh1, sh2));
    return false;
}

void mapGhostAddressSpace(
        SymJoinCtx              &ctx,
        const TValId            addrReal,
        const TValId            addrGhost,
        const EJoinStatus       action)
{
    CL_BREAK_IF(!ctx.joiningData());
    CL_BREAK_IF(addrReal < 0 || addrGhost < 0);

    TValMap &vMap = (JS_USE_SH1 == action)
        ? ctx.valMap1[/* ltr */ 0]
        : ctx.valMap2[/* ltr */ 0];

    CL_BREAK_IF(!hasKey(vMap, addrReal));
    const TValId image = vMap[addrReal];

    CL_BREAK_IF(hasKey(vMap, addrGhost) && vMap[addrGhost] != image);
    vMap[addrGhost] = image;
}

/// this runs only in debug build
bool dlSegCheckProtoConsistency(const SymJoinCtx &ctx)
{
    BOOST_FOREACH(const TObjId proto, ctx.protos) {
        if (OK_DLS != ctx.dst.objKind(proto))
            // we are interested only DLSs here
            continue;

        const TObjId peer = dlSegPeer(ctx.dst, proto);
        if (!hasKey(ctx.protos, peer)) {
            CL_ERROR("DLS prototype peer not a prototype");
            return false;
        }
    }

    return true;
}

// FIXME: this works only for nullified blocks anyway
void killUniBlocksUnderBindingPtrs(
        SymHeap                &sh,
        const BindingOff       &bf,
        const TObjId            obj)
{
    // go through next/prev pointers
    TFldSet blackList;
    buildIgnoreList(blackList, sh, obj, bf);
    BOOST_FOREACH(const FldHandle &fld, blackList) {
        if (VAL_NULL != fld.value())
            continue;

        // if there is a nullified block under next/prev pointer, kill it now
        fld.setValue(VAL_TRUE);
        fld.setValue(VAL_NULL);
    }
}

bool joinDataCore(
        SymJoinCtx             &ctx,
        const BindingOff       &off,
        const TObjId            obj1,
        const TObjId            obj2)
{
    CL_BREAK_IF(!ctx.joiningData());
    SymHeap &sh = ctx.sh1;

    CL_BREAK_IF(!sh.isValid(obj1) || !sh.isValid(obj2));

    TSizeRange size;
    if (!joinObjSize(&size, ctx, obj1, obj2))
        return false;

    // start with the given pair of objects and create a ghost object for them
    // create an image in ctx.dst
    const TObjId objDst = ctx.dst.heapAlloc(size);

    const TObjType clt1 = ctx.sh1.objEstimatedType(obj1);
    const TObjType clt2 = ctx.sh2.objEstimatedType(obj2);
    const TObjType clt = joinClt(ctx, clt1, clt2);
    if (clt)
        // preserve estimated type-info of the root
        ctx.dst.objSetEstimatedType(objDst, clt);

    TProtoLevel ldiff = 0;

    const EObjKind kind1 = sh.objKind(obj1);
    if (OK_REGION != kind1)
        --ldiff;

    const EObjKind kind2 = sh.objKind(obj2);
    if (OK_REGION != kind2)
        ++ldiff;

    // TODO: drop this!
    const TValId addr1     = ctx.sh1.addrOfTarget(obj1,   /* XXX */ TS_REGION);
    const TValId addr2     = ctx.sh2.addrOfTarget(obj2,   /* XXX */ TS_REGION);

    if (!joinFields(ctx, objDst, obj1, obj2, ldiff, &off))
        return false;

    ctx.sset1.insert(addr1);
    ctx.sset2.insert(addr2);

    // never step over DLS peer
    if (OK_DLS == kind1) {
        const TValId peer = sh.addrOfTarget(dlSegPeer(sh, obj1),
                /* XXX */ TS_REGION);
        ctx.sset1.insert(peer);
        if (peer != addr2)
            mapGhostAddressSpace(ctx, addr1, peer, JS_USE_SH1);
    }
    if (OK_DLS == kind2) {
        const TValId peer = sh.addrOfTarget(dlSegPeer(sh, obj2),
                /* XXX */ TS_REGION);
        ctx.sset2.insert(peer);
        if (peer != addr1)
            mapGhostAddressSpace(ctx, addr2, peer, JS_USE_SH2);
    }

    // perform main loop
    if (!joinPendingValues(ctx))
        return false;

    // batch assignment of all values in ctx.dst
    TFldSet blackList;
    buildIgnoreList(blackList, ctx.dst, objDst, off);
    if (!setDstValues(ctx, &blackList))
        return false;

    killUniBlocksUnderBindingPtrs(sh, off, obj1);
    killUniBlocksUnderBindingPtrs(sh, off, obj2);
    if (!joinUniBlocks(ctx, objDst, obj1, obj2))
        // failed to complement uniform blocks
        return false;

    // check consistency of DLS prototype peers
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    CL_BREAK_IF(!dlSegCheckProtoConsistency(ctx));

    // go through Neq predicates
    handleDstPreds(ctx);

    // if the result is three-way join, check if it is a good idea
    return validateStatus(ctx);
}

bool joinDataReadOnly(
        EJoinStatus             *pStatus,
        SymHeap                  sh,
        const BindingOff        &off,
        const TObjId             obj1,
        const TObjId             obj2,
        TObjSet                  protoObjs[1][2])
{
    SJ_DEBUG("--> joinDataReadOnly" << SJ_OBJP(obj1, obj2));
    Trace::waiveCloneOperation(sh);

    // go through the common part of joinData()/joinDataReadOnly()
    SymHeap tmp(sh.stor(), new Trace::TransientNode("joinDataReadOnly()"));
    SymJoinCtx ctx(tmp, sh);

    if (!joinDataCore(ctx, off, obj1, obj2))
        return false;

    unsigned cntProto1 = 0;
    unsigned cntProto2 = 0;

    // go through prototypes
    BOOST_FOREACH(const TObjId protoDst, ctx.protos) {
        const TObjId proto1 = roMapLookup(ctx.objMap1[/* rtl */ 1], protoDst);
        const TObjId proto2 = roMapLookup(ctx.objMap2[/* rtl */ 1], protoDst);

        if (OBJ_INVALID != proto1) {
            ++cntProto1;
            if (protoObjs)
                (*protoObjs)[0].insert(proto1);
        }

        if (OBJ_INVALID != proto2) {
            ++cntProto2;
            if (protoObjs)
                (*protoObjs)[1].insert(proto2);
        }
    }

    SJ_DEBUG("<-- joinDataReadOnly() says " << ctx.status << " (found "
             << cntProto1 << " | "
             << cntProto2 << " prototype objects)");

    *pStatus = ctx.status;
    return true;
}

void recoverPointersToSelf(
        SymHeap                 &sh,
        const TObjId            dst,
        const TObjId            src,
        const TObjId            ghost,
        const bool              bidir)
{
    redirectRefs(sh,
            /* pointingFrom */  dst,
            /* pointingTo   */  ghost,
            /* pointingWith */  TS_INVALID,
            /* redirectTo   */  dst,
            /* redirectWith */  TS_INVALID);

    if (!bidir)
        return;

    redirectRefs(sh,
            /* pointingFrom */  src,
            /* pointingTo   */  ghost,
            /* pointingWith */  TS_INVALID,
            /* redirectTo   */  src,
            /* redirectWith */  TS_INVALID);
}

void recoverPrototypes(
        SymJoinCtx             &ctx,
        const TObjId            dst,
        const TObjId            ghost)
{
    const unsigned cntProto = ctx.protos.size();
    if (cntProto)
        CL_DEBUG("    joinData() joins " << cntProto << " prototype objects");

    // go through prototypes
    BOOST_FOREACH(const TObjId protoGhost, ctx.protos) {
        redirectRefs(ctx.dst,
                /* pointingFrom */  protoGhost,
                /* pointingTo   */  ghost,
                /* pointingWith */  TS_INVALID,
                /* redirectTo   */  dst,
                /* redirectWith */  TS_INVALID);
    }
}

void restorePrototypeLengths(SymJoinCtx &ctx)
{
    CL_BREAK_IF(!ctx.joiningDataReadWrite());
    SymHeap &sh = ctx.dst;

    // restore minimal length of segment prototypes
    BOOST_FOREACH(const TObjId protoDst, ctx.protos) {
        typedef SymJoinCtx::TSegLengths TLens;

        const TLens &lens = ctx.segLengths;
        TLens::const_iterator it = lens.find(protoDst);
        if (lens.end() == it)
            continue;

        const TMinLen len = it->second;
        if (!len)
            continue;

        sh.segSetMinLength(protoDst, len);
        sh.segSetMinLength(segPeer(sh, protoDst), len);
    }
}

void transferContentsOfGhost(
        SymHeap                 &sh,
        const BindingOff        &bf,
        const TObjId            dst,
        const TObjId            ghost)
{
    TFldSet ignoreList;
    buildIgnoreList(ignoreList, sh, dst, bf);

    FldList live;
    sh.gatherLiveFields(live, ghost);
    BOOST_FOREACH(const FldHandle &fldGhost, live) {
        const FldHandle fldDst = translateFldHandle(sh, dst, fldGhost);
        if (hasKey(ignoreList, fldDst))
            // preserve binding pointers
            continue;

        const TValId valOld = fldDst.value();
        const TValId valNew = fldGhost.value();
        fldDst.setValue(valNew);

        const TObjId objOld = sh.objByAddr(valOld);
        if (collectJunk(sh, objOld))
            CL_DEBUG("    transferContentsOfGhost() drops a sub-heap (objOld)");
    }
}

void joinData(
        SymHeap                 &sh,
        const BindingOff        &bf,
        const TObjId             dst,
        const TObjId             src,
        const bool               bidir)
{
    // TODO: drop this!
    const TValId dstAt = sh.addrOfTarget(dst, /* XXX */ TS_REGION);
    const TValId srcAt = sh.addrOfTarget(src, /* XXX */ TS_REGION);

    SJ_DEBUG("--> joinData" << SJ_OBJP(dst, src));
    ++cntJoinOps;

    // used only for debugging (in case the debugging is enabled)
    bool isomorphismWasExpected = false;
#ifndef NDEBUG
    SymHeap inputHeapSnap(sh);
#endif
    if (debuggingSymJoin) {
        EJoinStatus status = JS_USE_ANY;
        if (!joinDataReadOnly(&status, sh, bf, dst, src, 0))
            CL_BREAK_IF("joinDataReadOnly() fails, why joinData() is called?");
        if (JS_USE_ANY == status)
            isomorphismWasExpected = true;
        else
            debugPlot(sh, "joinData", dstAt, srcAt, "00");
    }

    // go through the common part of joinData()/joinDataReadOnly()
    SymJoinCtx ctx(sh);
    if (!joinDataCore(ctx, bf, dst, src)) {
        CL_BREAK_IF("joinData() has failed, did joinDataReadOnly() succeed?");
        return;
    }

    if (!updateMayExistLevels(ctx)) {
        CL_BREAK_IF("updateMayExistLevels() has failed in joinData()");
        return;
    }

    // ghost is a transiently existing object representing the join of dst/src
    const TObjId ghost = roMapLookup(ctx.objMap1[0], dst);
    CL_BREAK_IF(ghost != roMapLookup(ctx.objMap2[0], src));

    // assign values within dst (and also in src if bidir == true)
    transferContentsOfGhost(ctx.dst, bf, dst, ghost);
    if (bidir)
        transferContentsOfGhost(ctx.dst, bf, src, ghost);

    // redirect some edges if necessary
    recoverPrototypes(ctx, dst, ghost);
    recoverPointersToSelf(sh, dst, src, ghost, bidir);
    restorePrototypeLengths(ctx);

    if (collectJunk(sh, ghost))
        CL_DEBUG("    joinData() drops a sub-heap (ghost)");

    SJ_DEBUG("<-- joinData() has finished " << ctx.status);
    if (JS_USE_ANY != ctx.status) {
        debugPlot(sh, "joinData", dstAt, srcAt, "01");
        if (isomorphismWasExpected)
            CL_BREAK_IF("joinData() status differs from joinDataReadOnly()");
    }
}
