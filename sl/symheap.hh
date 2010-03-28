/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
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

/**
 * @file symheap.hh
 * SymHeap - @b symbolic @b heap representation, the core part of "symexec"
 * code listener
 */

#include "symid.hh"

#include <vector>

struct cl_accessor;
struct cl_type;

/**
 * enumeration of unknown values
 */
enum EUnknownValue {
    UV_KNOWN = 0,           ///< known value - what we usually wish we had
    UV_UNKNOWN,             ///< unknown value - what we usually have in reality
    UV_UNINITIALIZED,       ///< unknown value of an uninitialised object
    UV_DEREF_FAILED         ///< value equivalent of symid.hh::OBJ_DEREF_FAILED
};

class SymHeap;

/**
 * symbolic heap @b core - no type-info, no object composition on this level
 */
class SymHeapCore {
    public:
        /// create an empty symbolic heap
        SymHeapCore();

        /// destruction of the symbolic heap invalidates all IDs of its entities
        virtual ~SymHeapCore();

        /// @note there is no such thing like COW implemented for now
        SymHeapCore(const SymHeapCore &);

        /// @note there is no such thing like COW implemented for now
        SymHeapCore& operator=(const SymHeapCore &);

    public:
        /// container used to store object IDs to
        typedef std::vector<TObjId>     TContObj;

        /// container used to store value IDs to
        typedef std::vector<TValueId>   TContValue;

    public:
        /**
         * return a value @b stored @b in the given object
         * @param obj ID of the object to look into
         * @return A valid value ID in case of success, invalid otherwise.
         * @note It may also return @b unknown, @b composite or @b custom value,
         * depending on kind of the queried object.
         * @note It may acquire a new value ID in case the value is not known.
         * @todo SymHeapCore::valueOf is declared const - we should either
         * implement it such, or declare it non-const.  Once it is acomplished,
         * we may also declare it non-virtual.
         */
        virtual TValueId valueOf(TObjId obj) const;

        /**
         * return a value corresponding to @b symbolic @b address of the given
         * object
         * @param obj ID of the object to look for
         * @return A valid value ID when a valid object ID is given, VAL_INVALID
         * otherwise.
         * @note It can be also abused to check if an object ID is valid, or
         * not.
         * @note The operation has always a unique result.
         */
        TValueId placedAt(TObjId obj) const;

        /**
         * return an object ID which the given value @b points @b to
         * @param val ID of the value to follow
         * @return A valid object ID in case of success, invalid otherwise.
         * @note The operation has always a unique result.
         */
        TObjId pointsTo(TValueId val) const;

        /**
         * collect all objects having the given value
         * @param dst reference to a container to store the result to
         * @param val ID of the value to look for
         * @note The operation may return from 0 to n results.
         */
        void haveValue(TContObj &dst, TValueId val) const;

    protected:
        /**
         * create a new symbolic heap object
         * @return ID of the just created symbolic heap object
         */
        TObjId objCreate();

        /**
         * create a new symbolic heap value
         * @param code kind of the unknown value, UV_KNOWN if not unknown
         * @param target pointed object's ID
         * @return ID of the just created symbolic heap value
         */
        TValueId valCreate(EUnknownValue code, TObjId target);

    public:
        TObjId lastObjId() const;
        TValueId lastValueId() const;

    public:
        /**
         * @b set @b value of the given object, which has to be @b valid and may
         * @b not be a composite object
         * @param obj ID of the object to set value of
         * @param val ID requested to be stored into the object
         * @note This is really @b low-level @b implementation.  It does not
         * check for junk, delayed type-info definition, etc.  If you are
         * interested in such abilities, you are looking for
         * SymHeapProcessor::heapSetVal().
         */
        virtual void objSetValue(TObjId obj, TValueId val);

    protected:
        /**
         * @b destroy the given heap object.  All values which have been
         * pointing to the object, will now point to either OBJ_DELETED or
         * OBJ_LOST, depending on kind of the object being destroyed.
         * @param obj ID of the object to destroy
         * @param kind OBJ_DELETED for heap object, or OBJ_LOST for
         * static/automatic object
         * @note This is really @b low-level @b implementation.  It does not
         * e.g. check for junk.  If you are interested in this ability, you
         * are looking for SymHeapProcessor::destroyObj().
         */
        void objDestroy(TObjId obj, TObjId kind);

