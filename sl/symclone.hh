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

#include <boost/tuple/tuple.hpp>

template <class T>
struct DummyVisitor {
    void operator()(const T &) { }
};

template <class TValVisitor, class TObjVisitor>
void digSubObjs(
        const SymHeap           &src,
        SymHeap                 &dst,
        const TObjPair          root,
        TValVisitor             &valVisitor = DummyVisitor<TValPair>(),
        TObjVisitor             &objVisitor = DummyVisitor<TObjPair>())
{
    std::stack<TObjPair> todo;
    todo.push(root);

    while (!todo.empty()) {
        TObjId objSrc, objDst;
        boost::tie(objSrc, objDst) = todo.top();
        todo.pop();

        const struct cl_type *cltSrc = src.objType(objSrc);
        SE_BREAK_IF(dst.objType(objDst) != cltSrc);
        if (!cltSrc)
            // anonymous object of known size
            continue;

        if (!isComposite(cltSrc))
            // we should be set up
            continue;

        // store mapping of composite value
        const TValPair values(src.valueOf(objSrc), src.valueOf(objDst));
        valVisitor(values);

        // go through fields
        for (int i = 0; i < cltSrc->item_cnt; ++i) {
            const TObjId subSrc = src.subObj(objSrc, i);
            const TObjId subDst = dst.subObj(objDst, i);
            SE_BREAK_IF(subSrc < 0 || subDst < 0);

            const TObjPair objs(subSrc, subDst);
            objVisitor(objs);
            todo.push(objs);
        }
    }
}

#endif /* H_GUARD_SYM_CLONE_H */
