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
#include "symdump.hh"

#include <cl/code_listener.h>

#include "symheap.hh"

#include <iostream>

#include <boost/foreach.hpp>

const int have_symdump = 1;

void dump_clt(const struct cl_type *clt) {
    using std::cout;

    cout << "*((const struct cl_type *)"
        << static_cast<const void *>(clt)
        << ")";
    if (!clt)
        return;

    cout << " (#" << clt->uid << ", code = ";
    switch (clt->code) {
        case CL_TYPE_UNKNOWN:   cout << "CL_TYPE_UNKNOWN"   ; break;
        case CL_TYPE_VOID:      cout << "CL_TYPE_VOID"      ; break;
        case CL_TYPE_FNC:       cout << "CL_TYPE_FNC"       ; break;
        case CL_TYPE_PTR:       cout << "CL_TYPE_PTR"       ; break;
        case CL_TYPE_ARRAY:     cout << "CL_TYPE_ARRAY"     ; break;
        case CL_TYPE_STRUCT:    cout << "CL_TYPE_STRUCT"    ; break;
        case CL_TYPE_UNION:     cout << "CL_TYPE_UNION"     ; break;
        case CL_TYPE_ENUM:      cout << "CL_TYPE_ENUM"      ; break;
        case CL_TYPE_INT:       cout << "CL_TYPE_INT"       ; break;
        case CL_TYPE_BOOL:      cout << "CL_TYPE_BOOL"      ; break;
        case CL_TYPE_CHAR:      cout << "CL_TYPE_CHAR"      ; break;
        case CL_TYPE_STRING:    cout << "CL_TYPE_STRING"    ; break;
    }
    cout << ")";
}

namespace {
    bool isHeapObject(const SymHeap &heap, int id) {
        const TObjId obj = static_cast<TObjId>(id);
        return (VAL_INVALID != heap.placedAt(obj));
    }

    bool isIdValid(int id) {
        if (0 <= id)
            return true;

        std::cout << "    error: invalid ID\n";
        return false;
    }
}

void dump_obj(const SymHeap &heap, TObjId obj) {
    using std::cout;
    cout << "dump_obj(#" << obj << ")\n";

    switch (obj) {
        case OBJ_INVALID:       cout << "    OBJ_INVALID\n";       return;
        case OBJ_DELETED:       cout << "    OBJ_DELETED\n";       return;
        case OBJ_UNKNOWN:       cout << "    OBJ_UNKNOWN\n";       return;
        case OBJ_DEREF_FAILED:  cout << "    OBJ_DEREF_FAILED\n";  return;
        case OBJ_LOST:          cout << "    OBJ_LOST\n";          return;

        case OBJ_RETURN:        cout << "    OBJ_RETURN\n";
        default:
            break;
    }

    if (!isIdValid(obj))
        return;

    if (!isHeapObject(heap, obj)) {
        cout << "    error: #" << obj << " is not ID of a heap object\n";
        return;
    }

    const struct cl_type *clt = heap.objType(obj);
    if (clt) {
        cout << "    clt       = ";
        dump_clt(clt);
        cout << "\n";
    }

    const int cVar = heap.cVar(obj);
    if (-1 != cVar)
        cout << "    cVar      = /* CodeStorage var uid */ #" << cVar << "\n";

    const TValueId placedAt = heap.placedAt(obj);
    if (0 < placedAt)
        cout << "    placedAt  = /* value */ #" << placedAt << "\n";

    // TODO: check valPointsToAnon()
    const TValueId value = heap.valueOf(obj);
    if (0 < value)
        cout << "    value     = /* value */ #" << value << "\n";

    const TObjId parent = heap.objParent(obj);
    if (-1 != parent)
        cout << "    parent    = /* obj */ #" << parent << "\n";

    if (clt && clt->code == CL_TYPE_STRUCT) {
        for (int i = 0; i < clt->item_cnt; ++i) {
            const int sub = heap.subObj(obj, i);
            if (sub <= 0)
                continue;

            cout << "    subObj[" << i << "] = /* obj */ #" << sub << "\n";
        }
    }

    if (-1 != parent)
        // unguarded recursion
        dump_obj(heap, parent);
}

