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

#ifndef H_GUARD_SYMUTIL_H
#define H_GUARD_SYMUTIL_H

/**
 * @file symutil.hh
 * some generic utilities working on top of a symbolic heap
 * @todo API documentation
 */

#include "config.h"

#include <cl/code_listener.h>
#include <cl/clutil.hh>

#include "symheap.hh"

#include <stack>

namespace CodeStorage {
    struct Var;
}

TObjId subObjByChain(const SymHeap &sh, TObjId obj, TFieldIdxChain ic);
TObjId subObjByInvChain(const SymHeap &sh, TObjId obj, TFieldIdxChain ic);

bool isHeapObject(const SymHeap &heap, TObjId obj);

void digRootObject(const SymHeap &heap, TValueId *pValue);

inline TObjId /* root */ objRoot(const SymHeapTyped &sh, TObjId obj) {
    TObjId root = obj;
    while (OBJ_INVALID != (obj = sh.objParent(root)))
        root = obj;

    return root;
}

inline TObjId /* root */ objRootByVal(const SymHeapTyped &sh, TValueId val) {
    const TObjId target = sh.pointsTo(val);
    return objRoot(sh, target);
}

inline TObjId /* root */ objRootByPtr(const SymHeapTyped &sh, TObjId ptr) {
    const TValueId val = sh.valueOf(ptr);
    return objRootByVal(sh, val);
}

/// return offset of an object within another object;  -1 if not found
inline int subOffsetIn(const SymHeapTyped &sh, TObjId in, TObjId of) {
    int offset = 0;
    TObjId parent;

    int nth;
    while (OBJ_INVALID != (parent = sh.objParent(of, &nth))) {
        const struct cl_type *clt = sh.objType(parent);
        SE_BREAK_IF(!clt || clt->item_cnt <= nth);

        offset += clt->items[nth].offset;
        if (parent == in)
            return offset;

        of = parent;
    }

    return /* not found */ -1;
}

void getPtrValues(SymHeapCore::TContValue &dst, const SymHeap &heap,
                  TObjId obj);

void objReplace(SymHeap &sh, TObjId oldObj, TObjId newObj);

void skipObj(const SymHeap &sh, TObjId *pObj, TFieldIdxChain icHead,
             TFieldIdxChain icNext);

void initVariable(SymHeap                       &sh,
                  TObjId                        obj,
                  const CodeStorage::Var        &var);

typedef std::pair<TObjId, TObjId> TObjPair;

#ifndef BUILDING_DOX
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
        const struct cl_type *clt = sh.objType(item.first);
        SE_BREAK_IF(clt != sh.objType(item.second));
        return clt;
    }
    static TObjPair getNextItem(const SymHeap &sh, TObjPair item, int nth) {
        item.first  = sh.subObj(item.first,  nth);
        item.second = sh.subObj(item.second, nth);
        return item;
    }
};
#endif

/// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, class TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjs(THeap &sh, TItem item, TVisitor &visitor,
                                    bool leavesOnly)
{
    std::stack<TItem> todo;
    todo.push(item);
    while (!todo.empty()) {
        item = todo.top();
        todo.pop();

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, item);
        SE_BREAK_IF(!clt || !isComposite(clt));

        for (int i = 0; i < clt->item_cnt; ++i) {
            const TItem next = THelper::getNextItem(sh, item, i);

            const struct cl_type *subClt = THelper::getItemClt(sh, next);
            if (subClt && isComposite(subClt)) {
                todo.push(next);

                if (leavesOnly)
                    // do not call the visitor for internal nodes, if requested
                    continue;
            }

            if (!/* continue */visitor(sh, next))
                return false;
        }
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

#ifndef BUILDING_DOX
template <class TItem>
struct SubTraversalStackItem {
    TItem               item;
    TFieldIdxChain      ic;
};
#endif

/// take the given visitor through a composite object (or whatever you pass in)
template <class THeap, class TVisitor, class TItem = TObjId>
bool /* complete */ traverseSubObjsIc(THeap &sh, TItem item, TVisitor &visitor)
{
    typedef SubTraversalStackItem<TItem> TStackItem;
    TStackItem si;
    si.item = item;
    si.ic.push_back(0);

    std::stack<TStackItem> todo;
    todo.push(si);
    while (!todo.empty()) {
        TStackItem &si = todo.top();
        SE_BREAK_IF(si.ic.empty());

        typedef TraverseSubObjsHelper<TItem> THelper;
        const struct cl_type *clt = THelper::getItemClt(sh, si.item);
        SE_BREAK_IF(!clt || !isComposite(clt));

        typename TFieldIdxChain::reference nth = si.ic.back();
        if (nth == clt->item_cnt) {
            // done at this level
            todo.pop();
            continue;
        }

        TStackItem next = si;
        next.item = THelper::getNextItem(sh, si.item, nth);
        if (!/* continue */visitor(sh, next.item, si.ic))
            return false;

        const struct cl_type *cltNext = THelper::getItemClt(sh, next.item);
        if (!cltNext || !isComposite(cltNext)) {
            // move to the next field at this level
            ++nth;
            continue;
        }

        // nest into a sub-object
        next.ic.push_back(0);
        todo.push(next);
        ++nth;
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

void gatherPointingObjects(const SymHeap            &sh,
                           SymHeap::TContObj        &dst,
                           const TObjId             root,
                           bool                     toInsideOnly);

#endif /* H_GUARD_SYMUTIL_H */
