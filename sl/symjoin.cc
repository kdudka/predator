/*
 * Copyright (C) 2010-2011 Kamil Dudka <kdudka@redhat.com>
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

#include "symcmp.hh"
#include "symgc.hh"
#include "symplot.hh"
#include "symseg.hh"
#include "symstate.hh"
#include "symutil.hh"
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

void debugSymJoin(const bool enable) {
    if (enable == ::debuggingSymJoin)
        return;

    CL_DEBUG("symjoin: debugSymJoin(" << enable << ") takes effect");
    ::debuggingSymJoin = enable;
}

#define SJ_VALP(v1, v2) "(v1 = #" << v1 << ", v2 = #" << v2 << ")"
#define SJ_OBJP(o1, o2) "(o1 = #" << o1 << ", o2 = #" << o2 << ")"

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

/// known to work only with TObjId/TValId
template <class TMap>
typename TMap::mapped_type roMapLookup(
        const TMap                          &roMap,
        const typename TMap::key_type       id)
{
    if (id <= 0)
        return id;

    typename TMap::const_iterator iter = roMap.find(id);
    return (roMap.end() == iter)
        ? static_cast<typename TMap::mapped_type>(-1)
        : iter->second;
}

/// current state, common for joinSymHeaps(), joinDataReadOnly() and joinData()
struct SymJoinCtx {
    SymHeap                     &dst;
    SymHeap                     &sh1;
    SymHeap                     &sh2;

    // they need to be black-listed for joinAbstractValues()
    std::set<TValId>            sset1;
    std::set<TValId>            sset2;

    std::vector<TObjId>         liveList1;
    std::vector<TObjId>         liveList2;

    TValMapBidir                valMap1;
    TValMapBidir                valMap2;

    WorkListWithUndo<TValPair>  wl;
    EJoinStatus                 status;
    bool                        allowThreeWay;

    typedef std::map<TValId /* seg */, unsigned /* len */>      TSegLengths;
    TSegLengths                 segLengths;
    std::set<TValPair>          sharedNeqs;

    std::set<TValPair>          tieBreaking;
    std::set<TValPair>          alreadyJoined;

    std::set<TValId /* dst */>  protoRoots;

    void initValMaps() {
        // VAL_NULL should be always mapped to VAL_NULL
        valMap1[0][VAL_NULL] = VAL_NULL;
        valMap1[1][VAL_NULL] = VAL_NULL;
        valMap2[0][VAL_NULL] = VAL_NULL;
        valMap2[1][VAL_NULL] = VAL_NULL;
    }

    /// constructor used by joinSymHeaps()
    SymJoinCtx(SymHeap &dst_, const SymHeap &sh1_, const SymHeap &sh2_):
        dst(dst_),
        sh1(/* XXX */ const_cast<SymHeap &>(sh1_)),
        sh2(/* XXX */ const_cast<SymHeap &>(sh2_)),
        status(JS_USE_ANY),
        allowThreeWay(1 < (SE_ALLOW_THREE_WAY_JOIN))
    {
        initValMaps();
    }

    /// constructor used by joinDataReadOnly()
    SymJoinCtx(SymHeap &tmp_, const SymHeap &sh_):
        dst(tmp_),
        sh1(/* XXX */ const_cast<SymHeap &>(sh_)),
        sh2(/* XXX */ const_cast<SymHeap &>(sh_)),
        status(JS_USE_ANY),
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
void dump_ctx(const SymJoinCtx &ctx) {
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
    cout << "    ctx.valMap1[0]     .size() = " << ctx.valMap1[0].size()
        << "\n";
    cout << "    ctx.valMap2[0]     .size() = " << ctx.valMap2[0].size()
        << "\n\n";

    // sumarize aux containers
    cout << "    ctx.segLengths     .size() = " << ctx.segLengths.size()
        << "\n";
    cout << "    ctx.sharedNeqs     .size() = " << ctx.sharedNeqs.size()
        << "\n";
    cout << "    ctx.alreadyJoined  .size() = " << ctx.alreadyJoined.size()
        << "\n";
    cout << "    ctx.protoRoots     .size() = " << ctx.protoRoots.size()
        << "\n\n";

    // print queue stats
    cout << "    ctx.wl          .cntSeen() = " << ctx.wl.cntSeen() << "\n";
    cout << "    ctx.wl          .cntTodo() = " << ctx.wl.cntTodo() << "\n\n";

    // print the current status
    cout << "    ctx.status = " << ctx.status << "\n";
}

/// update ctx.status according to action
bool updateJoinStatus(SymJoinCtx &ctx, const EJoinStatus action) {
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
        || ctx.allowThreeWay;
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
        if (!ctx.sh1.SymHeapCore::proveNeq(v1, rel1))
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
        if (!ctx.sh2.SymHeapCore::proveNeq(v2, rel2))
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

/// read-only (in)consistency check among value pair (v1, v2)
bool checkValueMapping(
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              allowUnknownMapping)
{
    if (!checkNonPosValues(v1, v2))
        return false;

    const TOffset off1 = ctx.sh1.valOffset(v1);
    const TOffset off2 = ctx.sh2.valOffset(v2);
    if (off1 != off2)
        return false;

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

    if (hasMapping1 && hasMapping1 && (vDst1 == vDst2))
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

    if (ctx.joiningData())
        // TODO: explain exactly why we need it to digest test-0116.c
        return true;

    const EValueTarget code = (isNull2)
        ? ctx.sh1.valTarget(v1)
        : ctx.sh2.valTarget(v2);

    switch (code) {
#if 0
        case VT_ABSTRACT:
#endif
        case VT_STATIC:
        case VT_ON_STACK:
        case VT_ON_HEAP:
            // implies inconsistency
            return false;

        default:
            // OK
            return true;
    }
}

/// (OBJ_INVALID == objDst) means read-only!!!
bool joinFreshObjTripple(
        SymJoinCtx              &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const TObjId            objDst)
{
    const bool segClone = (OBJ_INVALID == obj1 || OBJ_INVALID == obj2);
    const bool readOnly = (OBJ_INVALID == objDst);
    CL_BREAK_IF(segClone && readOnly);
    CL_BREAK_IF(obj1 < 0 && obj2 < 0);

    const TValId v1 = ctx.sh1.valueOf(obj1);
    const TValId v2 = ctx.sh2.valueOf(obj2);
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
    }
    else if (readOnly)
        return checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ true);

    if (segClone) {
        const bool isGt1 = (OBJ_INVALID == obj2);
        const TValMapBidir &vm = (isGt1) ? ctx.valMap1 : ctx.valMap2;
        const TValId val = (isGt1)
            ? ctx.sh1.valRoot(v1)
            : ctx.sh2.valRoot(v2);

        if (val <= 0 || hasKey(vm[/* lrt */ 0], val))
            return true;

        // XXX
        if (!ctx.joiningData())
            return false;
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
    }

    if (ctx.wl.schedule(TValPair(v1, v2)))
        SJ_DEBUG("+++ " << SJ_VALP(v1, v2) << " <- " << SJ_OBJP(obj1, obj2));

    return true;
}

struct ObjJoinVisitor {
    SymJoinCtx              &ctx;
    std::set<TObjId>        blackList1;
    std::set<TObjId>        blackList2;
    bool                    noFollow;

    ObjJoinVisitor(SymJoinCtx &ctx_):
        ctx(ctx_),
        noFollow(false)
    {
    }

    bool operator()(const TObjId item[3]) {
        const TObjId obj1   = item[0];
        const TObjId obj2   = item[1];
        const TObjId objDst = item[2];

        // check black-list
        if (hasKey(blackList1, obj1) || hasKey(blackList2, obj2))
            return /* continue */ true;

        return /* continue */ joinFreshObjTripple(ctx, obj1, obj2, objDst);
    }
};

template <class TDst>
void dlSegBlackListPrevPtr(TDst &dst, SymHeap &sh, TValId root) {
    const EObjKind kind = sh.valTargetKind(root);
    if (OK_DLS != kind)
        return;

    const TObjId prevPtr = prevPtrFromSeg(sh, root);
    dst.insert(prevPtr);
}

struct SegMatchVisitor {
    SymJoinCtx              &ctx;
    std::set<TObjId>        blackList1;
    std::set<TObjId>        blackList2;

    public:
        SegMatchVisitor(SymJoinCtx &ctx_):
            ctx(ctx_)
        {
        }

        bool operator()(const TObjId item[2]) {
            const TObjId obj1 = item[0];
            const TObjId obj2 = item[1];

            if (hasKey(blackList1, obj1) || hasKey(blackList2, obj2))
                // black-listed
                return true;

            return joinFreshObjTripple(ctx, obj1, obj2,
                                       /* read-only */ OBJ_INVALID);
        }
};

