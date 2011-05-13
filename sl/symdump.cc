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
#include <cl/cldebug.hh>
#include <cl/clutil.hh>

#include "symheap.hh"
#include "symplot.hh"
#include "symseg.hh"
#include "symutil.hh"

#include <iostream>
#include <stack>

#include <boost/foreach.hpp>

using std::cout;

namespace {
    TObjId /* pointsTo */ dump_value_core(const SymHeap &heap, TValId value);
}

void dump_clt(const struct cl_type *clt) {
    cltToStream(cout, clt, /* depth */ 3U);
}

void dump_clt(const struct cl_type *clt, unsigned depth) {
    cltToStream(cout, clt, depth);
}

void dump_ac(const struct cl_accessor *ac) {
    acToStream(cout, ac, /* oneline */ false);
}

void dump_ac(const struct cl_accessor &ac) {
    dump_ac(&ac);
}

void dump_op(const struct cl_operand &op) {
    cout << op << "\n";
}

void dump_op(const struct cl_operand *op) {
    dump_op(*op);
}

// NOTE: symbol cl_insn clashes with a gdb internal debugging function
void dump_cl_insn(const struct CodeStorage::Insn &insn) {
    cout << insn << "\n";
}

// NOTE: symbol cl_insn clashes with a gdb internal debugging function
void dump_cl_insn(const struct CodeStorage::Insn *insn) {
    dump_cl_insn(*insn);
}

namespace {
    bool isObject(const SymHeap &heap, int id) {
        const TObjId obj = static_cast<TObjId>(id);
        return (OBJ_RETURN == obj)
            || (VAL_INVALID != heap.placedAt(obj));
    }

    bool isIdValid(int id) {
        if (0 <= id)
            return true;

        std::cout << "    error: invalid ID\n";
        return false;
    }
}

void dump_kind(const SymHeap &sh, TObjId obj) {
    const TValId at = sh.placedAt(obj);
    const EObjKind kind = sh.valTargetKind(at);
    switch (kind) {
        case OK_CONCRETE:
            cout << "OK_CONCRETE";
            return;

        case OK_MAY_EXIST:
            cout << "OK_MAY_EXIST";
            return;

        case OK_SLS:
            cout << "OK_SLS, offNext = " << sh.segBinding(at).next;
            break;

        case OK_DLS:
            cout << "OK_DLS, offPeer = " << sh.segBinding(at).prev
                << ", offNext = " << sh.segBinding(at).next;
    }

    const TOffset offHead = sh.segBinding(at).head;
    if (offHead)
        cout << ", icHead = " << offHead;
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
    if (clt)
        cout << "    clt       = " << (*clt) << "\n";

    const TValId objAt = heap.placedAt(obj);
    if (SymHeap::isProgramVar(heap.valTarget(objAt))) {
        CVar cVar = heap.cVarByRoot(heap.valRoot(objAt));
        cout << "    cVar      = /* CodeStorage var uid */ #" << cVar.uid
            << ", inst = " << cVar.inst
            << "\n";
    }

    cout << "    kind      = ";
    dump_kind(heap, obj);
    cout << "\n";

    if (0 < objAt)
        cout << "    placedAt  = /* value */ #" << objAt << "\n";

    const TValId value = heap.valueOf(obj);
    if (VAL_NULL == value && clt && clt->code == CL_TYPE_PTR)
        cout << "    value     = VAL_NULL\n";
    else if (0 < value)
        cout << "    value     = /* value */ #" << value << "\n";

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
            if (subClt)
                cout << ", clt = " << (*clt);

            cout << ", val = " << heap.valueOf(sub) << "\n";
        }
    }

    const EObjKind kind = heap.valTargetKind(objAt);
    if (OK_DLS == kind) {
        cout << "    peer      = ";
        const TValId at = heap.placedAt(obj);
        const TOffset offPeer = heap.segBinding(at).prev;
        SymHeap &writable = const_cast<SymHeap &>(heap);
        const TValId valPeer = valOfPtrAt(writable, at, offPeer);
        if (0 < valPeer) {
            const TObjId peer = const_cast<SymHeap &>(heap).objAt(valPeer);
            cout << "/* obj */ #" << peer << ", kind = ";
            dump_kind(heap, peer);
            cout << "\n";
            if (heap.valOffset(heap.placedAt(peer))) {
                cout << "    peerObj   = ";
                cout << "\n";
            }
        }
        else
            cout << "XXX\n";
    }

    cout << "    isProto   = ";
    if (heap.valTargetIsProto(heap.placedAt(obj)))
        cout << "true\n";
    else
        cout << "false\n";

    if (OK_CONCRETE != kind) {
        cout << "    next      = ";
        const TValId at = heap.placedAt(obj);
        const TOffset offNext = heap.segBinding(at).next;
        SymHeap &writable = const_cast<SymHeap &>(heap);
        const TValId valNext = valOfPtrAt(writable, at, offNext);
        if (0 < valNext) {
            const TObjId next = const_cast<SymHeap &>(heap).objAt(valNext);
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
TObjId /* pointsTo */ dump_value_core(const SymHeap &heap, TValId value)
{
    cout << "dump_value(#" << value << ")\n";

    TObjList refs;
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

    const TOffset off = heap.valOffset(value);
    cout << "    offset    = " << off << "\n";
    if (off)
        cout << "    valRoot   = " << heap.valRoot(value) << "\n";

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

        case UV_DONT_CARE:
            cout << "    code      = UV_DONT_CARE\n";
            return OBJ_INVALID;

        case UV_UNINITIALIZED:
            cout << "    code      = UV_UNINITIALIZED\n";
            return OBJ_INVALID;
    }

    // FIXME: not tested
    const int custom = heap.valGetCustom(value);
    if (-1 != custom) {
        cout << "    cVal      = /* custom value */ #" << custom << "\n";
        return OBJ_INVALID;
    }

    const TObjId compObj = heap.valGetCompositeObj(value);
    if (OBJ_INVALID != compObj) {
        cout << "    compObj   = /* obj */ #" << compObj << "\n";
        return OBJ_INVALID;
    }

    const TObjId pointsTo = const_cast<SymHeap &>(heap).objAt(value);
    if (0 < pointsTo)
        cout << "    pointsTo  = /* obj */ #" << pointsTo << "\n";

    return pointsTo;
}
} // namespace

