/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
#include "symcmp.hh"

#include <cl/cl_msg.hh>

#include "symutil.hh"
#include "util.hh"
#include "worklist.hh"

#include <algorithm>            // for std::copy
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#define SC_DEBUG(...) do {                                                  \
    if (::debugSymCmp)                                                      \
        CL_DEBUG("SymCmp: " << __VA_ARGS__);                                \
} while (0)

#define SC_DUMP_ID(sh, id) \
    "dump_id((SymHeap *)" << &(sh) << ", " << (id) << ")"

#define SC_DUMP_V1_V2(sh1, sh2, v1, v2)                                     \
    ", v1 = " << SC_DUMP_ID(sh1, v1) <<                                     \
    ", v2 = " << SC_DUMP_ID(sh2, v2)

#define SC_DEBUG_VAL_SCHEDULE(who, sh1, sh2, v1, v2)                        \
    SC_DEBUG("+++ " << who                                                  \
            << SC_DUMP_V1_V2(sh1, sh2, v1, v2))

#define SC_DEBUG_VAL_SCHEDULE_BY(who, o1, o2, sh1, sh2, v1, v2)             \
    SC_DEBUG_VAL_SCHEDULE(who << "(" << o1 << ", " << o2 << ")",            \
                          sh1, sh2, v1, v2)

#define SC_DEBUG_VAL_MISMATCH(...)                                          \
    SC_DEBUG("<-- "                                                         \
            << __VA_ARGS__                                                  \
            << SC_DUMP_V1_V2(sh1, sh2, v1, v2))

// set to 'true' if you wonder why SymCmp matches states as it does (noisy)
static bool debugSymCmp = static_cast<bool>(DEBUG_SYMCMP);