bool traverseRoots(
        SymJoinCtx              &ctx,
        const TValId            rootDst,
        const TValId            root1,
        const TValId            root2,
        const BindingOff        *offBlackList = 0)
{
    if (!defineValueMapping(ctx, root1, root2, rootDst))
        return false;

    TValId roots[] = {
        root1,
        root2,
        rootDst
    };

    SymHeap *const heaps[] = {
        &ctx.sh1,
        &ctx.sh2,
        &ctx.dst
    };

    if (VAL_INVALID != root1)
        ctx.sh1.gatherLiveObjects(ctx.liveList1, root1);
    if (VAL_INVALID != root2)
        ctx.sh2.gatherLiveObjects(ctx.liveList2, root2);

    // initialize visitor
    ObjJoinVisitor objVisitor(ctx);
    dlSegBlackListPrevPtr(objVisitor.blackList1, ctx.sh1, root1);
    dlSegBlackListPrevPtr(objVisitor.blackList2, ctx.sh2, root2);

    if (offBlackList) {
        buildIgnoreList(objVisitor.blackList1, ctx.sh1, root1, *offBlackList);
        buildIgnoreList(objVisitor.blackList2, ctx.sh2, root2, *offBlackList);
    }
    else if (ctx.joiningData()) {
        if (root1 == root2)
            // do not follow shared data
            return true;
        else
            ctx.protoRoots.insert(rootDst);
    }

    // guide the visitors through them
    return traverseLiveObjsGeneric<3>(heaps, roots, objVisitor);
}

bool segMatchLookAhead(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2)
{
    const int size1 = ctx.sh1.valSizeOfTarget(root1);
    const int size2 = ctx.sh2.valSizeOfTarget(root2);
    if (size1 != size2)
        // size mismatch
        return false;

    // set up a visitor
    SymHeap *const heaps[] = { &ctx.sh1, &ctx.sh2 };
    TValId roots[] = { root1, root2 };
    SegMatchVisitor visitor(ctx);

    dlSegBlackListPrevPtr(visitor.blackList1, ctx.sh1, root1);
    dlSegBlackListPrevPtr(visitor.blackList2, ctx.sh2, root2);

    // FIXME: this will break as soon as we switch to delayed objects creation
    return traverseLiveObjsGeneric<2>(heaps, roots, visitor);
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
        EObjKind                *pDst,
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const EJoinStatus       action)
{
    CL_BREAK_IF(VAL_INVALID == v1 && VAL_INVALID == v2);

    const EObjKind kind1 = ctx.sh1.valTargetKind(v1);
    if (VAL_INVALID == v2) {
        *pDst = kind1;
        return true;
    }

    const EObjKind kind2 = ctx.sh2.valTargetKind(v2);
    if (VAL_INVALID == v1) {
        *pDst = kind2;
        return true;
    }

    if (OK_CONCRETE == kind1) {
        CL_BREAK_IF(action == JS_USE_SH1);
        *pDst = kind2;
        return true;
    }

    if (OK_CONCRETE == kind2) {
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
            SJ_DEBUG("<-- object kind mismatch " << SJ_VALP(v1, v2));
            return false;
    }
}

bool joinSegBindingOfMayExist(
        bool                    *pResult,
        BindingOff              *pOff,
        const SymJoinCtx        &ctx,
        const TValId            seg1,
        const TValId            seg2)
{
    const bool isMayExist1 = (OK_MAY_EXIST == ctx.sh1.valTargetKind(seg1));
    const bool isMayExist2 = (OK_MAY_EXIST == ctx.sh2.valTargetKind(seg2));
    if (!isMayExist1 && !isMayExist2)
        // no OK_MAY_EXIST involved
        return false;

    const BindingOff off1 = ctx.sh1.segBinding(seg1);
    const BindingOff off2 = ctx.sh2.segBinding(seg2);
    if (off1.head != off2.head) {
        // head mismatch
        *pResult = false;
        return true;
    }

    *pOff = (isMayExist2) ? off1 : off2;

    const TOffset offNext1 = off1.next;
    const TOffset offNext2 = off2.next;
    if (offNext1 == offNext2) {
        // the 'next' offset matches trivially
        *pResult = true;
        return true;
    }

    // NOTE: test-0129 utilizes this code path

    if (isMayExist1) {
        const TValId valNextBy1 = valOfPtrAt(ctx.sh1, seg1, offNext1);
        const TValId valNextBy2 = valOfPtrAt(ctx.sh1, seg1, offNext2);
        if (valNextBy1 == valNextBy2)
            goto match;
    }

    if (isMayExist2) {
        const TValId valNextBy1 = valOfPtrAt(ctx.sh2, seg2, offNext1);
        const TValId valNextBy2 = valOfPtrAt(ctx.sh2, seg2, offNext2);
        if (valNextBy1 == valNextBy2)
            goto match;
    }

    // giving up
    *pResult = false;
    return true;

match:
    SJ_DEBUG("non-trivial match of 'next' offset of OK_MAY_EXIST");
    *pResult = true;
    return true;
}

