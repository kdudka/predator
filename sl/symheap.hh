/*
 * Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
 * Copyright (C) 2010 Petr Peringer, FIT
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

#ifndef H_GUARD_SYM_HEAP_H
#define H_GUARD_SYM_HEAP_H

/**
 * @file symheap.hh
 * SymHeapTyped - @b symbolic @b heap representation, the core part of "symexec"
 * code listener
 */

#include "config.h"
#include "symid.hh"

#include <list>
#include <map>                  // for SymHeapCore::TValMap
#include <vector>

#ifndef SE_STATE_HASH_OPTIMIZATION
#   define SE_STATE_HASH_OPTIMIZATION 0
#endif

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

struct DeepCopyData;
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

        // TODO: explicit Clone operation, use shared_ptr for Privete
        // implementation

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
        void usedBy(TContObj &dst, TValueId val) const;

        /// return how many objects use the value
        unsigned usedByCount(TValueId val) const;

        /// create a duplicate of the given object with a new object ID
        virtual TObjId objDup(TObjId obj);

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
         * SymHeapProcessor::objSetValue().
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
         * are looking for SymHeapProcessor::objDestroy().
         */
        void objDestroy(TObjId obj, TObjId kind);

        friend TValueId handleValue(DeepCopyData &dc, TValueId valSrc, bool);

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

        /// change all variables using value _val to value _newval (all known)
        void valReplace(TValueId _val, TValueId _newval);

    public:
        /**
         * introduce a new @b Neq @b predicate (if not present already)
         * @param valA one side of the inequality
         * @param valB one side of the inequality
         */
        void addNeq(TValueId valA, TValueId valB);

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

    public:
        /// a type used for (injective) value IDs mapping
        typedef std::map<TValueId, TValueId> TValMap;

        /**
         * return the list of values that are connected to the given value by a
         * Neq/EqIf predicate
         * @param dst a container to place the result in
         * @param val the reference value, used to search the predicates and
         * then all the related values accordingly
         */
        void gatherRelatedValues(TContValue &dst, TValueId val) const;

        /**
         * copy all @b relevant predicates from the symbolic heap to another
         * symbolic heap, using the given (injective) value IDs mapping.  Here
         * @b relevant means that there exists a suitable mapping for all the
         * values which are connected by the predicate
         * @param dst destination heap, there will be added the relevant
         * predicates
         * @param valMap an (injective) value mapping, used for translation
         * of value IDs among heaps
         */
        void copyRelevantPreds(SymHeapCore &dst, const TValMap &valMap) const;

    protected:
        // safe to disclose for public?
        void delNeq(TValueId val1, TValueId val2);

    private:
        struct Private;
        Private *d;
};

/**
 * bundles static identification of a variable with its instance number
 *
 * In order to enable call recursion, we need to distinguish among various
 * instances of the same automatic variable in case a function is called
 * recursively.
 */
struct CVar {
    /// static identification of a variable
    int uid;

    /// zero for global/static variables, instance number 1..n otherwise
    int inst;

    CVar(): uid(-1) { }

    CVar(int uid_, int inst_):
        uid(uid_),
        inst(inst_)
    {
    }
};

/**
 * lexicographical comparison of CVar objects
 * @note we need it in order to place the objects into ordered containers
 */
inline bool operator<(const CVar &a, const CVar &b) {
    if (a.uid < b.uid)
        return true;
    else if (b.uid < a.uid)
        return false;
    else
        // we know (a.uid == b.uid) at this point, let's compare .inst
        return a.inst < b.inst;
}

/**
 * @b symbolic @b heap representation, the core part of "symexec" project
 */
class SymHeapTyped: public SymHeapCore {
    public:
        /// create an empty symbolic heap
        SymHeapTyped();

        /// destruction of the symbolic heap invalidates all IDs of its entities
        virtual ~SymHeapTyped();

        /// @note there is no such thing like COW implemented for now
        SymHeapTyped(const SymHeapTyped &);

        /// @note there is no such thing like COW implemented for now
        SymHeapTyped& operator=(const SymHeapTyped &);

#if SE_STATE_HASH_OPTIMIZATION
        virtual size_t hash() const;
#endif

    public:
        /// container used to store CVar objects to
        typedef std::vector<CVar> TContCVar;

    public:
        // XXX
        virtual TValueId valueOf(TObjId obj) const;