    public:
        /**
         * check the state of a @b possibly @b unknown @b value
         * @param val ID of the value to check
         * @return fine-grained kind of the unknown value, or UV_KNOWN in case
         * of known value
         * @todo rename SymHeapCore::valGetUnknown
         */
        EUnknownValue valGetUnknown(TValueId val) const;

        /// duplicate the given @b unknown @b value
        virtual TValueId valDuplicateUnknown(TValueId tpl);

        /**
         * replace all occurrences of the given @b unknown value by @b any (not
         * necessarily unknown) value, given as the second argument
         * @param val ID of the value about to be replaced.  It @b must be an
         * unknown value.
         * @param replaceBy @b any value to replace by.  It may be another
         * unknown value, known value or even a special value.
         * @note This method is heavily used by non-deterministic execution of
         * CL_INSN_COND.
         */
        void valReplaceUnknown(TValueId val, TValueId replaceBy);

    public:
        /**
         * introduce a new @b EqIf @b predicate (if not present already)
         * @todo Documentation of addEqIf() needs to be re-worded.
         * @param valCond as soon as valCond becomes a known value, an explicit
         * (in)equality will be deduced for valA and valB.  This means valCond
         * stands for a @b trigger.  It must be an @b unknown @b Boolean value.
         * @param valA one side of the (in)equality scheduled for consideration
         * @param valB one side of the (in)equality scheduled for consideration
         * @param neg @b polarity, (valA == valB) iff (valCond != @b neg)
         */
        void addEqIf(TValueId valCond, TValueId valA, TValueId valB, bool neg);

        /**
         * @b reasoning about possibly unknown heap values
         * @param result where to store the result to in case of @b success
         * @param valA one side of the (in)equality being proved
         * @param valB one side of the (in)equality being proved
         * @return true, if we know the values are either equal, or non-equal;
         * false if @b don't @b know
         * @note The content of *result is not touched at all in case the
         * reasoning has been not successful.
         */
        bool proveEq(bool *result, TValueId valA, TValueId valB) const;

    private:
        struct Private;
        Private *d;
};

/**
 * @b symbolic @b heap representation, the core part of "symexec" project
 */
class SymHeap: public SymHeapCore {
    public:
        /// create an empty symbolic heap
        SymHeap();

        /// destruction of the symbolic heap invalidates all IDs of its entities
        virtual ~SymHeap();

        /// @note there is no such thing like COW implemented for now
        SymHeap(const SymHeap &);

        /// @note there is no such thing like COW implemented for now
        SymHeap& operator=(const SymHeap &);

    public:
        /// container used to store foreign (integral) IDs to
        typedef std::vector<int>        TCont;

    public:
        // XXX
        virtual TValueId valueOf(TObjId obj) const;

        /**
         * look for static type-info of the given object, which has to be @b
         * valid
         * @param obj ID of the object to look for
         * @return pointer to the instance cl_type in case of success, 0
         * otherwise
         * @note The heap can contain valid objects without any type-info.
         * That's usually the case when malloc() has been called, but the result
         * of the call has not been yet assigned to a type-safe pointer.
         */
        const struct cl_type* objType(TObjId obj) const;

        /**
         * look for static type-info of the given value, which has to be @b
         * valid
         * @param val ID of the value to look for
         * @return pointer to the instance cl_type in case of success, 0
         * otherwise
         * @note The heap can contain valid values without any type-info.
         * That's usually the case when malloc() has been called, but the result
         * of the call has not been yet assigned to a type-safe pointer.
         */
        const struct cl_type* valType(TValueId val) const;

        /**
         * look for a static/automatic variable corresponding to the given
         * symbolic heap object, which has to be @b valid
         * @param obj ID of the object to look for
         * @return ID of a static/automatic variable (see CodeStorage) in case
         * of success, -1 otherwise (which usually means the given object was
         * allocated dynamically)
         */
        int cVar(TObjId obj) const;

        /**
         * look for a heap object corresponding to the given static/automatic
         * variable
         * @param uid ID of the static/automatic variable to look for
         * @return A valid object ID in case of success, invalid otherwise.
         * @attention This interface is not strong enough as soon as we allow
         * recursive call of functions.
         * @todo extend the interface constituted by SymHeap::objByCVar() to
         * deal with the recursive call of functions.
         */
        TObjId objByCVar(int uid) const;

        /**
         * collect all static/automatic variables (see CodeStorage) which have
         * any equivalent in the symbolic heap
         * @param dst reference to a container to store the result to
         * @note The operation may return from 0 to n results.
         */
        void gatherCVars(TCont &dst) const;