bool joinSegBinding(
        BindingOff              *pOff,
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2)
{
    const bool isSeg1 = isAbstract(ctx.sh1.valTarget(v1));
    const bool isSeg2 = isAbstract(ctx.sh2.valTarget(v2));
    if (!isSeg1 && !isSeg2)
        // nothing to join here
        return true;

    if (isSeg1 && isSeg2) {
        bool result;
        if (!joinSegBindingOfMayExist(&result, pOff, ctx, v1, v2)) {
            // just compare the binding offsets
            const BindingOff off1 = ctx.sh1.segBinding(v1);
            const BindingOff off2 = ctx.sh2.segBinding(v2);
            if ((result = (off1 == off2)))
                *pOff = off1;
        }

        if (result)
            return true;

        SJ_DEBUG("<-- segment binding mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (isSeg1) {
        *pOff = ctx.sh1.segBinding(v1);
        return true;
    }

    if (isSeg2) {
        *pOff = ctx.sh2.segBinding(v2);
        return true;
    }

    // not reachable
    CL_TRAP;
    return false;
}

bool considerImplicitPrototype(
        const SymJoinCtx        &ctx,
        const TValId            root1,
        const TValId            root2)
{
    const bool isProto1 = ctx.sh1.valTargetIsProto(root1);
    const bool isProto2 = ctx.sh2.valTargetIsProto(root2);
    CL_BREAK_IF(isProto1 == isProto2);
    (void) isProto1;

    SymHeap &sh = (isProto2) ? ctx.sh1 : ctx.sh2;
    const TValId root = (isProto2) ? root1 : root2;

    TObjList refs;
    sh.pointedBy(refs, root);
    BOOST_FOREACH(const TObjId obj, refs) {
        const TValId at = sh.placedAt(obj);
        if (OK_CONCRETE != sh.valTargetKind(at))
            return false;
    }

    SJ_DEBUG("P-P considerImplicitPrototype() matches a pair of objects: "
             << SJ_VALP(root1, root2));
    return true;
}

bool joinProtoFlag(
        bool                    *pDst,
        const SymJoinCtx        &ctx,
        const TValId            root1,
        const TValId            root2)
{
    const bool isProto1 = ctx.sh1.valTargetIsProto(root1);
    const bool isProto2 = ctx.sh2.valTargetIsProto(root2);

    if (VAL_INVALID == root2) {
        *pDst = isProto1;
        return true;
    }

    if (VAL_INVALID == root1) {
        *pDst = isProto2;
        return true;
    }

    *pDst = isProto1;
    if (isProto2 == *pDst)
        return true;

    if (ctx.joiningData() || considerImplicitPrototype(ctx, root1, root2)) {
        *pDst = true;
        return true;
    }

    SJ_DEBUG("<-- prototype vs shared: " << SJ_VALP(root1, root2));
    return false;
}

unsigned joinMinLength(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2)
{
    if (VAL_INVALID == root1 || VAL_INVALID == root2) {
        // the status should have been already updated
        CL_BREAK_IF(JS_USE_ANY == ctx.status);

        if (objMinLength(ctx.sh1, root1) || objMinLength(ctx.sh2, root2))
            // insertion of non-empty object does not cover both variants
            updateJoinStatus(ctx, JS_THREE_WAY);

        return 0;
    }

    const int len1 = objMinLength(ctx.sh1, root1);
    const int len2 = objMinLength(ctx.sh2, root2);
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

bool joinObjSize(
        int                     *pDst,
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
    if (VAL_INVALID == v1) {
        *pDst = ctx.sh2.valSizeOfTarget(v2);
        return true;
    }

    if (VAL_INVALID == v2) {
        *pDst = ctx.sh1.valSizeOfTarget(v1);
        return true;
    }

    const int cbSize1 = ctx.sh1.valSizeOfTarget(v1);
    const int cbSize2 = ctx.sh2.valSizeOfTarget(v2);
    if (cbSize1 != cbSize2) {
        SJ_DEBUG("<-- object size mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    *pDst = cbSize1;
    return true;
}

void importBlockMap(
        TUniBlockMap            *pMap,
        SymHeap                 &dst,
        const SymHeap           &src)
{
    BOOST_FOREACH(TUniBlockMap::reference item, *pMap) {
        UniformBlock &bl = item.second;
        translateValProto(&bl.tplValue, dst, src);
    }
}

void joinUniBlocksCore(
        TUniBlockMap            *pMap,
        bool                    *pExtra1,
        bool                    *pExtra2,
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2)
{
    const SymHeap &sh1 = ctx.sh1;
    const SymHeap &sh2 = ctx.sh2;

    TUniBlockMap bMap1, bMap2;
    sh1.gatherUniformBlocks(bMap1, root1);
    sh2.gatherUniformBlocks(bMap2, root2);
    CL_BREAK_IF(bMap1.empty() && bMap2.empty());

    if (bMap2.empty()) {
        *pExtra1 = true;
        return;
    }
    if (bMap1.empty()) {
        *pExtra2 = true;
        return;
    }

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap1) {
        const TOffset off = item.first;
        const UniformBlock &bl1 = item.second;
        UniformBlock bl2;
        if (!sh2.findCoveringUniBlock(&bl2, root2, off, bl1.size)
                || !areValProtosEqual(sh1, sh2, bl1.tplValue, bl2.tplValue))
        {
            *pExtra1 = true;
            continue;
        }

        UniformBlock blDst(bl1);
        translateValProto(&blDst.tplValue, ctx.dst, /* src */ sh1);
        (*pMap)[off] = blDst;
    }

    BOOST_FOREACH(TUniBlockMap::const_reference item, bMap2) {
        const TOffset off = item.first;
        const UniformBlock &bl2 = item.second;
        UniformBlock bl1;
        if (!sh1.findCoveringUniBlock(&bl1, root1, off, bl2.size)
                || !areValProtosEqual(sh1, sh2, bl1.tplValue, bl2.tplValue))
        {
            *pExtra2 = true;
            continue;
        }

        if (hasKey(*pMap, off)) {
            // symmetric match (should have been already handled)
            CL_BREAK_IF(!areUniBlocksEqual(ctx.dst, sh2, (*pMap)[off], bl2));
            continue;
        }

        UniformBlock blDst(bl2);
        translateValProto(&blDst.tplValue, ctx.dst, /* src */ sh2);
        (*pMap)[off] = blDst;
    }
}

bool joinUniBlocks(
        SymJoinCtx              &ctx,
        const TValId            rootDst,
        const TValId            root1,
        const TValId            root2)
{
    TUniBlockMap bMapDst;
    bool hasExtra1 = false;
    bool hasExtra2 = false;

    if (VAL_INVALID == root2) {
        hasExtra1 = true;
        ctx.sh1.gatherUniformBlocks(bMapDst, root1);
        importBlockMap(&bMapDst, ctx.dst, /* src */ ctx.sh1);
    }
    else if (VAL_INVALID == root1) {
        hasExtra2 = true;
        ctx.sh2.gatherUniformBlocks(bMapDst, root2);
        importBlockMap(&bMapDst, ctx.dst, /* src */ ctx.sh2);
    }
    else if (matchUniBlocks(ctx.sh1, ctx.sh2, root1, root2)) {
        // block maps are 100% equal, pick any
        ctx.sh1.gatherUniformBlocks(bMapDst, root1);
        importBlockMap(&bMapDst, ctx.dst, /* src */ ctx.sh1);
        goto write_them;
    }
    else
        joinUniBlocksCore(&bMapDst, &hasExtra1, &hasExtra2, ctx, root1, root2);

    // FIXME: updating the status now may trigger an unnecessary JS_THREE_WAY
    if (hasExtra1 && !updateJoinStatus(ctx, JS_USE_SH2))
        return false;
    if (hasExtra2 && !updateJoinStatus(ctx, JS_USE_SH1))
        return false;

write_them:
    BOOST_FOREACH(TUniBlockMap::const_reference item, bMapDst) {
        const UniformBlock &bl = item.second;
        const TValId addrDst = ctx.dst.valByOffset(rootDst, bl.off);
        ctx.dst.writeUniformBlock(addrDst, bl.tplValue, bl.size);
    }

    return true;
}

/// (NULL != offMayExist) means 'create OK_MAY_EXIST'
bool createObject(
        SymJoinCtx              &ctx,
        const struct cl_type    *clt,
        const TValId            root1,
        const TValId            root2,
        const EJoinStatus       action,
        const BindingOff        *offMayExist = 0)
{
    EObjKind kind;
    if (!joinObjKind(&kind, ctx, root1, root2, action))
        return false;

    BindingOff off;
    if (!joinSegBinding(&off, ctx, root1, root2))
        return false;

    bool isProto;
    if (!joinProtoFlag(&isProto, ctx, root1, root2))
        return false;

    if (offMayExist) {
        // we are asked to introduce OK_MAY_EXIST
        CL_BREAK_IF(OK_CONCRETE != kind && OK_MAY_EXIST != kind);
        kind = OK_MAY_EXIST;
        off = *offMayExist;
    }

    int size;
    if (!joinObjSize(&size, ctx, root1, root2))
        return false;

    if (!updateJoinStatus(ctx, action))
        return false;

    // create an image in ctx.dst
    const TValId rootDst = ctx.dst.heapAlloc(size);

    if (!joinUniBlocks(ctx, rootDst, root1, root2))
        // failed to complement uniform blocks
        return false;

    if (clt)
        // preserve estimated type-info of the root
        ctx.dst.valSetLastKnownTypeOfTarget(rootDst, clt);

    // preserve 'prototype' flag
    ctx.dst.valTargetSetProto(rootDst, isProto);

    if (OK_CONCRETE != kind) {
        // abstract object
        ctx.dst.valTargetSetAbstract(rootDst, kind, off);

        // compute minimal length of the resulting segment
        ctx.segLengths[rootDst] = joinMinLength(ctx, root1, root2);
    }

    return traverseRoots(ctx, rootDst, root1, root2);
}

bool followRootValuesCore(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2,
        const EJoinStatus       action,
        const bool              readOnly)
{
    if (!checkValueMapping(ctx, root1, root2, /* allowUnknownMapping */ true))
        return false;

    if (hasKey(ctx.valMap1[0], root1) && hasKey(ctx.valMap2[0], root2))
        return true;

    const TObjType clt1 = ctx.sh1.valLastKnownTypeOfTarget(root1);
    const TObjType clt2 = ctx.sh2.valLastKnownTypeOfTarget(root2);
    const TObjType clt = joinClt(ctx, clt1, clt2);

    if (readOnly)
        // do not create any object, just check if it was possible
        return segMatchLookAhead(ctx, root1, root2);

    if (ctx.joiningDataReadWrite() && root1 == root2)
        // we are on the way from joinData() and hit shared data
        return traverseRoots(ctx, root1, root1, root1);

    return createObject(ctx, clt, root1, root2, action);
}

bool dlSegHandleShared(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2,
        const EJoinStatus       action,
        const bool              readOnly)
{
    CL_BREAK_IF(ctx.sh1.valOffset(root1) || ctx.sh2.valOffset(root2));

    const bool isDls = (OK_DLS == ctx.sh1.valTargetKind(root1));
    CL_BREAK_IF(isDls != (OK_DLS == ctx.sh2.valTargetKind(root2)));
    if (!isDls)
        // not a DLS
        return true;

    // this should follow the 'next' pointer as long as we have a consistent DLS
    const TValId peer1 = dlSegPeer(ctx.sh1, root1);
    const TValId peer2 = dlSegPeer(ctx.sh2, root2);
    if (!followRootValuesCore(ctx, peer1, peer2, action, readOnly))
        return false;

    if (readOnly)
        // we are done
        return true;

    // check the mapping
    TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
#ifndef NDEBUG
    TValMap &vMap2 = ctx.valMap2[/* ltr */ 0];
    CL_BREAK_IF(!hasKey(vMap1, root1));
    CL_BREAK_IF(!hasKey(vMap2, root2));
    CL_BREAK_IF(!hasKey(vMap1, peer1));
    CL_BREAK_IF(!hasKey(vMap2, peer2));
#endif

    // we might have just joined a DLS pair as shared data, which would lead to
    // unconnected DLS pair in ctx.dst and later cause some problems;  the best
    // thing to do at this point, is to recover the binding of DLS in ctx.dst
    const TValId seg  = vMap1[root1];
    const TValId peer = vMap1[peer1];
    CL_BREAK_IF(seg  != vMap2[root2]);
    CL_BREAK_IF(peer != vMap2[peer2]);

    SymHeap &sh = ctx.dst;
    const TObjId prev1 = prevPtrFromSeg(sh,  seg);
    const TObjId prev2 = prevPtrFromSeg(sh, peer);

    sh.objSetValue(prev1, segHeadAt(sh, peer));
    sh.objSetValue(prev2, segHeadAt(sh,  seg));

    sh.objReleaseId(prev1);
    sh.objReleaseId(prev2);

    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    return true;
}

bool joinReturnAddrs(SymJoinCtx &ctx) {
    TObjType clt;
    const TObjType clt1 = ctx.sh1.valLastKnownTypeOfTarget(VAL_ADDR_OF_RET);
    const TObjType clt2 = ctx.sh2.valLastKnownTypeOfTarget(VAL_ADDR_OF_RET);
    if (!joinClt(&clt, clt1, clt2))
        // mismatch in type of return value
        return false;

    if (!clt)
        // nothing to join here
        return true;

    ctx.dst.valSetLastKnownTypeOfTarget(VAL_ADDR_OF_RET, clt);
    return traverseRoots(ctx,
            VAL_ADDR_OF_RET,
            VAL_ADDR_OF_RET,
            VAL_ADDR_OF_RET);
}

bool joinCustomValues(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
    const CustomValue cVal1 = ctx.sh1.valUnwrapCustom(v1);
    const CustomValue cVal2 = ctx.sh2.valUnwrapCustom(v2);
    if (cVal1 == cVal2) {
        // full match
        const TValId vDst = ctx.dst.valWrapCustom(cVal1);
        return defineValueMapping(ctx, v1, v2, vDst);
    }

    const ECustomValue code = cVal1.code;
    if (cVal2.code != code || CV_INT != code) {
        SJ_DEBUG("<-- custom values mismatch " << SJ_VALP(v1, v2));
        return false;
    }

#if SE_INT_ARITHMETIC_LIMIT
    const long abs1 = std::abs(cVal1.data.num);
    const long abs2 = std::abs(cVal2.data.num);
    const long max = std::max(abs1, abs2);
    if (max <= (SE_INT_ARITHMETIC_LIMIT)) {
        SJ_DEBUG("<-- integral values preserved by SE_INT_ARITHMETIC_LIMIT "
                << SJ_VALP(v1, v2));

        return false;
    }
#endif

    // throw custom values away and abstract them by a fresh unknown value
    const TValId vDst = ctx.dst.valCreate(VT_UNKNOWN, VO_UNKNOWN);
    updateJoinStatus(ctx, JS_THREE_WAY);
    return defineValueMapping(ctx, v1, v2, vDst);
}

bool followRootValues(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2,
        const EJoinStatus       action,
        const bool              readOnly = false)
{
    if (!followRootValuesCore(ctx, root1, root2, action, readOnly))
        return false;

    if (!ctx.joiningData())
        // we are on the way from joinSymHeaps()
        return true;

    if (root1 == root2)
        // shared data
        return dlSegHandleShared(ctx, root1, root2, action, readOnly);

    if (readOnly)
        // postpone it till the read-write attempt
        return true;

    const bool isDls1 = (OK_DLS == ctx.sh1.valTargetKind(root1));
    const bool isDls2 = (OK_DLS == ctx.sh2.valTargetKind(root2));
    if (isDls1 == isDls2)
        return true;

    CL_BREAK_IF(isDls1 && JS_USE_SH1 != action);
    CL_BREAK_IF(isDls2 && JS_USE_SH2 != action);

    const TValId peer1 = (isDls1) ? dlSegPeer(ctx.sh1, root1) : VAL_INVALID;
    const TValId peer2 = (isDls2) ? dlSegPeer(ctx.sh2, root2) : VAL_INVALID;

    const TValMapBidir &vm = (isDls1) ? ctx.valMap1 : ctx.valMap2;
    if (hasKey(vm[0], (isDls1) ? peer1 : peer2))
        // alredy cloned
        return true;

    // clone peer object
    const TValPair tb(peer1, peer2);
    ctx.tieBreaking.insert(tb);

    const struct cl_type *clt = (isDls1)
        ? ctx.sh1.valLastKnownTypeOfTarget(peer1)
        : ctx.sh2.valLastKnownTypeOfTarget(peer2);
    return createObject(ctx, clt, peer1, peer2, action);
}

bool followValuePair(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              readOnly)
{
    if (readOnly)
        // shallow scan only!
        return checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ true);

    if (ctx.sh1.valOffset(v1) != ctx.sh2.valOffset(v2)) {
        SJ_DEBUG("<-- value offset mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    const bool isCustom1 = (VT_CUSTOM == ctx.sh1.valTarget(v1));
    const bool isCustom2 = (VT_CUSTOM == ctx.sh2.valTarget(v2));
    if (isCustom1 || isCustom2) {
        if (!isCustom1 || !isCustom2) {
            SJ_DEBUG("<-- custom value vs. something else " << SJ_VALP(v1, v2));
            return false;
        }

        return joinCustomValues(ctx, v1, v2);
    }

    // follow the roots
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    return followRootValues(ctx, root1, root2, JS_USE_ANY);
}

void considerValSchedule(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            byVal1,
        const TValId            byVal2)
{
    if (VAL_NULL == v1 && VAL_NULL == v2)
        return;

    if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
        return;

    const TValPair vp(v1, v2);
    if (!ctx.wl.schedule(vp))
        return;

    SJ_DEBUG("+++ " << SJ_VALP(v1, v2) << " <- " << SJ_VALP(byVal1, byVal2));
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
        TValId                  root1,
        TValId                  root2,
        const EJoinStatus       action,
        bool                    firstTryReadOnly = true)
{
    if (segAlreadyJoined(ctx, root1, root2, action)) {
        // already joined
        *pResult = true;
        return true;
    }

    SJ_DEBUG(">>> joinSegmentWithAny" << SJ_VALP(root1, root2));
    if (root1 <= 0 || root2 <= 0) {
        CL_BREAK_IF(firstTryReadOnly);
        *pResult = false;
        return true;
    }

    const bool isDls1 = (OK_DLS == ctx.sh1.valTargetKind(root1));
    const bool isDls2 = (OK_DLS == ctx.sh2.valTargetKind(root2));

    TValId peer1 = root1;
    if (isDls1)
        peer1 = dlSegPeer(ctx.sh1, root1);

    TValId peer2 = root2;
    if (isDls2)
        peer2 = dlSegPeer(ctx.sh2, root2);

    if (firstTryReadOnly && !followRootValues(ctx, root1, root2, action,
                /* read-only */ true))
    {
        SJ_DEBUG("<<< joinSegmentWithAny" << SJ_VALP(root1, root2));
        return false;
    }

    const bool haveDls = (isDls1 || isDls2);
    if (firstTryReadOnly
            && haveDls
            && !followRootValues(ctx, peer1, peer2, action,
                /* read-only */ true))
    {
        SJ_DEBUG("<<< joinSegmentWithAny" << SJ_VALP(peer1, peer2));
        return false;
    }

    // BindingOff is assumed to be already matching at this point
    BindingOff off = (JS_USE_SH1 == action)
        ? ctx.sh1.segBinding(peer1)
        : ctx.sh2.segBinding(peer2);

    const TValId valNext1 = valOfPtrAt(ctx.sh1, peer1, off.next);
    const TValId valNext2 = valOfPtrAt(ctx.sh2, peer2, off.next);
    if (firstTryReadOnly && !checkValueMapping(ctx, valNext1, valNext2,
                           /* allowUnknownMapping */ true))
    {
        SJ_DEBUG("<<< joinSegmentWithAny" << SJ_VALP(root1, root2));
        return false;
    }

    if (firstTryReadOnly && haveDls) {
        const TValId valPrev1 = valOfPtrAt(ctx.sh1, root1, off.prev);
        const TValId valPrev2 = valOfPtrAt(ctx.sh2, root2, off.prev);
        if (!checkValueMapping(ctx, valPrev1, valPrev2,
                               /* allowUnknownMapping */ true))
        {
            SJ_DEBUG("<<< joinSegmentWithAny" << SJ_VALP(root1, root2));
            return false;
        }
    }

    // go ahead, try it read-write!
    *pResult = followRootValues(ctx, root1, root2, action);
    if (!haveDls || !*pResult)
        return true;

    if (!segAlreadyJoined(ctx, peer1, peer2, action))
        *pResult = followRootValues(ctx, peer1, peer2, action);

    return true;
}

/// (NULL != off) means 'introduce OK_MAY_EXIST'
bool segmentCloneCore(
        SymJoinCtx                  &ctx,
        SymHeap                     &shGt,
        const TValId                valGt,
        const TValMapBidir          &valMapGt,
        const EJoinStatus           action,
        const BindingOff            *off)
{
    if (!isPossibleToDeref(shGt.valTarget(valGt)))
        // not valid target
        return false;

    const TValId addrGt = shGt.valRoot(valGt);
    if (hasKey(valMapGt[0], addrGt))
        // mapping already available for objGt
        return true;

    const TObjType clt = shGt.valLastKnownTypeOfTarget(addrGt);
    SJ_DEBUG("+i+ insertSegmentClone: cloning object at #" << addrGt <<
             ", action = " << action);

    const TValId root1 = (JS_USE_SH1 == action) ? addrGt : VAL_INVALID;
    const TValId root2 = (JS_USE_SH2 == action) ? addrGt : VAL_INVALID;

    // clone the object
    const TValPair tb(root1, root2);
    ctx.tieBreaking.insert(tb);
    if (createObject(ctx, clt, root1, root2, action, off))
        return true;

    SJ_DEBUG("<-- insertSegmentClone: failed to create object "
             << SJ_VALP(root1, root2));
    return false;
}

template <class TWorkList>
void scheduleSegAddr(
        TWorkList               &wl,
        const TValId            seg,
        const TValId            peer,
        const EJoinStatus       action)
{
    CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);

    const TValPair vpSeg(
            (JS_USE_SH1 == action) ? seg : VAL_INVALID,
            (JS_USE_SH2 == action) ? seg : VAL_INVALID);
    if (!wl.schedule(vpSeg))
        // XXX
        wl.undo(vpSeg);

    if (seg == peer)
        return;

    const TValPair vpPeer(
            (JS_USE_SH1 == action) ? peer : VAL_INVALID,
            (JS_USE_SH2 == action) ? peer : VAL_INVALID);
    if (!wl.schedule(vpPeer))
        // XXX
        wl.undo(vpPeer);
}

bool handleUnknownValues(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    const bool isNull1 = (VAL_NULL == v1);
    const bool isNull2 = (VAL_NULL == v2);
    if (isNull1 != isNull2)
        return false;

    CL_BREAK_IF(isNull1 && isNull2);
    return defineValueMapping(ctx, v1, v2, vDst);
}

/// (NULL != off) means 'introduce OK_MAY_EXIST'
bool insertSegmentClone(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const EJoinStatus       action,
        const BindingOff        *off = 0)
{
    SJ_DEBUG(">>> insertSegmentClone" << SJ_VALP(v1, v2));

    const bool isGt1 = (JS_USE_SH1 == action);
    const bool isGt2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(isGt1 == isGt2);

    // resolve the existing segment in shGt
    SymHeap &shGt = ((isGt1) ? ctx.sh1 : ctx.sh2);
    const TValId seg = shGt.valRoot((isGt1) ? v1 : v2);
    const bool isDls = (OK_DLS == shGt.valTargetKind(seg));
    CL_BREAK_IF(off && isDls);

    TValId peer = seg;
    if (isDls)
        peer = dlSegPeer(shGt, seg);

    // resolve the 'next' pointer and check its validity
    const TObjId nextPtr = (off)
        ? shGt.ptrAt(shGt.valByOffset(seg, off->next))
        : nextPtrFromSeg(shGt, peer);

    const TValId nextGt = shGt.valueOf(nextPtr);
    shGt.objReleaseId(nextPtr);

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
    if (!ctx.joiningData())
        // on the way from joinSymHeaps(), some three way joins are destructive
        ctx.allowThreeWay = false;
#endif

    const TValMapBidir &valMapGt = (isGt1)
        ? ctx.valMap1
        : ctx.valMap2;

    TValPair vp;
    scheduleSegAddr(ctx.wl, seg, peer, action);
    while (ctx.wl.next(vp)) {
        const TValId valGt = (isGt1) ? vp.first : vp.second;
        const TValId valLt = (isGt2) ? vp.first : vp.second;
        if (VAL_INVALID != valLt) {
            // process the rest of ctx.wl rather in joinPendingValues()
            ctx.wl.undo(vp);
            break;
        }

        if (nextGt == valGt)
            // do not go byond the segment, just follow its data
            continue;

        if (seg != valGt)
            // OK_MAY_EXIST is applicable only on the first object
            off = 0;

        const EValueTarget code = shGt.valTarget(valGt);
        if (isPossibleToDeref(code)) {
            if (segmentCloneCore(ctx, shGt, valGt, valMapGt, action, off))
                continue;
        }
        else {
            // clone unknown value
            const EValueOrigin vo = shGt.valOrigin(valGt);
            const TValId vDst = ctx.dst.valCreate(code, vo);
            if (handleUnknownValues(ctx, vp.first, vp.second, vDst))
                continue;
        }

        // clone failed
        *pResult = false;
        return true;
    }

    // schedule the next object in the row (TODO: check if really necessary)
    const TValId valNext1 = (isGt1) ? nextGt : nextLt;
    const TValId valNext2 = (isGt2) ? nextGt : nextLt;
    considerValSchedule(ctx, valNext1, valNext2, VAL_INVALID, VAL_INVALID);
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

    const EObjKind kind1 = ctx.sh1.valTargetKind(v1);
    const EObjKind kind2 = ctx.sh2.valTargetKind(v2);
    if (kind1 == kind2)
        // kind of abstract object matches in both cases
        return;

    if (OK_MAY_EXIST == kind1)
        *isAbs1 = false;

    if (OK_MAY_EXIST == kind2)
        *isAbs2 = false;
}

bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const EValueTarget      code1,
        const EValueTarget      code2)
{
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);

    bool isAbs1 = (VT_ABSTRACT == code1);
    bool isAbs2 = (VT_ABSTRACT == code2);
    resolveMayExist(ctx, &isAbs1, &isAbs2, v1, v2);

    const EJoinStatus subStatus = (isAbs1)
        ? JS_USE_SH1
        : JS_USE_SH2;

    const bool isValid1 = isPossibleToDeref(code1);
    const bool isValid2 = isPossibleToDeref(code2);
    if (isValid1 && isValid2) {
        if (isAbs1 && isAbs2) {
            if (!joinSegmentWithAny(pResult, ctx, root1, root2, JS_USE_ANY))
                *pResult = false;

            return true;
        }

        else if (joinSegmentWithAny(pResult, ctx, root1, root2, subStatus))
            return true;
    }

    if (!insertSegmentClone(pResult, ctx, v1, v2, subStatus)) {
        if (ctx.joiningData()) {
            return joinSegmentWithAny(pResult, ctx, root1, root2, subStatus,
                        /* firstTryReadOnly */ false);
        }

        *pResult = false;
    }

    return true;
}

class MayExistVisitor {
    private:
        SymJoinCtx              ctx_;
        const EJoinStatus       action_;
        const TValId            valRef_;
        const TValId            root_;
        bool                    lookThrough_;
        TOffset                 offNext_;

    public:
        MayExistVisitor(
                SymJoinCtx          &ctx,
                const EJoinStatus   action,
                const TValId        valRef,
                const TValId        root):
            ctx_(ctx),
            action_(action),
            valRef_(valRef),
            root_(root),
            lookThrough_(false),
            offNext_(0)
        {
            CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);
        }

        TOffset offNext() const {
            return offNext_;
        }

        void enableLookThroughMode(bool enable = true) {
            lookThrough_ = enable;
        }

        bool operator()(SymHeap &sh, const TObjId sub) {
            TValId val = sh.valueOf(sub);

            for (;;) {
                const TValId v1 = (JS_USE_SH1 == action_) ? val : valRef_;
                const TValId v2 = (JS_USE_SH2 == action_) ? val : valRef_;
                if (followValuePair(ctx_, v1, v2, /* read-only */ true))
                    // looks like we have a candidate
                    break;

                if (!lookThrough_ || !isAbstract(sh.valTarget(val)))
                    return /* continue */ true;

                TValId seg = sh.valRoot(val);
                if (sh.segMinLength(seg) || segHeadAt(sh, seg) != val)
                    return /* continue */ true;

                if (OK_DLS == sh.valTargetKind(seg))
                    seg = dlSegPeer(sh, seg);

                val = nextValFromSeg(sh, seg);
            }

            offNext_ = sh.valOffset(sh.placedAt(sub));
            return /* continue */ false;
        }
};

