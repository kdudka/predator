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

#ifndef H_GUARD_SYM_HEAP_H
#define H_GUARD_SYM_HEAP_H

#include "symid.hh"

#include <vector>

struct cl_accessor;
struct cl_type;

// various types of unknown values
enum EUnknownValue {
    UV_KNOWN = 0,
    UV_UNKNOWN,
    UV_UNINITIALIZED,
    UV_DEREF_FAILED
};

// FIXME: the interface of SymHeap tends to be crowded
// TODO: break to a sensible object model
class SymHeap {
    public:
        SymHeap();
        ~SymHeap();
        SymHeap(const SymHeap &);
        SymHeap& operator=(const SymHeap &);

    public:
        typedef std::vector<int> TCont;

    public:
        // basic heap lookup
        int /* val */ valueOf(int obj) const;
        int /* val */ placedAt(int obj) const;
        int /* obj */ pointsTo(int val) const;
        void haveValue(TCont /* obj[] */ &dst, int val) const;

    public:
        // static info lookup
        const struct cl_type* /* clt */ objType(int obj) const;
        const struct cl_type* /* clt */ valType(int val) const;
        int /* CodeStorage var uid */ cVar(int var) const;
        int /* var */ varByCVar(int /* CodeStorage var */ uid) const;
        void gatherCVars(TCont &out) const;

    public:
        // complex variable (de)composition
        int /* var */ subVar(int var, int nth) const;
        int /* var */ varParent(int var) const;
        int /* obj */ valGetCompositeObj(int val) const;

    public:
        // heap objects construction
        int /* var */ varCreate(const struct cl_type *clt,
                                int /* CodeStorage var */ uid);

        int /* var */ varCreateAnon(int cbSize);
        int varSizeOfAnon(int var) const;
        bool valPointsToAnon(int val) const;
        void varDefineType(int var, const struct cl_type *clt);

        int /* sls */ slsCreate(const struct cl_type *clt,
                                const struct cl_accessor *selector);

    public:
        // heap objects alternation and destruction
        void objSetValue(int obj, int val);
        void objDestroy(int obj);

    public:
        // unkown values manipulation
        int /* val */ valCreateUnknown(EUnknownValue code,
                                       const struct cl_type *clt);
        EUnknownValue valGetUnknown(int val) const;
        int /* val */ valDuplicateUnknown(int /* val */ tpl);
        void valReplaceUnknown(int val, int /* val */ replaceBy);

    public:
        // custom values manipulation (e.g. fnc pointers)
        int /* val */ valCreateCustom(const struct cl_type *clt, int cVal);
        int /* cVal */ valGetCustom(const struct cl_type **pClt, int val) const;

    public:
        // equality reasoning
        void addEqIf(int valCond, int valA, int valB, bool neg);
        bool proveEq(bool *result, int valA, int valB) const;

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_HEAP_H */
