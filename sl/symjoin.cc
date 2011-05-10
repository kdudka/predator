/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#include "symclone.hh"
#include "symcmp.hh"
#include "symdump.hh"
#include "symgc.hh"
#include "symseg.hh"
#include "symstate.hh"
#include "symutil.hh"
#include "worklist.hh"
#include "util.hh"

#include <algorithm>            // for std::copy_if
#include <functional>           // for std::bind
#include <iomanip>
#include <map>
#include <set>

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

static bool debugSymJoin = static_cast<bool>(DEBUG_SYMJOIN);

#define SJ_DEBUG(...) do {                                                  \
    if (::debugSymJoin)                                                     \
        CL_DEBUG("SymJoin: " << __VA_ARGS__);                               \
} while (0)

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
        if (!::debugSymJoin)
            return;

        std::ostringstream str;
        str << "symjoin-" << FIXW(6) << ::cntJoinOps
            << "-" << name << "-"
            << FIXW(3) << dst << "-"
            << FIXW(3) << src << "-"
            << suffix;

        dump_plot(sh, str.str().c_str());
    }
}

typedef boost::array<TObjId, 3>                                 TObjTriple;
typedef boost::array<TValId, 3>                                 TValTriple;

template <class T>
class WorkListWithUndo: public WorkList<T> {
    private:
        typedef WorkList<T> TBase;

    public:
        /// push an @b already @b processed item back to WorkList
        void undo(const T &item) {
            CL_BREAK_IF(!hasKey(TBase::done_, item));
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

    // they need to be black-listed in joinAbstractValues()
    std::set<TValId>            sset1;
    std::set<TValId>            sset2;

    typedef std::map<TObjId /* src */, TObjId /* dst */>        TObjMap;
    TObjMap                     objMap1;
    TObjMap                     objMap2;

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

    std::set<TValTriple>        protoRoots;

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
        allowThreeWay(true)
    {
        initValMaps();
    }

    /// constructor used by joinDataReadOnly()
    SymJoinCtx(SymHeap &tmp_, const SymHeap &sh_):
        dst(tmp_),
        sh1(/* XXX */ const_cast<SymHeap &>(sh_)),
        sh2(/* XXX */ const_cast<SymHeap &>(sh_)),
        status(JS_USE_ANY),
        allowThreeWay(true)
    {
        initValMaps();
    }

    /// constructor used by joinData()
    SymJoinCtx(SymHeap &sh_):
        dst(sh_),
        sh1(sh_),
        sh2(sh_),
        status(JS_USE_ANY),
        allowThreeWay(true)
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
        dump_plot(ctx.sh1);
    }
    if (!ctx.joiningData()) {
        cout << "    plotting ctx.sh2...\n";
        dump_plot(ctx.sh2);
    }
    cout << "    plotting ctx.dst...\n";
    dump_plot(ctx.dst);

    // print entry-point
    cout << "\ndump_ctx: ";
    if (ctx.joiningDataReadWrite())
        cout << "joinData()\n";
    else if (ctx.joiningData())
        cout << "joinDataReadOnly()\n";
    else
        cout << "joinSymHeaps()\n";