bool mayExistFallback(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const EJoinStatus       action)
{
    const bool use1 = (JS_USE_SH1 == action);
    const bool use2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(use1 == use2);

    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);

    const bool hasMapping1 = hasKey(ctx.valMap1[0], root1);
    const bool hasMapping2 = hasKey(ctx.valMap2[0], root2);
    if ((hasMapping1 != hasMapping2) && (hasMapping1 == use1))
        // try it the other way around
        return false;

    SymHeap &sh = (use1) ? ctx.sh1 : ctx.sh2;
    const TValId val = (use1) ? v1 : v2;
    if (!isPossibleToDeref(sh.valTarget(val)))
        // no valid target
        return false;

    const TValId valRoot = (use1) ? root1 : root2;
    if (OK_CONCRETE != sh.valTargetKind(valRoot))
        // only concrete objects/prototypes are candidates for OK_MAY_EXIST
        return false;

    const TValId ref = (use2) ? v1 : v2;
    MayExistVisitor visitor(ctx, action, ref, /* root */ valRoot);
    if (traverseLivePtrs(sh, valRoot, visitor)) {
        // reference value not matched directly, try to look through
        visitor.enableLookThroughMode();
        if (traverseLivePtrs(sh, valRoot, visitor))
            // no match
            return false;
    }

    // mayExistFallback() always implies JS_THREE_WAY
    if (!updateJoinStatus(ctx, JS_THREE_WAY))
        return false;

    BindingOff off;
    off.head = sh.valOffset(val);
    off.next = visitor.offNext();

    bool result;
    if (!insertSegmentClone(&result, ctx, v1, v2, action, &off))
        result = false;

    return result;
}

