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
    VO_UNKNOWN,             ///< unknown result of an operation (e.g. < >)
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
    VT_COMPOSITE,           ///< value of composite object (not a pointer to!)
    VT_CUSTOM,              ///< not a pointer to data
    VT_STATIC,              ///< target is static data
    VT_ON_STACK,            ///< target is on stack
    VT_ON_HEAP,             ///< target is on heap
    VT_LOST,                ///< target was on stack, but it is no longer valid
    VT_DELETED,             ///< target was on heap, but it is no longer valid
    VT_RANGE,               ///< an offset value where offset is given by range
    VT_ABSTRACT             ///< abstract object (segment)
};

/// true for VT_ABSTRACT
bool isAbstract(EValueTarget);

/// true for VT_STATIC, VT_ON_HEAP, and VT_ON_STACK
bool isKnownObject(EValueTarget);

/// true for VT_DELETED and VT_LOST
bool isGone(EValueTarget);

/// true for VT_ON_HEAP and VT_ABSTRACT
bool isOnHeap(EValueTarget);

/// true for VT_STATIC and VT_ON_STACK
bool isProgramVar(EValueTarget);

/// true for VT_STATIC, VT_ON_STACK, VT_ON_STACK, and VT_ABSTRACT
bool isPossibleToDeref(EValueTarget);

/// true for VT_STATIC, VT_ON_STACK, VT_ON_STACK, VT_ABSTRACT, and VT_RANGE
bool isAnyDataArea(EValueTarget);

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

/// container used to store value IDs to
typedef std::set<TValId>                                TValSet;

/// a type used for (injective) value IDs mapping
typedef std::map<TValId, TValId>                        TValMap;

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
    if (a.uid < b.uid)
        return true;
    else if (b.uid < a.uid)
        return false;
    else
        // we know (a.uid == b.uid) at this point, let's compare .inst
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
    if (a.uid < b.uid)
        return true;
    else if (b.uid < a.uid)
        return false;
    else
        // we know (a.uid == b.uid) at this point, let's compare .inst
        return a.inst < b.inst;
}

