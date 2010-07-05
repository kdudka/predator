/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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
#include "symabstract.hh"

#include <cl/cl_msg.hh>
#include <cl/storage.hh>

#include "util.hh"

#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

/// abstraction trigger threshold for SLS
static const unsigned SLS_LEN_THRESHOLD = 1;
static const unsigned SLS_SPARE_PREFIX  = 1;
static const unsigned SLS_SPARE_SUFFIX  = 0;

typedef std::pair<TObjId, TObjId> TObjPair;

// helper template for traverseSubObjs()
template <class TItem> struct TraverseSubObjsHelper { };

// specialisation for TObjId, which means basic implementation of the traversal
template <> struct TraverseSubObjsHelper<TObjId> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjId obj) {
        return sh.objType(obj);
    }
    static TObjId getNextItem(const SymHeap &sh, TObjId obj, int nth) {
        return sh.subObj(obj, nth);
    }
};

// specialisation suitable for traversing two composite objects simultaneously
template <> struct TraverseSubObjsHelper<TObjPair> {
    static const struct cl_type* getItemClt(const SymHeap &sh, TObjPair item) {
        const struct cl_type *clt1 = sh.objType(item.first);
        const struct cl_type *clt2 = sh.objType(item.second);
        if (clt1 != clt2)
            TRAP;

        return clt1;
    }
    static TObjPair getNextItem(const SymHeap &sh, TObjPair item, int nth) {
        item.first  = sh.subObj(item.first,  nth);
        item.second = sh.subObj(item.second, nth);
        return item;
    }
};

// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, typename TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjs(THeap &sh, TItem item, TVisitor visitor) {
    std::stack<TItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, item);
        if (!clt || clt->code != CL_TYPE_STRUCT)
            TRAP;

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TItem next = THelper::getNextItem(sh, item, i);
            if (!/* continue */visitor(sh, next))
                return false;

            const struct cl_type *subClt = THelper::getItemClt(sh, next);
            if (subClt && subClt->code == CL_TYPE_STRUCT)
                todo.push(next);
        }
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

namespace {

bool doesAnyonePointToInsideVisitor(const SymHeap &sh, TObjId sub) {
    const TValueId subAddr = sh.placedAt(sub);
    return /* continue */ !sh.usedByCount(subAddr);
}

bool doesAnyonePointToInside(const SymHeap &sh, TObjId obj) {
    return !traverseSubObjs(sh, obj, doesAnyonePointToInsideVisitor);
}

bool abstractNonMatchingValuesVisitor(SymHeap &sh, TObjPair item) {
    const TObjId dst = item.second;
    const TValueId valSrc = sh.valueOf(item.first);
    const TValueId valDst = sh.valueOf(dst);
    bool eq;
    if (sh.proveEq(&eq, valSrc, valDst) && eq)
        // values are equal
        return /* continue */ true;

    // attempt to dig some type-info for the new unknown value
    const struct cl_type *clt = sh.valType(valSrc);
    const struct cl_type *cltDst = sh.valType(valDst);
    if (!clt)
        clt = cltDst;
    else if (cltDst && cltDst != clt)
        // should be safe to ignore
        TRAP;

    // create a new unknown value as a placeholder
    const TValueId valNew = sh.valCreateUnknown(UV_UNKNOWN, clt);

    // FIXME: A virtual junk may be introduced at this point!  The junk is not
    // anyhow reported to user, but causes the annoying warnings about dangling
    // root objects.  We should probably treat it as regular (false?) alarm,
    // utilize SymHeapProcessor to collect it and properly report.  However it
    // requires to pull-in location info, backtrace and the like.  Luckily, the
    // junk is not going to survive next run of symcut anyway, so it should not
    // shoot down the analysis completely.
    sh.objSetValue(dst, valNew);
    return /* continue */ true;
}

// when abstracting an object, we need to abstract all non-matching values in
void abstractNonMatchingValues(SymHeap &sh, TObjId src, TObjId dst) {
    if (OK_CONCRETE == sh.objKind(dst))
        // invalid call of abstractNonMatchingValues()
        TRAP;

    // wait, first preserve the value of binder
    const TObjId objBind = subObjByChain(sh, dst, sh.objBinderField(dst));
    const TValueId valBind = sh.valueOf(objBind);
    if (!sh.objPeerField(dst).empty())
        // TODO: do the same for the 'peer' field, DLS are involved
        TRAP;

    // traverse all sub-objects
    const TObjPair item(src, dst);
    traverseSubObjs(sh, item, abstractNonMatchingValuesVisitor);

    // now restore the possibly smashed value of binder
    sh.objSetValue(objBind, valBind);
}

void objReplace(SymHeap &sh, TObjId oldObj, TObjId newObj) {
    if (OBJ_INVALID != sh.objParent(oldObj)
            || OBJ_INVALID != sh.objParent(newObj))
        // attempt to replace a sub-object
        TRAP;

    // resolve object addresses
    const TValueId oldAddr = sh.placedAt(oldObj);
    const TValueId newAddr = sh.placedAt(newObj);
    if (oldAddr <= 0 || newAddr <= 0)
        TRAP;

    // update all references
    sh.valReplace(oldAddr, newAddr);

    // now destroy the old object
    sh.objDestroy(oldObj);
}

class ProbeVisitor {
    private:
        TValueId                addr_;
        const struct cl_type    *clt_;
        unsigned                arrity_;

