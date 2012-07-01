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

#ifndef H_GUARD_CLUTIL_H
#define H_GUARD_CLUTIL_H

/**
 * @file clutil.hh
 * some generic utilities working on top of code_listener/CodeStorage
 */

#include "code_listener.h"

#include <cassert>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace CodeStorage {
    struct Storage;
}

/// compare given two pieces of static type-info semantically
bool operator==(const struct cl_type &cltA, const struct cl_type &cltB);

/// compare given two pieces of static type-info semantically
inline bool operator!=(const struct cl_type &cltA, const struct cl_type &cltB) {
    return !(cltA == cltB);
}

/// return type of the @b target object that the pointer type can point to
const struct cl_type* targetTypeOfPtr(const struct cl_type *clt);

/// return type of the @b target object that the array type can point to
const struct cl_type* targetTypeOfArray(const struct cl_type *clt);

inline bool isComposite(const struct cl_type *clt, bool includingArray = true) {
    if (!clt)
        return false;

    switch (clt->code) {
        case CL_TYPE_ARRAY:
            return includingArray;

        case CL_TYPE_STRUCT:
        case CL_TYPE_UNION:
            return true;

        default:
            return false;
    }
}

inline bool isDataPtr(const struct cl_type *clt) {
    if (!clt || clt->code != CL_TYPE_PTR)
        return false;

    clt = targetTypeOfPtr(clt);
    return (CL_TYPE_FNC != clt->code);
}

inline bool isCodePtr(const struct cl_type *clt) {
    if (!clt || clt->code != CL_TYPE_PTR)
        return false;

    clt = targetTypeOfPtr(clt);
    return (CL_TYPE_FNC == clt->code);
}

/// return true if the given operand is a local variable
inline bool isLcVar(const cl_operand &op) {
    if (CL_OPERAND_VAR != op.code)
        // not a variable
        return false;

    const enum cl_scope_e code = op.scope;
    switch (code) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
            // global variable
            return false;

        default:
            return true;
    }
}

/**
 * return true if there is any CL_ACCESSOR_REF in the given chain of accessors
 * @note CL_ACCESSOR_REF accessors can't be chained with each other, as it makes
 * semantically no sense
 */
bool seekRefAccessor(const struct cl_accessor *ac);

/// return integral value from the integral constant given as operand
int intCstFromOperand(const struct cl_operand *op);

/// return unique ID of the variable/register given as operand
int varIdFromOperand(const struct cl_operand *op, const char **pName = 0);

/// get name of an @b external function given as CL_OPERAND_CST, true on success
bool fncNameFromCst(const char **pName, const struct cl_operand *op);

/// get uid of a function from the given operand if available, true on success
bool fncUidFromOperand(int *pUid, const struct cl_operand *op);

typedef std::vector<int /* nth */> TFieldIdxChain;

int offsetByIdxChain(const struct cl_type *, const TFieldIdxChain &);

// TODO: do not define the helper type at gl scope
struct CltStackItem {
    const struct cl_type    *clt;
    TFieldIdxChain          ic;
};

// take the given visitor through a composite type (or whatever you pass in)
template <class TVisitor>
bool /* complete */ traverseTypeIc(const struct cl_type *clt, TVisitor &visitor,
                                   bool digOnlyComposite = false)
{
    assert(clt);

    // we use std::set to avoid an infinite loop
    std::set<int /* uid */> done;
    done.insert(clt->uid);

    // initialize DFS
    typedef CltStackItem TItem;
    TItem si;
    si.clt = clt;
    si.ic.push_back(0);

    // DFS loop
    std::stack<TItem> todo;
    todo.push(si);
    while (!todo.empty()) {
        TItem &si = todo.top();
        assert(!si.ic.empty());

        typename TFieldIdxChain::reference nth = si.ic.back();
        if (nth == si.clt->item_cnt) {
            // done at this level
            if (isComposite(si.clt))
                done.erase(si.clt->uid);

            todo.pop();
            continue;
        }

        if (digOnlyComposite && !isComposite(si.clt)) {
            // caller is interested only in composite types, skip this
            ++nth;
            continue;
        }

        const struct cl_type_item *item = si.clt->items + nth;
        const TFieldIdxChain &icConst = si.ic;
        if (!/* continue */visitor(icConst, item))
            return false;

        if (!item->type->item_cnt) {
            // non-composite type item
            ++nth;
            continue;
        }

        const int uid = item->type->uid;
        if (done.end() == done.find(uid)) {
            done.insert(uid);

            // nest into sub-type
            TItem next;
            next.clt = item->type;
            next.ic = si.ic;
            next.ic.push_back(0);
            todo.push(next);
        }

        // move to the next at this level
        ++nth;
    }

    // the traversal is done, without any interruption by visitor
    return true;
}

std::string varToString(
        const CodeStorage::Storage      &stor,
        const int                       uid,
        const struct cl_loc             **pLoc = 0);

#endif /* H_GUARD_CLUTIL_H */