EValueOrigin joinOrigin(const EValueOrigin vo1, const EValueOrigin vo2) {
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
    const TOffset off1 = ctx.sh1.valOffset(v1);
    const TOffset off2 = ctx.sh2.valOffset(v2);

    // classify the targets
    const EValueTarget code1 = ctx.sh1.valTarget(v1);
    const EValueTarget code2 = ctx.sh2.valTarget(v2);

    // check for VT_DELETED/VT_LOST
    const bool gone1 = isGone(code1)
        || /* FIXME: misleading */ (off1 && VAL_NULL == ctx.sh1.valRoot(v1));
    const bool gone2 = isGone(code2)
        || /* FIXME: misleading */ (off2 && VAL_NULL == ctx.sh2.valRoot(v2));

    if (gone1 || gone2) {
        if (code1 == code2 && off1 == off2) {
            const TValId vDstRoot = ctx.dst.valCreate(code1, VO_ASSIGNED);
            const TValId vDst = ctx.dst.valByOffset(vDstRoot, off1);
            *pResult = handleUnknownValues(ctx, v1, v2, vDst);
        }
        else
            *pResult = false;

        return true;
    }

    // check for VT_UNKNOWN
    const bool isUnknown1 = (VT_UNKNOWN == code1);
    const bool isUnknown2 = (VT_UNKNOWN == code2);
    if (!isUnknown1 && !isUnknown2)
        // nothing to join here
        return false;

    // join the origin
    const EValueOrigin vo1 = ctx.sh1.valOrigin(v1);
    const EValueOrigin vo2 = ctx.sh2.valOrigin(v2);
    const EValueOrigin origin = joinOrigin(vo1, vo2);

    // do not join VT_UNKNOWN with a valid pointer
    if (VO_DEREF_FAILED != origin) {
        const bool haveTarget1 = isPossibleToDeref(code1);
        const bool haveTarget2 = isPossibleToDeref(code2);
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

bool joinValuePair(SymJoinCtx &ctx, const TValId v1, const TValId v2) {
    if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
        // already joined
        return true;

    bool result;
    if (joinValuesByCode(&result, ctx, v1, v2))
        return result;

    EValueTarget vt1 = ctx.sh1.valTarget(v1);
    if ((VT_ABSTRACT == vt1) && hasKey(ctx.sset1, ctx.sh1.valRoot(v1)))
        // do not treat the starting point as encountered segment
        vt1 = VT_ON_HEAP;

    EValueTarget vt2 = ctx.sh2.valTarget(v2);
    if ((VT_ABSTRACT == vt2) && hasKey(ctx.sset2, ctx.sh2.valRoot(v2)))
        // do not treat the starting point as encountered segment
        vt2 = VT_ON_HEAP;

    if ((VT_ABSTRACT == vt1 || VT_ABSTRACT == vt2)
            && joinAbstractValues(&result, ctx, v1, v2, vt1, vt2))
        return result;

    if (VAL_NULL != v1 && VAL_NULL != v2) {
        const bool haveTarget1 = isPossibleToDeref(vt1);
        const bool haveTarget2 = isPossibleToDeref(vt2);
        if (haveTarget1 != haveTarget2) {
            SJ_DEBUG("<-- target validity mismatch " << SJ_VALP(v1, v2));
            return false;
        }
    }

    if (followValuePair(ctx, v1, v2, /* read-only */ true))
        return followValuePair(ctx, v1, v2, /* read-only */ false);

    return mayExistFallback(ctx, v1, v2, JS_USE_SH1)
        || mayExistFallback(ctx, v1, v2, JS_USE_SH2);
}

bool joinPendingValues(SymJoinCtx &ctx) {
    TValPair vp;
    while (ctx.wl.next(vp)) {
        const TValId v1 = vp.first;
        const TValId v2 = vp.second;

        SJ_DEBUG("--- " << SJ_VALP(v1, v2));
        if (!joinValuePair(ctx, v1, v2))
            return false;

        ctx.alreadyJoined.insert(vp);
    }

    return true;
}

class JoinVarVisitor {
    private:
        SymJoinCtx &ctx_;

    public:
        JoinVarVisitor(SymJoinCtx &ctx):
            ctx_(ctx)
        {
        }

        bool operator()(const TValId roots[3]) {
            const TValId rootDst   = roots[/* dst */ 0];
            const TValId root1     = roots[/* sh1 */ 1];
            const TValId root2     = roots[/* sh2 */ 2];

            return joinUniBlocks(ctx_, rootDst, root1, root2)
                && traverseRoots(ctx_, rootDst, root1, root2);
        }
};

bool joinCVars(SymJoinCtx &ctx) {
    SymHeap *const heaps[] = {
        &ctx.dst,
        &ctx.sh1,
        &ctx.sh2
    };

    // go through all program variables
    JoinVarVisitor visitor(ctx);
    return traverseProgramVarsGeneric<
        /* N_DST */ 1,
        /* N_SRC */ 2>
            (heaps,visitor);
}

TValId joinDstValue(
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              validObj1,
        const bool              validObj2)
{
    // translate the roots into 'dst'
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);
    const TValId valRootDstBy1 = roMapLookup(ctx.valMap1[/* ltr */ 0], root1);
    const TValId valRootDstBy2 = roMapLookup(ctx.valMap2[/* ltr */ 0], root2);

    // translate the offsets into 'dst'
    const TOffset off1 = ctx.sh1.valOffset(v1);
    const TOffset off2 = ctx.sh2.valOffset(v2);
    const TValId vDstBy1 = ctx.dst.valByOffset(valRootDstBy1, off1);
    const TValId vDstBy2 = ctx.dst.valByOffset(valRootDstBy2, off2);
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

template <class TItem, class TBlackList>
bool setDstValuesCore(
        SymJoinCtx              &ctx,
        const TItem             &rItem,
        const TBlackList        &blackList)
{
    const TObjId objDst = rItem.first;
    CL_BREAK_IF(objDst < 0);
    if (hasKey(blackList, objDst))
        return true;

    const TObjPair &orig = rItem.second;
    const TObjId obj1 = orig.first;
    const TObjId obj2 = orig.second;
    CL_BREAK_IF(OBJ_INVALID == obj1 && OBJ_INVALID == obj2);

    const TValId v1 = ctx.sh1.valueOf(obj1);
    const TValId v2 = ctx.sh2.valueOf(obj2);

    const bool isComp1 = (isComposite(ctx.sh1.objType(obj1)));
    const bool isComp2 = (isComposite(ctx.sh2.objType(obj2)));
    if (isComp1 || isComp2) {
        // do not bother by composite values
        CL_BREAK_IF(OBJ_INVALID != obj1 && !isComp1);
        CL_BREAK_IF(OBJ_INVALID != obj2 && !isComp2);
        return true;
    }

    if (ctx.joiningData() && obj1 == obj2) {
        // shared data
        CL_BREAK_IF(v1 != v2);
        if (ctx.joiningDataReadWrite())
            // read-write mode
            ctx.dst.objSetValue(objDst, v1);

        return true;
    }

    // compute the resulting value
    const bool validObj1 = (OBJ_INVALID != obj1);
    const bool validObj2 = (OBJ_INVALID != obj2);
    const TValId vDst = joinDstValue(ctx, v1, v2, validObj1, validObj2);
    if (VAL_INVALID == vDst)
        return false;

    // set the value
    ctx.dst.objSetValue(objDst, vDst);
    return true;
}

bool setDstValues(SymJoinCtx &ctx, const std::set<TObjId> *blackList = 0) {
    SymHeap &dst = ctx.dst;
    SymHeap &sh1 = ctx.sh1;
    SymHeap &sh2 = ctx.sh2;

    typedef std::map<TObjId /* objDst */, TObjPair> TMap;
    TMap rMap;

    // reverse mapping for ctx.liveList1
    const TValMap &vMap1 = ctx.valMap1[0];
    BOOST_FOREACH(const TObjId objSrc, ctx.liveList1) {
        const TValId rootSrcAt = sh1.valRoot(sh1.placedAt(objSrc));
        const TValId rootDstAt = roMapLookup(vMap1, rootSrcAt);
        const TObjId objDst = translateObjId(dst, sh1, rootDstAt, objSrc);
        if (!hasKey(rMap, objDst))
            rMap[objDst].second = OBJ_INVALID;

        // objDst -> obj1
        rMap[objDst].first = objSrc;
    }

    // reverse mapping for ctx.liveList2
    const TValMap &vMap2 = ctx.valMap2[0];
    BOOST_FOREACH(const TObjId objSrc, ctx.liveList2) {
        const TValId rootSrcAt = sh2.valRoot(sh2.placedAt(objSrc));
        const TValId rootDstAt = roMapLookup(vMap2, rootSrcAt);
        const TObjId objDst = translateObjId(dst, sh2, rootDstAt, objSrc);
        if (!hasKey(rMap, objDst))
            rMap[objDst].first = OBJ_INVALID;

        // objDst -> obj2
        rMap[objDst].second = objSrc;
    }

    std::set<TObjId> emptyBlackList;
    if (!blackList)
        blackList = &emptyBlackList;

    BOOST_FOREACH(TMap::const_reference rItem, rMap) {
        if (!ctx.dst.objType(rItem.first))
            // do not set value of anonymous objects
            continue;

        if (!setDstValuesCore(ctx, rItem, blackList))
            return false;
    }

    return true;
}

bool matchPreds(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValMapBidir      &vMap)
{
    return sh1.matchPreds(sh2, vMap[/* ltr */ 0])
        && sh2.matchPreds(sh1, vMap[/* rtl */ 1]);
}

bool handleDstPreds(SymJoinCtx &ctx) {
    // go through all segments and initialize minLength
    BOOST_FOREACH(SymJoinCtx::TSegLengths::const_reference ref, ctx.segLengths)
    {
        const TValId    seg = ref.first;
        const unsigned  len = ref.second;
        ctx.dst.segSetMinLength(seg, len);
    }

    // go through shared Neq predicates
    BOOST_FOREACH(const TValPair neq, ctx.sharedNeqs) {
        TValId valLt, valGt;
        boost::tie(valLt, valGt) = neq;

        const TValId targetLt = ctx.dst.valRoot(valLt);
        const TValId targetGt = ctx.dst.valRoot(valGt);
        if (hasKey(ctx.segLengths, targetLt)
                || hasKey(ctx.segLengths, targetGt))
            // preserve segment length
            continue;

        // handle generic Neq predicate
        ctx.dst.neqOp(SymHeap::NEQ_ADD, valLt, valGt);
    }

    if (!ctx.joiningData()) {
        // cross-over check of Neq predicates

        if (!matchPreds(ctx.sh1, ctx.dst, ctx.valMap1)
                && !updateJoinStatus(ctx, JS_USE_SH2))
            return false;

        if (!matchPreds(ctx.sh2, ctx.dst, ctx.valMap2)
                && !updateJoinStatus(ctx, JS_USE_SH1))
            return false;
    }

    // TODO: match gneric Neq predicates also in prototypes;  for now we
    // consider only minimal segment lengths
    BOOST_FOREACH(const TValId protoDst, ctx.protoRoots) {
        const TValId proto1 = roMapLookup(ctx.valMap1[/* rtl */ 1], protoDst);
        const TValId proto2 = roMapLookup(ctx.valMap2[/* rtl */ 1], protoDst);

        const unsigned len1 = objMinLength(ctx.sh1, proto1);
        const unsigned len2 = objMinLength(ctx.sh2, proto2);
        const unsigned lenDst = objMinLength(ctx.dst, protoDst);

        if ((lenDst < len1) && !updateJoinStatus(ctx, JS_USE_SH2))
            return false;

        if ((lenDst < len2) && !updateJoinStatus(ctx, JS_USE_SH1))
            return false;
    }

    // all OK
    return true;
}

bool segDetectSelfLoopHelper(
        SymHeap                 &sh,
        std::set<TValId>        &haveSeen,
        TValId                  seg)
{
    // remember original kind of object
    const EObjKind kind = sh.valTargetKind(seg);

    // find a loop-less path
    std::set<TValId> path;
    while (insertOnce(path, seg)) {
        TValId peer = seg;
        if (OK_DLS == kind) {
            // jump to peer in case of DLS
            peer = dlSegPeer(sh, seg);
            if (!insertOnce(path, peer))
                break;
        }

        const TValId valNext = nextValFromSeg(sh, peer);
        if (!isAbstract(sh.valTarget(valNext)))
            // no next segment --> no loop
            return false;

        seg = sh.valRoot(valNext);
        if (kind != sh.valTargetKind(seg))
            // no compatible next segment --> no loop
            return false;

        // optimization
        haveSeen.insert(seg);
    }

    // loop detected!
    return true;
}

bool segDetectSelfLoop(SymHeap &sh) {
    // gather all abstract objects
    TValList segRoots;
    sh.gatherRootObjects(segRoots, isAbstract);

    // go through all entries
    std::set<TValId> haveSeen;
    BOOST_FOREACH(const TValId seg, segRoots) {
        if (!insertOnce(haveSeen, seg))
            continue;

        if (segDetectSelfLoopHelper(sh, haveSeen, seg))
            // cycle detected!
            return true;
    }

    // found nothing harmful
    return false;
}

bool validateStatus(const SymJoinCtx &ctx) {
    if (segDetectSelfLoop(ctx.dst)) {
        // purely segmental loops cause us problems
        CL_DEBUG(">J< segment cycle detected, cancelling join");
        return false;
    }

    if (ctx.allowThreeWay || (JS_THREE_WAY != ctx.status))
        return true;

    CL_DEBUG(">J< cancelling three-way join");
    return false;
}

bool joinSymHeaps(
        EJoinStatus             *pStatus,
        SymHeap                 *pDst,
        const SymHeap           &sh1,
        const SymHeap           &sh2)
{
    SJ_DEBUG("--> joinSymHeaps()");
    TStorRef stor = sh1.stor();
    CL_BREAK_IF(&stor != &sh2.stor());
    *pDst = SymHeap(stor);

    // initialize symbolic join ctx
    SymJoinCtx ctx(*pDst, sh1, sh2);

    // first try to join return addresses (if in use)
    if (!joinReturnAddrs(ctx))
        goto fail;

    // start with program variables
    if (!joinCVars(ctx))
        goto fail;

    // go through all values in them
    if (!joinPendingValues(ctx))
        goto fail;

    // time to preserve all 'hasValue' edges
    if (!setDstValues(ctx))
        goto fail;

    // go through shared Neq predicates and set minimal segment lengths
    if (!handleDstPreds(ctx))
        goto fail;

    // if the result is three-way join, check if it is a good idea
    if (!validateStatus(ctx))
        goto fail;

    if (debuggingSymJoin) {
        // catch possible regression at this point
        CL_BREAK_IF((JS_USE_ANY == ctx.status) != areEqual(sh1, sh2));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh1, ctx.dst));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh2, ctx.dst));
    }

    // all OK
    *pStatus = ctx.status;
    SJ_DEBUG("<-- joinSymHeaps() says " << ctx.status);
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
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
bool dlSegCheckProtoConsistency(const SymJoinCtx &ctx) {
    BOOST_FOREACH(const TValId proto, ctx.protoRoots) {
        if (OK_DLS != ctx.dst.valTargetKind(proto))
            // we are interested only DLSs here
            continue;

        const TValId peer = dlSegPeer(ctx.dst, proto);
        if (!hasKey(ctx.protoRoots, peer)) {
            CL_ERROR("DLS prototype peer not a prototype");
            return false;
        }
    }

    return true;
}

