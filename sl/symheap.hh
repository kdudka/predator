/*
 * Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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
 * SymHeapCore - @b symbolic @b heap representation, the core part of "symexec"
 * code listener
 */

#include "config.h"
#include "symid.hh"

#include <cl/code_listener.h>

#include <list>
#include <map>              // for SymHeapCore::TValMap
#include <vector>

enum EValueTarget {
    VT_UNKNOWN              ///< arbitrary target
};

/**
 * enumeration of unknown values
 */
enum EUnknownValue {
    UV_KNOWN = 0,           ///< known value - what we usually wish we had
    UV_ABSTRACT,            ///< points to SLS/DLS (the prover needs to know it)
    UV_UNKNOWN,             ///< unknown value - what we usually have in reality
    UV_DONT_CARE,           ///< unknown value that we don't want to know anyway
    UV_UNINITIALIZED        ///< unknown value of an uninitialised object
};

struct DeepCopyData;
struct SymJoinCtx;

/// a type used by SymHeap for byte offsets
typedef short                                           TOffset;

/// container used to store object IDs to
typedef std::vector<TObjId>                             TObjList;

/// container used to store value IDs to
typedef std::vector<TValId>                             TValList;

/// a type used for type-info
typedef const struct cl_type                            *TObjType;

/// a class of type (structure, pointer, union, ...)
typedef enum cl_type_e                                  TObjCode;

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

    CVar():
        uid(-1),
        inst(-1)
    {
    }

    CVar(int uid_, int inst_):
        uid(uid_),
        inst(inst_)
    {
    }
};

inline bool operator==(const CVar &a, const CVar &b) {
    if (a.uid != b.uid)
        return false;

    if (-1 == a.uid)
        // do not match .inst when a.uid == -1 == b.uid
        return true;

    return a.inst == b.inst;
}

