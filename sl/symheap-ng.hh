#include "symid.hh"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace CodeStorage {
    struct Storage;
}

namespace SH {

enum EValueTarget {
    VT_UNKNOWN,
    VT_CUSTOM,
    VT_STATIC,
    VT_ON_STACK,
    VT_ON_HEAP
};

struct Var {
    int uid;
    int inst;
};

class ICustomValue {
    public:
        virtual ~ICustomValue() { }
};

typedef std::vector<TObjId>                     TContObj;
typedef const struct cl_type                    *TObjType;
typedef boost::shared_ptr<ICustomValue>         PCustomValue;

class SymHeapCore {
    public:
        // ctor/dtor
        SymHeapCore(const CodeStorage::Storage &);
        ~SymHeapCore();
        SymHeapCore(const SymHeapCore &);
        const SymHeapCore& operator=(const SymHeapCore &);
        virtual void swap(SymHeapCore &);

    public:
        // values manipulation
        TValueId valByOffset(TValueId, int offset);
        EValueTarget valTarget(int *offset = 0);
        bool valInvalidate(TValueId);

        // objects manipulation
        TObjId objAt(TValueId at, TObjType clt);
        TObjType objType(TObjId);
        TValueId objGetValue(TObjId);
        bool objSetValue(TObjId obj, TValueId val);
        bool objKillValue(TObjId obj);
        bool objInvalidate(TObjId);

        // collecting of objects
        void gatherRootObjects(TContVal &dst, EValueTarget filter);
        void gatherLiveObjects(TContObj &dst, TValueId atAddr);
        void gatherLivePointers(TContObj &dst, TValueId atAddr);

        // address manipulation
        TValueId addrOfVar(Var);
        TValueId heapAlloc(int size);
        bool valDestroyTarget(TValueId);

        // custom values
        TValueId valWrapCustom(PCustomValue);
        PCustomValue valUnwrapCustom(TValueId);
};

}