bool joinDataCore(
        SymJoinCtx              &ctx,
        const BindingOff        &off,
        const TValId            addr1,
        const TValId            addr2)
{
    CL_BREAK_IF(!ctx.joiningData());
    SymHeap &sh = ctx.sh1;

    const TObjType clt1 = ctx.sh1.valLastKnownTypeOfTarget(addr1);
    const TObjType clt2 = ctx.sh2.valLastKnownTypeOfTarget(addr2);
    const TObjType clt = joinClt(ctx, clt1, clt2);

    int size;
    if (!joinObjSize(&size, ctx, addr1, addr2))
        return false;

    // start with the given pair of objects and create a ghost object for them
    // create an image in ctx.dst
    const TValId rootDstAt = ctx.dst.heapAlloc(size);

    if (!joinUniBlocks(ctx, rootDstAt, addr1, addr2))
        // failed to complement uniform blocks
        return false;

    if (clt)
        // preserve estimated type-info of the root
        ctx.dst.valSetLastKnownTypeOfTarget(rootDstAt, clt);

    if (!traverseRoots(ctx, rootDstAt, addr1, addr2, &off))
        return false;

    ctx.sset1.insert(addr1);
    ctx.sset2.insert(addr2);

    // never step over DLS peer
    if (OK_DLS == sh.valTargetKind(addr1)) {
        const TValId peer = dlSegPeer(sh, addr1);
        ctx.sset1.insert(peer);
        if (peer != addr2)
            mapGhostAddressSpace(ctx, addr1, peer, JS_USE_SH1);
    }
    if (OK_DLS == sh.valTargetKind(addr2)) {
        const TValId peer = dlSegPeer(sh, addr2);
        ctx.sset2.insert(peer);
        if (peer != addr1)
            mapGhostAddressSpace(ctx, addr2, peer, JS_USE_SH2);
    }

    // perform main loop
    if (!joinPendingValues(ctx))
        return false;

    // batch assignment of all values in ctx.dst
    std::set<TObjId> blackList;
    buildIgnoreList(blackList, ctx.dst, rootDstAt, off);
    if (!setDstValues(ctx, &blackList))
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
        const SymHeap           &sh,
        const BindingOff        &off,
        const TValId            addr1,
        const TValId            addr2,
        TValList                protoRoots[1][2])
{
    SJ_DEBUG("--> joinDataReadOnly" << SJ_VALP(addr1, addr2));

    // go through the commont part of joinData()/joinDataReadOnly()
    SymHeap tmp(sh.stor());
    SymJoinCtx ctx(tmp, sh);
    
    if (!joinDataCore(ctx, off, addr1, addr2))
        return false;

    unsigned cntProto1 = 0;
    unsigned cntProto2 = 0;

    // go through prototypes
    BOOST_FOREACH(const TValId protoDst, ctx.protoRoots) {
        const TValId proto1 = roMapLookup(ctx.valMap1[/* rtl */ 1], protoDst);
        const TValId proto2 = roMapLookup(ctx.valMap2[/* rtl */ 1], protoDst);

        if (VAL_INVALID != proto1) {
            ++cntProto1;
            if (protoRoots)
                (*protoRoots)[0].push_back(proto1);
        }

        if (VAL_INVALID != proto2) {
            ++cntProto2;
            if (protoRoots)
                (*protoRoots)[1].push_back(proto2);
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
        const TValId            dst,
        const TValId            src,
        const TValId            ghost,
        const bool              bidir)
{
    redirectRefs(sh,
            /* pointingFrom */  dst,
            /* pointingTo   */  ghost,
            /* redirectTo   */  dst);

    if (!bidir)
        return;

    redirectRefs(sh,
            /* pointingFrom */  src,
            /* pointingTo   */  ghost,
            /* redirectTo   */  src);
}

void recoverPrototypes(
        SymJoinCtx              &ctx,
        const TValId            dst,
        const TValId            ghost,
        const bool              bidir)
{
    SymHeap &sh = ctx.dst;

    const unsigned cntProto = ctx.protoRoots.size();
    if (cntProto)
        CL_DEBUG("    joinData() joins " << cntProto << " prototype objects");

    // go through prototypes
    BOOST_FOREACH(const TValId protoGhost, ctx.protoRoots) {
        const TValId proto1 = roMapLookup(ctx.valMap1[/* rtl */ 1], protoGhost);
        const TValId proto2 = roMapLookup(ctx.valMap2[/* rtl */ 1], protoGhost);

        if (isAbstract(sh.valTarget(proto1)))
            // remove Neq predicates, their targets are going to vanish soon
            sh.segSetMinLength(proto1, 0);

        if (bidir && isAbstract(sh.valTarget(proto2)))
            // remove Neq predicates, their targets are going to vanish soon
            sh.segSetMinLength(proto2, 0);

        if (isAbstract(sh.valTarget(protoGhost)))
            // temporarily remove Neq predicates
            sh.segSetMinLength(protoGhost, 0);

        redirectRefs(sh,
                /* pointingFrom */  protoGhost,
                /* pointingTo   */  ghost,
                /* redirectTo   */  dst);

        sh.valTargetSetProto(protoGhost, true);
    }
}

void restorePrototypeLengths(SymJoinCtx &ctx) {
    CL_BREAK_IF(!ctx.joiningDataReadWrite());
    SymHeap &sh = ctx.dst;

    // restore minimal length of segment prototypes
    BOOST_FOREACH(const TValId protoDst, ctx.protoRoots) {
        typedef SymJoinCtx::TSegLengths TLens;

        const TLens &lens = ctx.segLengths;
        TLens::const_iterator it = lens.find(protoDst);
        if (lens.end() == it)
            continue;

        const unsigned len = it->second;
        if (len)
            sh.segSetMinLength(protoDst, len);
    }
}

void transferContentsOfGhost(
        SymHeap                 &sh,
        const TValId            dst,
        const TValId            ghost)
{
    std::set<TObjId> ignoreList;
    buildIgnoreList(ignoreList, sh, dst);

    TObjList live;
    sh.gatherLiveObjects(live, ghost);
    BOOST_FOREACH(const TObjId objGhost, live) {
        const TObjId objDst = translateObjId(sh, sh, dst, objGhost);
        if (hasKey(ignoreList, objDst))
            // preserve binding pointers
            continue;

        const TValId valOld = sh.valueOf(objDst);
        const TValId valNew = sh.valueOf(objGhost);
        sh.objSetValue(objDst, valNew);

        if (collectJunk(sh, valOld))
            CL_DEBUG("    transferContentsOfGhost() drops a sub-heap (valOld)");
    }
}

/// replacement of matchData() from symdiscover
bool joinData(
        SymHeap                 &sh,
        const TValId            dst,
        const TValId            src,
        const bool              bidir)
{
    SJ_DEBUG("--> joinData" << SJ_VALP(dst, src));
    ++cntJoinOps;

    // dst is expected to be a segment
    CL_BREAK_IF(!isAbstract(sh.valTarget(dst)));
    const BindingOff off(sh.segBinding(dst));
    if (debuggingSymJoin) {
        EJoinStatus status = JS_USE_ANY;
        joinDataReadOnly(&status, sh, off, dst, src, 0);
        if (JS_USE_ANY != status)
            debugPlot(sh, "joinData", dst, src, "00");
    }

    // go through the commont part of joinData()/joinDataReadOnly()
    SymJoinCtx ctx(sh);
    if (!joinDataCore(ctx, off, dst, src)) {
        CL_BREAK_IF("joinData() has failed, did joinDataReadOnly() succeed?");
        return false;
    }

    // ghost is a transiently existing object representing the join of dst/src
    const TValId ghost = roMapLookup(ctx.valMap1[0], dst);
    CL_BREAK_IF(ghost != roMapLookup(ctx.valMap2[0], src));

    // assign values within dst (and also in src if bidir == true)
    transferContentsOfGhost(ctx.dst, dst, ghost);
    if (bidir)
        transferContentsOfGhost(ctx.dst, src, ghost);

    // redirect some edges if necessary
    recoverPrototypes(ctx, dst, ghost, bidir);
    recoverPointersToSelf(sh, dst, src, ghost, bidir);
    restorePrototypeLengths(ctx);

    if (collectJunk(sh, ghost))
        CL_DEBUG("    joinData() drops a sub-heap (ghost)");

    SJ_DEBUG("<-- joinData() has finished " << ctx.status);
    if (JS_USE_ANY != ctx.status)
        debugPlot(sh, "joinData", dst, src, "01");

    return true;
}