bool compareClt(
        const struct cl_type    *clt1,
        const struct cl_type    *clt2)
{
    if (clt1 == clt2)
        // exact match
        return true;

    if (!clt1 || !clt2)
        // one of them is NULL
        return false;

    // compare the types semantically
    return (*clt1 == *clt2);
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

bool matchPlainValues(
        TValMapBidir            valMapping,
        const TValueId          v1,
        const TValueId          v2)
{
    if (!checkNonPosValues(v1, v2))
        // null vs. non-null, etc.
        return false;

    else if (v1 <= VAL_NULL)
        // no need to save mapping of special values, they're fixed anyway
        return true;

    // left-to-right check
    TValMap &ltr = valMapping[/* ltr */ 0];
    TValMap::iterator iter1 = ltr.find(v1);
    if (iter1 != ltr.end())
        // substitution already defined, check if it applies seamlessly
        return iter1->second == v2;

    // right-to-left check
    TValMap &rtl = valMapping[/* rtl */ 1];
    TValMap::iterator iter2 = rtl.find(v2);
    if (iter2 != rtl.end())
        // substitution already defined, check if it applies seamlessly
        return iter2->second == v1;

    // not found --> define a new substitution
    ltr[v1] = v2;
    rtl[v2] = v1;
    return true;
}

template <class TMapping>
bool matchValues(
        bool                    *follow,
        TMapping                &valMapping,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValueId          v1,
        const TValueId          v2)
{
    *follow = false;
    if (!matchPlainValues(valMapping, v1, v2))
        return false;

    // check for special values
    const bool isSpecial = (v1 <= 0);
    CL_BREAK_IF(isSpecial && 0 < v2);
    if (isSpecial)
        // already checked by matchPlainValues()/checkNonPosValues()
        return true;

    const struct cl_type *clt1 = sh1.valType(v1);
    const struct cl_type *clt2 = sh2.valType(v2);
    if (!compareClt(clt1, clt2))
        // value clt mismatch
        return false;

    // check for unknown values
    const EUnknownValue code = sh1.valGetUnknown(v1);
    if (code != sh2.valGetUnknown(v2))
        // mismatch in kind of unknown values
        return false;

    switch (code) {
        case UV_KNOWN:
        case UV_ABSTRACT:
            break;

        case UV_UNKNOWN:
        case UV_DONT_CARE:
        case UV_UNINITIALIZED:
            // do not follow unknown values
            return true;
    }

    // check custom values state
    const int cVal1 = sh1.valGetCustom(&clt1, v1);
    const int cVal2 = sh2.valGetCustom(&clt2, v2);
    if ((OBJ_INVALID == cVal1) != (OBJ_INVALID == cVal2))
        return false;

    if (OBJ_INVALID != cVal1) {
        if (!compareClt(clt1, clt2))
            // custom value clt mismatch
            return false;

        // match pair of custom values
        return (cVal1 == cVal2);
    }

    // follow all other values
    *follow = true;
    return true;
}

template<class TWorkList>
bool digComposite(
        bool                    *isComp,
        TWorkList               &wl,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const TValueId          v1,
        const TValueId          v2)
{
    *isComp = false;
    const TObjId cObj1 = sh1.valGetCompositeObj(v1);
    const TObjId cObj2 = sh2.valGetCompositeObj(v2);

    const bool isComp1 = (OBJ_INVALID != sh1.valGetCompositeObj(v1));
    const bool isComp2 = (OBJ_INVALID != sh2.valGetCompositeObj(v2));
    if (isComp1 != isComp2)
        // scalar vs. composite objects, the heaps can't be equal
        return false;

    if (isComp1)
        *isComp = true;
    else
        return true;

    // we _have_ to jump to the roots at this point as long as we admit
    // to see through multi-level Linux lists
    const TObjId root1 = objRoot(sh1, cObj1);
    const TObjId root2 = objRoot(sh2, cObj2);
    CL_BREAK_IF(OBJ_INVALID == root1 || OBJ_INVALID == root2);

    typedef std::pair<TObjId, TObjId> TItem;
    std::stack<TItem> todo;
    push(todo, root1, root2);
    while (!todo.empty()) {
        TObjId o1, o2;
        boost::tie(o1, o2) = todo.top();
        todo.pop();

        const struct cl_type *clt = sh1.objType(o1);
        if (clt != sh2.objType(o2))
            // type mismatch
            return false;

        if (isComposite(clt)) {
            for (int i = 0; i < clt->item_cnt; ++i) {
                const TObjId sub1 = sh1.subObj(o1, i);
                const TObjId sub2 = sh2.subObj(o2, i);
                CL_BREAK_IF(sub1 < 0 || sub2 < 0);

                push(todo, sub1, sub2);
            }

            continue;
        }

        const TValueId val1 = sh1.valueOf(o1);
        const TValueId val2 = sh2.valueOf(o2);
        if (wl.schedule(val1, val2)) {
            SC_DEBUG_VAL_SCHEDULE_BY("digComposite", o1, o2,
                                  sh1, sh2, val1, val2);
        }
    }
    return true;
}

bool cmpAbstractObjects(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        TObjId                  o1,
        TObjId                  o2)
{
    if (&sh1 != &sh2 && sh1.objIsProto(o1) != sh2.objIsProto(o2))
        // prototype vs. shared object while called from areEqual()
        return false;

    const EObjKind kind = sh1.objKind(o1);
    if (sh2.objKind(o2) != kind)
        // kind of object mismatch
        return false;

    if (OK_CONCRETE == kind || OK_PART == kind)
        // no abstract objects comparison
        return true;

    // compare binding fields
    const BindingOff &off1 = sh1.objBinding(o1);
    const BindingOff &off2 = sh2.objBinding(o2);
    return (off1 == off2);
}

template <class TWorkList, class TMapping>
bool dfsCmp(
        TWorkList               &wl,
        TMapping                &valMapping,
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const bool              *pCancel = 0)
{
    // DFS loop
    typename TWorkList::value_type item;
    while (wl.next(item)) {
        TValueId v1, v2;
        boost::tie(v1, v2) = item;

        if (pCancel && *pCancel)
            // traversal completely canceled by visitor
            return false;

        bool follow;
        if (!matchValues(&follow, valMapping, sh1, sh2, v1, v2)) {
            SC_DEBUG_VAL_MISMATCH("value mismatch");
            return false;
        }

        if (!follow)
            // no need for next wheel
            continue;

        bool isComp;
        if (!digComposite(&isComp, wl, sh1, sh2, v1, v2)) {
            SC_DEBUG_VAL_MISMATCH("object composition mismatch");
            return false;
        }

        if (isComp)
            continue;

        const TObjId obj1 = sh1.pointsTo(v1);
        const TObjId obj2 = sh2.pointsTo(v2);
        if (!checkNonPosValues(obj1, obj2)) {
            SC_DEBUG("non-matched targets");
            return false;
        }

        if (!cmpAbstractObjects(sh1, sh2, obj1, obj2)) {
            SC_DEBUG_VAL_MISMATCH("incompatible abstract objects");
            return false;
        }

        v1 = sh1.valueOf(obj1);
        v2 = sh2.valueOf(obj2);

        // schedule values for next wheel
        if (wl.schedule(v1, v2))
            SC_DEBUG_VAL_SCHEDULE_BY("dfsCmp", obj1, obj2, sh1, sh2, v1, v2);
    }

    // finally match heap predicates
    if (!sh1.matchPreds(sh2, valMapping[/* ltr */ 0])
            || !sh2.matchPreds(sh1, valMapping[/* rtl */ 1]))
    {
        SC_DEBUG("<-- failed to match heap predicates");
        return false;
    }

    // heaps are equal (isomorphism)
    return true;
}

bool areEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        TValMap                 *srcToDst,
        TValMap                 *dstToSrc)
{
    CL_BREAK_IF(srcToDst && !srcToDst->empty());
    CL_BREAK_IF(dstToSrc && !dstToSrc->empty());

    // DFS stack
    WorkList<TValPair> wl;

    // value substitution (isomorphism)
    TValMap valMapping[/* left-to-right + right-to-left */ 2];

    // FIXME: suboptimal interface of SymHeap::gatherCVars()
    SymHeap::TContCVar cVars1, cVars2;
    sh1.gatherCVars(cVars1);
    sh2.gatherCVars(cVars2);
    if (cVars1 != cVars2) {
        SC_DEBUG("<-- different program variables");
        return false;
    }

    BOOST_FOREACH(CVar cv, cVars1) {
        const TObjId o1 = sh1.objByCVar(cv);
        const TObjId o2 = sh2.objByCVar(cv);
        CL_BREAK_IF(o1 < 0 || o2 < 0);

        // retrieve values of static variables
        const TValueId v1 = sh1.valueOf(o1);
        const TValueId v2 = sh2.valueOf(o2);

        // optimization
        bool follow;
        if (!matchValues(&follow, valMapping, sh1, sh2, v1, v2)) {
            SC_DEBUG_VAL_MISMATCH("value mismatch");
            return false;
        }

        // schedule for DFS
        if (follow && wl.schedule(v1, v2))
            SC_DEBUG_VAL_SCHEDULE("cVar(" << cv.uid << ")", sh1, sh2, v1, v2);
    }

    // run DFS
    if (!dfsCmp(wl, valMapping, sh1, sh2))
        return false;

    if (srcToDst)
        *srcToDst = valMapping[/* ltr */ 0];

    if (dstToSrc)
        *dstToSrc = valMapping[/* rtl */ 1];

    // match!
    return true;
}

