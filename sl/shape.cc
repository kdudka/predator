/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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

#include "shape.hh"

#include "symutil.hh"

template <class TVisitor>
bool traverseShape(const SymHeap &sh, const Shape &shape, TVisitor &visitor)
{
    const ShapeProps &props = shape.props;
    const EObjKind kind = props.kind;
    switch (kind) {
        case OK_SLS:
        case OK_DLS:
            break;

        default:
            CL_BREAK_IF("invalid call of objSetByShape()");
    }

    const TOffset offNext = props.bOff.next;
    SymHeap &shWritable = const_cast<SymHeap &>(sh);
    TObjId obj = shape.entry;

    for (unsigned i = 0; i < shape.length; ++i) {
        if (!/* continue */ visitor(obj))
            return false;

        obj = nextObj(shWritable, obj, offNext);
    }

    return /* done */ true;
}

class ObjListCollector {
    public:
        ObjListCollector(TObjList *pDst):
            pDst_(pDst)
        {
        }

        bool operator()(const TObjId obj) {
            pDst_->push_back(obj);
            return /* continue */ true;
        }

    private:
        TObjList       *pDst_;
};

void objListByShape(TObjList *pDst, const SymHeap &sh, const Shape &shape)
{
    ObjListCollector visitor(pDst);
    traverseShape(sh, shape, visitor);
}

class ObjSetCollector {
    public:
        ObjSetCollector(TObjSet *pDst):
            pDst_(pDst)
        {
        }

        bool operator()(const TObjId obj) {
            CL_BREAK_IF(hasKey(pDst_, obj));
            pDst_->insert(obj);
            return /* continue */ true;
        }

    private:
        TObjSet        *pDst_;
};

void objSetByShape(TObjSet *pDst, const SymHeap &sh, const Shape &shape)
{
    ObjSetCollector visitor(pDst);
    traverseShape(sh, shape, visitor);
}

struct LastObjCollector {
    TObjId lastObj;

    LastObjCollector():
        lastObj(OBJ_INVALID)
    {
    }

    bool operator()(const TObjId obj) {
        lastObj = obj;
        return /* continue */ true;
    }
};

TObjId lastObjOfShape(const SymHeap &sh, const Shape &shape)
{
    LastObjCollector visitor;
    traverseShape(sh, shape, visitor);
    return visitor.lastObj;
}