void dump_value(const SymHeap &heap, TValId value) {
    const TObjId pointsTo = dump_value_core(heap, value);
    if (0 < pointsTo) {
        std::cout << "\n";
        dump_obj(heap, pointsTo);
    }
}

void dump_value_refs(const SymHeap &heap, TValId value) {
    // dump value itself
    dump_value_core(heap, value);

    // dump all referrer objects
    TObjList refs;
    heap.usedBy(refs, value);
    BOOST_FOREACH(const TObjId obj, refs) {
        std::cout << "\n";
        dump_obj(heap, obj);
    }
}

void dump_cvar(SymHeap &heap, CVar cVar) {
    const TValId val = heap.addrOfVar(cVar);
    dump_value(heap, val);
}

void dump_cvar(SymHeap &heap, int uid, int inst) {
    const CVar cv(uid, inst);
    dump_cvar(heap, cv);
}

void dump_heap(const SymHeap &heap) {
    cout << "dump_heap(SymHeap object at "
        << static_cast<const void *>(&heap)
        << ")\n";

    TCVarList cVars;
    heap.gatherCVars(cVars);
    BOOST_FOREACH(const CVar cv, cVars) {
        dump_cvar(const_cast<SymHeap &>(heap), cv);
        cout << "\n";
    }
}

void dump_id(const SymHeapCore *core, int id) {
    if (!core) {
        cout << "dump_plot: error: got a NULL pointer\n";
        return;
    }

    const SymHeap *sh = dynamic_cast<const SymHeap *>(core);
    if (!sh) {
        cout << "dump_plot: error: failed to downcast SymHeapCore to SymHeap\n";
        return;
    }

    cout << "dump_id(#" << id << ")\n";
    if (id <= 0) {
        cout << "    error: given ID is not a positive number\n";
        return;
    }

    const SymHeap &heap = *sh;
    if (isObject(heap, id))
        // assume object ID
        dump_obj(heap, static_cast<TObjId>(id));

    else
        // assume value ID
        dump_value(heap, static_cast<TValId>(id));
}

void dump_id(const SymHeapCore &core, int id) {
    dump_id(&core, id);
}

void dump_plot_core(const SymHeapCore *core, const char *name) {
    if (!core) {
        cout << "dump_plot: error: got a NULL pointer\n";
        return;
    }

    const SymHeap *sh = dynamic_cast<const SymHeap *>(core);
    if (!sh) {
        cout << "dump_plot: error: failed to downcast SymHeapCore to SymHeap\n";
        return;
    }

    // create an instance of SymPlot
    SymPlot plotter(*sh);

    // attempt to plot heap
    if (!plotter.plot(name))
        cout << "dump_plot: warning: call of SymPlot::plot() has failed\n";
}

void dump_plot(const SymHeapCore *sh) {
    dump_plot_core(sh, "symdump");
}

void dump_plot(const SymHeapCore &sh, const char *name) {
    dump_plot_core(&sh, name);
}

void dump_plot(const SymHeapCore &sh) {
    dump_plot(&sh);
}