inline bool operator!=(const CVar &a, const CVar &b) {
    return !operator==(a, b);
}

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

        virtual void swap(SymHeapCore &);

        unsigned lastId() const;

    public:
        /**
         * return a value @b stored @b in the given object
         * @param obj ID of the object to look into
         * @return A valid value ID in case of success, invalid otherwise.
         * @note It may also return @b unknown, @b composite or @b custom value,
         * depending on kind of the queried object.
         * @note It may acquire a new value ID in case the value is not known.
         */
        TValId valueOf(TObjId obj) /* FIXME */ const;

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
        TValId placedAt(TObjId obj) /* FIXME */ const;

        /**
         * collect all objects having the given value
         * @param dst reference to a container to store the result to
         * @param val ID of the value to look for
         * @note The operation may return from 0 to n results.
         */
        void usedBy(TObjList &dst, TValId val) const;

        /// return how many objects use the value
        unsigned usedByCount(TValId val) const;

        /**
         * @b set @b value of the given object, which has to be @b valid and may
         * @b not be a composite object
         * @param obj ID of the object to set value of
         * @param val ID requested to be stored into the object
         * @note This is really @b low-level @b implementation.  It does not
         * check for junk, delayed type-info definition, etc.  If you are
         * interested in such abilities, you are looking for
         * SymProc::objSetValue().
         */
        void objSetValue(TObjId obj, TValId val);

    protected:
        /**
         * @b destroy the given heap object.  All values which have been
         * pointing to the object, will now point to either OBJ_DELETED or
         * OBJ_LOST, depending on kind of the object being destroyed.
         * @param obj ID of the object to destroy
         * @note This is really @b low-level @b implementation.  It does not
         * e.g. check for junk.  If you are interested in this ability, you
         * are looking for SymProc::objDestroy().
         */
        virtual void objDestroy(TObjId obj);

    public:
        TValId valCreateDangling(TObjId kind);

        /**
         * check the state of a @b possibly @b unknown @b value
         * @param val ID of the value to check
         * @return fine-grained kind of the unknown value, or UV_KNOWN in case
         * of known value
         * @todo remove SymHeapCore::valGetUnknown
         */
        virtual EUnknownValue valGetUnknown(TValId val) const;

    public:
        enum ENeqOp {
            NEQ_NOP = 0,
            NEQ_ADD,
            NEQ_DEL
        };

        /**
         * introduce a new @b Neq @b predicate (if not present already), or
         * remove an existing one
         * @param op requested operation - NEQ_ADD or NEQ_DEL
         * @param valA one side of the inequality
         * @param valB one side of the inequality
         */
        virtual void neqOp(ENeqOp op, TValId valA, TValId valB);

        /// return true if the given pair of values is proven to be non-equal
        virtual bool proveNeq(TValId valA, TValId valB) const;

    public:
        /// a type used for (injective) value IDs mapping
        typedef std::map<TValId, TValId> TValMap;

        /**
         * return the list of values that are connected to the given value by a
         * Neq predicate
         * @param dst a container to place the result in
         * @param val the reference value, used to search the predicates and
         * then all the related values accordingly
         */
        void gatherRelatedValues(TValList &dst, TValId val) const;

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

        /**
         * pick up all heap predicates that can be fully mapped by valMap into
         * ref and check if they have their own reflection in ref
         * @param ref instance of another symbolic heap
         * @param valMap an (injective) mapping of values from this symbolic
         * heap into the symbolic heap that is given by ref
         * @return return true if all such predicates have their reflection in
         * ref, false otherwise
         */
        bool matchPreds(const SymHeapCore &ref, const TValMap &valMap) const;

    public:

        /**
         * look for static type-info of the given object
         * @param obj ID of the object to look for
         * @return pointer to the instance cl_type in case of success, 0
         * otherwise
         */
        TObjType objType(TObjId obj) const;

        TObjId pointsTo(TValId val) const;

        // symheap-ng
        TValId valByOffset(TValId, TOffset offset);
        EValueTarget valTarget(TValId) const;
        TValId valRoot(TValId) const;
        TOffset valOffset(TValId) const;

        TObjId objAt(TValId at, TObjType clt);
        TObjId objAt(TValId at, TObjCode code);

        /// clone of the given value (deep copy)
        TValId valClone(TValId);

        /// replace all occurences of val by replaceBy
        virtual void valReplace(TValId val, TValId replaceBy);

        /**
         * assume that v1 and v2 are equal.  Useful when e.g. traversing a
         * non-deterministic condition.  This implies that one of them may be
         * dropped.  You can utilize SymHeapCore::usedByCount() to check which
         * one (if any).
         */
        virtual void valMerge(TValId v1, TValId v2);

    public:
        /// container used to store CVar objects to
        typedef std::vector<CVar> TContCVar;

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

        void gatherRootObjs(TObjList &dst) const;

    public:
        /**
         * check if the given value represents a @b composite @b object, return
         * the object needed to be traversed in that case, OBJ_INVALID otherwise
         * @param val ID of the value to check
         * @return ID of a composite object in case of success, OBJ_INVALID
         * otherwise
         */
        TObjId valGetCompositeObj(TValId val) const;

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
         * @param nth if not NULL, return position of the sub-object within the
         * parent object
         * @return A valid object ID in case of success, invalid otherwise.
         */
        TObjId objParent(TObjId obj, int *nth = 0) const;

    public:
        /**
         * create a new symbolic heap object of @b known @b type
         * @param clt pointer to static type-info (see CodeStorage), compulsory.
         * @param cVar variable identification
         * @return ID of the just created symbolic heap object
         */
        TObjId objCreate(TObjType clt, CVar cVar);

        /**
         * create a new heap object of known size
         * @param cbSize size of the object in @b bytes
         * @return value ID of the acquired address
         */
        TValId heapAlloc(int cbSize);

        /**
         * destroy target of the given value (root has to be at zero offset)
         * @return true, if the operation succeeds
         */
        bool valDestroyTarget(TValId);

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
        void objDefineType(TObjId obj, TObjType clt);

        void gatherLiveObjects(TObjList &dst, TValId atAddr) const;
        void gatherLivePointers(TObjList &dst, TValId atAddr) const;

    public:
        /**
         * create an @b unkown @b value, which does not guarantee trivial
         * inequality with other values - see proveEq()
         * @param code fine-grained kind of the unknown value, UV_KNOWN is not
         * allowed here
         * @return ID of the just created value
         */
        TValId valCreateUnknown(EUnknownValue code);

        /**
         * @b wrap a foreign (integral) value into a symbolic heap value
         * @note this approach is currently used to deal with @b function @b
         * pointers among others
         * @param cVal any integral value you need to wrap
         * @return ID of the just created value
         */
        TValId valCreateCustom(int cVal);

        /**
         * retrieve a foreign value, previously @b wrapped by valCreateCustom(),
         * from the given heap value
         * @param val ID of the value to look into
         * @return the requested foreign value, or -1 if no such value exists
         */
        int valGetCustom(TValId val) const;

        /**
         * true, if the given object should be cloned on concretization of the
         * parent object
         */
        bool objIsProto(TObjId obj) const;

        /**
         * set it to true, if the given @b root object should be cloned on
         * concretization of the parent object.  By default, all nested objects
         * are shared.
         */
        void objSetProto(TObjId obj, bool isProto);

    protected:
        /// create a deep copy of the given object with new object IDs
        virtual TObjId objDup(TObjId obj);

    private:
        struct Private;
        Private *d;
};