template <class TItem, class TVisitor>
class CustomWorkList: public WorkList<TItem> {
    public:
        bool                    cancel;

    private:
        typedef WorkList<TItem> TBase;
        const SymHeap           &sh_;
        TVisitor                *visitor_;
        std::set<TItem>         haveSeen_;

    public:
        CustomWorkList(const SymHeap &sh, TVisitor *visitor):
            cancel(false),
            sh_(sh),
            visitor_(visitor)
        {
        }

        bool schedule(const TItem &vp) {
            if (this->cancel)
                return false;

            if (!visitor_)
                // no visitor anyway, keep going
                return TBase::schedule(vp);

            if (!insertOnce(haveSeen_, vp))
                // already seen by visitor
                return false;

            SC_DEBUG("CustomWorkList::schedule() calls visitor"
                     << SC_DUMP_V1_V2(sh_, sh_, vp.first, vp.second));

            bool wantTraverse = true;
            if (!visitor_->handleValuePair(&wantTraverse, vp)) {
                // traversal completely canceled by the visitor
                this->cancel = true;
                return false;
            }

            if (!wantTraverse)
                // visitor does not want to traverse this pair of values
                return false;

            const bool rv = TBase::schedule(vp);
            CL_BREAK_IF(!rv);
            return rv;
        }

        // FIXME: there is no TBase::schedule() virtual method --> subtle
        bool schedule(const TValueId v1, const TValueId v2) {
            const TValPair vp(v1, v2);
            return this->schedule(vp);
        }
};

bool matchCVars(const SymHeap &sh, const TValMap &valMap) {
    BOOST_FOREACH(TValMap::const_reference vp, valMap) {
        const TObjId o1 = sh.pointsTo(vp.first);
        const TObjId o2 = sh.pointsTo(vp.second);
        if (o1 <= 0)
            continue;

        CL_BREAK_IF(o2 <= 0);

        CVar cv1, cv2;
        const bool isCVar1 = sh.cVar(&cv1, o1);
        const bool isCVar2 = sh.cVar(&cv2, o2);
        if (isCVar1 != isCVar2 || cv1 != cv2)
            return false;
    }

    return true;
}

bool matchSubHeaps(
        const SymHeap           &sh,
        const TValPairList      &startingPoints,
        ISubMatchVisitor        *visitor)
{
    // DFS stack
    typedef CustomWorkList<TValPair, ISubMatchVisitor>      TWorkList;
    TWorkList wl(sh, visitor);

    // value substitution (isomorphism)
    TValMap valMapping[/* left-to-right + right-to-left */ 2];

    BOOST_FOREACH(const TValPair &vp, startingPoints) {
        // FIXME: it's not clear from the dox, if startingPoints should be also
        //        given to the visitor, or not (but they _should_)
        if (wl.schedule(vp))
            SC_DEBUG("matchSubHeaps() picks up a starting point"
                     << SC_DUMP_V1_V2(sh, sh, vp.first, vp.second));
    }

    // run DFS
    if (!dfsCmp(wl, valMapping, sh, sh, &wl.cancel) || wl.cancel)
        return false;

    // FIXME: too late (significant performance waste)
    return matchCVars(sh, valMapping[0]);
}
