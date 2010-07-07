/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYM_PROC_H
#define H_GUARD_SYM_PROC_H

/**
 * @file symproc.hh
 * SymProc - a layer on top of SymHeap, providing some additional
 * operations
 */

#include <cl/location.hh>
#include <cl/storage.hh>

#include "symid.hh"
#include "symheap.hh"

class SymBackTrace;
class SymHeap;
class SymHeapUnion;

/**
 * check if a sub-heap reachable from the given value is also reachable from
 * somewhere else.  If not, such a sub-heap is considered as garbage and
 * removed.  Some warnings may be issued during the garbage collecting, but no
 * backtraces are printed.  The caller is responsible for printing of backtraces
 * (if ever motivated to do so).
 * @param sh instance of the symbolic heap to search in
 * @param val ID of the heap value to check for junk
 * @param lw pass some location info in, if you want to issue warning
 * @return true if any junk has been detected/collected
 */
bool checkForJunk(SymHeap &sh, TValueId val,
                  LocationWriter lw = LocationWriter());

/**
 * a layer on top of SymHeap, providing some additional operations
 */
class SymProc {
    public:
        /// a type used to insert resulting SymHeap objects to
        typedef SymHeapUnion TState;

    public:
        /**
         * creation of the symbolic heap processor is a really cheap operation
         * @todo split SymProc into class operating only on const
         * SymHeap and a class providing the write access on top of that.  I
         * guess it will be not that easy as it sounds.
         * @param heap an instance of SymHeap to operate on
         * @param bt an instance of symbolic backtrace used to distinguish among
         * instances of automatic variables and to print backtraces when
         * necessary/suitable
         */
        SymProc(SymHeap &heap, const SymBackTrace *bt):
            heap_(heap),
            bt_(bt)
        {
        }

        ~SymProc() {
        }

        /**
         * update location info
         * @note this method is used to provide as accurate as possible location
         * info in error/warning messages
         * @param lw where to retrieve the location info from
         */
        void setLocation(const LocationWriter &lw) {
            lw_ = lw;
        }

        /**
         * execute a @b non-terminal instruction using the managed symbolic heap
         * @note see also CodeStorage::Insn
         * @note see also code_listener.h::cl_is_term_insn()
         * @param dst a container to store the result(s) to
         * @param insn an instruction to be executed
         * @param fastMode if true, omit some computationally expensive
         * techniques
         * @return true, if the requested instruction has been processed; false
         * if the instruction has to be processed elsewhere (usually
         * CL_INSN_CALL)
         * @note returning false in this case does @b not mean there has been an
         * error
         */
        bool exec(TState &dst, const CodeStorage::Insn &insn, bool fastMode);

    public:
        /// obtain a heap object corresponding to the given operand
        TObjId heapObjFromOperand(const struct cl_operand &op,
                                  bool silent = false);

        /// obtain a heap value corresponding to the given operand
        TValueId heapValFromOperand(const struct cl_operand &op,
                                    bool silent = false);

        /// resolve Fnc uid from the given opreand, -1 if there is no such Fnc
        int /* uid */ fncFromOperand(const struct cl_operand &op);

        /// high-level interface to SymHeap::objSetValue()
        void objSetValue(TObjId lhs, TValueId rhs);

        /// high-level interface to SymHeap::objDestroy()
        void objDestroy(TObjId obj);

    private:
        void heapSetSingleVal(TObjId lhs, TValueId rhs);
        void heapObjDefineType(TObjId lhs, TValueId rhs);
        void heapObjHandleAccessorDeref(TObjId *pObj, bool silent);
        void heapObjHandleAccessorItem(TObjId *pObj,
                                       const struct cl_accessor *ac);
        void heapObjHandleAccessor(TObjId *pObj, const struct cl_accessor *ac,
                                   bool silent);
        TValueId heapValFromCst(const struct cl_operand &op);
        bool lhsFromOperand(TObjId *pObj, const struct cl_operand &op);
        template <int ARITY> void execOp(const CodeStorage::Insn &insn);
        void execMalloc(TState &dst, const CodeStorage::TOperandList &opList,
                        bool fastMode);
        void execFree(const CodeStorage::TOperandList &opList);
        bool execCall(TState &dst, const CodeStorage::Insn &insn,
                      bool fastMode);
        void concretizeLoop(TState &dst, const CodeStorage::Insn &insn,
                            const struct cl_operand &src);
        bool concretizeIfNeeded(TState &dst, const CodeStorage::Insn &insn);
        bool execCore(TState &dst, const CodeStorage::Insn &insn, bool fast);

    private:
        SymHeap                     &heap_;     /// heap to operate on
        const SymBackTrace          *bt_;
        LocationWriter              lw_;

        template <int N, class T> friend struct OpHandler;
};

#endif /* H_GUARD_SYM_PROC_H */
