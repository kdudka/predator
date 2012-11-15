/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SYMUTIL_H
#define H_GUARD_SYMUTIL_H

/**
 * @file symutil.hh
 * some generic utilities working on top of a symbolic heap
 * @todo API documentation
 */

#include "config.h"

#include <cl/code_listener.h>
#include <cl/clutil.hh>

#include "symheap.hh"
#include "util.hh"

#include <set>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

inline TValId boolToVal(const bool b)
{
    return (b)
        ? VAL_TRUE
        : VAL_FALSE;
}

/// extract integral constant from the given value if possible, fail otherwise
bool numFromVal(IR::TInt *pDst, const SymHeapCore &, const TValId);

/// extract integral range from the given value if possible, fail otherwise
bool rngFromVal(IR::Range *pDst, const SymHeapCore &, const TValId);

/// extract either offset range, or integral range from the given value
bool anyRangeFromVal(IR::Range *pDst, const SymHeap &, const TValId);

/// extract string literal from the given value if possible, fail otherwise
bool stringFromVal(const char **pDst, const SymHeap &, const TValId);

void moveKnownValueToLeft(const SymHeapCore &sh, TValId &valA, TValId &valB);

bool valInsideSafeRange(const SymHeapCore &sh, TValId val);

bool canWriteDataPtrAt(const SymHeapCore &sh, TValId val);

/// true if the given values are proven to be non-equal in non-abstract world
bool proveNeq(const SymHeapCore &sh, TValId v1, TValId v2);

/// extract an integral range from an unwrapped CV_INT/CV_INT_RANGE custom value
const IR::Range& rngFromCustom(const CustomValue &);

/// return size (in bytes) that we can safely write at the given addr
TSizeRange valSizeOfTarget(const SymHeapCore &, const TValId at);

bool compareIntRanges(
        bool                                *pDst,
        const enum cl_binop_e               code,
        const IR::Range                     &range1,
        const IR::Range                     &range2);

/// known to work only with TFldId/TValId
template <class TMap>
typename TMap::mapped_type roMapLookup(
        const TMap                          &roMap,
        const typename TMap::key_type       id)
{
    if (id <= 0)
        return id;

    typename TMap::const_iterator iter = roMap.find(id);
    return (roMap.end() == iter)
        ? static_cast<typename TMap::mapped_type>(-1)
        : iter->second;
}

bool translateValId(
        TValId                  *pVal,
        SymHeapCore             &dst,
        const SymHeapCore       &src,
        const TValMap           &valMap);

TValId translateValProto(
        SymHeap                 &dst,
        const SymHeap           &src,
        const TValId             valProto);

inline FldHandle translateObjId(
        SymHeap                 &dst,
        SymHeap                 &src,
        const TValId            dstRootAt,
        const FldHandle         &srcField)
{
    CL_BREAK_IF(&src != srcField.sh());
    (void) src;

    // gather properties of the field in 'src'
    const TOffset  off = srcField.offset();
    const TObjType clt = srcField.type();

    // use them to obtain the corresponding object in 'dst'
    const TValId dstAt = dst.valByOffset(dstRootAt, off);
    return FldHandle(dst, dstAt, clt);
}

/// TODO: drop this!
inline TValId valOfPtrAt(SymHeap &sh, TValId at)
{
    CL_BREAK_IF(!canWriteDataPtrAt(sh, at));

    const PtrHandle ptr(sh, at);
    return ptr.value();
}

/// TODO: drop this!
inline TValId valOfPtrAt(SymHeap &sh, TValId at, TOffset off)
{
    const TValId ptrAt = sh.valByOffset(at, off);
    return valOfPtrAt(sh, ptrAt);
}

inline TValId valOfPtr(SymHeap &sh, TObjId obj, TOffset off)
{
    const PtrHandle ptr(sh, obj, off);
    return ptr.value();
}

inline bool isAbstractValue(const SymHeap &sh, const TValId val)
{
    const TObjId obj = sh.objByAddr(val);
    const EObjKind kind = sh.objKind(obj);
    return (OK_REGION != kind);
}