    public:
        ProbeVisitor(const SymHeap &sh, TObjId obj, EObjKind kind) {
            addr_ = sh.placedAt(obj);
            clt_  = sh.objType(obj);
            if (!addr_ || !clt_ || CL_TYPE_STRUCT != clt_->code)
                TRAP;

            arrity_ = static_cast<unsigned>(kind);
            if (!arrity_)
                TRAP;
        }

    bool operator()(const SymHeap &sh, TObjId obj) const {
        const TValueId valNext = sh.valueOf(obj);
        if (valNext <= 0 || valNext == addr_ || sh.valType(valNext) != clt_)
            return /* continue */ true;

        const EUnknownValue code = sh.valGetUnknown(valNext);
        switch (code) {
            case UV_KNOWN:
            case UV_ABSTRACT:
                // only known objects can be chained
                break;

            default:
                return /* continue */ true;
        }

        const TObjId target = sh.pointsTo(valNext);
        const TValueId targetAddr = sh.placedAt(target);
        if (targetAddr <= 0)
            TRAP;

        if (sh.cVar(0, obj))
            // a list segment through non-heap objects basically makes no sense
            return /* continue */ true;

        if (sh.usedByCount(targetAddr) != arrity_)
            return /* continue */ true;

        return doesAnyonePointToInside(sh, target);
    }
};

bool probe(SymHeap &sh, TObjId obj, EObjKind kind) {
    if (doesAnyonePointToInside(sh, obj))
        return false;

    const ProbeVisitor visitor(sh, obj, kind);
    return !traverseSubObjs(sh, obj, visitor);
}

// TODO: hook this somehow on the existing visitor infrastructure in order
//       to avoid code duplicity ... challenge? ;-)
//
// NOTE: we have basically the same code in SymHeapPlotter::Private::digObj()
template <class TDst>
void digAnyListSelectors(TDst &dst, const SymHeap &sh, TObjId obj,
                         EObjKind kind)
{
    const ProbeVisitor visitor(sh, obj, kind);
    TFieldIdxChain ic;

    typedef boost::tuple<TObjId, int /* nth */, bool /* last */> TStackItem;
    std::stack<TStackItem> todo;
    todo.push(TStackItem(obj, -1, false));
    while (!todo.empty()) {
        bool last;
        int nth;
        boost::tie(obj, nth, last) = todo.top();
        todo.pop();

        const struct cl_type *clt = sh.objType(obj);
        if (clt && clt->code == CL_TYPE_STRUCT) {
            if (-1 != nth)
                // nest into structure
                ic.push_back(nth);

            for (int i = 0; i < clt->item_cnt; ++i) {
                const TObjId sub = sh.subObj(obj, i);
                if (!visitor(sh, sub)) {
                    // great, we have a candidate
                    ic.push_back(i);
                    dst.push_back(ic);
                    ic.pop_back();
                }

                const struct cl_type *subClt = sh.objType(sub);
                if (subClt && subClt->code == CL_TYPE_STRUCT)
                    todo.push(TStackItem(sub, i, /* last */ (0 == i)));
            }
        }

        if (last)
            // leave the structure
            ic.pop_back();
    }
}

unsigned /* len */ discoverSls(const SymHeap &sh, TObjId obj,
                               TFieldIdxChain icNext)
{
    // we use std::set to avoid an infinite loop
    std::set<TObjId> path;
    while (!hasKey(path, obj)) {
        path.insert(obj);

        const TObjId objPtrNext = subObjByChain(sh, obj, icNext);
        const ProbeVisitor visitor(sh, obj, OK_SLS);
        if (visitor(sh, objPtrNext))
            // we can't go further
            break;

        const TValueId valNext = sh.valueOf(objPtrNext);
        const TObjId objNext = sh.pointsTo(valNext);
        if (0 < objNext)
            obj = objNext;
    }

    return path.size() - 1;
}

void skipObj(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext) {
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TValueId valNext = sh.valueOf(objPtrNext);
    const TObjId objNext = sh.pointsTo(valNext);
    if (OBJ_INVALID == objNext)
        TRAP;

    // move to the next object
    *pObj = objNext;
}

void conjureSls(SymHeap &sh, TObjId *pObj, TFieldIdxChain icNext) {
    const TObjId objPtrNext = subObjByChain(sh, *pObj, icNext);
    const TValueId valNext = sh.valueOf(objPtrNext);
    if (valNext <= 0 || 1 != sh.usedByCount(valNext))
        // this looks like a failure of discoverSls()
        TRAP;

    const TObjId objNext = sh.pointsTo(valNext);
    if (OK_CONCRETE == sh.objKind(objNext)) {
        // abstract the _next_ object
        sh.objAbstract(objNext, OK_SLS, icNext);

        // we're constructing the abstract object from a concrete one --> it
        // implies non-empty LS at this point
        const TValueId addrNext = sh.placedAt(objNext);
        const TObjId objNextNextPtr = subObjByChain(sh, objNext, icNext);
        const TValueId valNextNext = sh.valueOf(objNextNextPtr);
        if (addrNext <= 0 || valNextNext < /* we allow NULL here */ 0)
            TRAP;
        sh.addNeq(addrNext, valNextNext);
    }

    if (OK_SLS != sh.objKind(objNext))
        TRAP;

    // replace self by the next object
    abstractNonMatchingValues(sh, *pObj, objNext);
    objReplace(sh, *pObj, objNext);

    // move to the next object
    *pObj = objNext;
}

void abstract(SymHeap &sh, TObjId obj, EObjKind kind) {
    switch (kind) {
        case OK_CONCRETE:
            // invalid call of abstract()
            TRAP;

        case OK_SLS:
            CL_DEBUG("abstract: considering SLS abstraction...");
            break;

        case OK_DLS:
            CL_DEBUG("abstract: considering DLS abstraction...");
            break;
    }

    if (probe(sh, obj, kind))
        CL_DEBUG("abstract: initial probe was successful!");
    else
        return;

    // gather suitable selectors
    std::vector<TFieldIdxChain> selectors;
    digAnyListSelectors(selectors, sh, obj, kind);
    const unsigned cnt = selectors.size();
    CL_DEBUG("abstract: found " << cnt << " list selector candidate(s)");
    if (!cnt)
        TRAP;

    // choose the best selectors for SLS
    unsigned idxBest;
    unsigned slsBestLenght = 0;
    for (unsigned i = 0; i < cnt; ++i) {
        CL_DEBUG("abstract: calling discoverSls() on selector "
                << (i + 1) << "/" << cnt);

        const unsigned len = discoverSls(sh, obj, selectors[i]);
        // TODO: look for DLS at this point
        if (!len)
            continue;

        CL_DEBUG("abstract: found SLS of length " << len);
        if (slsBestLenght < len) {
            slsBestLenght = len;
            idxBest = i;
        }
    }

    // did we find at least something?
    if (!slsBestLenght) {
        CL_DEBUG("abstract: no list segment found");
        return;
    }

    // yes, we did ... check the threshold
    static const unsigned threshold = SLS_LEN_THRESHOLD
        + SLS_SPARE_PREFIX
        + SLS_SPARE_SUFFIX;
    if (slsBestLenght < threshold) {
        CL_DEBUG("abstract: the best SLS length (" << slsBestLenght
                << ") is under the threshold (" << threshold << ")");
        return;
    }

    // all OK
    const TFieldIdxChain &icNext = selectors[idxBest];

    // handle SLS_SPARE_PREFIX/SLS_SPARE_SUFFIX
    const int len = slsBestLenght - SLS_SPARE_PREFIX - SLS_SPARE_SUFFIX;
    for (int i = 0; i < static_cast<int>(SLS_SPARE_PREFIX); ++i)
        skipObj(sh, &obj, icNext);

    // now create the SLS as requested
    for (int i = 0; i < len; ++i)
        conjureSls(sh, &obj, icNext);
}

} // namespace

