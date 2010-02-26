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
#include "symid.hh"

class IBtPrinter;
class SymHeap;
class SymHeapUnion;

class SymHeapProcessor {
    public:
        typedef SymHeapUnion TState;

    public:
        SymHeapProcessor(SymHeap &heap, IBtPrinter *btPrinter = 0):
            heap_(heap),
            btPrinter_(btPrinter)
        {
        }

        bool exec(SymHeapUnion &dst, const CodeStorage::Insn &insn,
                  bool fastMode);

        void setLocation(const LocationWriter &lw) {
            lw_ = lw;
        }

    public:
        TValueId heapValFromCst(const struct cl_operand &op);
        TObjId heapVarFromOperand(const struct cl_operand &op);
        TValueId heapValFromOperand(const struct cl_operand &op);
        bool checkForJunk(TValueId val);
        void heapSetVal(TObjId lhs, TValueId rhs);
        void destroyObj(TObjId obj);

    private:
        void printBackTrace();
        void heapSetSingleVal(TObjId lhs, TValueId rhs);
        void heapVarDefineType(TObjId lhs, TValueId rhs);
        void heapVarHandleAccessorDeref(TObjId *pVar);
        void heapVarHandleAccessorItem(TObjId *pVar,
                                       const struct cl_accessor *ac);
        void heapVarHandleAccessor(TObjId *pVar, const struct cl_accessor *ac);
        bool lhsFromOperand(TObjId *pVar, const struct cl_operand &op);
        template <int ARITY> void execOp(const CodeStorage::Insn &insn);
        void execMalloc(TState &dst, const CodeStorage::TOperandList &opList,
                        bool fastMode);
        void execFree(const CodeStorage::TOperandList &opList);
        bool execCall(TState &dst, const CodeStorage::Insn &insn,
                      bool fastMode);

    private:
        SymHeap                     &heap_;
        IBtPrinter                  *btPrinter_;
        LocationWriter              lw_;
};

#endif /* H_GUARD_SYM_PROC_H */