        /// create a deep copy of the given object with new object IDs
        virtual TObjId objDup(TObjId obj);

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
         * @param dst instance of CVar object, to store the lookup result in
         * case of success.  This may be also NULL, if there is no need to
         * return the identification.
         * @param obj ID of the object to look for
         * @return true for in case of success (which means the variable is
         * static/automatic, false otherwise (which usually means the given
         * object was allocated dynamically)
         */
        bool cVar(CVar *dst, TObjId obj) const;

        /**
         * look for a heap object corresponding to the given static/automatic
         * variable
         * @param cVar static/automatic variable to look for
         * @return A valid object ID in case of success, invalid otherwise.
         */
        TObjId objByCVar(CVar cVar) const;

        /**
         * collect all static/automatic variables (see CodeStorage) which have
         * any equivalent in the symbolic heap
         * @param dst reference to a container to store the result to
         * @note The operation may return from 0 to n results.
         */
        void gatherCVars(TContCVar &dst) const;

        /**
         * collect all (either static/automatic) or dynamic @b root objects.
         * Here @b root means that the object contains some sub-objects, but is
         * @b not part of another object
         * @param dst reference to a container to store the result to
         * @note The operation may return from 0 to n results.
         */
        void gatherRootObjs(TContObj &dst) const;

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
         * @param cVar variable identification for non-heap object.  Use the
         * default CVar constructor for a heap object of known type
         * @note If you need to create a heap object of unknown type, use
         * objCreateAnon() instead.
         * @return ID of the just created symbolic heap object
         */
        TObjId objCreate(const struct cl_type *clt, CVar cVar = CVar());

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
         * are looking for SymHeapProcessor::objDestroy().
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
         * @param pClt store type-info to *pClt, if (pClt != 0)
         * @param val ID of the value to look into
         * @return the requested foreign value, or -1 if no such value exists
         */
        int valGetCustom(const struct cl_type **pClt, TValueId val) const;

    private:
        struct Private;
        Private *d;

    private:
        void resizeIfNeeded();
        void initValClt(TObjId obj);
        TValueId createCompValue(const struct cl_type *clt, TObjId obj);
        TObjId createSubVar(const struct cl_type *clt, TObjId parent);
        void createSubs(TObjId obj);
        void objDestroyPriv(TObjId obj);
};

enum EObjKind {
    OK_CONCRETE = 0,
    OK_SLS,
    OK_DLS
};

typedef std::vector<int /* nth */> TFieldIdxChain;

TObjId subObjByChain(const SymHeapTyped &sh, TObjId obj, TFieldIdxChain ic);

class SymHeap: public SymHeapTyped {
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
        virtual TObjId objDup(TObjId obj);

    public:
        /**
         * return @b kind of the object. Here @b kind means concrete object,
         * SLS, DLS, and the like.
         */
        EObjKind objKind(TObjId obj) const;

        /**
         * return @b abstraction level of the given object.  Zero means concrete
         * object, 1 means regular SLS or DLS object, 2 stands for an abstract
         * object that consists of another abstract objects, and so on and so
         * forth.
         */
        int objAbstractLevel(TObjId obj) const;

        /**
         * return a @b binding sub-object of the given @b abstract object.  Here
         * @b binding objects stands for the filed usually called @c next or @c
         * prev.
         */
        TFieldIdxChain objBinderField(TObjId obj) const;

        /**
         * return a @b peer sub-object - for a head object, you get the field
         * containing a pointer to tail and vice versa.  Note we don't
         * distinguish among head and tail, it's just a @b peer in our
         * terminology.
         * @note This method basically makes sense only for DLS.
         */
        TFieldIdxChain objPeerField(TObjId obj) const;

        /**
         * convert the given @b concrete object to an abstract object.  If you
         * need to create higher level abstract object, just call this method
         * more times, but always on the root object, which should be a concrete
         * object.
         */
        void objAbstract(TObjId obj, EObjKind kind, TFieldIdxChain bidnerField,
                         TFieldIdxChain /* not used for SLS */ peerField);

        /**
         * convert the given @b abstract object to a less abstract object.  If
         * the given object is a regular SLS/DLS object, it results to a
         * concrete object.  Otherwise, the abstraction level is just decreased.
         */
        void objConcretize(TObjId obj);

        // TODO: override objDestroy() some day
    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_HEAP_H */
