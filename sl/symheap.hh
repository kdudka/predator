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
 * SymHeap - the elementary representation of the state of program memory
 */

#include "config.h"

#include "intrange.hh"
#include "symid.hh"
#include "util.hh"

#include <cl/code_listener.h>

#include <map>              // for TValMap
#include <set>              // for TCVarSet
#include <string>
#include <vector>           // for many types

class SymBackTrace;

/// classification of kind of origins a value may come from
enum EValueOrigin {
    VO_INVALID,             ///< reserved for signalling error states
    VO_ASSIGNED,            ///< known result of an operation
    VO_UNKNOWN,             ///< value was abstracted out and guessed later on
    VO_REINTERPRET,         ///< a result of unsupported data reinterpretation
    VO_DEREF_FAILED,        ///< a result of invalid dereference
    VO_STACK,               ///< untouched contents of stack
    VO_HEAP                 ///< untouched contents of heap
};

/// true for VO_HEAP and VO_STACK
bool isUninitialized(EValueOrigin);

/// classification of kind of objects a value may point to
enum EValueTarget {
    VT_INVALID,             ///< completely invalid target
    VT_UNKNOWN,             ///< arbitrary target
    VT_COMPOSITE,           ///< value of a composite field (not a pointer to!)
    VT_CUSTOM,              ///< non-pointer data, classified by ECustomValue
    VT_OBJECT,              ///< target is a (possibly invalid) object

    // TODO: drop this?
    VT_RANGE                ///< an offset value where offset is given by range
};

/// TODO: drop this!
bool isAnyDataArea(EValueTarget);

/// classification of the storage class for objects
enum EStorageClass {
    SC_INVALID,             ///< reserved for signalling error states
    SC_UNKNOWN,             ///< no assumptions, the object may be even shared
    SC_STATIC,              ///< safely allocated in static data if not a 0+ obj
    SC_ON_HEAP,             ///< safely allocated on heap  except for 0+ objects
    SC_ON_STACK             ///< safely allocated on stack except for 0+ objects
};

/// true for SC_ON_HEAP
bool isOnHeap(EStorageClass);

/// true for SC_STATIC and SC_ON_STACK
bool isProgramVar(EStorageClass);

/// classification of the target of an address (a.k.a. target specifier)
enum ETargetSpecifier {
    TS_INVALID,             ///< reserved for signalling error states
    TS_REGION,              ///< the only allowed TS for addresses of regions
    TS_FIRST,               ///< target is the first node of an abstract object
    TS_LAST,                ///< target is the last node of an abstract object
    TS_ALL                  ///< target is any node of an abstract object
};

/// enumeration of custom values, such as integer literals, or code pointers
enum ECustomValue {
    CV_INVALID,             ///< reserved for signalling error states
    CV_FNC,                 ///< code pointer
    CV_INT_RANGE,           ///< a closed interval over integral domain
    CV_REAL,                ///< floating-point number
    CV_STRING               ///< string literal
};

union CustomValueData {
    int             uid;    ///< unique ID as assigned by Code Listener
    double          fpn;    ///< floating-point number
    std::string    *str;    ///< string literal
    IR::Range       rng;    ///< closed interval over integral domain
};

/// representation of a custom value, such as integer literal, or code pointer
class CustomValue {
    public:
        // cppcheck-suppress uninitVar
        CustomValue():
            code_(CV_INVALID)
        {
        }

        ~CustomValue();
        CustomValue(const CustomValue &);
        CustomValue& operator=(const CustomValue &);

        explicit CustomValue(int uid):
            code_(CV_FNC)
        {
            data_.uid = uid;
        }

        explicit CustomValue(const IR::Range &rng):
            code_(CV_INT_RANGE)
        {
            data_.rng = rng;
        }

        explicit CustomValue(const double fpn):
            code_(CV_REAL)
        {
            data_.fpn = fpn;
        }

        explicit CustomValue(const char *str):
            code_(CV_STRING)
        {
            data_.str = new std::string(str);
        }

        /// custom value classification
        ECustomValue code() const {
            return code_;
        }