namespace {
TObjId /* pointsTo */ dump_value_core(const SymHeap &heap, TValueId value)
{
    using std::cout;
    cout << "dump_value(#" << value << ")\n";

    SymHeap::TContObj refs;
    heap.haveValue(refs, value);
    BOOST_FOREACH(const int obj, refs) {
        cout << "    ref found : /* obj */ #" << obj << "\n";
    }

    switch (value) {
        case VAL_INVALID:
            cout << "    VAL_INVALID\n";
            return OBJ_INVALID;

        case VAL_NULL:
            cout << "    VAL_NULL == VAL_FALSE\n";
            return OBJ_INVALID;

        case VAL_TRUE:
            cout << "    VAL_TRUE\n";
            return OBJ_INVALID;

        default:
            break;
    }

    if (!isIdValid(value))
        return OBJ_INVALID;

    if (isHeapObject(heap, value)) {
        cout << "    error: #" << value << " is ID of a heap object\n";
        return OBJ_INVALID;
    }

    const struct cl_type *clt = heap.valType(value);
    if (clt) {
        cout << "    clt       = ";
        dump_clt(clt);
        cout << "\n";
    }

    const EUnknownValue code = heap.valGetUnknown(value);
    switch (code) {
        case UV_KNOWN:
            break;

        case UV_UNKNOWN:
            cout << "    code      = UV_UNKNOWN\n";
            return OBJ_INVALID;

        case UV_UNINITIALIZED:
            cout << "    code      = UV_UNINITIALIZED\n";
            return OBJ_INVALID;

        case UV_DEREF_FAILED:
            cout << "    code      = UV_DEREF_FAILED\n";
            return OBJ_INVALID;
    }

    // FIXME: not tested
    const struct cl_type *cltCustom;
    const int custom = heap.valGetCustom(&cltCustom, value);
    if (-1 != custom) {
        cout << "    cVal      = /* custom value */ #" << custom;
        if (cltCustom) {
            cout << ", clt = ";
            dump_clt(clt);
        }
        cout << "\n";
        return OBJ_INVALID;
    }

    const TObjId compObj = heap.valGetCompositeObj(value);
    if (OBJ_INVALID != compObj) {
        cout << "    compObj   = /* obj */ #" << compObj << "\n";
        return OBJ_INVALID;
    }

    const TObjId pointsTo = heap.pointsTo(value);
    if (0 < pointsTo)
        cout << "    pointsTo  = /* obj */ #" << pointsTo << "\n";

    return pointsTo;
}
} // namespace

void dump_value(const SymHeap &heap, TValueId value) {
    const TObjId pointsTo = dump_value_core(heap, value);
    if (0 < pointsTo) {
        std::cout << "\n";
        dump_obj(heap, pointsTo);
    }
}

void dump_value_refs(const SymHeap &heap, TValueId value) {
    // dump value itself
    dump_value_core(heap, value);

    // dump all referrer objects
    SymHeap::TContObj refs;
    heap.haveValue(refs, value);
    BOOST_FOREACH(const TObjId obj, refs) {
        std::cout << "\n";
        dump_obj(heap, obj);
    }
}

void dump_cvar(const SymHeap &heap, int cVar) {
    const TObjId obj = heap.objByCVar(cVar);
    dump_obj(heap, obj);
}

void dump_heap(const SymHeap &heap) {
    using std::cout;

    cout << "dump_heap(SymHeap object at "
        << static_cast<const void *>(&heap)
        << ")\n";

    SymHeap::TCont cVars;
    heap.gatherCVars(cVars);
    BOOST_FOREACH(const int cv, cVars) {
        dump_cvar(heap, cv);
        cout << "\n";
    }
}

void dump_id(const SymHeap &heap, int id) {
    using std::cout;
    cout << "dump_id(#" << id << ")\n";
    if (id <= 0) {
        cout << "    error: given ID is not a positive number\n";
        return;
    }

    if (isHeapObject(heap, id))
        // assume object ID
        dump_obj(heap, static_cast<TObjId>(id));

    else
        // assume value ID
        dump_value(heap, static_cast<TValueId>(id));
}