class ObjList;

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
        void usedBy(ObjList &dst, TValId val, bool liveOnly = false) const;

        /// return how many objects have the value inside
        unsigned usedByCount(TValId val) const;

        /// return all objects that point at/inside the given root entity
        void pointedBy(ObjList &dst, TValId root) const;

        /// return how many objects point at/inside the given root entity
        unsigned pointedByCount(TValId root) const;

        /// write an uninitialized or nullified block of memory
        void writeUniformBlock(
                const TValId                addr,
                const TValId                tplValue,
                const TSizeOf               size,
                TValSet                     *killedPtrs = 0);

        /// copy 'size' bytes of raw memory from 'src' to 'dst'
        void copyBlockOfRawMemory(
                const TValId                dst,
                const TValId                src,
                const TSizeOf               size,
                TValSet                     *killedPtrs = 0);

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
        bool matchPreds(const SymHeapCore &ref, const TValMap &valMap) const;

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

        /// return size (in bytes) that we can safely write at the given addr
        TSizeRange valSizeOfTarget(TValId) const;

        /// return count of bytes (including NULL) we can safely read as string
        TSizeRange valSizeOfString(TValId) const;

        /// return address of the given program variable
        TValId addrOfVar(CVar, bool createIfNeeded);

        /// clone of the given value (deep copy)
        virtual TValId valClone(TValId);

    public:
        /// replace all occurrences of val by replaceBy
        virtual void valReplace(TValId val, TValId replaceBy);

        /// list of root heap entities satisfying the given filtering predicate
        void gatherRootObjects(TValList &dst, bool (*)(EValueTarget) = 0) const;

        /// list of live objects (including ptrs) owned by the given root entity
        void gatherLiveObjects(ObjList &dst, TValId root) const;

        /// list of live pointers owned by the given root entity
        void gatherLivePointers(ObjList &dst, TValId root) const;

        /// list of uninitialized and nullified uniform blocks
        void gatherUniformBlocks(TUniBlockMap &dst, TValId root) const;

        /// experimental optimization of joinUniBlocksCore()
        bool findCoveringUniBlocks(
                TUniBlockMap               *pCovered,
                const TValId                root,
                UniformBlock                block)
            const;

        /**
         * return the corresponding program variable of the given @b root
         * address pointing to VT_STATIC/VT_ON_STACK
         */
        CVar cVarByRoot(TValId root) const;

        /**
         * composite object given by val (applicable only on VT_COMPOSITE vals)
         * @todo should we operate on ObjHandle instead?
         */
        TObjId valGetComposite(TValId val) const;

    public:
        /// allocate a chunk of stack of known size from the select call stack
        TValId stackAlloc(const TSizeRange &size, const CallInst &from);

        /// clear the list of anonymous stack objects of the given call instance
        void clearAnonStackObjects(TValList &dst, const CallInst &of);

        /// allocate a chunk of heap of known size
        TValId heapAlloc(const TSizeRange &size);

        /// destroy target of the given root value
        virtual void valDestroyTarget(TValId root);

        /// initialize (estimated) type-info of the given root entity
        void valSetLastKnownTypeOfTarget(TValId root, TObjType clt);

        /// read the (estimated) type-info of the given root entity
        TObjType valLastKnownTypeOfTarget(TValId root) const;

        /// create a @b generic value (heapAlloc() or addrOfVar() otherwise)
        TValId valCreate(EValueTarget code, EValueOrigin origin);

        /// wrap a custom value, such as integer literal, or code pointer
        TValId valWrapCustom(CustomValue data);

        /// unwrap a custom value, such as integer literal, or code pointer
        const CustomValue& valUnwrapCustom(TValId) const;

    public:
        /// prototype level of the target root entity (0 means not a prototype)
        TProtoLevel valTargetProtoLevel(TValId) const;

        /// set prototype level of the given root (0 means not a prototype)
        void valTargetSetProtoLevel(TValId root, TProtoLevel level);

    protected:
        /// return a @b data pointer placed at the given address
        TObjId ptrAt(TValId at);

        /// return an object of the given type at the given address
        TObjId objAt(TValId at, TObjType clt);

        /// increment the external reference count of the given object
        void objEnter(TObjId);

        /// decrement the external reference count (may trigger its destruction)
        void objLeave(TObjId);

        /// ObjHandle takes care of external reference count
        friend class ObjHandle;
        friend class PtrHandle;

    protected:
        // these should be accessed indirectly via ObjHandle
        TValId valueOf(TObjId obj);
        TValId placedAt(TObjId obj);
        TObjType objType(TObjId obj) const;
        void objSetValue(TObjId obj, TValId val, TValSet *killedPtrs = 0);

    protected:
        TStorRef stor_;

        /// return true if the given value points to/inside an abstract object
        virtual bool hasAbstractTarget(TValId) const {
            // no abstract objects at this level
            return false;
        }

    private:
        struct Private;
        Private *d;
};

class ObjHandle {
    public:
        ObjHandle():
            sh_(0),
            id_(OBJ_INVALID)
        {
        }

        explicit ObjHandle(const TObjId special):
            sh_(0),
            id_(special)
        {
            CL_BREAK_IF(0 < special);
        }

        ObjHandle(SymHeapCore &sh, TValId addr, TObjType clt):
            sh_(&sh),
            id_(sh.objAt(addr, clt))
        {
            if (0 < id_)
                sh_->objEnter(id_);
        }

        ObjHandle(const ObjHandle &tpl):
            sh_(tpl.sh_),
            id_(tpl.id_)
        {
            if (0 < id_)
                sh_->objEnter(id_);
        }

        ObjHandle(SymHeapCore &sh, const ObjHandle &tpl):
            sh_(&sh),
            id_(tpl.id_)
        {
            if (0 < id_)
                sh_->objEnter(id_);
        }

        ~ObjHandle() {
            if (0 < id_)
                sh_->objLeave(id_);
        }

        ObjHandle& operator=(const ObjHandle &tpl) {
            if (0 < id_)
                sh_->objLeave(id_);

            sh_ = tpl.sh_;
            id_ = tpl.id_;
            if (0 < id_)
                sh_->objEnter(id_);

            return *this;
        }