        /// unique ID as assigned by Code Listener (only for CV_FNC)
        int uid() const;

        /// closed interval over integral domain (only for CV_INT_RANGE)
        IR::Range& rng();

        /// closed interval over integral domain (only for CV_INT_RANGE)
        const IR::Range& rng() const {
            return const_cast<CustomValue *>(this)->rng();
        }

        /// floating-point number (only for CV_REAL)
        double fpn() const;

        /// string literal (only for CV_STRING)
        const std::string &str() const;

    private:
        friend bool operator==(const CustomValue &, const CustomValue &);

        ECustomValue        code_;
        CustomValueData     data_;
};

bool operator==(const CustomValue &a, const CustomValue &b);

inline bool operator!=(const CustomValue &a, const CustomValue &b)
{
    return !operator==(a, b);
}

namespace CodeStorage {
    struct Storage;
}

namespace Trace {
    class Node;
}

/// a type used for integral offsets (changing this is known to cause problems)
typedef IR::TInt                                        TOffset;

/// a type used for block sizes (do not set this to anything else than TOffset)
typedef IR::TInt                                        TSizeOf;

/// a type used for block size ranges (do not try to change this one either)
typedef IR::Range                                       TSizeRange;

/// a container to store offsets to
typedef std::vector<TOffset>                            TOffList;

/// container used to store value IDs to
typedef std::vector<TValId>                             TValList;

/// container used to store object IDs to
typedef std::vector<TObjId>                             TObjList;

/// container used to store value IDs to
typedef std::set<TValId>                                TValSet;

/// container used to store object IDs to
typedef std::set<TObjId>                                TObjSet;

/// a type used for (injective) value IDs mapping
typedef std::map<TValId, TValId>                        TValMap;

/// a type used for (injective) object IDs mapping
typedef std::map<TObjId, TObjId>                        TObjMap;

/// a type used for type-info
typedef const struct cl_type                           *TObjType;

/// a class of type (structure, pointer, union, ...)
typedef enum cl_type_e                                  TObjCode;

/// a reference to CodeStorage::Storage instance describing the analyzed code
typedef const CodeStorage::Storage                     &TStorRef;

/// a type used for prototype level (0 means not a prototype)
typedef short                                           TProtoLevel;

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

inline bool operator==(const CVar &a, const CVar &b)
{
    if (a.uid != b.uid)
        return false;

    if (-1 == a.uid)
        // do not match .inst when a.uid == -1 == b.uid
        return true;

    return a.inst == b.inst;
}

inline bool operator!=(const CVar &a, const CVar &b)
{
    return !operator==(a, b);
}

/// bundles static identification of a function with its call instance number
struct CallInst {
    int uid;        ///< uid of the function
    int inst;       ///< how many instances of the fnc we have on the stack

    CallInst(const SymBackTrace *);

    CallInst(int uid_, int inst_):
        uid(uid_),
        inst(inst_)
    {
    }
};

/**
 * lexicographical comparison of CallInst objects
 * @note we need it in order to place the objects into ordered containers
 */
inline bool operator<(const CallInst &a, const CallInst &b)
{
    // first compare uid
    RETURN_IF_COMPARED(a, b, uid);

    // then compare inst
    return a.inst < b.inst;
}

/// a list of _program_ variables
typedef std::vector<CVar>                               TCVarList;

/// a set of _program_ variables
typedef std::set<CVar>                                  TCVarSet;

/// only uninitialized or nullified blocks; generic arrays and strings need more
struct UniformBlock {
    TOffset     off;        ///< relative placement of the block wrt. the root
    TSizeOf     size;       ///< size of the block in bytes
    TValId      tplValue;   ///< value you need to clone on object instantiation
};

/// a container used to return list of uniform blocks
typedef std::map<TOffset, UniformBlock>                 TUniBlockMap;

/**
 * lexicographical comparison of CVar objects
 * @note we need it in order to place the objects into ordered containers
 */
inline bool operator<(const CVar &a, const CVar &b)
{
    // first compare uid
    RETURN_IF_COMPARED(a, b, uid);

    // then compare inst
    return a.inst < b.inst;
}

