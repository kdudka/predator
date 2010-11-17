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

#ifndef H_GUARD_SYM_CLONE_H
#define H_GUARD_SYM_CLONE_H

/**
 * @file symclone.hh
 * @todo some dox
 */

#include "config.h"
#include "symheap.hh"

#include <cl/clutil.hh>

#include <stack>

#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

template <class T>
struct DummyVisitor {
    bool operator()(const T &) const {
        return true;
    }
};

template <int N, class TObjVisitor>
bool traverseSubObjs(
        boost::array<const SymHeap *, N>    sh,
        boost::array<TObjId, N>             root,
        TObjVisitor                         &objVisitor)
{
    typedef boost::array<TObjId, N>         TObjTuple;
    std::stack<TObjTuple>                   todo;
    todo.push(root);

    // TODO: this should definitely appear in dox
    int idxValid = 0;
    while (idxValid < N && OBJ_INVALID == root[idxValid])
        ++idxValid;

    CL_BREAK_IF(N <= idxValid);

    while (!todo.empty()) {
        TObjTuple item = todo.top();
        todo.pop();

        const struct cl_type *const clt = sh[idxValid]->objType(item[idxValid]);
#ifndef NDEBUG
        for (int h = 1 + idxValid; h < N; ++h)
            CL_BREAK_IF(clt != sh[h]->objType(item[h]));
#endif
        if (!clt)
            // anonymous object of known size
            continue;

        if (!isComposite(clt))
            // we should be set up
            continue;

        // go through fields
        for (int i = 0; i < clt->item_cnt; ++i) {

            TObjTuple subItem;
            for (int h = 0; h < N; ++h)
                subItem[h] = sh[h]->subObj(item[h], i);

            // call sub-object visitor
            if (!objVisitor(subItem))
                return false;

            todo.push(subItem);
        }
    }

    // not interrupted by the visitor
    return true;
}

#endif /* H_GUARD_SYM_CLONE_H */
