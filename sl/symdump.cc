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

#include "clutil.hh"
#include "symheap.hh"
#include "symutil.hh"

#include <iostream>
#include <stack>

#include <boost/foreach.hpp>

const int have_symdump = 1;

using std::cout;

namespace {
TObjId /* pointsTo */ dump_value_core(const SymHeap &heap, TValueId value);

void dump_clt_core(const struct cl_type *clt) {
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

} // namespace

bool dump_clt_visitor(TFieldIdxChain ic, const struct cl_type_item *item) {
    // indent regarding the current nest level
    const unsigned nestLevel = ic.size();
    const std::string indent(nestLevel << 2, ' ');
    cout << indent;

    // print field name if any
    const char *name = item->name;
    if (name)
        cout << "." << name << " = ";

    // print type at the current level
    const struct cl_type *clt = item->type;
    SE_BREAK_IF(!clt);
    dump_clt_core(clt);
    cout << "\n";

    return /* continue */ true;
}

void dump_clt(const struct cl_type *clt) {
    if (!clt) {
        cout << "NULL\n";
        return;
    }

    // print type at the current level
    dump_clt_core(clt);
    cout << "\n";

    // go through the type recursively
    traverseTypeIc<TFieldIdxChain>(clt, dump_clt_visitor);
}

void dump_ac(const struct cl_accessor *ac) {
    if (!ac)
        cout << "(empty)\n";

    for (int i = 0; ac; ac = ac->next, ++i) {
        cout << i << ". ";
        const struct cl_type *clt = ac->type;

        const enum cl_accessor_e code = ac->code;
        switch (code) {
            case CL_ACCESSOR_REF:
                cout << "CL_ACCESSOR_REF:";
                break;

            case CL_ACCESSOR_DEREF:
                cout << "CL_ACCESSOR_DEREF:";
                break;

            case CL_ACCESSOR_ITEM: {
                const struct cl_type_item *item = clt->items + ac->data.item.id;
                cout << "CL_ACCESSOR_ITEM: [+"
                    << item->offset << "]";
                const char *name = item->name;
                if (name)
                    cout << " ." << name;
                cout << ",";
                break;
            }

            case CL_ACCESSOR_DEREF_ARRAY:
                cout << "CL_ACCESSOR_DEREF_ARRAY: ["
                    << ac->data.array.index << "],";
                break;
        }

        cout << " clt = ";
        dump_clt(clt);
        cout << "\n";
    }
}

namespace {
    bool isObject(const SymHeap &heap, int id) {
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

void dump_ic(const struct cl_type *clt, TFieldIdxChain ic) {
    for (unsigned i = 0; i < ic.size(); ++i) {
        if (i)
            cout << ".";

        const int idx = ic[i];
        if (!clt || clt->code != CL_TYPE_STRUCT) {
            cout << "XXX";
            return;
        }

        cout << idx;
        const char *name = clt->items[idx].name;
        if (name)
            cout << "(" << name << ")";

        clt = clt->items[idx].type;
    }
}

void dump_kind(const SymHeap &heap, TObjId obj) {
    const struct cl_type *clt = heap.objType(obj);
    const EObjKind kind = heap.objKind(obj);
    switch (kind) {
        case OK_CONCRETE:
            cout << "OK_CONCRETE";
            return;

        case OK_HEAD:
            cout << "OK_HEAD";
            return;

        case OK_PART:
            cout << "OK_PART";
            return;

        case OK_SLS:
            cout << "OK_SLS, icNext = ";
            break;

        case OK_DLS:
            cout << "OK_DLS, icPeer = ";
            dump_ic(clt, heap.objBinding(obj).peer );
            cout << ", icNext = ";
    }

    dump_ic(clt, heap.objBinding(obj).next);

    const TFieldIdxChain icHead = heap.objBinding(obj).head;
    if (icHead.empty())
        return;

    cout << ", icHead = ";
    dump_ic(clt, icHead);
}

void dump_obj(const SymHeap &heap, TObjId obj) {
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

    if (!isObject(heap, obj)) {
        cout << "    error: #" << obj << " is not ID of a heap object\n";
        return;
    }

    const struct cl_type *clt = heap.objType(obj);
    if (clt) {
        cout << "    clt       = ";
        dump_clt_core(clt);
        cout << "\n";
    }

    CVar cVar;
    if (heap.cVar(&cVar, obj)) {
        cout << "    cVar      = /* CodeStorage var uid */ #" << cVar.uid
            << ", inst = " << cVar.inst
            << "\n";
    }

    cout << "    kind      = ";
    dump_kind(heap, obj);
    cout << "\n";

    const TValueId placedAt = heap.placedAt(obj);
    if (0 < placedAt)
        cout << "    placedAt  = /* value */ #" << placedAt << "\n";

    // TODO: check valPointsToAnon()
    const TValueId value = heap.valueOf(obj);
    if (VAL_NULL == value && clt && clt->code == CL_TYPE_PTR)
        cout << "    value     = VAL_NULL\n";
    else if (0 < value) {
        cout << "    value     = /* value */ #" << value;
        const struct cl_type *cltVal = heap.valType(value);
        if (cltVal) {
            cout << ", clt = ";
            dump_clt_core(cltVal);
        }
        cout << "\n";
    }

    const TObjId parent = heap.objParent(obj);
    if (-1 != parent)
        cout << "    parent    = /* obj */ #" << parent << "\n";

    if (clt && clt->code == CL_TYPE_STRUCT) {
        for (int i = 0; i < clt->item_cnt; ++i) {
            const TObjId sub = heap.subObj(obj, i);
            if (sub <= 0)
                continue;

            cout << "    subObj[" << i << "] = /* obj */ #" << sub;
            const char *name = clt->items[i].name;
            if (name)
                cout << ", \"" << name << "\"";
            const struct cl_type *subClt = clt->items[i].type;
            if (subClt) {
                cout << ", clt = ";
                dump_clt_core(subClt);
            }
            cout << ", val = " << heap.valueOf(sub) << "\n";
        }
    }

    if (-1 != parent)
        // unguarded recursion
        dump_obj(heap, parent);

    const EObjKind kind = heap.objKind(obj);
    if (OK_DLS == kind) {
        cout << "    peer      = ";
        const TFieldIdxChain icPeer = heap.objBinding(obj).peer;
        const TObjId peerPtr = subObjByChain(heap, obj, icPeer);
        const TValueId valPeer = heap.valueOf(peerPtr);
        if (0 < valPeer) {
            const TObjId peer = heap.pointsTo(valPeer);
            cout << "/* obj */ #" << peer << ", kind = ";
            dump_kind(heap, peer);
            cout << "\n";
            if (OK_HEAD == heap.objKind(peer)) {
                cout << "    peerObj   = ";
                const TObjId peerObj = objRoot(heap, peer);
                cout << "/* obj */ #" << peerObj << ", kind = ";
                dump_kind(heap, peerObj);
                cout << "\n";
            }
        }
        else
            cout << "XXX\n";
    }

    if (OK_CONCRETE != kind && OK_HEAD != kind && OK_PART != kind) {
        cout << "    shared    = ";
        if (heap.objShared(obj))
            cout << "true\n";
        else
            cout << "false\n";

        cout << "    next      = ";
        const TFieldIdxChain icNext = heap.objBinding(obj).next;
        const TObjId nextPtr = subObjByChain(heap, obj, icNext);
        const TValueId valNext = heap.valueOf(nextPtr);
        if (0 < valNext) {
            const TObjId next = heap.pointsTo(valNext);
            cout << "/* obj */ #" << next << ", kind = ";
            dump_kind(heap, next);
            cout << "\n";
        }
        else
            // FIXME: possible recursion
            dump_value_core(heap, valNext);
    }
}

namespace {
TObjId /* pointsTo */ dump_value_core(const SymHeap &heap, TValueId value)
{
    cout << "dump_value(#" << value << ")\n";

    SymHeap::TContObj refs;
    heap.usedBy(refs, value);
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

        case VAL_DEREF_FAILED:
            cout << "    VAL_DEREF_FAILED\n";
            return OBJ_INVALID;

        default:
            break;
    }

    if (!isIdValid(value))
        return OBJ_INVALID;

    if (isObject(heap, value)) {
        cout << "    error: #" << value << " is ID of a heap object\n";
        return OBJ_INVALID;
    }

    const struct cl_type *clt = heap.valType(value);
    if (clt) {
        cout << "    clt       = ";
        dump_clt_core(clt);
        cout << "\n";
    }

    const EUnknownValue code = heap.valGetUnknown(value);
    switch (code) {
        case UV_KNOWN:
            break;

        case UV_ABSTRACT:
            cout << "    code      = UV_ABSTRACT\n";
            break;

        case UV_UNKNOWN:
            cout << "    code      = UV_UNKNOWN\n";
            return OBJ_INVALID;

        case UV_UNINITIALIZED:
            cout << "    code      = UV_UNINITIALIZED\n";
            return OBJ_INVALID;
    }

    // FIXME: not tested
    const struct cl_type *cltCustom;
    const int custom = heap.valGetCustom(&cltCustom, value);
    if (-1 != custom) {
        cout << "    cVal      = /* custom value */ #" << custom;
        if (cltCustom) {
            cout << ", clt = ";
            dump_clt_core(clt);
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
    heap.usedBy(refs, value);
    BOOST_FOREACH(const TObjId obj, refs) {
        std::cout << "\n";
        dump_obj(heap, obj);
    }
}

void dump_cvar(const SymHeap &heap, CVar cVar) {
    const TObjId obj = heap.objByCVar(cVar);
    dump_obj(heap, obj);
}

void dump_heap(const SymHeap &heap) {
    cout << "dump_heap(SymHeap object at "
        << static_cast<const void *>(&heap)
        << ")\n";

    SymHeap::TContCVar cVars;
    heap.gatherCVars(cVars);
    BOOST_FOREACH(const CVar cv, cVars) {
        dump_cvar(heap, cv);
        cout << "\n";
    }
}

void dump_id(const SymHeap &heap, int id) {
    cout << "dump_id(#" << id << ")\n";
    if (id <= 0) {
        cout << "    error: given ID is not a positive number\n";
        return;
    }

    if (isObject(heap, id))
        // assume object ID
        dump_obj(heap, static_cast<TObjId>(id));

    else
        // assume value ID
        dump_value(heap, static_cast<TValueId>(id));
}