inline bool isPossibleToDeref(const SymHeapCore &sh, const TValId val)
{
    const EValueTarget code = sh.valTarget(val);
    if (VT_RANGE == code)
        // address with offset ranges are not allowed to be dreferenced for now
        return false;

    const TObjId obj = sh.objByAddr(val);
    return sh.isValid(obj);
}

inline bool isKnownObjectAt(
        const SymHeapCore          &sh,
        const TValId                val,
        const bool                  allowInvalid = false)
{
    const EValueTarget code = sh.valTarget(val);
    if (VT_RANGE == code)
        // address with offset ranges are not allowed to be dreferenced for now
        return false;

    if (allowInvalid) {
        if (OBJ_INVALID == sh.objByAddr(val))
            return false;
    }
    else {
        if (!isPossibleToDeref(sh, val))
            return false;
    }

    return !isAbstractValue(/* XXX */dynamic_cast<const SymHeap &>(sh), val);
}

inline bool isAddressToFreedObj(SymHeapCore &sh, const TValId val)
{
    const TObjId obj = sh.objByAddr(val);
    if (OBJ_INVALID == obj)
        return false;

    return !sh.isValid(obj);
}

inline bool isVarAlive(SymHeap &sh, const CVar &cv)
{
    const TObjId reg = sh.regionByVar(cv, /* createIfNeeded */ false);
    const TValId at = sh.addrOfRegion(reg);
    return 0 < at;
}

void initGlVar(SymHeap &sh, const CVar &cv);

inline TObjId nextObj(SymHeap &sh, TObjId obj, TOffset offNext)
{
    if (OBJ_INVALID == obj)
        return OBJ_INVALID;

    const TValId valNext = valOfPtr(sh, obj, offNext);
    return sh.objByAddr(valNext);
}

/// TODO: drop this!
inline TValId nextRootObj(SymHeap &sh, TValId root, TOffset offNext)
{
    CL_BREAK_IF(sh.valOffset(root));
    const TValId valNext = valOfPtrAt(sh, root, offNext);
    return sh.valRoot(valNext);
}

inline bool areValProtosEqual(
        const SymHeapCore       &sh1,
        const SymHeapCore       &sh2,
        const TValId            v1,
        const TValId            v2)
{
    if (v1 == v2)
        // matches trivially
        return true;

    if (v1 <= 0 || v2 <= 0)
        // special values have to match
        return false;

    const EValueTarget code1 = sh1.valTarget(v1);
    const EValueTarget code2 = sh2.valTarget(v2);

    if (VT_UNKNOWN != code1 || VT_UNKNOWN != code2)
        // for now, we handle only unknown values here
        return false;

    CL_BREAK_IF(sh1.valOffset(v1) || sh2.valOffset(v2));

    // just compare kinds of unknown values
    const EValueOrigin origin1 = sh1.valOrigin(v1);
    const EValueOrigin origin2 = sh2.valOrigin(v2);
    return (origin1 == origin2);
}

inline bool areUniBlocksEqual(
        const SymHeap           &sh1,
        const SymHeap           &sh2,
        const UniformBlock      &bl1,
        const UniformBlock      &bl2)
{
    if (bl1.off != bl2.off)
        // offset mismatch
        return false;

    if (bl1.size != bl2.size)
        // size mismatch
        return false;

    // compare value prototypes
    return areValProtosEqual(sh1, sh2, bl1.tplValue, bl2.tplValue);
}

template <class TDst, typename TInserter>
void gatherProgramVarsCore(
        TDst                    &dst,
        const SymHeap           &sh,
        TInserter               ins)
{
    TObjList vars;
    sh.gatherObjects(vars, isProgramVar);

    BOOST_FOREACH(const TObjId obj, vars) {
        if (OBJ_RETURN == obj)
            continue;

        (dst.*ins)(sh.cVarByObject(obj));
    }
}