    // summarize mapping
    cout << "    ctx.objMap1        .size() = " << ctx.objMap1.size() << "\n";
    cout << "    ctx.objMap2        .size() = " << ctx.objMap2.size() << "\n\n";
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

/// define address mapping for the given object triple (obj1, obj2, objDst)
bool defineAddressMapping(
        SymJoinCtx              &ctx,
        const TObjId            obj1,
        const TObjId            obj2,
        const TObjId            objDst)
{
    const TValId addr1 = ctx.sh1.placedAt(obj1);
    const TValId addr2 = ctx.sh2.placedAt(obj2);
    const TValId dstAt = ctx.dst.placedAt(objDst);
    return defineValueMapping(ctx, addr1, addr2, dstAt);
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

    // read-only value lookup
    const TValMap &vMap1 = ctx.valMap1[/* ltr */ 0];
    const TValMap &vMap2 = ctx.valMap2[/* ltr */ 0];
    TValMap::const_iterator i1 = vMap1.find(v1);
    TValMap::const_iterator i2 = vMap2.find(v2);

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

    if (segClone && (VAL_NULL == v1 || VAL_NULL == v2))
        // same as above, but now only one value of v1 and v2 is valid
        return true;

    if (hasKey(ctx.alreadyJoined, TValPair(v1, v2)))
        // the join has been already successful
        return true;

    if (VAL_NULL == v1 && (v2 < 0 || hasKey(ctx.valMap2[/* lrt */ 0], v2)
            || (!ctx.joiningData() && UV_KNOWN == ctx.sh2.valGetUnknown(v2))))
        // mapping already inconsistent
        return false;

    if (VAL_NULL == v2 && (v1 < 0 || hasKey(ctx.valMap1[/* lrt */ 0], v1)
            || (!ctx.joiningData() && UV_KNOWN == ctx.sh1.valGetUnknown(v1))))
        // mapping already inconsistent
        return false;

    const TObjId cObj1 = ctx.sh1.valGetCompositeObj(v1);
    const TObjId cObj2 = ctx.sh2.valGetCompositeObj(v2);
    if (!segClone && (OBJ_INVALID == cObj1) != (OBJ_INVALID == cObj2)) {
        SJ_DEBUG("<-- scalar vs. composite value " << SJ_VALP(v1, v2));
        return false;
    }

    if (OBJ_INVALID != cObj1 || OBJ_INVALID != cObj2) {
        // store mapping of composite object's values
        const TValId vDst = ctx.dst.valueOf(objDst);
        return readOnly
            || defineValueMapping(ctx, v1, v2, vDst);
    }

    if (segClone) {
        const bool isGt1 = (OBJ_INVALID == obj2);
        const TValMapBidir &vm = (isGt1) ? ctx.valMap1 : ctx.valMap2;
        const TValId val = (isGt1) ? v1 : v2;
        return (val <= 0 || hasKey(vm[/* lrt */ 0], val));
    }

    // special values have to match (NULL not treated as special here)
    // TODO: should we consider also join of VAL_TRUE/VAL_FALSE?
    if (v1 < 0 || v2 < 0) {
        if (v1 == v2)
            return true;

        SJ_DEBUG("<-- special value mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (readOnly)
        return checkValueMapping(ctx, v1, v2,
                                 /* allowUnknownMapping */ true);

    if (checkValueMapping(ctx, v1, v2, /* allowUnknownMapping */ false))
        return true;

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

    bool operator()(const TObjTriple &item) {
        const TObjId obj1   = item[0];
        const TObjId obj2   = item[1];
        const TObjId objDst = item[2];

        // store object IDs mapping
        if (OBJ_INVALID != obj1)
            ctx.objMap1[obj1] = objDst;

        if (OBJ_INVALID != obj2)
            ctx.objMap2[obj2] = objDst;

        // store object's addresses
        if (!defineAddressMapping(ctx, obj1, obj2, objDst))
            return false;

        // check black-list
        if (noFollow || hasKey(blackList1, obj1) || hasKey(blackList2, obj2))
            return /* continue */ true;

        return /* continue */ joinFreshObjTripple(ctx, obj1, obj2, objDst);
    }
};

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

bool traverseSubObjs(
        SymJoinCtx              &ctx,
        const TValId            addr1,
        const TValId            addr2,
        const TValId            addrDst,
        const BindingOff        *offBlackList = 0)
{
    // TODO: remove this
    const TObjId root1    = ctx.sh1.objAt(addr1);
    const TObjId root2    = ctx.sh2.objAt(addr2);
    const TObjId rootDst  = ctx.dst.objAt(addrDst);

    typedef boost::array<const SymHeap *, 3>        TSymHeapTriple;
#ifndef NDEBUG
    const struct cl_type *clt1   = ctx.sh1.objType(root1);
    const struct cl_type *clt2   = ctx.sh2.objType(root2);
    const struct cl_type *cltDst = ctx.dst.objType(rootDst);
    CL_BREAK_IF(!cltDst || (OBJ_INVALID == root1 && OBJ_INVALID == root2));
    CL_BREAK_IF(OBJ_INVALID != root1 && *clt1 != *cltDst);
    CL_BREAK_IF(OBJ_INVALID != root2 && *clt2 != *cltDst);
#endif
    TObjTriple roots;
    roots[/* sh1 */ 0] = root1;
    roots[/* sh2 */ 1] = root2;
    roots[/* dst */ 2] = rootDst;

    TSymHeapTriple sht;
    sht[0] = &ctx.sh1;
    sht[1] = &ctx.sh2;
    sht[2] = &ctx.dst;

    // initialize visitor
    ObjJoinVisitor objVisitor(ctx);
    if (offBlackList) {
        buildIgnoreList(objVisitor.blackList1, ctx.sh1, root1, *offBlackList);
        buildIgnoreList(objVisitor.blackList2, ctx.sh2, root2, *offBlackList);
    }

    else if (ctx.joiningData()) {
        if (root1 == root2)
            // do not follow shared data
            objVisitor.noFollow = true;
        else {
            TValTriple proto;
            proto[/* sh1 */ 0] = addr1;
            proto[/* sh2 */ 1] = addr2;
            proto[/* dst */ 2] = addrDst;
            ctx.protoRoots.insert(proto);
        }
    }

    // guide the visitors through them
    return objVisitor(roots)
        && traverseSubObjs<3>(sht, roots, objVisitor);
}

bool segMatchLookAhead(
        SymJoinCtx              &ctx,
        const TValId            root1,
        const TValId            root2)
{
    // set up a visitor
    SymHeap *const heaps[] = { &ctx.sh1, &ctx.sh2 };
    TValId roots[] = { root1, root2 };
    SegMatchVisitor visitor(ctx);

    if (OK_DLS == ctx.sh1.valTargetKind(root1)) {
        const TObjId peerPtr1 = prevPtrFromSeg(ctx.sh1, root1);
        visitor.blackList1.insert(peerPtr1);
    }

    if (OK_DLS == ctx.sh2.valTargetKind(root2)) {
        const TObjId peerPtr2 = prevPtrFromSeg(ctx.sh2, root2);
        visitor.blackList2.insert(peerPtr2);
    }

    // FIXME: this will break as soon as we switch to delayed objects creation
    return traverseLiveObjsGeneric<2>(heaps, roots, visitor);
}

bool joinClt(
        const struct cl_type    *clt1,
        const struct cl_type    *clt2,
        const struct cl_type    **pDst)
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

bool joinObjClt(
        const struct cl_type    **pDst,
        const SymJoinCtx        &ctx,
        const TObjId            o1,
        const TObjId            o2)
{
    const struct cl_type *clt1 = ctx.sh1.objType(o1);
    const struct cl_type *clt2 = ctx.sh2.objType(o2);
    if (joinClt(clt1, clt2, pDst))
        return true;

    SJ_DEBUG("<-- object clt mismatch " << SJ_OBJP(o1, o2));
    return false;
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

bool joinSegBinding(
        BindingOff              *pOff,
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2)
{
    const bool isSeg1 = SymHeap::isAbstract(ctx.sh1.valTarget(v1));
    const bool isSeg2 = SymHeap::isAbstract(ctx.sh2.valTarget(v2));
    if (!isSeg1 && !isSeg2)
        // nothing to join here
        return true;

    if (isSeg1 && isSeg2) {
        const BindingOff off = ctx.sh1.segBinding(v1);
        if (off == ctx.sh2.segBinding(v2)) {
            *pOff = off;
            return true;
        }

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

    const SymHeap &sh = (isProto2) ? ctx.sh1 : ctx.sh2;
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
    if (clt)
        ctx.dst.objDefineType(ctx.dst.objAt(rootDst), clt);

    // preserve 'prototype' flag
    ctx.dst.valTargetSetProto(rootDst, isProto);

    if (OK_CONCRETE != kind) {
        // abstract object
        ctx.dst.valTargetSetAbstract(rootDst, kind, off);

        // compute minimal length of the resulting segment
        const unsigned len1 = objMinLength(ctx.sh1, root1);
        const unsigned len2 = objMinLength(ctx.sh2, root2);
        ctx.segLengths[rootDst] = std::min(len1, len2);
    }

    return traverseSubObjs(ctx, root1, root2, rootDst);
}

bool createAnonObject(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2)
{
    int size;
    if (!joinObjSize(&size, ctx, v1, v2))
        return false;

    // create the join object
    const TObjId anon = ctx.dst.pointsTo(ctx.dst.heapAlloc(size));
    ctx.objMap1[/* XXX */ ctx.sh1.objAt(v1)] = anon;
    ctx.objMap2[/* XXX */ ctx.sh1.objAt(v2)] = anon;
    return defineValueMapping(ctx, v1, v2, /* XXX */ ctx.dst.placedAt(anon));
}

bool followObjPairCore(
        SymJoinCtx              &ctx,
        const TValId            addr1,
        const TValId            addr2,
        const EJoinStatus       action,
        const bool              readOnly)
{
    if (hasKey(ctx.valMap1[0], addr1)) {
        const TValId rootDstAt = ctx.valMap1[0][addr1];
        TValMap::const_iterator i2 = ctx.valMap2[0].find(addr2);
        if (ctx.valMap2[0].end() == i2 || i2->second != rootDstAt) {
            SJ_DEBUG("<-- object root mismatch " << SJ_VALP(addr1, addr2));
            return false;
        }

        // join mapping of object's address
        return defineValueMapping(ctx, addr1, addr2, rootDstAt);
    }

    const TObjId o1 = ctx.sh1.objAt(addr1);
    const TObjId o2 = ctx.sh2.objAt(addr2);
    CL_BREAK_IF(o1 <= 0 || o2 <= 0);

    TObjType clt;
    if (!joinObjClt(&clt, ctx, o1, o2))
        return false;

    if (!clt) {
        // anonymous object of known size
        return !readOnly
            && createAnonObject(ctx, addr1, addr2);
    }

    if (readOnly)
        // do not create any object, just check if it was possible
        return segMatchLookAhead(ctx, addr1, addr2);

    if (ctx.joiningDataReadWrite() && addr1 == addr2)
        // we are on the way from joinData() and hit shared data
        return traverseSubObjs(ctx, addr1, addr1, addr1);

    return createObject(ctx, clt, addr1, addr2, action);
}

bool dlSegHandleShared(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const EJoinStatus       action,
        const bool              readOnly)
{
    const bool isDls = (OK_DLS == ctx.sh1.valTargetKind(v1));
    CL_BREAK_IF(isDls != (OK_DLS == ctx.sh2.valTargetKind(v2)));
    if (!isDls)
        // not a DLS
        return true;

    const TValId peer1 = dlSegPeer(ctx.sh1, v1);
    const TValId peer2 = dlSegPeer(ctx.sh2, v2);
    if (!followObjPairCore(ctx, peer1, peer2, action, readOnly))
        return false;

    if (readOnly)
        return true;

    // we might have just joined a DLS pair as shared data, which would lead to
    // unconnected DLS pair in ctx.dst and later cause some problems;  the best
    // thing to do at this point, is to recover the binding of DLS in ctx.dst
    const TValId seg = roMapLookup(ctx.valMap1[0], v1);
    CL_BREAK_IF(VAL_INVALID == seg || seg != roMapLookup(ctx.valMap2[0], v2));

    const TValId peer = roMapLookup(ctx.valMap1[0], peer2);
    CL_BREAK_IF(VAL_INVALID == peer || peer != roMapLookup(ctx.valMap2[0], peer2));

    SymHeap &sh = ctx.dst;
    sh.objSetValue(prevPtrFromSeg(sh,  seg), segHeadAt(sh, peer));
    sh.objSetValue(prevPtrFromSeg(sh, peer), segHeadAt(sh,  seg));
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    return true;
}

bool followObjPair(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const EJoinStatus       action,
        const bool              readOnly = false)
{
    // TODO: remove this
    const TValId addr1 = ctx.sh1.valRoot(v1);
    const TValId addr2 = ctx.sh2.valRoot(v2);

    if (!followObjPairCore(ctx, addr1, addr2, action, readOnly))
        return false;

    if (!ctx.joiningData())
        // we are on the way from joinSymHeaps()
        return true;

    if (addr1 == addr2)
        // shared data
        return dlSegHandleShared(ctx, addr1, addr2, action, readOnly);

    if (readOnly)
        // postpone it till the read-write attempt
        return true;

    const bool isDls1 = (OK_DLS == ctx.sh1.valTargetKind(addr1));
    const bool isDls2 = (OK_DLS == ctx.sh2.valTargetKind(addr2));
    if (isDls1 == isDls2)
        return true;

    CL_BREAK_IF(isDls1 && JS_USE_SH1 != action);
    CL_BREAK_IF(isDls2 && JS_USE_SH2 != action);

    const TValId peer1 = (isDls1) ? dlSegPeer(ctx.sh1, addr1) : VAL_INVALID;
    const TValId peer2 = (isDls2) ? dlSegPeer(ctx.sh2, addr2) : VAL_INVALID;

    const TValMapBidir &vm = (isDls1) ? ctx.valMap1 : ctx.valMap2;
    if (hasKey(vm[0], (isDls1) ? peer1 : peer2))
        // alredy cloned
        return true;

    // clone peer object
    const TValPair tb(peer1, peer2);
    ctx.tieBreaking.insert(tb);

    const struct cl_type *clt = (isDls1)
        ? ctx.sh1.objType(ctx.sh1.objAt(peer1))
        : ctx.sh2.objType(ctx.sh2.objAt(peer2));
    return createObject(ctx, clt, peer1, peer2, action);
}

bool followValuePair(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              readOnly)
{
    const int cVal1 = ctx.sh1.valGetCustom(v1);
    const int cVal2 = ctx.sh2.valGetCustom(v2);
    if ((-1 == cVal1) != (-1 == cVal2) || (cVal1 != cVal2)) {
        SJ_DEBUG("<-- custom values mismatch " << SJ_VALP(v1, v2));
        return false;
    }

    if (OBJ_INVALID != cVal1) {
        // matching pair of custom values
        if (readOnly) {
            return checkValueMapping(ctx, v1, v2,
                                     /* allowUnknownMapping */ true);
        }

        const TValId vDst = ctx.dst.valCreateCustom(cVal1);
        return defineValueMapping(ctx, v1, v2, vDst);
    }

    if (ctx.sh1.valOffset(v1) != ctx.sh2.valOffset(v2)) {
        SJ_DEBUG("<-- value offset mismatch: " << SJ_VALP(v1, v2));
        return false;
    }

    const TObjId o1 = /* XXX */ ctx.sh1.pointsTo(v1);
    const TObjId o2 = /* XXX */ ctx.sh2.pointsTo(v2);
    if (!checkNonPosValues(o1, o2)) {
        SJ_DEBUG("<-- target validity mismatch: " << SJ_VALP(v1, v2)
                 << " -> " << SJ_OBJP(o1, o2));
        return false;
    }

    if (readOnly) {
        // shallow scan only!
        return checkValueMapping(ctx, v1, v2,
                                 /* allowUnknownMapping */ true);
    }

    if (0 < o1)
        return followObjPair(ctx, v1, v2, JS_USE_ANY);

    // special handling for OBJ_DELETED/OBJ_LOST
    CL_BREAK_IF(o1 != OBJ_DELETED && o1 != OBJ_LOST);
    const TValId vDst = ctx.dst.valCreateDangling(o1);
    return defineValueMapping(ctx, v1, v2, vDst);
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

bool joinSegmentWithAny(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        TValId                  root1,
        TValId                  root2,
        const EJoinStatus       action)
{
    SJ_DEBUG(">>> joinSegmentWithAny" << SJ_OBJP(root1, root2));
    const bool isDls1 = (OK_DLS == ctx.sh1.valTargetKind(root1));
    const bool isDls2 = (OK_DLS == ctx.sh2.valTargetKind(root2));
    if (followObjPair(ctx, root1, root2, action, /* read-only */ true))
        goto read_only_ok;

    if (isDls1) {
        root1 = dlSegPeer(ctx.sh1, root1);
        if (followObjPair(ctx, root1, root2, action, /* read-only */ true))
            goto read_only_ok;
    }

    if (isDls2) {
        root2 = dlSegPeer(ctx.sh2, root2);
        if (followObjPair(ctx, root1, root2, action, /* read-only */ true))
            goto read_only_ok;
    }

    if (isDls1 && isDls2) {
        root1 = dlSegPeer(ctx.sh1, root1);
        if (followObjPair(ctx, root1, root2, action, /* read-only */ true))
            goto read_only_ok;
    }

    SJ_DEBUG("<<< joinSegmentWithAny" << SJ_OBJP(root1, root2));
    return false;

read_only_ok:
    // BindingOff is assumed to be already matching at this point
    BindingOff off = (JS_USE_SH1 == action)
        ? ctx.sh1.segBinding(root1)
        : ctx.sh2.segBinding(root2);

    TValId peer1 = root1;
    if (OK_DLS == ctx.sh1.valTargetKind(root1)) {
        peer1 = dlSegPeer(ctx.sh1, root1);
        off = ctx.sh1.segBinding(peer1);
    }

    TValId peer2 = root2;
    if (OK_DLS == ctx.sh2.valTargetKind(root2)) {
        peer2 = dlSegPeer(ctx.sh2, root2);
        off = ctx.sh2.segBinding(peer2);
    }

    const TValId valNext1 = valOfPtrAt(ctx.sh1, peer1, off.next);
    const TValId valNext2 = valOfPtrAt(ctx.sh2, peer2, off.next);
    if (!checkValueMapping(ctx, valNext1, valNext2,
                           /* allowUnknownMapping */ true))
    {
        SJ_DEBUG("<<< joinSegmentWithAny" << SJ_OBJP(root1, root2));
        return false;
    }

    // go ahead, try it read-write!
    *pResult = followObjPair(ctx, root1, root2, action);
    if (!*pResult)
        return true;

    considerValSchedule(ctx, valNext1, valNext2, peer1, peer2);
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
    const TValId addrGt = shGt.valRoot(valGt);
    const TObjId objGt = shGt.objAt(addrGt);
    if (objGt < 0)
        // nothing to clone here
        return false;

    if (hasKey(valMapGt[0], addrGt))
        // mapping already available for objGt
        return true;

    const struct cl_type *clt = shGt.objType(objGt);
    if (!clt)
        // TODO: clone anonymous prototypes?
        return true;

    SJ_DEBUG("+i+ insertSegmentClone: cloning object #" << objGt <<
             ", clt = " << *clt <<
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
        const SymHeap           &shGt,
        const TObjId            seg,
        const TObjId            peer,
        const EJoinStatus       action)
{
    CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);

    const TValPair vpSeg(
            (JS_USE_SH1 == action) ? shGt.placedAt(seg) : VAL_INVALID,
            (JS_USE_SH2 == action) ? shGt.placedAt(seg) : VAL_INVALID);
    wl.schedule(vpSeg);

    if (seg == peer)
        return;

    const TValPair vpPeer(
            (JS_USE_SH1 == action) ? shGt.placedAt(peer) : VAL_INVALID,
            (JS_USE_SH2 == action) ? shGt.placedAt(peer) : VAL_INVALID);
    wl.schedule(vpPeer);
}

TValId /* old */ vmRemoveMappingOf(TValMapBidir &vm, const TValId val) {
    if (val <= 0)
        // do not remove special values
        return val;

    TValMap &ltr = vm[/* ltr */ 0];
    TValMap &rtl = vm[/* rtl */ 1];

    TValMap::iterator it = ltr.find(val);
    CL_BREAK_IF(ltr.end() == it);

    const TValId old = it->second;
    CL_BREAK_IF(!hasKey(rtl, old));

    ltr.erase(it);
    rtl.erase(old);
    return old;
}

bool disjoinUnknownValues(
        SymJoinCtx              &ctx,
        const TValId            val,
        const TValId            tpl,
        const EJoinStatus       action)
{
    if (val <= 0)
        return updateJoinStatus(ctx, action);

    const bool isGt2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(!isGt2 && (JS_USE_SH1 != action));

    // forget all up to now value mapping of 'val'
    TValMapBidir &vm = (isGt2) ? ctx.valMap1 : ctx.valMap2;
    const TValId old = vmRemoveMappingOf(vm, val);

    // gather all objects that hold 'val' inside
    TObjList refs;
    const SymHeap &sh = (isGt2) ? ctx.sh1 : ctx.sh2;
    sh.usedBy(refs, val);

    // go through all referrers that have their image in ctx.dst
    SymJoinCtx::TObjMap &objMap = (isGt2) ? ctx.objMap1 : ctx.objMap2;
    BOOST_FOREACH(const TObjId objSrc, refs) {
        const TObjId objDst = roMapLookup(objMap, objSrc);
        if (OBJ_INVALID == objDst)
            // no image in ctx.dst yet
            continue;

        const TValId valDst = ctx.dst.valClone(tpl);
        SJ_DEBUG("-u- disjoinUnknownValues() rewrites mapping" <<
                 ", old = " << old <<
                 ", new = " << valDst <<
                 ", action = " << action);

        ctx.dst.objSetValue(objDst, valDst);
    }

    // FIXME: this may be inaccurate in some cases
    return updateJoinStatus(ctx, action);
}

bool unknownValueFallBack(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    const bool hasMapping1 = hasKey(ctx.valMap1[/* ltr */ 0], v1);
    const bool hasMapping2 = hasKey(ctx.valMap2[/* ltr */ 0], v2);
    CL_BREAK_IF(!hasMapping1 && !hasMapping2);

    if (hasMapping1) {
        if (!disjoinUnknownValues(ctx, v1, vDst, JS_USE_SH2))
            return false;
    }
    else {
        if (!defineValueMapping(ctx, v1, VAL_INVALID, vDst))
            return false;
    }

    if (hasMapping2) {
        if (!disjoinUnknownValues(ctx, v2, vDst, JS_USE_SH1))
            return false;
    }
    else {
        if (!defineValueMapping(ctx, VAL_INVALID, v2, vDst))
            return false;
    }

    return true;
}

bool handleUnknownValues(
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        const TValId            vDst)
{
    if (defineValueMapping(ctx, v1, v2, vDst))
        // no inconsistency here
        return true;

    return unknownValueFallBack(ctx, v1, v2, vDst);
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
    if (!ctx.joiningData())
        // on the way from joinSymHeaps(), insertSegmentClone() based three way
        // joins are destructive
        ctx.allowThreeWay = false;

    const bool isGt1 = (JS_USE_SH1 == action);
    const bool isGt2 = (JS_USE_SH2 == action);
    CL_BREAK_IF(isGt1 == isGt2);

    // resolve the existing segment in shGt
    SymHeap &shGt = ((isGt1) ? ctx.sh1 : ctx.sh2);
    const TValId segAt = shGt.valRoot((isGt1) ? v1 : v2);
    const TObjId seg = shGt.objAt(segAt);
    TObjId peer = seg;
    if (OK_DLS == objKind(shGt, seg))
        peer = dlSegPeer(shGt, seg);

    // resolve the 'next' pointer and check its validity
    const TObjId nextPtr = (off)
        ? shGt.ptrAt(shGt.valByOffset(segAt, off->next))
        : nextPtrFromSeg(shGt, peer);

    const TValId nextGt = shGt.valueOf(nextPtr);
    const TValId nextLt = (isGt2) ? v1 : v2;
    if (!checkValueMapping(ctx, 
                (isGt1) ? nextGt : nextLt,
                (isGt2) ? nextGt : nextLt,
                /* allowUnknownMapping */ true))
    {
        SJ_DEBUG("<-- insertSegmentClone: value mismatch "
                 "(nextLt = #" << nextLt << ", nextGt = #" << nextGt << ")");
        return false;
    }

    const TValMapBidir &valMapGt = (isGt1) ? ctx.valMap1 : ctx.valMap2;

    const TValId segGtAt = shGt.placedAt(seg);
    TValPair vp(
            (isGt1) ? segGtAt : VAL_INVALID,
            (isGt2) ? segGtAt : VAL_INVALID);

    scheduleSegAddr(ctx.wl, shGt, seg, peer, action);
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

        if (segGtAt != valGt)
            // OK_MAY_EXIST is applicable only on the first object
            off = 0;

        const EUnknownValue code = shGt.valGetUnknown(valGt);
        switch (code) {
            case UV_UNINITIALIZED:
            case UV_UNKNOWN:
            case UV_DONT_CARE: {
                // clone unknown value
                const TValId vDst = ctx.dst.valCreateUnknown(code);
                if (handleUnknownValues(ctx, vp.first, vp.second, vDst))
                    continue;
                else
                    break;
            }

            default:
                if (segmentCloneCore(ctx, shGt, valGt, valMapGt, action, off))
                    continue;
                else
                    break;
        }

        // clone failed
        *pResult = false;
        return true;
    }

    // schedule the next object in the row
    const TValId valNext1 = (isGt1) ? nextGt : nextLt;
    const TValId valNext2 = (isGt2) ? nextGt : nextLt;
    considerValSchedule(ctx, valNext1, valNext2, VAL_INVALID, VAL_INVALID);
    *pResult = true;
    return true;
}

bool joinAbstractValues(
        bool                    *pResult,
        SymJoinCtx              &ctx,
        const TValId            v1,
        const TValId            v2,
        EUnknownValue           *pCode1,
        EUnknownValue           *pCode2)
{
    const TValId root1 = ctx.sh1.valRoot(v1);
    const TValId root2 = ctx.sh2.valRoot(v2);

    bool isAbs1 = (UV_ABSTRACT == *pCode1);
    if (isAbs1 && hasKey(ctx.sset1, root1)) {
        // do not treat the starting point as encountered segment
        isAbs1 = false;
        *pCode1 = UV_KNOWN;
    }

    bool isAbs2 = (UV_ABSTRACT == *pCode2);
    if (isAbs2 && hasKey(ctx.sset2, root2)) {
        // do not treat the starting point as encountered segment
        isAbs2 = false;
        *pCode2 = UV_KNOWN;
    }

    if (!isAbs1 && !isAbs2)
        // nothing to join here
        return false;

    const EJoinStatus subStatus = (isAbs1)
        ? JS_USE_SH1
        : JS_USE_SH2;

    const bool isValid1 = SymHeap::isPossibleToDeref(ctx.sh1.valTarget(root1));
    const bool isValid2 = SymHeap::isPossibleToDeref(ctx.sh2.valTarget(root2));
    if (isValid1 && isValid2) {
        if (isAbs1 && isAbs2)
            return joinSegmentWithAny(pResult, ctx, root1, root2, JS_USE_ANY);

        else if (joinSegmentWithAny(pResult, ctx, root1, root2, subStatus))
            return true;
    }

    if (UV_UNINITIALIZED == *pCode1 || UV_UNINITIALIZED == *pCode2)
        // such values could be hardly used as reliable anchors
        return false;

    return insertSegmentClone(pResult, ctx, v1, v2, subStatus);
}

class MayExistVisitor {
    private:
        SymJoinCtx              ctx_;
        const EJoinStatus       action_;
        const TValId            valRef_;
        const TValId            root_;
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
            offNext_(0)
        {
            CL_BREAK_IF(JS_USE_SH1 != action && JS_USE_SH2 != action);
        }

        TOffset offNext() const {
            return offNext_;
        }

        bool operator()(
                const SymHeap   &sh,
                const TObjId    sub)
        {
            const TValId val = sh.valueOf(sub);
            const TValId v1 = (JS_USE_SH1 == action_) ? val : valRef_;
            const TValId v2 = (JS_USE_SH2 == action_) ? val : valRef_;
            if (!followValuePair(ctx_, v1, v2, /* read-only */ true))
                return /* continue */ true;

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

    const bool hasMapping1 = hasKey(ctx.valMap1[/* ltr */ 0], v1);
    const bool hasMapping2 = hasKey(ctx.valMap2[/* ltr */ 0], v2);
    if ((hasMapping1 != hasMapping2) && (hasMapping1 == use1))
        // try it the other way around
        return false;

    const SymHeap &sh = (use1) ? ctx.sh1 : ctx.sh2;
    const TValId val = (use1) ? v1 : v2;
    const TObjId target = objRootByVal(sh, val);
    if (target <= 0 || !isComposite(sh.objType(target)))
        // non-starter
        return false;

    if (OK_CONCRETE != objKind(sh, target))
        // only concrete objects/prototypes are candidates for OK_MAY_EXIST
        return false;

    const TValId ref = (use2) ? v1 : v2;
    const TValId targetAt = sh.placedAt(target);
    CL_BREAK_IF(sh.valOffset(targetAt));
    MayExistVisitor visitor(ctx, action, ref, /* root */ targetAt);
    if (traverseLivePtrs(sh, targetAt, visitor))
        // no match
        return false;

    // dig head
    if (sh.valRoot(val) != sh.placedAt(target)) {
        CL_BREAK_IF("MayExistVisitor malfunction");
        return false;
    }

    // mayExistFallback() always implies JS_THREE_WAY
    if (!updateJoinStatus(ctx, JS_THREE_WAY))
        return false;

    BindingOff off;
    off.head = sh.valOffset(val);
    off.next = visitor.offNext();
    bool result = false;

    const bool ok = insertSegmentClone(&result, ctx, v1, v2, action, &off);
    CL_BREAK_IF(!ok);
    (void) ok;

    return result;
}

bool joinUnknownValuesCode(
        EUnknownValue           *pDst,
        const EUnknownValue     code1,
        const EUnknownValue     code2)
{
    if (UV_UNINITIALIZED == code1 && UV_UNINITIALIZED == code2) {
        *pDst = UV_UNINITIALIZED;
        return true;
    }

    if (UV_DONT_CARE == code1 || UV_DONT_CARE == code2) {
        *pDst = UV_DONT_CARE;
        return true;
    }

    if (UV_UNKNOWN == code1 || UV_UNKNOWN == code2) {
        *pDst = UV_UNKNOWN;
        return true;
    }

    return false;
}

bool joinValuePair(SymJoinCtx &ctx, const TValId v1, const TValId v2) {
    const bool err1 = (VAL_DEREF_FAILED == v1);
    const bool err2 = (VAL_DEREF_FAILED == v2);
    if (err1 && err2)
        return true;
    if (err1 || err2)
        return false;

    EUnknownValue code1 = ctx.sh1.valGetUnknown(v1);
    EUnknownValue code2 = ctx.sh2.valGetUnknown(v2);

    EUnknownValue code;
    if (joinUnknownValuesCode(&code, code1, code2)) {
        // create a new unknown value in ctx.dst
        const TValId vDst = ctx.dst.valCreateUnknown(code);
        return handleUnknownValues(ctx, v1, v2, vDst);
    }

    bool result;
    if ((UV_ABSTRACT == code1 || UV_ABSTRACT == code2)
            && joinAbstractValues(&result, ctx, v1, v2, &code1, &code2))
        return result;

    if (code1 != code2) {
        SJ_DEBUG("<-- unknown value code mismatch " << SJ_VALP(v1, v2));
        return false;
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

bool joinCVars(SymJoinCtx &ctx) {
    // gather program variables
    TCVarList cVars1, cVars2;
    ctx.sh1.gatherCVars(cVars1);
    ctx.sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2) {
        SJ_DEBUG("<-- different program variables");
        return false;
    }

    // go through all program variables
    BOOST_FOREACH(const CVar &cv, cVars1) {
        const TValId addr1 = ctx.sh1.addrOfVar(cv);
        const TValId addr2 = ctx.sh2.addrOfVar(cv);

        // create a corresponding program variable in the resulting heap
        const TValId rootDst = ctx.dst.addrOfVar(cv);

        // look at the values inside
        if (!traverseSubObjs(ctx, addr1, addr2, rootDst))
            return false;
    }

    // all OK
    return true;
}

TValId joinDstValue(
        const SymJoinCtx        &ctx,
        const TValId            v1,
        const TValId            v2,
        const bool              validObj1,
        const bool              validObj2)
{
    const TValId vDstBy1 = roMapLookup(ctx.valMap1[/* ltr */ 0], v1);
    const TValId vDstBy2 = roMapLookup(ctx.valMap2[/* ltr */ 0], v2);
    if (vDstBy1 == vDstBy2)
        // the values are equal --> pick any
        return vDstBy1;

    if (!validObj2)
        return vDstBy1;

    if (!validObj1)
        return vDstBy2;

    // tie breaking
    const TValId addr1 = ctx.sh1.valRoot(v1);
    const TValId addr2 = ctx.sh2.valRoot(v2);

    const TValPair tb1(addr1, VAL_INVALID);
    const TValPair tb2(VAL_INVALID, addr2);

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

bool seenUnknown(const SymHeap &sh, const TValId val) {
    if (val <= 0)
        return false;

    const EUnknownValue code = sh.valGetUnknown(val);
    switch (code) {
        case UV_UNINITIALIZED:
        case UV_UNKNOWN:
        case UV_DONT_CARE:
            return true;

        case UV_KNOWN:
        case UV_ABSTRACT:
            return (sh.valOffset(val) < 0);
    }

    // not reachable
    CL_TRAP;
    return false;
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

    const bool isComp1 = (OBJ_INVALID != ctx.sh1.valGetCompositeObj(v1));
    const bool isComp2 = (OBJ_INVALID != ctx.sh2.valGetCompositeObj(v2));
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
        return seenUnknown(ctx.sh1, v1)
            || seenUnknown(ctx.sh2, v2);

    // set the value
    ctx.dst.objSetValue(objDst, vDst);
    return true;
}

bool setDstValues(SymJoinCtx &ctx, const std::set<TObjId> *blackList = 0) {
    typedef SymJoinCtx::TObjMap TObjMap;
    typedef std::map<TObjId /* objDst */, TObjPair> TMap;
    TMap rMap;

    // reverse mapping for ctx.objMap1
    BOOST_FOREACH(TObjMap::const_reference ref, ctx.objMap1) {
        const TObjId objDst = ref.second;
        if (!hasKey(rMap, objDst))
            rMap[objDst].second = OBJ_INVALID;

        // objDst -> obj1
        rMap[objDst].first = ref.first;
    }

    // reverse mapping for ctx.objMap2
    BOOST_FOREACH(TObjMap::const_reference ref, ctx.objMap2) {
        const TObjId objDst = ref.second;
        if (!hasKey(rMap, objDst))
            rMap[objDst].first = OBJ_INVALID;

        // objDst -> obj2
        rMap[objDst].second = ref.first;
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

void handleDstPreds(SymJoinCtx &ctx) {
    // go through all segments and initialize minLength
    BOOST_FOREACH(SymJoinCtx::TSegLengths::const_reference ref, ctx.segLengths)
    {
        const TValId    seg = ref.first;
        const unsigned  len = ref.second;
        segSetMinLength(ctx.dst, seg, len);
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
        if (!matchPreds(ctx.sh1, ctx.dst, ctx.valMap1))
            updateJoinStatus(ctx, JS_USE_SH2);
        if (!matchPreds(ctx.sh2, ctx.dst, ctx.valMap2))
            updateJoinStatus(ctx, JS_USE_SH1);
        return;
    }

    // TODO: match gneric Neq predicates also in prototypes;  for now we
    // consider only minimal segment lengths
    BOOST_FOREACH(const TValTriple &proto, ctx.protoRoots) {
        const TValId proto1     = proto[/* sh1 */ 0];
        const TValId proto2     = proto[/* sh2 */ 1];
        const TValId protoDst   = proto[/* dst */ 2];

        const unsigned len1 = objMinLength(ctx.sh1, proto1);
        const unsigned len2 = objMinLength(ctx.sh2, proto2);
        const unsigned lenDst = objMinLength(ctx.dst, protoDst);

        if (lenDst < len1)
            updateJoinStatus(ctx, JS_USE_SH2);

        if (lenDst < len2)
            updateJoinStatus(ctx, JS_USE_SH1);
    }
}

bool segDetectSelfLoopHelper(
        const SymHeap           &sh,
        std::set<TObjId>        &haveSeen,
        TObjId                  seg)
{
    // remember original kind of object
    const EObjKind kind = objKind(sh, seg);

    // find a loop-less path
    std::set<TObjId> path;
    while (insertOnce(path, seg)) {
        TObjId peer = seg;
        if (OK_DLS == kind) {
            // jump to peer in case of DLS
            peer = dlSegPeer(sh, seg);
            if (!insertOnce(path, peer))
                break;
        }

        const TValId valNext = sh.valueOf(nextPtrFromSeg(sh, peer));
        TObjId next = sh.pointsTo(valNext);
        if (next < 0)
            // no valid next object --> no loop
            return false;

        const EObjKind kindNext = objKind(sh, next);
        if (kindNext != kind)
            // no compatible next segment --> no loop
            return false;

        seg = objRoot(sh, next);
        if (kind != objKind(sh, seg))
            // no compatible next segment --> no loop
            return false;

        // optimization
        haveSeen.insert(seg);
    }

    // loop detected!
    return true;
}

bool segDetectSelfLoop(const SymHeap &sh) {
    using namespace boost::lambda;

    // gather all abstract objects
    TValList valRoots;
    sh.gatherRootObjects(valRoots, SymHeap::isAbstract);

    std::set<TObjId> segRoots;
    BOOST_FOREACH(const TValId at, valRoots)
        segRoots.insert(const_cast<SymHeap &>(sh).objAt(at));

    // go through all entries
    std::set<TObjId> haveSeen;
    BOOST_FOREACH(const TObjId seg, segRoots) {
        if (!insertOnce(haveSeen, seg))
            continue;

        if (segDetectSelfLoopHelper(sh, haveSeen, seg))
            // cycle detected!
            return true;
    }

    // found nothing harmful
    return false;
}

bool validateThreeWayStatus(const SymJoinCtx &ctx) {
    if (JS_THREE_WAY != ctx.status)
        return true;

    if (!ctx.allowThreeWay) {
        CL_DEBUG(">J< destructive information lost detected"
                 ", cancelling three-way join...");
        return false;
    }

    if (segDetectSelfLoop(ctx.dst)) {
        // purely segmental loops cause us problems
        CL_DEBUG(">J< segment cycle detected, cancelling three-way join...");
        return false;
    }

#if !SE_DISABLE_THREE_WAY_JOIN
    return true;
#endif

    CL_WARN("three-way join disabled by configuration, recompile "
            "with SE_DISABLE_THREE_WAY_JOIN == 0 to enable it");
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

    // start with program variables
    if (!joinCVars(ctx)) {
        CL_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // go through all values in them
    if (!joinPendingValues(ctx)) {
        CL_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // time to preserve all 'hasValue' edges
    if (!setDstValues(ctx)) {
        CL_BREAK_IF(areEqual(sh1, sh2));
        return false;
    }

    // go through shared Neq predicates and set minimal segment lengths
    handleDstPreds(ctx);

    if (debugSymJoin) {
        // catch possible regression at this point
        CL_BREAK_IF((JS_USE_ANY == ctx.status) != areEqual(sh1, sh2));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh1, ctx.dst));
        CL_BREAK_IF((JS_THREE_WAY == ctx.status) && areEqual(sh2, ctx.dst));
    }

    // if the result is three-way join, check if it is a good idea
    if (!validateThreeWayStatus(ctx))
        return false;

    // all OK
    *pStatus = ctx.status;
    SJ_DEBUG("<-- joinSymHeaps() says " << ctx.status);
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    return true;
}

class GhostMapper {
    private:
        const SymHeap           &sh_;
        TValMap                 &vMap_;

    public:
        GhostMapper(const SymHeap &sh, TValMap &vMap):
            sh_(sh),
            vMap_(vMap)
        {
        }

        bool operator()(const TObjId item[2]) {
            // obtain addresses
            const TValId addrReal  = sh_.placedAt(item[0]);
            const TValId addrGhost = sh_.placedAt(item[1]);
            CL_BREAK_IF(addrReal < 0 || addrGhost < 0);
            CL_BREAK_IF(addrReal == addrGhost);

            // wait, first we need to translate the address into ctx.dst world
            const TValId image = roMapLookup(vMap_, addrReal);
            CL_BREAK_IF(image <= 0);

            // introduce ghost mapping
            CL_BREAK_IF(hasKey(vMap_, addrGhost) && vMap_[addrGhost] != image);
            vMap_[addrGhost] = image;

            return /* continue */ true;
        }
};

void mapGhostAddressSpace(
        SymJoinCtx              &ctx,
        const TValId            addrReal,
        const TValId            addrGhost,
        const EJoinStatus       action)
{
    CL_BREAK_IF(!ctx.joiningData());
    CL_BREAK_IF(addrReal < 0 || addrGhost < 0);

    TValMapBidir &vMap = (JS_USE_SH1 == action)
        ? ctx.valMap1
        : ctx.valMap2;

    GhostMapper visitor(ctx.sh1, vMap[/* ltr */ 0]);
    const TValId roots[] = { addrReal, addrGhost };

    // FIXME: this will break as soon as we switch to delayed objects creation
    traverseLiveObjs<2>(ctx.sh1, roots, visitor);
}

/// this runs only in debug build
bool dlSegCheckProtoConsistency(const SymJoinCtx &ctx) {
    BOOST_FOREACH(const TValTriple &proto, ctx.protoRoots) {
        const TValId proto1   = proto[0];
        const TValId proto2   = proto[1];
        const TValId protoDst = proto[2];
        if (OK_DLS != ctx.dst.valTargetKind(protoDst))
            // we are intersted only DLSs here
            continue;

        TValTriple protoPeer;
        const TValId peerDst = dlSegPeer(ctx.dst, protoDst);

        if (OK_DLS == ctx.sh1.valTargetKind(proto1))
            protoPeer[0] = dlSegPeer(ctx.sh1, proto1);
        else {
            const TValMap &vMap1r = ctx.valMap1[/* rtl */ 1];
            protoPeer[0] = roMapLookup(vMap1r, peerDst);
        }

        if (OK_DLS == ctx.sh2.valTargetKind(proto2))
            protoPeer[1] = dlSegPeer(ctx.sh2, proto2);
        else {
            const TValMap &vMap2r = ctx.valMap2[/* rtl */ 1];
            protoPeer[1] = roMapLookup(vMap2r, peerDst);
        }

        protoPeer[2] = dlSegPeer(ctx.dst, protoDst);
        if (hasKey(ctx.protoRoots, protoPeer))
            continue;

        CL_ERROR("DLS prototype peer not a prototype "
                << SJ_VALP(proto1, proto2));
        return false;
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

    // TODO: remove this
    const TObjId o1 = sh.objAt(addr1);
    const TObjId o2 = sh.objAt(addr2);

    const struct cl_type *clt;
    if (!joinObjClt(&clt, ctx, o1, o2) || !clt) {
        CL_BREAK_IF("joinDataCore() called on objects with incompatible clt");
        return false;
    }

    int size;
    if (!joinObjSize(&size, ctx, addr1, addr2))
        return false;

    // start with the given pair of objects and create a ghost object for them
    // create an image in ctx.dst
    const TValId rootDstAt = ctx.dst.heapAlloc(size);
    const TObjId rootDst = ctx.dst.objAt(rootDstAt);
    if (clt)
        ctx.dst.objDefineType(rootDst, clt);

    if (!traverseSubObjs(ctx, addr1, addr2, rootDstAt, &off))
        return false;

    ctx.sset1.insert(addr1);
    ctx.sset2.insert(addr2);

    // never step over DLS peer
    if (OK_DLS == objKind(sh, o1)) {
        const TValId peer = dlSegPeer(sh, addr1);
        ctx.sset1.insert(peer);
        if (peer != addr2)
            mapGhostAddressSpace(ctx, addr1, peer, JS_USE_SH1);
    }
    if (OK_DLS == objKind(sh, o2)) {
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
    buildIgnoreList(blackList, ctx.dst, rootDst, off);
    if (!setDstValues(ctx, &blackList))
        return false;

    // check consistency of DLS prototype peers
    CL_BREAK_IF(!dlSegCheckConsistency(ctx.dst));
    CL_BREAK_IF(!dlSegCheckProtoConsistency(ctx));

    // go through Neq predicates
    handleDstPreds(ctx);

    // if the result is three-way join, check if it is a good idea
    return validateThreeWayStatus(ctx);
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
    BOOST_FOREACH(const TValTriple &proto, ctx.protoRoots) {
        const TValId proto1 = proto[0];
        const TValId proto2 = proto[1];

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

struct JoinValueVisitor {
    SymJoinCtx                  &ctx;
    std::set<TObjId>            ignoreList;
    const bool                  bidir;

    JoinValueVisitor(SymJoinCtx &ctx_, bool bidir_):
        ctx(ctx_),
        bidir(bidir_)
    {
    }

    TValId joinValues(const TValId oldDst, const TValId oldSrc) const {
        const TValId newDst = roMapLookup(ctx.valMap1[/* ltr */ 0], oldDst);
        const TValId newSrc = roMapLookup(ctx.valMap2[/* ltr */ 0], oldSrc);
        if (newDst == newSrc)
            // values are equal --> pick any
            return newDst;

        // asymmetric prototype match (src < dst)
        TValTriple proto;
        proto[0] = ctx.dst.valRoot(oldDst);
        proto[1] = VAL_INVALID;
        proto[2] = ctx.dst.valRoot(newDst);
        if (hasKey(ctx.protoRoots, proto))
            return newDst;

        // asymmetric prototype match (dst < src)
        proto[0] = VAL_INVALID;
        proto[1] = ctx.dst.valRoot(oldSrc);
        proto[2] = ctx.dst.valRoot(newSrc);
        if (hasKey(ctx.protoRoots, proto))
            return newSrc;

        if (VAL_NULL == newSrc && UV_UNKNOWN == ctx.dst.valGetUnknown(newDst))
            return newDst;

        if (VAL_NULL == newDst && UV_UNKNOWN == ctx.dst.valGetUnknown(newSrc))
            return newSrc;

        CL_ERROR("JoinValueVisitor failed to join values");
        CL_BREAK_IF("JoinValueVisitor is not yet fully implemented");
        return VAL_INVALID;
    }

    bool operator()(TObjId item[2]) const {
        SymHeap &sh = ctx.dst;
        const TObjId dst = item[0];
        const TObjId src = item[1];
        if (hasKey(this->ignoreList, dst))
            return /* continue */ true;

        const TValId oldDst = sh.valueOf(dst);
        const TValId oldSrc = sh.valueOf(src);

        const TValId valNew = this->joinValues(oldDst, oldSrc);
        if (VAL_INVALID == valNew)
            return /* continue */ true;

        sh.objSetValue(dst, valNew);

        if (collectJunk(sh, oldDst))
            CL_DEBUG("    JoinValueVisitor drops a sub-heap (oldDst)");

        if (!this->bidir)
            return /* continue */ true;

        sh.objSetValue(src, valNew);
        if (collectJunk(sh, oldSrc))
            CL_DEBUG("    JoinValueVisitor drops a sub-heap (oldSrc)");

        return /* continue */ true;
    }
};

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
    BOOST_FOREACH(const TValTriple &proto, ctx.protoRoots) {
        const TValId protoDst   = proto[/* sh1 */ 0];
        const TValId protoSrc   = proto[/* sh2 */ 1];
        const TValId protoGhost = proto[/* dst */ 2];

        if (SymHeap::isAbstract(sh.valTarget(protoDst)))
            // remove Neq predicates, their targets are going to vanish soon
            segSetMinLength(sh, protoDst, 0);

        if (bidir && SymHeap::isAbstract(sh.valTarget(protoSrc)))
            // remove Neq predicates, their targets are going to vanish soon
            segSetMinLength(sh, protoSrc, 0);

        if (SymHeap::isAbstract(sh.valTarget(protoGhost)))
            // temporarily remove Neq predicates
            segSetMinLength(sh, protoGhost, 0);

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
    BOOST_FOREACH(const TValTriple &proto, ctx.protoRoots) {
        typedef SymJoinCtx::TSegLengths TLens;
        const TLens &lens = ctx.segLengths;

        const TValId protoDst = proto[/* dst */ 2];
        TLens::const_iterator it = lens.find(protoDst);
        if (lens.end() == it)
            continue;

        const unsigned len = it->second;
        if (len)
            segSetMinLength(sh, protoDst, len);
    }
}

/// replacement of matchData() from symdiscover
bool joinData(
        SymHeap                 &sh,
        const TValId            dst,
        const TValId            src,
        const bool              bidir)
{
    // TODO: remove this
    SJ_DEBUG("--> joinData" << SJ_VALP(dst, src));
    ++cntJoinOps;

    // dst is expected to be a segment
    CL_BREAK_IF(!SymHeap::isAbstract(sh.valTarget(dst)));
    const BindingOff off(sh.segBinding(dst));
    if (debugSymJoin) {
        EJoinStatus status = JS_USE_ANY;
        joinDataReadOnly(&status, sh, off, dst, src, 0);
        if (JS_USE_ANY != status)
            debugPlot(sh, "joinData", dst, src, "00");
    }

    // go through the commont part of joinData()/joinDataReadOnly()
    SymJoinCtx ctx(sh);
    if (!joinDataCore(ctx, off, dst, src))
        // TODO: collect the already created dangling objects and return
        //       the heap in a more consistent shape!
        return false;

    // ghost is a transiently existing object representing the join of dst/src
    const TValId ghost = roMapLookup(ctx.valMap1[0], dst);
    CL_BREAK_IF(ghost != roMapLookup(ctx.valMap2[0], src));

    // assign values within dst (and also in src if bidir == true)
    JoinValueVisitor visitor(ctx, bidir);
    buildIgnoreList(visitor.ignoreList, sh, dst);

    // FIXME: this will break as soon as we switch to delayed objects creation
    const TValId roots[] = { dst, src};
    traverseLiveObjs<2>(sh, roots, visitor);

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