    public:
        /**
         * check if the given value represents a @b composite @b object, return
         * the object needed to be traversed in that case, OBJ_INVALID otherwise
         * @param val ID of the value to check
         * @return ID of a composite object in case of success, OBJ_INVALID
         * otherwise
         */
        TObjId valGetCompositeObj(TValueId val) const;

        /**
         * return nth sub-object of the given object, which has to be a
         * @b composite @b object
         * @param obj a composite object, which is about to be traversed
         * @param nth index of the sub-object (starting with 0)
         * @note the upper bound of nth can be obtained e.g. by objType()
         * @return A valid object ID in case of success, invalid otherwise.
         */
        TObjId subObj(TObjId obj, int nth) const;

        /**
         * check if the given value is part of a composite object, return ID of
         * the @b surrounding @b object in that case, OBJ_INVALID otherwise
         * @param obj ID of the object to check
         * @return A valid object ID in case of success, invalid otherwise.
         */
        TObjId objParent(TObjId obj) const;

    public:
        /**
         * create a new symbolic heap object of @b known @b type
         * @param clt pointer to static type-info (see CodeStorage), compulsory.
         * @param uid CodeStorage variable ID for non-heap object, or -1 for a
         * heap object of known type
         * @note If you need to create a heap object of unknown type, use
         * objCreateAnon() instead.
         * @return ID of the just created symbolic heap object
         */
        TObjId objCreate(const struct cl_type *clt, int uid);

        /**
         * create a new symbolic heap object of known size, but @b unknown @b
         * type
         * @param cbSize size of the object in @b bytes
         * @return ID of the just created symbolic heap object
         */
        TObjId objCreateAnon(int cbSize);

        /**
         * return size of the given object of @b unknown @b type
         * @param obj ID of the object to look for
         * @attention This method may not be called on objects with known type.
         * @return the size in @b bytes
         */
        int objSizeOfAnon(TObjId obj) const;

        /**
         * return true iff the given value points to an object of @b unknown @b
         * type
         * @param val ID of the value to check
         */
        bool valPointsToAnon(TValueId val) const;

        /**
         * delayed static type-info definition of the given object
         * @param obj ID of the object to define type of
         * @param clt static type-info (see CodeStorage) to be set
         * @note This may trigger creation of all sub-objects, if the given type
         * is a composite type.
         */
        void objDefineType(TObjId obj, const struct cl_type *clt);

    public:

        /**
         * @b destroy the given heap object.  All values which have been
         * pointing to the object, will now point to either OBJ_DELETED or
         * OBJ_LOST, depending on kind of the object being destroyed.
         * @param obj ID of the object to destroy
         * @note This is really @b low-level @b implementation.  It does not
         * e.g. check for junk.  If you are interested in this ability, you
         * are looking for SymHeapProcessor::destroyObj().
         */
        virtual void objDestroy(TObjId obj);

    public:
        /**
         * create an @b unkown @b value, which does not guarantee trivial
         * inequality with other values - see proveEq()
         * @param code fine-grained kind of the unknown value, UV_KNOWN is not
         * allowed here
         * @param clt static type-info about the unknown value, compulsory.
         * @return ID of the just created value
         */
        TValueId valCreateUnknown(EUnknownValue code,
                                  const struct cl_type *clt);

    public:
        /**
         * @b wrap a foreign (integral) value into a symbolic heap value
         * @note this approach is currently used to deal with @b function @b
         * pointers among others
         * @param clt static type-info about the @b wrapped value
         * @param cVal any integral value you need to wrap
         * @return ID of the just created value
         */
        TValueId valCreateCustom(const struct cl_type *clt, int cVal);

        /**
         * retrieve a foreign value, previously @b wrapped by valCreateCustom(),
         * from the given heap value
         * @param pClt store type-info to *pClt, if not 0
         * @param val ID of the value to look into
         * @return the requested foreign value, or -1 if no such value exists
         */
        int valGetCustom(const struct cl_type **pClt, TValueId val) const;

    private:
        struct Private;
        Private *d;

    private:
        void resizeIfNeeded();
        TValueId createCompValue(const struct cl_type *clt, TObjId obj);
        TObjId createSubVar(const struct cl_type *clt, TObjId parent);
        void createSubs(TObjId obj);
        void destroyObj(TObjId obj);
};

#endif /* H_GUARD_SYM_HEAP_H */