class FldList;
class SymHeap;

/// SymHeapCore - the elementary representation of the state of program memory
class SymHeapCore {
    public:
        /// create an empty symbolic heap
        SymHeapCore(TStorRef, Trace::Node *);

        /// destruction of the symbolic heap invalidates all IDs of its entities
        virtual ~SymHeapCore();

        /// relatively cheap operation as long as SH_COPY_ON_WRITE is enabled
        SymHeapCore(const SymHeapCore &);

        /// relatively cheap operation as long as SH_COPY_ON_WRITE is enabled
        SymHeapCore& operator=(const SymHeapCore &);

        /// exchange the contents with the other heap (works in constant time)
        virtual void swap(SymHeapCore &);

        /// each symbolic heap is associated with a CodeStorage model of code
        TStorRef stor() const { return stor_; }

        /// each symbolic heap is associated with a trace graph node
        Trace::Node* traceNode() const;

        /// update the current trace graph node that the heap is associated with
        void traceUpdate(Trace::Node *);

        /// the last assigned ID of a heap entity (not necessarily still valid)
        unsigned lastId() const;

    public:
        /**
         * collect all objects having the given value inside
         * @param dst reference to a container to store the result to
         * @param val ID of the value to look for
         * @param liveOnly if true, exclude objects that are no longer alive
         * @note The operation may return from 0 to n objects.
         */
        void usedBy(FldList &dst, TValId val, bool liveOnly = false) const;

        /// return how many objects have the value inside
        unsigned usedByCount(TValId val) const;

        /// return all objects that point at/inside the given object
        void pointedBy(FldList &dst, TObjId) const;

        /// return how many objects point at/inside the given object
        unsigned pointedByCount(TObjId) const;

        /// write an uninitialized or nullified block of memory
        void writeUniformBlock(
                const TObjId                obj,
                const UniformBlock         &ub,
                TValSet                    *killedPtrs = 0);

        /// copy 'size' bytes of raw memory from 'src' to 'dst'
        void copyBlockOfRawMemory(
                const TValId                dst,
                const TValId                src,
                const TSizeOf               size,
                TValSet                    *killedPtrs = 0);

    public:
        /// define an explicit Neq predicate
        void addNeq(TValId v1, TValId v2);

        /// remove an explicit Neq predicate if defined
        void delNeq(TValId v1, TValId v2);

        /// true if there is an @b explicit Neq relation over the given values
        bool chkNeq(TValId v1, TValId v2) const;

        /// collect values connect with the given value via an extra predicate
        void gatherRelatedValues(TValList &dst, TValId val) const;

        /// transfer as many as possible extra heap predicates from this to dst
        void copyRelevantPreds(SymHeapCore &dst, const TValMap &valMap) const;

        /// true if all Neq predicates can be mapped to Neq predicates in ref
        bool matchPreds(
                const SymHeapCore           &ref,
                const TValMap               &valMap,
                const bool                  nonZeroOnly = false)
            const;

    public:
        /// translate the given address by the given offset
        TValId valByOffset(TValId, TOffset offset);

        /// create (or recycle) a VT_RANGE value at the given allocated address
        TValId valByRange(TValId at, IR::Range range);

        /// translate the given value by the given offset
        TValId valShift(TValId valToShift, TValId shiftBy);

        /// classify the object the given value points to
        EValueTarget valTarget(TValId) const;

        /// classify where the given value originates from
        EValueOrigin valOrigin(TValId) const;

        /// return the target specifier of the given address
        ETargetSpecifier targetSpec(TValId addr) const;

        /// return the object that the given address points to
        TObjId objByAddr(TValId addr) const;

        /// classify the storage class of the given object
        EStorageClass objStorClass(TObjId) const;

        /// return size (in bytes) of the given object
        TSizeRange objSize(TObjId) const;

        /// target address at the given object with target specifier and offset
        TValId addrOfTarget(TObjId, ETargetSpecifier, TOffset off = 0);