inline void gatherProgramVars(
        TCVarList               &dst,
        const SymHeap           &sh)
{
#if defined(__GNUC_MINOR__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 3)
    // workaround for gcc-4.3.x parsing problem on std::vector::push_back()
    void (TCVarList::*ins)(const CVar &&) = &TCVarList::push_back;
#else
    void (TCVarList::*ins)(const CVar &) = &TCVarList::push_back;
#endif
    gatherProgramVarsCore(dst, sh, ins);
}

inline void gatherProgramVars(
        TCVarSet                &dst,
        const SymHeap           &sh)
{
    typedef std::pair<TCVarSet::iterator, bool> TRet;
    TRet (TCVarSet::*ins)(const CVar &) = &TCVarSet::insert;
    gatherProgramVarsCore(dst, sh, ins);
}

/// take the given visitor through all live pointers
template <class THeap, class TVisitor, typename TMethod>
bool /* complete */ traverseCore(
        THeap                      &sh,
        const TObjId                obj,
        TVisitor                   &visitor,
        TMethod                     method)
{
    // check that we got a valid object
    CL_BREAK_IF(OBJ_INVALID == obj);

    // gather live fields using the requested method
    FldList fields;
    (sh.*method)(fields, obj);

    // guide the visitor through the fields
    BOOST_FOREACH(const FldHandle &fld, fields)
        if (!/* continue */visitor(fld))
            return false;

    // all fields traversed successfully
    return true;
}

/// take the given visitor through all live pointers
template <class THeap, class TVisitor>
bool /* complete */ traverseLivePtrs(
        THeap                      &sh,
        const TObjId                obj,
        TVisitor                   &visitor)
{
    return traverseCore(sh, obj, visitor, &SymHeap::gatherLivePointers);
}

/// take the given visitor through all live objects
template <class THeap, class TVisitor>
bool /* complete */ traverseLiveFields(
        THeap                      &sh,
        const TObjId                obj,
        TVisitor                   &visitor)
{
    return traverseCore(sh, obj, visitor, &SymHeap::gatherLiveFields);
}

/// take the given visitor through all uniform blocks
template <class THeap, class TVisitor>
bool /* complete */ traverseUniformBlocks(
        THeap                       &sh,
        const TObjId                obj,
        TVisitor                    &visitor)
{
    // check that we got a valid root object
    CL_BREAK_IF(OBJ_INVALID == obj);

    TUniBlockMap bMap;
    sh.gatherUniformBlocks(bMap, obj);
    BOOST_FOREACH(TUniBlockMap::const_reference bItem, bMap) {
        if (!visitor(sh, /* UniformBlock */ bItem.second))
            // traversal cancelled by visitor
            return false;
    }

    // done
    return true;
}

/// take the given visitor through all live objects object-wise
template <unsigned N, class THeap, class TVisitor>
bool /* complete */ traverseLiveFieldsGeneric(
        THeap                *const heaps[N],
        const TObjId                objs[N],
        TVisitor                    &visitor)
{
    // collect all live objects from everywhere
    typedef std::pair<TOffset, TObjType> TItem;
    std::set<TItem> all;
    for (unsigned i = 0; i < N; ++i) {
        SymHeap &sh = *heaps[i];
        const TObjId obj = objs[i];
        if (OBJ_INVALID == obj)
            continue;

        FldList fields;
        sh.gatherLiveFields(fields, obj);
        BOOST_FOREACH(const FldHandle &fld, fields) {
            const TOffset off = fld.offset();
            const TObjType clt = fld.type();
            const TItem item(off, clt);
            all.insert(item);
        }
    }

    // go through all live objects
    BOOST_FOREACH(const TItem &item, all) {
        const TOffset  off = item.first;
        const TObjType clt = item.second;

        FldHandle fields[N];
        for (unsigned i = 0; i < N; ++i) {
            SymHeap &sh = *heaps[i];
            const TObjId obj = objs[i];
            fields[i] = FldHandle(sh, obj, off, clt);
        }

        if (!visitor(fields))
            // traversal cancelled by visitor
            return false;
    }

    // done
    return true;
}