void abstractIfNeeded(SymHeap &sh) {
#if SE_DISABLE_ABSTRACT
    return;
#endif
    SymHeapCore::TContObj roots;
    sh.gatherRootObjs(roots);
    BOOST_FOREACH(const TObjId obj, roots) {
        if (sh.cVar(0, obj))
            // skip static/automatic objects
            continue;

        const TValueId addr = sh.placedAt(obj);
        if (VAL_INVALID ==addr)
            continue;

        const unsigned uses = sh.usedByCount(addr);
        switch (uses) {
            case 0:
                CL_WARN("abstractIfNeeded() encountered an unused root object #"
                        << obj);
                // fall through!

            default:
                continue;

            case 1:
                // a candidate for SLS
                abstract(sh, obj, OK_SLS);

            case 2:
                // a candidate for DLS
                abstract(sh, obj, OK_DLS);
        }
    }
}

void concretizeObj(SymHeap &sh, TObjId ao, TSymHeapList &todo) {
    const TFieldIdxChain ciBind = sh.objBinderField(ao);
    const TObjId objPtrNext = subObjByChain(sh, ao, ciBind);
    const TValueId valNext = sh.valueOf(objPtrNext);
    const TObjId objNext = sh.pointsTo(valNext);
    const TValueId addr = sh.placedAt(ao);

    // check if the LS may be empty
    bool eq;
    if (!sh.proveEq(&eq, addr, valNext)) {
        // possibly empty LS
        SymHeap sh0(sh);
        if (OBJ_INVALID == objNext) {
            // 'next' pointer does not point to a valid object
            sh0.valReplace(addr, valNext);
            sh0.objDestroy(ao);
        }
        else
            objReplace(sh0, ao, objNext);
        todo.push_back(sh0);
    }

    if (eq)
        // self loop?
        TRAP;

    // duplicate self as abstract object
    const TObjId aoDup = sh.objDup(ao);

    // concretize self
    sh.objConcretize(ao);

    // now chain it all together
    sh.objSetValue(objPtrNext, sh.placedAt(aoDup));
}