    private:
        /// experimental implementation helper of rejoinObj(), do not use!
        void rewriteTargetOfBase(TValId addr, TObjId target);
        friend void redirectAddrs(SymHeap &, const TObjId, const TObjId);

    public:
        /// return the address of the root which the given value is binded to
        TValId valRoot(TValId) const;

        /// return the relative placement from the root
        TOffset valOffset(TValId) const;

        /// return the offset range associated with the given VT_RANGE value
        IR::Range valOffsetRange(TValId) const;

        /// narrow down the offset range of the given VT_RANGE value
        void valRestrictRange(TValId, IR::Range win);

        /// difference between two pointers (makes sense only for shared roots)
        TValId diffPointers(const TValId v1, const TValId v2);

        /// return count of bytes (including NULL) we can safely read as string
        TSizeRange valSizeOfString(TValId) const;

        /// return the region corresponding to the given program variable
        TObjId regionByVar(CVar, bool createIfNeeded);

        /// clone the given object, including the outgoing has-value edges
        virtual TObjId objClone(TObjId);

    public:
        /// replace all occurrences of val by replaceBy
        virtual void valReplace(TValId val, TValId replaceBy);

        /// return the list of objects satisfying the given filtering predicate
        void gatherObjects(TObjList &dst, bool (*)(EStorageClass) = 0) const;

        /// list of live fields (including ptrs) inside the given object
        void gatherLiveFields(FldList &dst, TObjId) const;

        /// list of uninitialized and nullified uniform blocks of the given obj
        void gatherUniformBlocks(TUniBlockMap &dst, TObjId) const;

        /// experimental optimization of joinUniBlocksCore()
        bool findCoveringUniBlocks(
                TUniBlockMap               *pCovered,
                const TObjId                root,
                UniformBlock                block)
            const;

        /// return program variable that the given object maps to
        CVar cVarByObject(TObjId) const;

        /**
         * composite object given by val (applicable only on VT_COMPOSITE vals)
         * @todo should we operate on FldHandle instead?
         */
        TFldId valGetComposite(TValId val) const;

    public:
        /// allocate a chunk of stack of known size from the select call stack
        TObjId stackAlloc(const TSizeRange &size, const CallInst &from);

        /// clear the list of anonymous stack objects of the given call instance
        void clearAnonStackObjects(TObjList &dst, const CallInst &of);

        /// allocate a chunk of heap of known size
        TObjId heapAlloc(const TSizeRange &size);

        /// return true if the given object can be still accessed safely
        bool isValid(TObjId) const;

        /// invalidate the given object
        virtual void objInvalidate(TObjId);

        /// update the estimated type-info of the given object
        void objSetEstimatedType(TObjId obj, TObjType clt);

        /// return the estimated type-info of the given object
        TObjType objEstimatedType(TObjId obj) const;

        /// create a @b generic value, otherwise use addrOfTarget()
        TValId valCreate(EValueTarget code, EValueOrigin origin);

        /// wrap a custom value, such as integer literal, or code pointer
        TValId valWrapCustom(CustomValue data);

        /// unwrap a custom value, such as integer literal, or code pointer
        const CustomValue& valUnwrapCustom(TValId) const;

    public:
        /// prototype level of the given object (0 means not a prototype)
        TProtoLevel objProtoLevel(TObjId) const;

        /// set prototype level of the given boject (0 means not a prototype)
        void objSetProtoLevel(TObjId obj, TProtoLevel level);

    protected:
        /// return a @b data pointer inside the given object at the given offset
        TFldId ptrLookup(TObjId obj, TOffset off);

        /// return a field of the specified type at the specified offset in obj
        TFldId fldLookup(TObjId obj, TOffset off, TObjType clt);

        /// increment the external reference count of the given object
        void fldEnter(TFldId);

        /// decrement the external reference count (may trigger its destruction)
        void fldLeave(TFldId);

        /// FldHandle takes care of external reference count
        friend class FldHandle;
        friend class PtrHandle;