/// take the given visitor through all live objects object-wise
template <unsigned N, class THeap, class TVisitor>
bool /* complete */ traverseLiveFields(
        THeap                       &sh,
        const TObjId                objs[N],
        TVisitor                    &visitor)
{
    THeap *heaps[N];
    for (unsigned i = 0; i < N; ++i)
        heaps[i] = &sh;

    return traverseLiveFieldsGeneric<N>(heaps, objs, visitor);
}

/// (VAL_INVALID != pointingFrom) means 'pointing from anywhere'
bool redirectRefs(
        SymHeap                 &sh,
        const TValId            pointingFrom,
        const TValId            pointingTo,
        const TValId            redirectTo,
        const TOffset           offHead = 0);

inline TValId objClone(SymHeap &sh, const TValId root)
{
    CL_BREAK_IF(sh.valOffset(root));
    const TValId dup = sh.valClone(root);

    // if there was "a pointer to self", it should remain "a pointer to self";
    // however "self" has been changed, so that a redirection is necessary
    redirectRefs(sh, dup, root, dup);
    return dup;
}

/// take the given visitor through all live program variables in all heaps
template <unsigned N_DST, unsigned N_SRC, class THeap, class TVisitor>
bool /* complete */ traverseProgramVarsGeneric(
        THeap                *const heaps[N_DST + N_SRC],
        TVisitor                    &visitor,
        const bool                  allowRecovery = false)
{
    const unsigned N_TOTAL = N_DST + N_SRC;
    BOOST_STATIC_ASSERT(N_DST < N_TOTAL);

    // start with all program variables of the first SRC heap
    TCVarSet all;
    gatherProgramVars(all, *heaps[/* src0 */ N_DST]);
#ifndef NDEBUG
    bool tryingRecover = false;
#endif
    // try to match variables from the other heaps if possible
    for (unsigned i = /* src1 */ 1 + N_DST; i < N_TOTAL; ++i) {
        const SymHeap &sh = *heaps[i];

        TObjList live;
        sh.gatherObjects(live, isProgramVar);
        BOOST_FOREACH(const TObjId obj, live) {
            if (OBJ_RETURN == obj)
                continue;

            const CVar cv(sh.cVarByObject(obj));
            if (!insertOnce(all, cv))
                continue;

            // variable mismatch in src heaps
            if (!allowRecovery)
                return false;

            if (/* gl var */ !cv.inst)
                // asymmetric join of gl variables would break everything
                return false;
#ifndef NDEBUG
            tryingRecover = true;
#endif
        }

        if (live.size() == all.size())
            continue;

        // variable mismatch in src heaps
        if (!allowRecovery)
            return false;
#ifndef NDEBUG
        tryingRecover = true;
#endif
    }

    // go through all program variables
    BOOST_FOREACH(const CVar &cv, all) {
        TValId roots[N_TOTAL];
        for (signed i = 0; i < (signed)N_TOTAL; ++i) {
            SymHeap &sh = *heaps[i];

            const bool isDst = (i < (signed)N_DST);
            const bool isAsym = !isDst && !isVarAlive(sh, cv);
            if (isAsym && /* gl var */ !cv.inst)
                // asymmetric join of gl variables would break everything
                return false;

            const bool createIfNeeded = isDst || allowRecovery;
            const TObjId reg = sh.regionByVar(cv, createIfNeeded);
            const TValId at = sh.addrOfRegion(reg);

            if (isAsym) {
                // we have created a local variable in a _src_ heap
                CL_BREAK_IF(!tryingRecover);

                const TValId valInval = sh.valCreate(VT_UNKNOWN, VO_UNKNOWN);
                const TSizeRange size = sh.objSize(reg);
                CL_BREAK_IF(!isSingular(size));

                // mark its contents explicitly as unknown, not (un)initialized
                sh.writeUniformBlock(at, valInval, size.lo);
            }

            roots[i] = at;
        }

        if (!visitor(roots))
            // traversal cancelled by visitor
            return false;
    }

    // done
    return true;
}

#endif /* H_GUARD_SYMUTIL_H */