    public:
        SymHeapCore*    sh()        const { return sh_; }
        TObjId          objId()     const { return id_; }
        bool            isValid()   const { return 0 < id_; }

        /**
         * return a value @b stored @b in the given object
         * @return A valid value ID in case of success, invalid otherwise.
         * @note It may also return @b unknown, @b composite or @b custom value,
         * depending on kind of the queried object.
         * @note It may acquire a new value ID in case the value is not known.
         */
        TValId          value()     const { return sh_->valueOf(id_); }

        /**
         * return a value corresponding to @b symbolic @b address of the given
         * object
         * @return A valid value ID when a valid object ID is given, VAL_INVALID
         * otherwise.
         */
        TValId          placedAt()  const { return sh_->placedAt(id_); }

        /**
         * look for static type-info of the given object
         * @return pointer to the instance cl_type in case of success, 0
         * otherwise
         */
        TObjType objType() const {
            return (this->isValid())
                ? sh_->objType(id_)
                : 0;
        }

        /**
         * @b set @b value of the given object, which has to be @b valid and may
         * @b not be a composite object
         * @param val ID requested to be stored into the object
         * @param killedPtrs if not NULL, insert killed pointer values there
         * @note This is really @b low-level @b implementation.  It does not
         * check for junk, delayed type-info definition, etc.  If you are
         * interested in such abilities, you are looking for
         * SymProc::objSetValue().
         */
        void setValue(const TValId val, TValSet *killedPtrs = 0) const {
            sh_->objSetValue(id_, val, killedPtrs);
        }

    protected:
        ObjHandle(SymHeapCore &sh, TObjId id):
            sh_(&sh),
            id_(id)
        {
            if (0 < id_)
                sh_->objEnter(id_);
        }

        friend class SymHeapCore;

        // TODO: remove this
        friend class SymProc;

        // TODO: remove this
        friend const char* valNullLabel(const SymHeapCore &, const TObjId);

    protected:
        SymHeapCore     *sh_;
        TObjId           id_;
};

/// this allows to insert ObjHandle instances into std::set
inline bool operator<(const ObjHandle &a, const ObjHandle &b)
{
    if (a.sh() < b.sh())
        return true;

    if (b.sh() < a.sh())
        return false;

    return (a.objId() < b.objId());
}

inline bool operator==(const ObjHandle &a, const ObjHandle &b)
{
    return (a.sh()    == b.sh())
        && (a.objId() == b.objId());
}

inline bool operator!=(const ObjHandle &a, const ObjHandle &b)
{
    return !operator==(a, b);
}

class PtrHandle: public ObjHandle {
    public:
        PtrHandle(SymHeapCore &sh, TValId addr):
            ObjHandle(sh, sh.ptrAt(addr))
        {
        }
};

/// ugly, but typedefs do not support partial declarations
class ObjList: public std::vector<ObjHandle> { };

/// set of object handles
typedef std::set<ObjHandle>                             TObjSet;

/// a type used for minimal segment length (0+, 1+, ...)
typedef short                                           TMinLen;

/// enumeration of abstract object (although OK_CONCRETE is not abstract)
enum EObjKind {
    OK_CONCRETE = 0,        ///< concrete object (not a segment)
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
        /**
         * return @b kind of the target. Here @b kind means concrete object,
         * SLS, DLS, and the like.
         */
        EObjKind valTargetKind(TValId) const;

        /// tuple of binding offsets (next, prev, ...)
        const BindingOff& segBinding(TValId seg) const;

        /// set properties of an abstract object, set abstract if not already
        void valTargetSetAbstract(
                TValId                      root,
                EObjKind                    kind,
                const BindingOff            &off);

        /// set the given abstract object to be a concrete object (drops props)
        void valTargetSetConcrete(TValId root);

        /// read the minimal segment length of the given abstract object
        TMinLen segMinLength(TValId seg) const;

        /// re-initialize the minimal segment length of the given list segment
        void segSetMinLength(TValId seg, TMinLen len);

    public:
        // just overrides (inherits the dox)
        virtual void valDestroyTarget(TValId);
        virtual TValId valClone(TValId);

    protected:
        virtual bool hasAbstractTarget(TValId val) const;

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