    protected:
        // these should be accessed indirectly via FldHandle
        TValId valueOf(TFldId fld);
        TValId placedAt(TFldId fld);
        TObjId objByField(TFldId fld) const;
        TOffset fieldOffset(TFldId fld) const;
        TObjType fieldType(TFldId fld) const;
        void setValOfField(TFldId fld, TValId val, TValSet *killedPtrs = 0);

    protected:
        TStorRef stor_;

    private:
        struct Private;
        Private *d;
};

class FldHandle {
    public:
        FldHandle():
            sh_(0),
            id_(FLD_INVALID)
        {
        }

        FldHandle(SymHeapCore &sh, TObjId obj, TObjType clt, TOffset off = 0):
            sh_(&sh),
            id_(sh.fldLookup(obj, off, clt))
        {
            if (0 < id_)
                sh_->fldEnter(id_);
        }

        explicit FldHandle(const TFldId special):
            sh_(0),
            id_(special)
        {
            CL_BREAK_IF(0 < special);
        }

        FldHandle(const FldHandle &tpl):
            sh_(tpl.sh_),
            id_(tpl.id_)
        {
            if (0 < id_)
                sh_->fldEnter(id_);
        }

        FldHandle(SymHeapCore &sh, const FldHandle &tpl):
            sh_(&sh),
            id_(tpl.id_)
        {
            if (0 < id_)
                sh_->fldEnter(id_);
        }

        ~FldHandle() {
            if (0 < id_)
                sh_->fldLeave(id_);
        }

        FldHandle& operator=(const FldHandle &tpl) {
            if (0 < id_)
                sh_->fldLeave(id_);

            sh_ = tpl.sh_;
            id_ = tpl.id_;
            if (0 < id_)
                sh_->fldEnter(id_);

            return *this;
        }

    public:
        /// return the SymHeapCore instance associated with this handle
        SymHeapCore*    sh()            const { return sh_; }

        /// return raw field ID inside this handle (used mainly internally)
        TFldId          fieldId()       const { return id_; }

        /// true if the given handle is valid (does not imply field validity)
        bool            isValidHandle() const { return (0 < id_); }

        /// return the object that the field is part of
        TObjId          obj()           const { return sh_->objByField(id_); }

        /// return the offset at which the field is placed within the object
        TOffset         offset()        const { return sh_->fieldOffset(id_); }

        /// return the value inside the field (may trigger its initialization)
        TValId          value()         const { return sh_->valueOf(id_); }

        /// return the address of the field (may trigger address instantiation)
        TValId          placedAt()      const { return sh_->placedAt(id_); }

        /// static type-info of the given object (return 0 if not available)
        TObjType type() const {
            return (this->isValidHandle())
                ? sh_->fieldType(id_)
                : 0;
        }

        /// assign the given value, caller is responsible for garbage collecting
        void setValue(const TValId val, TValSet *killedPtrs = 0) const {
            sh_->setValOfField(id_, val, killedPtrs);
        }

    protected:
        FldHandle(SymHeapCore &sh, TFldId id):
            sh_(&sh),
            id_(id)
        {
            if (0 < id_)
                sh_->fldEnter(id_);
        }

        friend class SymHeapCore;

        // TODO: remove this
        friend class SymProc;

        // TODO: remove this
        friend const char* valNullLabel(const SymHeapCore &, const TFldId);

    protected:
        SymHeapCore     *sh_;
        TFldId           id_;
};

/// this allows to insert FldHandle instances into std::set
inline bool operator<(const FldHandle &a, const FldHandle &b)
{
    // first compare heap addresses
    RETURN_IF_COMPARED(a, b, sh());

    // then compare field IDs
    return (a.fieldId() < b.fieldId());
}

inline bool operator==(const FldHandle &a, const FldHandle &b)
{
    return (a.sh() == b.sh())
        && (a.fieldId() == b.fieldId());
}

inline bool operator!=(const FldHandle &a, const FldHandle &b)
{
    return !operator==(a, b);
}

class PtrHandle: public FldHandle {
    public:
        PtrHandle(SymHeapCore &sh, const TObjId obj, const TOffset off = 0):
            FldHandle(sh, sh.ptrLookup(obj, off))
        {
            if (0 < id_)
                sh_->fldEnter(id_);
        }
};