/**
 * object kind enumeration
 * @attention Please do not change the integral values, they're fixed in order
 * to simplify the code of ProbeVisitor.  Simply speaking, the integral values
 * stand for the count of binding pointers.
 */
enum EObjKind {
    OK_CONCRETE     = 0,    ///< concrete object (not a segment)
    OK_SLS          = 1,    ///< singly-linked list segment
    OK_DLS          = 2,    ///< doubly-linked list segment

    OK_MAY_EXIST,

    OK_HEAD,                ///< segment's head (useful for Linux lists)
    OK_PART                 ///< part of a segment (sub-object)
};

struct BindingOff {
    TOffset head;           ///< target offset
    TOffset next;           ///< offset of the 'next' or 'r' pointer
    TOffset prev;           ///< offset of the 'prev' or 'l' pointer

    BindingOff():
        head(0),
        next(0),
        prev(0)
    {
    }
};

inline bool operator==(const BindingOff &off1, const BindingOff &off2)
{
    return off1.head == off2.head
        && off1.next == off2.next
        && off1.prev == off2.prev;
}

inline bool operator!=(const BindingOff &off1, const BindingOff &off2)
{
    return !operator==(off1, off2);
}

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

        virtual void swap(SymHeapCore &);

    public:
        virtual EUnknownValue valGetUnknown(TValId val) const;

        /**
         * return @b kind of the object. Here @b kind means concrete object,
         * SLS, DLS, and the like.
         */
        EObjKind objKind(TObjId obj) const;
        const BindingOff& objBinding(TObjId obj) const;

    public:
        void objSetAbstract(TObjId obj, EObjKind kind, const BindingOff &off);
        void objSetConcrete(TObjId obj);

    public:
        /**
         * @copydoc SymHeapCore::neqOp
         * @note overridden in order to complement DLS Neq
         */
        virtual void neqOp(ENeqOp op, TValId valA, TValId valB);

        /**
         * @copydoc SymHeapCore::proveNeq
         * @note overridden in order to see through SLS/DLS
         */
        virtual bool proveNeq(TValId valA, TValId valB) const;

        /**
         * @copydoc SymHeapCore::proveNeq
         * @note overridden in order to splice-out SLS/DLS
         */
        virtual void valMerge(TValId v1, TValId v2);

    protected:
        /**
         * @copydoc SymHeapCore::objDestroy
         * @note overridden in order to remove internal data of abstract objects
         */
        virtual void objDestroy(TObjId obj);

        virtual TObjId objDup(TObjId obj);

        // XXX
        friend class SymExecEngine;
        friend class SymProc;

    private:
        void dlSegCrossNeqOp(ENeqOp op, TValId headAddr);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_SYM_HEAP_H */
