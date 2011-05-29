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

#include <iostream>

using std::cout;

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

    // attempt to plot heap
    if (!plotHeap(*sh, name))
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