/// ugly, but typedefs do not support partial declarations
class FldList: public std::vector<FldHandle> { };

/// set of object handles
typedef std::set<FldHandle>                             TFldSet;

/// a type used for minimal segment length (0+, 1+, ...)
typedef short                                           TMinLen;

/// enumeration of abstract object (although OK_REGION is not abstract)
enum EObjKind {
    OK_REGION = 0,          ///< concrete object (not a segment)
    OK_SLS,                 ///< singly-linked list segment
    OK_DLS,                 ///< doubly-linked list segment
    OK_OBJ_OR_NULL,         ///< 0..1 object, assume NULL if not allocated
    OK_SEE_THROUGH,         ///< 0..1 object, see through if not allocated
    OK_SEE_THROUGH_2N       ///< OK_SEE_THROUGH with two next pointers
};

inline bool isMayExistObj(const enum EObjKind kind)
{
    switch (kind) {
        case OK_OBJ_OR_NULL:
        case OK_SEE_THROUGH:
        case OK_SEE_THROUGH_2N:
            return true;

        default:
            return false;
    }
}

/// tuple of binding offsets assigned to abstract objects
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

    BindingOff(EObjKind kind):
        head(-1),
        next(-1),
        prev(-1)
    {
        CL_BREAK_IF(OK_OBJ_OR_NULL != kind);
        (void) kind;
    }
};

/// point-wise comparison of BindingOff
inline bool operator==(const BindingOff &off1, const BindingOff &off2)
{
    return off1.head == off2.head
        && off1.next == off2.next
        && off1.prev == off2.prev;
}

/// point-wise comparison of BindingOff
inline bool operator!=(const BindingOff &off1, const BindingOff &off2)
{
    return !operator==(off1, off2);
}

/// lexicographical comparison of BindingOff, need for std::map
inline bool operator<(const BindingOff &off1, const BindingOff &off2)
{
    RETURN_IF_COMPARED(off1, off2, next);
    RETURN_IF_COMPARED(off1, off2, prev);
    return (off1.head < off2.head);
}

/// extension of SymHeapCore dealing with abstract objects (list segments etc.)
class SymHeap: public SymHeapCore {
    public:
        /// create an empty symbolic heap
        SymHeap(TStorRef, Trace::Node *);

        /// destruction of the symbolic heap invalidates all IDs of its entities
        virtual ~SymHeap();

        /// relatively cheap operation as long as SH_COPY_ON_WRITE is enabled
        SymHeap(const SymHeap &);

        /// relatively cheap operation as long as SH_COPY_ON_WRITE is enabled
        SymHeap& operator=(const SymHeap &);

        virtual void swap(SymHeapCore &);

    public:
        /// kind of object (region, SLS, DLS, ...)
        EObjKind objKind(TObjId) const;

        /// tuple of binding offsets (next, prev, ...)
        const BindingOff& segBinding(TObjId seg) const;

        /// set properties of an abstract object, set abstract if not already
        void objSetAbstract(
                TObjId                      seg,
                EObjKind                    kind,
                const BindingOff            &off);

        /// set the given abstract object to be a concrete object (drops props)
        void objSetConcrete(TObjId);

        /// read the minimal segment length of the given abstract object
        TMinLen segMinLength(TObjId seg) const;

        /// assign the minimal segment length of the given abstract object
        void segSetMinLength(TObjId seg, TMinLen len);

    public:
        // just overrides (inherits the dox)
        virtual void objInvalidate(TObjId);
        virtual TObjId objClone(TObjId);

    private:
        struct Private;
        Private *d;
};

/// enable/disable built-in self-checks (takes effect only in debug build)
void enableProtectedMode(bool enable);

/// temporarily disable protected mode of SymHeap in a debug build
class ProtectionIntrusion {
    public:
        ProtectionIntrusion() {
            enableProtectedMode(false);
        }

        ~ProtectionIntrusion() {
            enableProtectedMode(true);
        }
};

#endif /* H_GUARD_SYM_HEAP_H */
