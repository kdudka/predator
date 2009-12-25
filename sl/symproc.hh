/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of sl.
 *
 * sl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sl.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H_GUARD_SYM_PROC_H
#define H_GUARD_SYM_PROC_H

#include "location.hh"
#include "storage.hh"

namespace SymbolicHeap {
    class SymHeap;
}

class IBtPrinter;
class SymHeapUnion;

class SymHeapProcessor {
    public:
        typedef SymbolicHeap::SymHeap   THeap;
        typedef SymHeapUnion            TState;

    public:
        SymHeapProcessor(THeap &heap, IBtPrinter *btPrinter = 0):
            heap_(heap),
            btPrinter_(btPrinter)
        {
        }

        bool exec(TState &dst, const CodeStorage::Insn &insn);

    public:
        int /* val */ heapValFromCst(const struct cl_operand &op);
        int /* var */ heapVarFromOperand(const struct cl_operand &op);
        int /* val */ heapValFromOperand(const struct cl_operand &op);
        void heapSetVal(int /* obj */ lhs, int /* val */ rhs);

    private:
        void printBackTrace();
        void heapVarHandleAccessorDeref(int *pVar);
        void heapVarHandleAccessorItem(int *pVar, const struct cl_accessor *ac);
        void heapVarHandleAccessor(int *pVar, const struct cl_accessor *ac);
        bool lhsFromOperand(int *pVar, const struct cl_operand &op);
        void execUnary(const CodeStorage::Insn &insn);
        void execBinary(const CodeStorage::Insn &insn);
        void execMalloc(TState &dst, const CodeStorage::TOperandList &opList);
        void execFree(const CodeStorage::TOperandList &opList);
        bool execCall(TState &dst, const CodeStorage::Insn &insn);

    private:
        THeap                       &heap_;
        IBtPrinter                  *btPrinter_;
        LocationWriter              lw_;
};

#endif /* H_GUARD_SYM_PROC_H */
