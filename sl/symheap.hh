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
        typedef std::vector<int>        TCont;
        typedef std::vector<TObjId>     TContObj;
        typedef std::vector<TValueId>   TContValue;

    public:
        // basic heap lookup
        TValueId valueOf(TObjId obj) const;
        TValueId placedAt(TObjId obj) const;
        TObjId pointsTo(TValueId val) const;
        void haveValue(TContObj &dst, TValueId val) const;

    public:
        // static info lookup
        const struct cl_type* /* clt */ objType(TObjId obj) const;
        const struct cl_type* /* clt */ valType(TValueId val) const;
        int /* CodeStorage var uid */ cVar(TObjId var) const;
        TObjId /* var */ varByCVar(int /* CodeStorage var */ uid) const;
        void gatherCVars(TCont &out) const;

    public:
        // complex variable (de)composition
        TObjId /* var */ subVar(TObjId var, int nth) const;
        TObjId /* var */ varParent(TObjId var) const;
        TObjId /* obj */ valGetCompositeObj(TValueId val) const;

    public:
        // heap objects construction
        TObjId /* var */ varCreate(const struct cl_type *clt,
                                   int /* CodeStorage var */ uid);

        TObjId /* var */ varCreateAnon(int cbSize);
        int varSizeOfAnon(TObjId var) const;
        bool valPointsToAnon(TValueId val) const;
        void varDefineType(TObjId, const struct cl_type *clt);

    public:
        // heap objects alternation and destruction
        void objSetValue(TObjId obj, TValueId val);
        void objDestroy(TObjId obj);

    public:
        // unkown values manipulation
        TValueId valCreateUnknown(EUnknownValue code,
                                  const struct cl_type *clt);
        EUnknownValue valGetUnknown(TValueId val) const;
        TValueId valDuplicateUnknown(TValueId tpl);
        void valReplaceUnknown(TValueId val, TValueId replaceBy);

    public:
        // custom values manipulation (e.g. fnc pointers)
        TValueId valCreateCustom(const struct cl_type *clt, int cVal);
        int /* cVal */ valGetCustom(const struct cl_type **pClt,
                                    TValueId val) const;

    public:
        // equality reasoning
        void addEqIf(TValueId valCond, TValueId valA, TValueId valB, bool neg);
        bool proveEq(bool *result, TValueId valA, TValueId valB) const;

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_HEAP_H */
