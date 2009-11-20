// SepLog --- Experimental implementation of separation logic based symbolic heap
//
// vim:autoindent:tw=120
//
// (c) Pert Peringer, FIT TU Brno, CZ
// (c) ........., FIT TU Brno, CZ
//
// License: GPL (GNU General Public Licence), TODO: version3?, afero?
//
// This is research prototype. Needs cleaning.

#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <queue>
#include <stack>
#include <iostream> // cout
#include <iomanip>  // setw()
#include <sstream>  // format
#include <cassert>  // assert
#include <algorithm>  // find
#define ASSERT(c) assert(c)


//#define DEBUG_CLONE

/// separation logic test namespace
namespace SepLog {

class symbolic_state; // forward declaration of main class

// basic types:
// VARIABLES and SEGMENTS == objects are identified by number (enum for type safety)
// object_id used as abstract address
//  0 == none/empty/garbage,
//  1 == nil,
enum object_id { undefined_object=0, nil=1, MAX_OBJECT_ID=4000000000UL }; // TODO UINT_MAX

// VALUES integer identification of all values
// the same values are shared (should be for pointers)
// TODO: more "undefined" values ? (if back-references needed)
enum value_id  { undefined_value=0, nil_value=1, MAX_VALUE_ID=4000000000UL };

// TYPES identification by index to global type description table
// TODO: rename, use GCC/sparse enums
enum type_id  {
    unknown_type=0,
    // general types:
    STRUCT, ARRAY, FUNCTION, PTR,
    // all basic types:
    BASIC_TYPES, // next are basic types
    // void
    VOID=BASIC_TYPES,
    // integer types
    BOOL, CHAR, SHORT, INT, LONG,
    // floating point:
    FLOAT, DOUBLE,
    // void pointer for nil ?
    TYPE_VOID_PTR,
    MAX_BASIC_TYPE_ID,          // this should be at the end of basic types
    // the space for user-defined types
    MAX_TYPE_ID=1000000L        // <<-- this should be last
};

// FIELDS (members) of structure numerical identification
// range: 0 .. number of fields in struct
enum field_id { MIN_FIELD_ID=0, MAX_FIELD_ID=100000 };


// CODE:
// command or line identification type (enum used for type safety)
enum line_id { MIN_LINE_ID=0, MAX_LINE_ID=100000000L};

// type of listsegments:
enum list_segment_length {
    _LS1_=1, // single item (== points_to in Berdine&spol) NOT USED here (==struct)
    LS0P,  // possibly empty
    LS1P   // non-empty
};


// module: names.cc
////////////////////////////////////////////////////////////////////////////
// PROGRAM VARIABLE NAMES
// should contain namespace for identification
// for example:   module1.function1.basicblock1.localvar1
//                module1.globalvar1
// TODO: is it complete?

typedef unsigned name_id;  /// index to global variable name table (TODO: wrap)

////////////////////////////////////////////////////////////////////////////
// container for program variable names (should be static?)
//
class variable_names {
    std::vector<std::string> name;  // dictionary for (non-primed?) variable names
                          // for all heaps
  public:
    // constructor
    variable_names() {
        name.push_back("***** unnamed memory object *****"); // default (name_id==0)
    }
    // default destructor is O.K.

    const std::string & operator[] (name_id i) {
        if(i>=name.size())
            throw "Bad name_id used";
        return name[i];
    }
    // add new name to table
    name_id add(std::string s) {
        name.push_back(s);
        return name.size() - 1; // return new name_id
    }
} variable_name; // singleton container for all clones


// global function interface:
const std::string & name(name_id i)     { return variable_name[i];     }
name_id add(std::string s)              { return variable_name.add(s); }
// end of module: names.cc



// module types.cc
// ======================================================================
// TYPE DESCRIPTION  (experimental, should be changed to wrapper for GCC etc)
// ======================================================================

// TYPE descriptor
struct Type {
    enum type_id base;      // basic type
    std::string id_string;  // string representation
    union {
        type_id pointer;
        type_id array;                        // size is in variable
        unsigned struct_size;
    };
    std::vector<std::pair<std::string,type_id> > struct_fields;

    Type() : base(unknown_type) {}
    Type(type_id t, std::string name) : base(t), id_string(name) {}
    // pointer
    Type(type_id target) : base(PTR), id_string("*"), pointer(target) {}
    Type(unsigned size, std::string name) : base(STRUCT), id_string(name), struct_size(size) {}
    // TODO: array

    // check for basic types
    bool is_pointer() const  { return base==PTR; }
    bool is_array() const    { return base==ARRAY; }
    bool is_struct() const   { return base==STRUCT; }

    // add structure field
    field_id add_field(const char *name, type_id typ) {
        size_t n = struct_fields.size();
        if(n>=struct_size)
            throw "Type::add_field: more than struct_size elements added";
        struct_fields.push_back(std::pair<std::string,type_id>(name,typ));
        return static_cast<field_id>(n);
    }
    unsigned struct_number_of_fields() { return struct_size; }
}; // Type

// TYPE TABLE
struct type_table {
  std::vector<Type> type; // all types in program in type table (finite)

  // constructor initializes basic types
  type_table(): type(MAX_BASIC_TYPE_ID) {
    // initialize basic types
    type[VOID]   = Type(VOID,"void");
    type[CHAR]   = Type(CHAR,"char");
    type[SHORT]  = Type(SHORT,"short");
    type[INT]    = Type(INT,"int");
    type[LONG]   = Type(LONG,"long");
    type[FLOAT]  = Type(FLOAT,"float");
    type[DOUBLE] = Type(DOUBLE,"double");
    type[TYPE_VOID_PTR] = Type(VOID);
  }
  // add pointer
  type_id add_pointer_to(type_id t) {
      type_id id = static_cast<type_id>(type.size());
      type.push_back(Type(t));
      return id;
  }
  // add structure
  type_id add_structure(const char* name, unsigned size=0) {
      type_id id = static_cast<type_id>(type.size());
      type.push_back(Type(size,name));
      return id;
  }
  // add field to structure (struct,"field",type_of_field)
  field_id add_structure_field(type_id s, const char*name, type_id field_type) {
      return type[s].add_field(name,field_type);
  }
//  type_id add_array_type(type_id basetype) {
//      type.push_back(Type(ARRAY));
//      return id;
//  }
  Type & operator[] (type_id i) {
      if(size_t(i)>=type.size())
          throw "bad type_id";
      return type[i];
  }
} types; // global table of types, should be filled before symbolic execution start





/////////////////////////////////////////////////////////////////////////////
/// reference counted shared objects
/// base for all memory_objects and values
/// this implementation should be hidden (used only to clone symbolic state)

/////////////////////////////////////////////////////////////////////////////
// internal containers wit reference counting used for symbolic state cloning

template<typename Tid, typename Tv>
class so_container; // forward

// reference counter with overflow/underflow check
// object of this class should be embedded in target type under the name _refcount
class _Refcount {
  public:
    typedef unsigned type;
  private:
    mutable type _refcount; // should be unsigned
    _Refcount&operator=(const _Refcount&); // delete default op=
  public:
    void up() {
        _refcount++;
        if( _refcount == 0 ) throw "_refcount: overflow";
    }
    void down() {
        if( _refcount == 0 ) throw "_refcount: underflow";
        _refcount--;
    }
    void reset()        { _refcount=0; }
    type get() const    { return _refcount; }
    _Refcount() : _refcount(0) {};
    _Refcount(const _Refcount &old) : _refcount(0) {};
    ~_Refcount() {
        if(_refcount)
            throw "_refcount: linked object destruction not allowed";
    };
}; // _Refcount

/////////////////////////////////////////////////////////////////////////////
/// containers of shared objects
// wrapper used for 2 indexed containers: memory_objects and values
// implements cloning, [[pointers are invisible?]]
template<typename Tid, typename T>
class so_container {
    // private abstract pointer, used only in so_container template
    class a_ptr {
        T *p;   // pointer to linked target object
        a_ptr();                                // delete this operation
        a_ptr &operator= (const a_ptr &o);      // delete this operation

        a_ptr(const a_ptr &o): p(o.p) { // copy-ctr - copy link
            p->_refcount.up();
        }
        a_ptr(T *optr) : p(optr) {      // initialize link
            p->_refcount.up();
        }
        void set(T*ptr) {               // change link
            if(ptr!=p) {
              p->_refcount.down();      // unlink old value
              p = ptr;
              p->_refcount.up();        // link new value
            }
        }
        ~a_ptr() {                      // destroy link
            p->_refcount.down();
        }
      public:
        operator T *()                  { return p; }
        operator const T *() const      { return p; }

        // our friends:
        template<typename _Ti, typename _Tx> friend class so_container;
        template<typename _Ti, typename _Tx> friend class std::pair;
        template<typename _Ti, typename _Tx> friend class std::map;
    };
    typedef Tid id_t;  // identification type
  private:
    typedef std::map< id_t, a_ptr > container_t;
    container_t m;
    //so_container(const so_container&);                  // use default copy-ctr
    so_container&operator=(const so_container&);        // delete operator=
  public:
    // iterator interface:
    typedef typename container_t::iterator              iterator;
    typedef typename container_t::const_iterator        const_iterator;
    iterator            begin()         { return m.begin(); }
    iterator            end()           { return m.end(); }
    const_iterator      begin() const   { return m.begin(); }
    const_iterator      end() const     { return m.end(); }

    // default-ctr: create empty container
    so_container()      { std::cout << "so_container()\n"; }
    // destructor: delete all links
    ~so_container()     { std::cout << "~so_container()\n"; }

    // access
    T *operator[] (id_t i)  {
        //std::cout << "so_container::operator[](" << i << ") \n";
        return m.at(i); // throws if not found
    }

    // read-only access:
    const T *operator[] (id_t i) const  {
        //std::cout << "so_container::operator[](" << i << ") const\n";
        return m.at(i); // throws if not found
    }

    // add new object, externel id allocation policy
    void create(id_t id, T *p) {
//        std::cout << "so_container::create(" << id << ",ptr) \n";
        if(p->_symbolic_state_refcount()!=0)  // should be fresh object
            throw "so_container::add: refcount!=0";
        if(m.count(id)!=0)
            throw "so_container::create: already allocated id used";
        else m.insert(typename container_t::value_type(id, p));
    }

    // remove object pointer from container
    void erase(id_t id) {
        std::cout << "so_container::erase(" << id << ") \n";
        if(m.count(id)!=1)
            throw "so_container::erase: bad id";
        else
            m.erase(id);
    }

    // clone object with id
    void clone(id_t id) {
#ifdef DEBUG_CLONE
        std::cout << "so_container::clone(" << id << ") \n";
#endif
        T *v = m.at(id);
        if(v->_writable())  // already cloned?
            return;
        v = v->clone();     // make a copy, reset refcount in copy
        m.at(id).set(v);    // old refcount down, new up
    }

    // number of
    size_t size() const { return m.size(); }
    // exist
    bool exist(id_t i) const { return m.count(i)!=0; }
}; // so_container


// module values.cc

/////////////////////////////////////////////////////////////////////////////
// (abstract) VALUE OF VARIABLE
/////////////////////////////////////////////////////////////////////////////
// TODO: add expression and eval(), type(), is_abstract()
// TODO: use inheritance+templates? value<type> ?
//
//
// type:
//       - pointer: points-to var (exact location, type of pointer depends on variable)
//       - pointer: points-to ls
//       - TODO: pointer: points-to array/item (special array:index address?) + array-segments?
//       - long
//       - double
//
// values not mutable  (= same value_id means same/constant (maybe abstract) value)
//
/// class value = base of value hierarchy
class value {
    _Refcount _refcount;
    template<typename _Tid, typename _T> friend class so_container;

    // cloning implementation
    virtual value *_clone() const = 0;  // exact copy
    // cloning interface (used for COW in container)
    value *clone() const {
#ifdef DEBUG_CLONE
        std::cout << "@clone_value" << this_value << "\n";
#endif
        value *new_copy = _clone();
        // done by _Refcount copy ctr:  new_copy->_refcount.reset();
        return new_copy;
    }
    value *clone(value_id id) const {
#ifdef DEBUG_CLONE
        std::cout << "@clone_value" << this_value << "->" << id << "\n";
#endif
        value *new_copy = _clone();
        // done by _Refcount copy ctr:  new_copy->_refcount.reset();
        new_copy->this_value = id;
        return new_copy;
    }

  protected:
    value_id this_value;                // handle,  can be changed after cloning by copyctr
    // not exact type of PTR value (problem with aliasing)
    // target of PTR is defined by VARIABLE type not address (VALUE)
    // TODO: add more abstract values
    enum value_type { UNKNOWN, PTR, STRUCT, ARRAY, INT, DBL, INTEXPR, DBLEXPR } _type;
    private:
    // two-level refcounting:
    //  a) which variables/segments have the value
    typedef std::set<object_id>                         object_id_container_t;
    typedef object_id_container_t::iterator             used_by_objects_iterator;
    typedef object_id_container_t::const_iterator       used_by_objects_const_iterator;
    object_id_container_t used_by_objects; // which variables/segments have this (=the same) value (size=refcount)
                                        // this is important for pointers, implements EQ relation
                                        // TODO: solve problem with integers/floats and UNIQUE id
    typedef std::set<value_id>                          value_id_container_t;
    typedef object_id_container_t::iterator             used_by_values_iterator;
    typedef object_id_container_t::const_iterator       used_by_values_const_iterator;
    //  b) how many structured values use the value
    value_id_container_t used_by_values;       // references to enclosing structured values
    // each _defined_ value has handle, undefined parts of struct/array do not exist

  protected:
    value(value_id h, value_type t=UNKNOWN): this_value(h), _type(t) {}
    virtual ~value() {
        // TODO: check if clean
        ASSERT(used_by_objects.empty());
        ASSERT(used_by_values.empty());
        if(refcount()!=0) std::cerr << "refcount problem\n"; // can not throw in destructor
    }

  private:
    friend class symbolic_state;
    // creation
    static value *create_ptr(value_id h);
    static value *create_struct(value_id h, type_id t); // TODO change 2nd param
    static value *create_int(value_id h,long v);
    // TODO: double, bool?, array

    // reverse link to var
    void link_object(object_id v)       { ASSERT(_writable()); used_by_objects.insert(v); }
    void unlink_object(object_id v)     { ASSERT(_writable()); used_by_objects.erase(v); }
    // part_of_counter update
    void link_value(value_id h)         { ASSERT(_writable()); used_by_values.insert(h); }
    void unlink_value(value_id h)       { ASSERT(_writable()); used_by_values.erase(h); }

  public:
    virtual void print() const = 0;
    bool is_pointer() const     { return _type==PTR; }
    bool is_struct() const      { return _type==STRUCT; }
    bool is_array() const       { return _type==ARRAY; }
    bool is_scalar() const      { return !(is_struct()||is_array()); }
//    value_type get_type() const   { return _type; }

    size_t refcount() const { return used_by_objects.size()+used_by_values.size(); }
    bool unused() const { return refcount()==0; }

    // inter symbolic_state sharing
// TODO: better names
    unsigned _symbolic_state_refcount() const { return _refcount.get(); }
    bool _is_shared() const { return _refcount.get() > 1; }
    bool _writable() const { return !_is_shared(); }

}; // class value

/////////////////////////////////////////////////////////////////////////////
// value of the structure
// contains value_id references to fields with known value
// TODO: add iterators over fields
class value_struct : public value {
    type_id type;
    typedef std::map<field_id,value_id> field_container_t;
    field_container_t m; // maps to member values   TODO: use vector?
    unsigned number_of_fields;
  public:
    field_id nfields() const { return static_cast<field_id>(number_of_fields); }
    typedef field_container_t::iterator iterator;
    typedef field_container_t::const_iterator const_iterator;
  private:
    friend class value;
    value_struct(value_id h, type_id t):
         value(h,STRUCT),
         type(t),
         number_of_fields(types[t].struct_number_of_fields())
    { }
    ~value_struct() {
        // TODO: check if unregistered OR unregister all members?
    }
    // use default copy-ctr
    value *_clone() const { return new value_struct(*this); }
  public:
    // TODO: add iterator for fields
    // TODO check, add const
    value_id get_field_value(field_id i) const {
//        std::cout << "get_field_value(" << i << ") = " << ( (m.count(i)!=0)?m.at(i):0 ) << "\n";
        if(i>nfields())
            throw "value_struct::get_item_value:  bad field id";
        if(m.count(i)!=0)  return m.at(i);
        else               return undefined_value;
    }
    void update_field(field_id i, value_id id) {
//        std::cout << "update_field(" << i << ", " << id << ")\n";
        if(!_writable())
            throw "value_struct::update_item: shared val modified !!!";
        if(i>nfields())
            throw "value_struct::update_item:  bad field id";
        m[i] = id;
    }

    // return all fields containing given value
    std::vector<field_id> search_fields(value_id id) const {
        std::vector<field_id> r;
        for(field_container_t::const_iterator i=m.begin(); i != m.end(); ++i) {
             if(i->second==id)
                r.push_back(i->first);
        }
        if(r.size()==0)
            throw "value_struct::search_field:  id not found";
        return r;
    }

    // print value of struct
    void print() const {
        std::cout << "val#" << this_value << ": ";
        std::cout << "struct{ " << number_of_fields << " fields: " ;
        for(unsigned i=0; i<number_of_fields; ++i) {
            std::cout << "[" << i << "]=";
            value_id id = get_field_value(static_cast<field_id>(i));
            if(id==undefined_value)
                std::cout << "*undefined* ";
            else
                std::cout << "val#" << id << " ";
        }
        std::cout << "}";
    } // print
};

/////////////////////////////////////////////////////////////////////////////
// value of the pointer == address
// pointer to global variable is unique in all symbolic_state instances
// no type of target stored -- allows reinterpret_cast (needs CHECK)
class value_address : public value {
    object_id addr;       // points to variable or any segment
  private:  // only friends can create values
    friend class value;
    value_address(value_id h): value(h,PTR), addr(undefined_object) { }
    ~value_address() {
        // TODO: check if unregistered OR unregister here?
    }
    // use default copy-ctr, which does refcount reset
    value *_clone() const { return new value_address(*this); }
  public:
    object_id points_to() const  { return addr; }
    void set_target(object_id o) { addr=o; }
    void print() const {
        std::cout << "val#" << this_value << ": ";
        if(this_value == 1)
            std::cout << "nil";
        else
            std::cout << "pointer_to_obj#" << addr ;
    }
};

/////////////////////////////////////////////////////////////////////////////
// integer value
// PROBLEM with unique int values:
//   a) how to allocate the same handle (or allow more?)
//   b) what to do if EQ(a,b) and interval values
// TODO: use intervals AND predicates
class value_int : public value {
    long v;
  private:  // only friends can create values
    friend class value;
    value_int(value_id h, long va): value(h,INT), v(va) { }
    ~value_int() {
        // TODO: check if unregistered OR unregister here?
    }
    // use default copy-ctr
    value *_clone() const { return new value_int(*this); }
  public:
    int val() const { return v; }
    void print() const {
        std::cout << "val#" << this_value << ": ";
        std::cout << "int(" << v << ")" ;
    }
};

// static create_* methods of value
value *value::create_ptr(value_id h)                    { return new value_address(h); }
value *value::create_struct(value_id h, type_id t)      { return new value_struct(h,t); }
value *value::create_int(value_id h,long v)             { return new value_int(h,v); }



// =============================================================
/// abstract memory object
// the base for all memory located objects: variables, segments(=dynamic_variables) etc.
// each object has address, can be pointed by pointer value
//
// Properties:
//  - this_object: abstract address == position in abstract memory
//  - target_of:   it is target of pointers with the same value
//  - _type:       variable, list segment, tree segment etc...
class object {
    _Refcount _refcount;
    template<typename _Tid, typename _T> friend class so_container;
    // cloning implementation
    virtual object *_clone() const = 0;

  protected:
    object_id this_object;    /// abstract logical address

    // size_t refcount() const { LOGICAL: pointed_by + num_of_values }

    // cloning interface (used for COW)
    friend class symbolic_state;
    // clone
    object *clone() const {
#ifdef DEBUG_CLONE
        std::cout << "@clone_object" << this_object << "\n";
#endif
        object *new_copy = _clone();
        // done by _Refcount copy-ctr:  new_copy->_refcount.reset();
        return new_copy;
    }
    // clone to new position, needs corrections
    object *clone(object_id id) const {
#ifdef DEBUG_CLONE
        std::cout << "@clone_object" << this_object << "->" << id << "\n";
#endif
        object *new_copy = _clone();
        // done by _Refcount copy ctr:  new_copy->_refcount.reset();
        new_copy->this_object = id;
        return new_copy;
    }

    // TODO: try to use type_info?
    enum object_type { UNKNOWN, VARIABLE=1, SLS, DLS };
  private:

    // TODO: name: pointed_by|target_of|other?
    value_id target_of;     // id of value pointing here [PROBLEM: type* SOLVED: type in variable, not value]
                            // ====> we can identify all variables pointing at the same location

    // default constructor not allowed (no objects without unique ID)
    object(); // delete

  public:
    // TODO: maybe pass the size of allocation and do the ID inside ctr ?
    object(object_id id, object_type t): this_object(id), target_of(undefined_value), _type(t) {}
    virtual ~object() {
        // TODO: check
        if(is_pointed())
            std::cerr << "~object: target of pointer deleted\n"; // can not throw in destructor
    }

    // sharing TODO: better names
    unsigned _symbolic_state_refcount() const { return _refcount.get(); }
    bool _is_shared() const { return _refcount.get() > 1; }
    bool _writable()  const { return !_is_shared(); }

    // basic types
//    object_type get_type() const { return type; }
    bool is_sls()          const { return _type==SLS; }
    bool is_variable()     const { return _type==VARIABLE; }
    bool is_abstract()     const { return !(is_variable()); }

    virtual void print() const = 0;

    virtual value_id get_value() const = 0;
    virtual void set_value(value_id v) = 0;
    // target of some pointer?
    bool      is_pointed() const { return target_of!=0; }
    value_id  pointed_by() const { return target_of; }
    void set_pointed_by(value_id ptrval) {
        ASSERT(_writable());
        target_of = ptrval; // should be pointer value
    }

  private:
    object_type _type;  // for basic type check
};

// module variables.cc
/////////////////////////////////////////////////////////////////////////////
// VARIABLE
/// variable description
/// identification by number
// TODO: variable description can be part of more heaps?
class variable : public object {

    // use bits for various boolean flags (TODO: consider bitfields?)
    enum flags_enum { VAR_PROGRAM=1, VAR_LOCAL=2, VAR_ALLOCATED=4, VAR_POINTER=8 };

    unsigned flags;     //
//    int last_op_line; // for debugging, maybe for some heuristics // PROBLEM:sharing

    const type_id type; // TYPE OF VARIABLE (can not be changed)

  public:
    // nesting of variables
    // only part of items can be concretized
    // TODO: if part of structure/array, we use offset [[[this_id-offset==enclosing_var]]]
    // needs correct allocation (PROBLEM: realy big arrays)
    object_id part_of;  // enclosing VARIABLE id -- for structure/array
                        // this is used only for pointed fields     [[[ ptr = &s.field ]]]
                        // SINGLE-LEVEL only!
    field_id offset;    // offset in structure (0..n) // TODO can be computed from associated struct value
                        // SINGLE-LEVEL (only value should be structured due to sharing)
    typedef std::set<object_id>         parts_container_t; // TODO: use vector/map ??
    parts_container_t parts;            // fields
    typedef parts_container_t::iterator         parts_iterator;
    typedef parts_container_t::const_iterator   parts_const_iterator;
  private:

    //TODO: link to stackframe == special variable

    // abstract sizeof -- TODO: is part of TYPE
    int size;           // number of locations, >1 for array/struct,  0==UNKNOWN

    // program variable name ("::var128", "func::s.next"), constant
    // TODO: use GCC interface and use pointer?
    int name_id;        // if named: name table index (var->string)

    value_id val;           // id of the VALUE of the variable
                            // same value means equal variables (type independent)
                            // TODO: possible problems: {ptr2int, int++, int2ptr}

    // TODO
//    eq_t ne;                // NEQ: not equal set (v1 != v2) v1.ne={v2} and v2.ne={v1}
    // TODO use external BOOST::bimap<object_id,object_id> for this

    variable *_clone() const { return new variable(*this); }
  public:

    // create NEW VARIABLE of type t at position v
    // v should be freshly allocated
    // we expect correct reservation of IDs for struct/array
    variable(object_id v, type_id t):
        object(v, VARIABLE), // index to container
        flags(0),
        type(t),
        part_of(undefined_object), //offset(0),
        name_id(0),
        val(undefined_value)            // special UNDEFINED value (type-independent)
        {}

    ~variable() {
        // TODO: check if unregistered
    }

    // is value defined?
    bool     has_value() const          { return val!=0; }
    // get/set (lowlevel interface)
    value_id get_value() const          { return val; }
    void     set_value(value_id v)      {
        ASSERT(_writable());
        val=v;
    }


    // TODO: consistency
    // is equal to other variable?
    bool is_equal(variable *v2) const {
        if(v2==this)
            throw "bad test: a=a";
        return val==v2->val; // the same value_id
    }

    // check variable type
    bool is_struct()  const   { return types[type].is_struct(); }
    bool is_array()   const   { return types[type].is_array(); }
    bool is_pointer() const   { return types[type].is_pointer(); }
    type_id get_type() const { return type; }

    /// after assigning the name can not be changed
    // no need to check if is writable
    void set_name(const std::string & name) {
        name_id = variable_name.add(name);
    }
    bool has_name() const { return (name_id!=0); }
    const std::string& name() const {
        return variable_name[name_id];
    }

    // print variable id, name, value
    void print() const {
        std::cout << "var" << this_object;
        if(has_name())
            std::cout << "_" << name();
        // TODO move to ...
        std::cout << " = ";
        if(val==0)
            std::cout << "undefined";
        else if(val==nil_value)
            std::cout << "nil";
        else
            std::cout << "value#" << val;
        std::cout << " ";
    }

 protected:
    // default variable & operator= (const variable &v) { }
    // CLONE: // use default variable(const variable &v);
}; // class variable



// module list_segments.cc
// implementation of all list-segments: double/single linked, 1, 0+, 1+, ...
//

/////////////////////////////////////////////////////////////////////////////
// list_segment is base class for all list segments (singly/double-linked)
class list_segment : public object {
    //void register_ls(value*ptr)   { ptr->register_ls(this); }
    //void unregister_ls(value*ptr) { ptr->unregister_ls(this); }
  public:
    list_segment(object_id id, object_type t): object(id, t) {}

};

/////////////////////////////////////////////////////////////////////////////
/// simple implementation of basic singly linked list segment
// hval = & ls(nextval, Xtype, nextfield_id, +info)
//          X         X
//  H -o-> [ ]  o--> [ ]
//         [n]--+    [N]--o-->nil
//              |
//          B --+
//  H is variable of (struct X*) type (Head of ls)
//  B variable of (struct X*) type  OR  nil
//  nil is special universal any-pointer-type pseudovariable (and associated value "points-to")
//
// empty list segment:   ls(Aval,Bval), A==B  (==same value) - Never used explicitly
//
// Examples:
// ls(Aval,nil) means tail from A to end
// ls(Aval,Bval), ls(Bval,nil) two list segments concatenated
// ls(Aval,Bval), ls(Bval,Aval) circular list?

// TODO: never use empty ls ! (they can be eliminated)

// single linked list segment, 3 variants: possibly empty, nonempty, 1 item
class slist_segment : public list_segment {
    list_segment_length t;

    // value of the type: pointer to structure
    value_id next_val;  // refcounted (it is pointed to by this)

    // Lambda
    type_id stype;      // type of structure           (like variable type)
    field_id nextid;    // ->next item identification (= offset in structure)
    value_id struct_value_id; // prototype item value, if SVALs (seplog)
    // TODO: add additional info: constraints, etc

    ////slist_segment(const slist_segment&) default;
    // disable op=
    slist_segment&operator=(slist_segment&);

  private:
    // constructors:
    slist_segment(object_id id,
                  list_segment_length type,
                  value_id b,
                  type_id st, field_id item):
        list_segment(id,SLS),
        t(type),
        next_val(b),
        stype(st),       // type of structure
        nextid(item),    // item number in structure   TODO-check
        struct_value_id(undefined_value)  // if(!=0) contains stored values shared by each listitem (headptr,...)
    {
        // TODO: register value b
        //current_context->val(b)->link(this_object);
    }
  public:
    ~slist_segment() {
        // TODO: unregister from values
        //current_context->val(b)->unlink(this_object);
    }

    // get head pointer value
    value_id    head()          const { return pointed_by(); }

    // next pointer value at the end of ls
    value_id next() const { return next_val; }
    value_id get_value() const { return next_val; }
    void set_value(value_id n) {
        ASSERT(_writable());
        next_val = n;
    }
    void set_next(value_id n) {
        set_value(n);
    }

    // next-field-id
    field_id    get_nextfield()     const { return nextid; }

    // length type of segment: possibly-empty = 0+, non-empty = 1+
    list_segment_length get_length() const { return t; }
    void        set_length(list_segment_length newlen) {
        ASSERT(_writable());
        t=newlen;
    }

    // list item structure
    type_id     target_struct_type() const { return stype; }
    value_id    target_struct()      const { return struct_value_id; }

    // clone list segment
    slist_segment *_clone() const { return new slist_segment(*this); }

    // create new ls
    static slist_segment * create(object_id id,
                  list_segment_length type,
                  value_id nextptr,
                  type_id st, field_id item) {
        return new slist_segment(id, type, nextptr, st, item);
    }

    void print() const {
        std::cout << "slist_SEGMENT" << this_object;
        std::cout << "(";
        if(t==LS0P)
            std::cout << "type=ls0+";
        else if(t==LS1P)
            std::cout << "type=ls1+";
        else
            std::cout << "type=" << t;
        std::cout << ", headval=" << head();
        if(next()==nil_value)
            std::cout << ", nextval=NULL";
        else
            std::cout << ", nextval=" << next();
        std::cout << ", lambda=" << stype << "," << nextid << ")" ;
        std::cout << ")";
    }
}; // slist_segment


#if 0
/////////////////////////////////////////////////////////////////////////////
/// simple implementation of basic dlist segment
// ls(A,B,C,D,X)
// TODO: derive empty list segment ?
//
class dlist_segment : public list_segment {
    object_id link[4]; // 4 variables
    type_id typ;

  public:
    // constructor:
    dlist_segment(object_id a, object_id b, object_id c, object_id d) {
        link[0] = a;
        link[1] = b;
        link[2] = c;
        link[3] = d;
    }

    // change:
    void assign(object_id a, object_id b, object_id c, object_id d) {
        link[0]=a;
        link[1]=b;
        link[2]=c;
        link[3]=d;
    }

    object_id operator[] (int i) const {
        if((unsigned)i >= 4)
            throw "dlist_segment[i]: index out of range";
        return link[i];
    }

    bool is_empty() {
        return(link[0]==link[2] && link[1]==link[3]);
    }

    dlist_segment *clone() const { return new dlist_segment(*this); }
};
// TODO: add ls_pool + allocator + ls_id  (smaller on 64bit,faster)

#endif



// module state.cc
/////////////////////////////////////////////////////////////////////////////
/// contains modified symbolic heap $(\Pi,\Sigma)$
//  can be created using operator new only   (TODO: check this)
//
// TODO: add stacklevels = (vars/segments,neq,values+eq,stack, etc?)
// TODO: can NEQs be removed?
//
class symbolic_state { // SH = (equations,predicates)
    static unsigned counter; // number of symbolic_state instances created
    static unsigned long _object_allocator_index;
    static object_id allocate_object(unsigned n=1) {
        object_id id = static_cast<object_id>(_object_allocator_index);
        _object_allocator_index += n; // skip n positions
        // TODO: check overflow
        return id;
    }
    static unsigned long _value_allocator_index;
    static value_id allocate_value(unsigned n=1) {
        value_id id = static_cast<value_id>(_value_allocator_index);
        _value_allocator_index += n;
        // TODO: check overflow
        return id;
    }
/// abstract memory
// location==object_id (PROBLEM: TODO: aliasing struct vs first item? -- solve by typechecking)
// -- set of variables and abstract objects (list and other segments)
// -- object_id is equal to abstract memory address
// -- unknown/garbage and nil are special
    so_container<object_id, object> objects;  // container of objects/locations
// -- set of object values
    so_container<value_id, value> values; // values of variables and abstract_segments
// -- stack
    std::stack<line_id> stack;  // stack of lines to return from CALL

// iterators over containers:
    typedef so_container<value_id,value>::const_iterator        value_const_iterator;
    typedef so_container<value_id,value>::iterator              value_iterator;
    typedef so_container<object_id,object>::const_iterator      object_const_iterator;
    typedef so_container<object_id,object>::iterator            object_iterator;

    unsigned num; // unique id of state
    unsigned cloned_from;
    mutable bool done;  // already executed

  public:
    // mark if already analyzed/executed
    void mark_done() const { done = true; }
    bool is_done() const { return done; }

// TODO: abstract array?
// possibilities:
//  use single id and use special pointer values ("relative" to array A)
//  or reserve N locations (N is size of array)
//  or use abstract array segments ?
//    object_id new_array(type_id t, int n, std::string name) {
//          object_id i = new_var(t,name); // array===array[0] ?
//          // TODO: set type
//          for(int i=1; i<n; i++)
//              new_var(t);
//          return i;
//    }


    // clone operation uses default copy_ctr
    symbolic_state * clone() const {
        symbolic_state *p = new symbolic_state(*this);  // sharing all content of containers, use refcount
        p->num = counter++;     // unique id for each state
        p->cloned_from = num;   // dependency
        p->done = false;        // not executed yet
#ifdef DEBUG_CLONE
        std::cout << "@@symbolic_state"<< p->cloned_from << "::clone() ==> "<< p->num <<"\n";
#endif
        return p;
    }
    // clone only selected set of objects [and all values used]
    symbolic_state * clone_partial(const std::list<object_id> &what) const {
#ifdef DEBUG_CLONE
        std::cout << "@@symbolic_state::clone_partial(set)\n";
#endif
        throw "clone_partial(): todo";
    }
    // create initial state
    static symbolic_state *create() {
        std::cout << "@@symbolic_state::create()\n";
        return new symbolic_state();
    }
    // TODO: add "derived from symbolic_state" link -- WARNING: merge problem = use set?

  private:
    // use default copy_ctr
    // symbolic_state(const symbolic_state &sh); // used in clone()

    // default constructor for initial state only, all other are clones
    symbolic_state(): num(counter++), cloned_from(0), done(false) {
        cloned_from = 0;
        if (num!=0)
            throw "symbolic_state() creation problem";

        //var_new(TYPE_UNKNOWN,"garbage"); // object_id==0 is not allocated
        var_new(TYPE_VOID_PTR,"nil");
        var_assign_pointer_at(nil, nil); // variable_nil, value_ptr_to_nil
        if(objects.size()!=1 || values.size()!=1)
            throw "bad initial symbolic_state construction";
        // TODO: add all global variables here? NO
    }

  private:

    // N:1  N objects can have the same value
    void obj_link_val(object_id o, value_id v) { // create association object<->value
        ASSERT(obj(o)->_writable());
        ASSERT(val(v)->_writable());
        obj(o)->set_value(v);
        val_refcount_up(v,o);   // add to container
    }
    void obj_unlink_val(object_id o) {  // remove association
        value_id v = obj(o)->get_value();
        if(v==undefined_value)          // already unlinked
            return;
        ASSERT(obj(o)->_writable());
        ASSERT(val(v)->_writable());
        val_refcount_down(v,o);       // remove from container
        obj(o)->set_value(undefined_value);
    }
    void obj_unlink_val_partial_del(object_id o) {  // remove backlink only
        // will not write to object
        value_id v = obj(o)->get_value();
        if(v==undefined_value)          // already unlinked
            return;
        ASSERT(val(v)->_writable());
        val_refcount_down(v,o);       // remove from container
        // delete
        if(val(v)->unused())
            val_delete(v);
    }
    // TODO: replace?
    void obj_unlink_val_del(object_id o) {  // remove association
        value_id v = obj(o)->get_value();
        if(v==undefined_value)          // already unlinked
            return;
        ASSERT(obj(o)->_writable());
        ASSERT(val(v)->_writable());
        val_refcount_down(v,o);       // remove from container
        obj(o)->set_value(undefined_value);
        // delete
        if(val(v)->unused())
            val_delete(v);
    }

    // 1:1 unique pointer value points to given single object
    void val_link_obj(value_id v, object_id o) { // create association object<->value
        if(!val(v)->is_pointer())
            throw "val_link_obj(p): only pointer can link memory object";
        ASSERT(obj(o)->_writable());
        ASSERT(val(v)->_writable());
        obj(o)->set_pointed_by(v);
        val_ptr(v)->set_target(o);
    }
    void val_unlink_obj(value_id v) { // remove association
        if(!val(v)->is_pointer())
            throw "val_unlink_obj(p): only pointer can link memory object";
        object_id o = val_ptr(v)->points_to();
        if(o==undefined_object)          // already unlinked
            return;
        ASSERT(obj(o)->_writable());
        ASSERT(val(v)->_writable());
        obj(o)->set_pointed_by(undefined_value);
        val_ptr(v)->set_target(undefined_object);
    }

  private:
    // access object
    object              *obj(object_id i)       { return objects[i]; }
    const object        *obj(object_id i) const { return objects[i]; }
    // access variable:
    variable            *var(object_id i)       { return dynamic_cast<variable*>(objects[i]); }
    const variable      *var(object_id i) const { return dynamic_cast<const variable*>(objects[i]); }
    // access single list segment
    slist_segment       *sls(object_id i)       { return dynamic_cast<slist_segment*>(objects[i]); }
    const slist_segment *sls(object_id i) const { return dynamic_cast<const slist_segment*>(objects[i]); }

    // access value
    value               *val(value_id i)        { return dynamic_cast<value*>(values[i]); }
    const value         *val(value_id i) const  { return dynamic_cast<const value*>(values[i]); }
    // access struct value
    value_struct        *val_s(value_id i)       { return dynamic_cast<value_struct*>(values[i]); }
    const value_struct  *val_s(value_id i) const { return dynamic_cast<const value_struct*>(values[i]); }
    // access pointer value
    value_address        *val_ptr(value_id i)       { return dynamic_cast<value_address*>(values[i]); }
    const value_address  *val_ptr(value_id i) const { return dynamic_cast<const value_address*>(values[i]); }

    // TODO val_struct, val_array

  public:
    // get state serial number (for identification)
    unsigned state_number() const { return num; }

    //////////////////////////////////////////////////////////////////
    // VARIABLES
    /// create new variable

    /// create variable at fresh location
    object_id var_new(type_id t);
    object_id var_new(type_id t, const std::string &name);
    /// create variable at fixed location
    object_id var_new_at(object_id at, type_id t);
    /// destroy variable
    void obj_delete(object_id x1);

    //////////////////////////////////////////////////////////////////
    // VALUES
    // low-level operations
    value_id var_get_value(object_id x1) const { return var(x1)->get_value(); }
    void obj_set_value(object_id x1, value_id h) {
        obj_COW(x1);
        obj(x1)->set_value(h);
    }
    void var_set_value(object_id x1, value_id h) {
        ASSERT(obj(x1)->is_variable());
        obj_set_value(x1,h);
    }
    bool var_has_value(object_id x1) const;
    value_id val_new_ptr(object_id x);
    value_id val_new_struct(type_id t);
    bool val_is_struct(value_id h) const    { return val(h)->is_struct(); }
    bool val_is_array(value_id h) const     { return val(h)->is_array(); }
    bool val_is_writable(value_id h) const  { return val(h)->_writable(); }

  // TODO: refcount operations
    void val_struct_unlink_field(value_id val, field_id f);
    void val_struct_link_field(value_id val, field_id f, value_id newfieldval);


    // EQUALITIES (\Pi)
    //void add_eq(object_id x1, object_id x2);    // add    x1 == x2
    //void remove_eq(object_id x1, object_id x2); // remove x1 == x2
    //void remove_eq(object_id x1);           // remove any equality x1 == any_x

    // test x1 == x2
    // WARNING: expects the same value sharing
    bool var_is_equal(object_id x1, object_id x2) const;
    // test any x1 == any_x
    // WARNING this refcount includes enclosing structured values
    bool var_is_equal_any(object_id v) const;

#ifdef xTODOx
// removed
    // NONEQUALITIES (\Pi)
    void add_ne(object_id x1, object_id x2);    // add     x1 != x2
    void remove_ne(object_id x1, object_id x2); // remove  x1 != x2
    void remove_ne(object_id x1);           // remove any inequality x1 != any_x

    bool is_neq(object_id x1, object_id x2) const;    // test
    bool is_neq(object_id x1) const;              // test any x1 != any_x
#endif

    //////////////////////////////////////////////////////////////////
    // POINTER   ***** WARNING: this is not points_to [Berdine&spol]

    // test if ( x1 == &x2 ) holds
    bool var_points_to(object_id x1, object_id x2) const {
#if 0
        const variable *v2 = var(x2);
        const value *h = val(v2->pointed_by());
        return h->variables.count(x1)>0;
#else
        value_id x1val = var_get_value(x1);
        value_id x2pointed_by = obj_pointed_by(x2);
        return x1val == x2pointed_by;
#endif
    }
    // test if ( x1 == &anyobject ) holds
    bool var_is_pointer(object_id x1)   const { return var(x1)->is_pointer(); }
    // test if value points to anyobject
    bool val_is_pointer(value_id v)     const { return val(v)->is_pointer(); }

    // return target of pointer
    object_id var_points_to(object_id x1) const {
        value_id h = var(x1)->get_value();
        return val_points_to(h);
    }

    // return target of pointer value
    object_id val_points_to(value_id v) const {
        const value_address *p = val_ptr(v);
        if(p==0)
            throw "val_points_to: not a pointer value";
        return p->points_to();
    }

    // return value pointing at variable x
    // 0 means "not pointed"
    value_id obj_pointed_by(object_id x) const {
        return obj(x)->pointed_by();
    }

    //////////////////////////////////////////////////////////////////
    // LIST SEGMENTS
    //dlist_segment * add_dls(object_id x1, object_id x2, object_id x3, object_id x4); // add

    // sls1p,sls0p
    object_id   sls_create(list_segment_length t, value_id nextptr, type_id st, field_id nextfield);
    object_id   sls1p_create_at(object_id i, value_id nextptr, type_id st, field_id nextfield);
    void        sls_delete(object_id ls); // delete list segment

    // concretize: create struct and rest of ls
    void sls_concretize(object_id ls);

    // return next pointer value
    value_id sls_get_next(object_id ls) const {
        value_id h = sls(ls)->next();
        return h;
    }
    // set next pointer value
    void sls_set_next(object_id ls, value_id next) {
        sls(ls)->set_next(next);
    }

    // change the state to eliminate empty listsegment
    value_id sls_eliminate_empty(object_id ls0) {
        std::cout << "sls_eliminate_empty(" << ls0 << ")\n";
        obj_COW(ls0); // TODO: remove - it is not needed ?
        // we need to join 2 values:
        value_id headptr = sls(ls0)->pointed_by();
        value_id next = sls(ls0)->next();
        ASSERT(headptr!=next);
        // will write
        val_COW(headptr);
        val_COW(next);

        value *h = val(headptr);
        // relink all objects (variables, segments, ...)
        for(value::object_id_container_t::iterator i = h->used_by_objects.begin();
                i != h->used_by_objects.end(); ++i ) {
            object_id v = (*i);
            obj_COW(v);
            obj_set_value(v,undefined_value);   // do not touch container!
            var_assign_value(v,next);           // make backlink, too
        }
        h->used_by_objects.clear(); // remove all links to objects

        // re-link all structured values
        for(value::value_id_container_t::iterator i = h->used_by_values.begin();
            i != h->used_by_values.end(); ++i ) {
            value_id sv = (*i);
            val_COW(sv);
            _val_change_subvalue(sv,headptr,next); // no unlink
        }
        h->used_by_values.clear(); // remove all links to structured values

        // unlink pointer<->target
        val_unlink_obj(headptr);
        // remove headptr value (now unused)
        val_delete(headptr);
        // unlink ls0<->nextval
        obj_unlink_val(ls0);
        // now we can delete listsegment
        sls_delete(ls0);
        return next;
    }

    // test if x1 points to any ls
    // RENAME? is_pointer_to_segment
    bool var_points_to_ls(object_id x) {
        if(!var_is_pointer(x))
            throw "var_points_to_ls: non-pointer used";
        object_id target = var_points_to(x); // target
        return obj_is_sls(target);
    }
    // test if x is slist segment
    bool obj_is_sls(object_id x) {
        return obj(x)->is_sls();
    }
    // test if x is slist segment
    bool obj_is_var(object_id x) {
        return obj(x)->is_variable();
    }
    // return LS0P, LS1P
    list_segment_length sls_length(object_id o) const { return sls(o)->get_length(); }
    void sls_set_length(object_id o, list_segment_length l) {
        obj_COW(o);
        sls(o)->set_length(l);
    }

    bool obj_is_nonempty(object_id x) const {
        if(obj(x)->is_sls()) {
            if(sls_length(x)==LS0P) return false;
        }
        return true; // any variable, LS1P
    }


    type_id var_type(object_id o) {
        return var(o)->get_type();
    }
    type_id sls_type(object_id o) {
        return sls(o)->target_struct_type();
    }
    field_id sls_next_id(object_id o) {
        return sls(o)->get_nextfield();
    }

    //////////////////////////////////////////////////////////////////
    // HEAP OPERATIONS

    // find all variables of given struct type
    // TODO: substructures [[[ search for values instead ]]]
    std::vector<object_id> find_all_struct_variables(type_id st) {
        std::vector<object_id> r;
        for(so_container<object_id,object>::const_iterator i=objects.begin(); i!=objects.end(); ++i) {
            object_id o = i->first;
            if(!obj(o)->is_variable()) continue;
            if(!var_is_struct(o)) continue;
            type_id t = var_type(o);
            if(t==st)
                r.push_back(o);
        } // for
        return r;
    }



    // find all pointer values
    // TODO: filter by target type? Problem: type-converted pointer values
    std::vector<value_id> find_all_pointer_values() {
        std::vector<value_id> r;
        for(value_const_iterator i=values.begin(); i!=values.end(); ++i) {
            value_id vid = i->first;
            if(val(vid)->is_pointer())
                r.push_back(vid);
        } // for
        return r;
    }

    // find all pointer values to given struct
    // target is valid structure variable
    // all pointer targets are concretized, all such structure parts are available
    // TODO: check if covers all possibilities
    std::vector<value_id> find_all_pointers_to_struct(object_id target) {
        ASSERT(var_is_struct(target));
        variable * s = var(target);
        ASSERT(s->part_of == undefined_object); // TODO: does not work for partial structs

        std::vector<value_id> r;

        value_id pv = obj_pointed_by(target);
        if(pv!=undefined_value) // there is pointer to whole struct
            r.push_back(pv);
        // search for pointers at (concretized) struct parts
        for(variable::parts_const_iterator i=s->parts.begin(); i != s->parts.end(); ++i) {
            pv = obj_pointed_by(*i);
            if(pv!=undefined_value) // there is pointer to nested variable
                r.push_back(pv);
        } // for
        return r;
    }

    // count the variables using value
    size_t val_var_count(value_id x) {
        return val(x)->used_by_objects.size();
    }
    // count the variables using value
    size_t val_val_count(value_id x) {
        return val(x)->used_by_values.size();
    }

    // return object with given value iff unambiguous
    // else return undefined_object
    object_id val_var_unique(value_id x) {
        value *v = val(x);
        if(v->used_by_objects.size()!=1) // value not used by single object
            return undefined_object;
        object_id o = *(v->used_by_objects.begin()); // contains single object id
        return o;
    }

    // find outermost struct, iff unique
    // go up in structured-value tree
    value_id val_find_unique_outer_struct_value(value_id f) {
        value_id out = f;
        value_id outer = undefined_value;
        do {
            if (val(out)->used_by_values.size()!=1)   // not unique == shared
                return undefined_value;
            outer = out;
            out = *(val(out)->used_by_values.begin());  // go up one level
        }while( out != undefined_value );
        // TODO: check outer value if shared ?
        return outer;
    }

    // TODO: change
    object_id struct_get_next_if_unique(object_id o, field_id next) {
        // check for pointers inside struct
        std::vector<value_id> pointers = find_all_pointers_to_struct(o);
        if(pointers.size()!=1) // not single pointer only
            return undefined_object;

        value_id svalue = var_get_value(o);
        value_id fvalue = val_struct_field(svalue,next);
        if(val(fvalue)->refcount() != 1)        // not unique next pointer value
            return undefined_object;
        // target object pointed from single place
        object_id nexto = val_points_to(fvalue);    // checks for pointer type
        // check target type
        if(obj_is_var(nexto) && var_is_struct(nexto))  {
            // check for pointers inside struct
            std::vector<value_id> pointers = find_all_pointers_to_struct(nexto);
            if(pointers.size()!=1) // not single pointer only
                return undefined_object;
            return nexto;
        }
        return undefined_object; // for listsegments and nil
    }

    // go back in structure sequence linked by pointer in next field  (sls candidate)
    // structvar -> structval -> { fieldvalues, next, fieldvalues }
    object_id struct_get_prev_if_unique(object_id o, field_id next) {
        // check for pointers inside struct
        std::vector<value_id> pointers = find_all_pointers_to_struct(o);
        if(pointers.size()!=1) // not single pointer only
            return undefined_object;

        value_id pvalue = obj_pointed_by(o);    // previous next-field value
        if(val(pvalue)->refcount() != 1)        // not unique == shared
            return undefined_object;

        // TODO: check ALL values in structval if pointed
        value_id svalue = val_find_unique_outer_struct_value(pvalue);  // go from next to struct value
        if(svalue==undefined_value)             // not found
            return undefined_object;
        if(val(svalue)->refcount() != 1)        // not unique
            return undefined_object;
        // object pointed from single place
        object_id prevo = *(val(svalue)->used_by_objects.begin());      // used by single var
        // check previous struct type
        if(obj_is_var(prevo) && var_is_struct(prevo)) {
            // check for pointers inside struct
            std::vector<value_id> pointers = find_all_pointers_to_struct(prevo);
            if(pointers.size()!=1) // not single pointer only
                return undefined_object;
            return prevo;
        }
        return undefined_object;        // for listsegments etc
    }

    // find all struct sequences linked through nextptr field
    // TODO: check correctness
    typedef std::vector< std::pair<object_id, object_id> > vec_pairs_t;
    vec_pairs_t find_all_struct_sequences(type_id st, field_id next) {
        vec_pairs_t r;
        std::vector<object_id> structs = find_all_struct_variables(st);
        std::set<object_id> done;
        for(unsigned i=0; i<structs.size(); ++i) {
            object_id s = structs[i];
            if(done.count(s)!=0) // already checked
                continue;
            done.insert(s);
            // check for pointers inside struct
            std::vector<value_id> pointers = find_all_pointers_to_struct(s);
            if(pointers.size()!=1) // not single pointer only
                return r;

            // go in both directions and search for sequence start and end values
            // a) go forward
            object_id o=s;
            object_id lasto=s; // last struct in sequence
            while( (o = struct_get_next_if_unique(o,next)) != undefined_object
                   && var_type(o)==st) {
                // go next and mark new struct
                done.insert(o);
                lasto=o;
            }
            ASSERT(o==undefined_object);
            // b) go back
            o=s;
            object_id firsto=s; // first struct in sequence
            while( (o = struct_get_prev_if_unique(o,next)) != undefined_object && var_type(o)==st) {
                // go back and mark new struct
                done.insert(o);
                firsto=o;
            }
            ASSERT(o==undefined_object);
            r.push_back(std::pair<object_id,object_id>(firsto,lasto));
        } // for
        return r;
    }

    // search list-segments of given struct type and next-item id
    std::vector<object_id> find_all_sls(type_id st, field_id nextfield) {
        std::vector<object_id> r;
        for(so_container<object_id,object>::const_iterator i=objects.begin(); i!=objects.end(); ++i) {
            object_id o = i->first;
            if(!obj(o)->is_sls()) continue;
            type_id t = sls_type(o);
            field_id f = sls_next_id(o);
            if(t==st && f==nextfield)
                r.push_back(o);
        } // for
        return r;
    }

    // TODO: change interface
    // abstract sequences of structures of type st using nextfield f
    void sls_abstract(type_id stype, field_id f) {
        // search for struct sequences
        //typedef std::vector< std::pair<object_id, object_id> > vec_pairs_t;
        vec_pairs_t seqs = find_all_struct_sequences(stype,f);
        for(unsigned i=0; i<seqs.size(); ++i) {
            // search for tail struct
            object_id  begin = seqs[i].first;
            object_id  end   = seqs[i].second;
            value_id ptr1 = obj(begin)->pointed_by(); // TODO: check pointers inside
            value_id ptr2 = var_get_field_value(end,f);

            // discard sequence, create sls
            object_id nexto;
            for(object_id o=begin; o!=end; o=nexto) {
                value_id next = var_get_field_value(o,f);
                nexto = val_ptr(next)->points_to();
                // unlink
                val_unlink_obj(next);   // unlink nextval <-> nexto
                obj_unlink_val_del(o);  // unlink o <-> structvalue and delete structvalue if possible
                value_id p = obj_pointed_by(o);
                if(p!=undefined_value)
                    val_unlink_obj(p);  // unlink prev <-> o
                // remove o
                obj_delete(o);
            } // for all structs in sequence

            // create sls
            sls1p_create_at(begin,ptr2,stype,f);
            // link ptr1->sls
            val_link_obj(ptr1,begin);

        } // for all sequences
        // search for sls
        // join
    }

    //////////////////////////////////////////////////////////////////
    /// VARIABLE OPERATIONS

    // check
    bool var_is_struct(object_id o) const {
        const variable *v = var(o);
        return v->is_struct();
    }

    value_id val_struct_field(value_id s, field_id f) const {
        // check
        if(s==undefined_value||s==nil_value)
            throw "val_struct_field: unknown or nil used";

        const value *v = val(s);
        // check if structure
        if(!v->is_struct())
            throw "val_struct_field: not structure";

        value_id field_value = val_s(s)->get_field_value(f);
        std::cout << "val_struct_field(s="<< s <<", f=" << f << ")\n";
        return field_value;
    }

    // COW -- clone object
    void obj_COW(object_id o) {
        if(obj(o)->_writable()) return; // already cloned
        std::cout << "obj_COW(" << o << ")\n";
        objects.clone(o);
    }

    // set value of variable
    // TODO: type-check, h should be correct
    void var_assign_value(object_id o, value_id h) {
        std::cerr << "=var_assign_value(" << o << ", val=" << h << ")\n";
        std::cout << "=var_assign_value(" << o << ", val=" << h << ")\n";
        //TODO: type-check
        if(!(var_is_struct(o) && val_is_struct(h)) &&
           !(var_is_pointer(o) && val_is_pointer(h))
           // TODO int int
          ) throw "var_assign_value: typecheck failed";

        obj_COW(o); // make writable copy, will write
        val_COW(h); // new value
        value_id oldh = obj(o)->get_value(); // old value
        if(oldh!=undefined_value)
            val_COW(oldh);
        obj_unlink_val_del(o);  // unlink old value and delete if unused
        obj_link_val(o,h);      // add link value<->variable
    }

    // set value of pointer to point at x      [[[o = &x]]]
    void var_assign_pointer_at(object_id o, object_id x) {
        std::cout << "=var_assign_pointer_at(" << o << ", target=" << x << ")\n";
        // TODO: type-check o,x
        // prepare value:
        value_id h = obj_pointed_by(x);      // use existing pointer-to-x value ...
        if(h==0) // if nothing points to x
            h = val_new_ptr(x);               // ... or create new pointer-to-x value
        // set value with reverse-link
        var_assign_value(o, h);
    }

    //////////////////////////////////////////////////////////////////
    /// VALUE OPERATIONS

    void val_delete(value_id x) {
      std::cout << "symbolic_state::val_delete(" << x << ") \n";
      value *v = val(x);
      // check if not used
      if(!v->unused())  // value used by object or struct-value
          throw "symbolic_state::val_delete: deleted target of link(s)";

      if(v->is_pointer()) {
          const value_address *p = val_ptr(x);

          // TODO: function "val_ptr_unlink_target"
          object_id t = p->points_to();
          if(t!=undefined_object) { // ptr-value points to object
              obj_COW(t);
              obj(t)->set_pointed_by(undefined_value); // unlink from this value
          }
      } else if (v->is_struct()) {
          const value_struct *s = val_s(x);
          // for each field
          for(int i = 0; i<s->nfields(); ++i) {
              value_id fv = s->get_field_value(static_cast<field_id>(i));
              if(fv==undefined_value)
                  continue;
              val_COW(fv);
              val_refcount_down(fv,x); // remove field from part-of x relation
              if(val(fv)->unused())    // was last usage
                  val_delete(fv);      // recurse
             // do not write to struct - will be destroyed
          }
      } else if (v->is_array()) {
          throw "todo val_delete array";
      }

      // val_COW(x); not needed, because we remove this value without write
      values.erase(x);  // unlink from container, manipulate refcounts

      // if not used by any state, delete
      if(v->_symbolic_state_refcount()==0)      // last pointer (refcount==0)
          delete v;                             // free object
    } // val_delete

    // update backlinks in values
    void val_refcount_up(value_id id, object_id v)      { val_COW(id); val(id)->link_object(v); }
    void val_refcount_down(value_id id, object_id v)    { val_COW(id); val(id)->unlink_object(v); }
    void val_refcount_up(value_id id, value_id sv)      { val_COW(id); val(id)->link_value(sv); }
    void val_refcount_down(value_id id, value_id sv)    { val_COW(id); val(id)->unlink_value(sv); }

    // return handle of structure field value
    value_id var_get_field_value(object_id st, field_id field) const {
        value_id v = var(st)->get_value();
        return val_get_field_value(v,field);
    }
    // return handle of structure field value
    value_id val_get_field_value(value_id st, field_id field) const {
        if(!val(st)->is_struct())
            throw "val_field_id: nonstruct->member used";
        const value_struct *s = val_s(st);
        // check if field value is defined
        return s->get_field_value(field);
    }

    // update field in structure value == change struct value
    value_id val_update_field(value_id st, field_id field, value_id newval) {
        // will COW, clone new value, update item (other shared), return new value (OR OLD IF NO CHANGE)

        // get current field value
        value_id id = val_get_field_value(st,field);

        if(newval==id)  // no change of value  (also 0->0 undef->undef)
            return st;  // return original structure value

        val_COW(st);    // structure will be changed in this state only
        // now we can change struct value links etc

        // if unused, we do not need clone
        if(val(st)->refcount()>0) {
            // clone new struct value (all items shared)
            st = val_clone_new(st);
        }

        // TODO: should be the same as old id
        id = val_get_field_value(st,field);

        // check if field value is defined or not
        if(id!=undefined_value) {     // update existing old field value
            val_refcount_down(id,st); // unlink from new struct value
        }
        // update field
        value_struct *s = val_s(st);
        s->update_field(field,newval); // create if not defined already

//std::cout << "XXXstruct"<<newst <<" field:" << s->get_field_value(field) << " == " << newval << " XXX\n";

        val_refcount_up(newval,st);
        return st;
    }

    // SPECIAL: patch structure item to use another value
    void _val_change_subvalue(value_id s, value_id oldval, value_id newval) {
        ASSERT(val_is_struct(s)); // TODO arrays
        value_struct *v = val_s(s);
        std::vector<field_id> fields = v->search_fields(oldval); // which field(s) use oldval
        // for each field with old value set the new one:
        for(unsigned i=0;i<fields.size();++i)
            v->update_field(fields[i],newval);    // replace oldval with newval
        val_refcount_up(newval,s); // backlink new value to struct (once)
    }

    // update variable of structure type
    void var_struct_update(object_id s, field_id f, value_id newfieldval) {

        // TODO: check if field is concretized (=in memory) --- we expect not - for now ***BUG***
        // TODO check struct. type
        value_id oldval  = var_get_value(s);                // old struct value
        value_id olditem = val_get_field_value(oldval,f);      // old field value

        // TODO: clone unconditionally ? (needed if we want write marking)

        if(newfieldval==olditem) // the same value == no change
            return;

        // clone new structure value (now with the same content)
        // remove sharing between symbolic_state
        value_id newval = val_update_field(oldval,f,newfieldval);

        // assign only new struct value
        if(newval!=oldval)
            var_assign_value(s,newval); // unlink old, assign, link new

        // done
        // WARNING:is there a problem if field is concretized in variable?
        // TODO: struct of structs = no problem if sharing substructs?
    }

    // clone shared value
    // this makes the value writable (_refcount==1)
    // TODO: clone all dependent values (struct/array) or not ?
    // COW -- clone value object
    void val_COW(value_id o) {
        if(val(o)->_writable()) return; // already cloned
        std::cout << "val_COW(" << o << ")\n";
        values.clone(o);

        return;

        // TODO: remove?
        if(val_is_struct(o)) {
            value_struct *s = val_s(o);
            // COW all fields at level 1
            for(int i =0; i<s->nfields(); i++) {
                value_id id = s->get_field_value(static_cast<field_id>(i));
                if(id) { // defined
                    if(!val(id)->_writable())
                        values.clone(id);
                }
            }
        }
    }

    // create new value, initialize it by copying old value
    value_id val_clone_new(value_id old) {
        if(!val_is_writable(old)) throw "****bug in val_clone_new";

        value *oldval = val(old);
        value_id newst = allocate_value();
        value *newval = oldval->clone(newst); // copy-create new value (shalow, share fields)
        values.create(newst,newval); // add to container at new position, update _refcount

        if(val_is_struct(newst)) {
            value_struct *s = val_s(newst);
            // update back-references in field values (single level, NOT recursively)
            // TODO: use iterators -- only defined
            for(int i = 0; i<s->nfields(); i++) {
                value_id id = s->get_field_value(static_cast<field_id>(i));
                if(id!=undefined_value) // if defined
                    val_refcount_up(id,newst); // COW, new copy shares old field values
            }
        }

        if(val_is_array(newst)) // TODO: array
            throw "arrays not implemented";

        return newst;
    }

    ///////////////////////////////////////////////////////////////////
    // STACK OPERATIONS
    void stack_push(line_id l)  { stack.push(l); }
    line_id stack_top()         { return stack.top(); }
    void stack_pop()            { stack.pop(); }

    ///////////////////////////////////////////////////////////////////
    // ASSIGNMENT COMMANDS:
    void assign_v_v(object_id x1, object_id x2);             //  x1 = x2;
    void assign_v_m(object_id x1, object_id x2, field_id f);  //  x1 = x2.f
    void assign_m_v(object_id x1, field_id f, object_id x2);  //  x1.f = x2;

    ///////////////////////////////////////////////////////////////////
    // utility
    void print() const;
    void print_short() const {
        std::cout << "[symbolic_state#" << num << ", cloned from #" << cloned_from << "]";
        if(done)
            std::cout << " DONE" << std::endl;
        std::cout << std::endl;
    }
    void obj_print(object_id x) {
        if(x==0)
            std::cout << "undefined";
        else
            obj(x)->print();
    }
    void val_print(value_id x) {
        if(x==0)
            std::cout << "undefined";
        else
            val(x)->print();
    }
    // variable has name == non-primed?
    bool has_name(object_id x) {
        return var(x)->has_name();
    }

}; // symbolic_state

// initialize static variables
unsigned long symbolic_state::_object_allocator_index = 1;
unsigned long symbolic_state::_value_allocator_index = 1;
unsigned      symbolic_state::counter = 0; // number of symbolic_state instances created


/////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
/////////////////////////////////////////////////////////////////////////////

// variables

#define CHECK_VAR_ID(x) \
    do { \
    if(x==nil)          throw __FILE__ " nil := X"; \
    if(x==undefined_object)      throw __FILE__ " use of uninitialized pointer"; \
    }while(0)

#define CHECK_PTR_TARGET(x) \
    do { \
    if(var_points_to(x)==nil)           throw " nil value dereference ";  \
    if(var_points_to(x)==undefined_object)       throw " use of uninitialized pointer value ";  \
    }while(0)

// allocate new variable
object_id symbolic_state::var_new(type_id t) {
    // TODO: check this simple implementation
    object_id i = allocate_object();
    variable *v = new variable(i,t);
    objects.create(i,v); // add to container
    return i;
}

// create new variable at given place
// used by concretize
object_id symbolic_state::var_new_at(object_id at, type_id t) {
    if(at>=_object_allocator_index) throw "symbolic_state::var_new_at: bad id";
    variable *v = new variable(at,t);
    objects.create(at,v);       // add to container, check, update refcount
    return at;
}

object_id symbolic_state::var_new(type_id t, const std::string &name) {
    object_id i = allocate_object();
    variable *v = new variable(i,t);
    v->set_name(name);
    objects.create(i,v); // add to container
    return i;
}

// TODO: BETTER NAME   delete variable, mark it, check
// delete single variable // TODO:delete array/struct???
void symbolic_state::obj_delete(object_id x) {
    std::cout << "symbolic_state::obj_delete(" << x << ") \n";
    // currently only simple implementation
    // TODO check if x exists
    object *v = obj(x);
    if(v->is_pointed())        // used as target of single pointer value
        throw "symbolic_state::obj_delete: deleted target of pointer(s)";

    if(obj(x)->_writable())
        obj_unlink_val_del(x);          // unlink old value, delete if unused
    else
        obj_unlink_val_partial_del(x);  // unlink old value backlink only [[[COW optimization]]]

    // assertion: eliminated from equations and predicates
    objects.erase(x);           // unlink from container, manipulate refcounts
    // if not used, delete
    if(v->_symbolic_state_refcount()==0)        // last pointer (refcount==0)
        delete v;                               // free object
}

/////////////////////////////////////////////////////////////////////////////
// VALUES
/////////////////////////////////////////////////////////////////////////////

// create new struct value with all fields undefined
value_id symbolic_state::val_new_struct(type_id t) {
    value_id i = allocate_value();
    value *v = value::create_struct(i,t);
    values.create(i,v); // add to container
    return i;
}


/// create new pointer-to-x value   [[ &x ]]
value_id symbolic_state::val_new_ptr(object_id x) {
    // TODO: assert
    if(obj(x)->is_pointed())
        throw "symbolic_state::val_new_ptr: can not create another value of pointer to x";
    value_id i = allocate_value();       // fresh value id allocation
    value *v = value::create_ptr(i);     // new pointer value - undefined
    values.create(i,v);                  // add to container
    obj_COW(x);         // will write
    val_link_obj(i,x);  // create link val<->obj
    return i;
}



/////////////////////////////////////////////////////////////////////////////
// .EQ.
bool symbolic_state::var_is_equal(object_id x1, object_id x2) const
{
    value_id v1 = var_get_value(x1);
    value_id v2 = var_get_value(x2);
    if (v1==0||v2==0)   // no value
        return false;
    return x1==x2;      // reference to the same value   WARNING (pointers only?)
}

// test any x1 == any_x
// WARNING this refcount includes enclosing structured values
bool symbolic_state::var_is_equal_any(object_id x1) const
{
    if(!var_has_value(x1))             // no value
        return false;
    return val(var_get_value(x1))->refcount() > 1;
}

bool symbolic_state::var_has_value(object_id x1) const
{
    const variable *v1 = var(x1);
    //CHECK-VAR
    return v1->has_value();
}


#if 0
/////////////////////////////////////////////////////////////////////////////
// .NE.
bool symbolic_state::is_neq(object_id x1, object_id x2) const
{
    throw "symbolic_state::is_neq: todo";
    return true;
}

bool symbolic_state::is_neq(object_id x1) const // is there neq(x1,__any__) variable?
{
    throw "symbolic_state::is_neq: todo";
    return true; // registered any inequality
}

// add inequality  a != b
void symbolic_state::add_ne(object_id x1, object_id x2)
{
    throw "symbolic_state::add_neq: todo";
}

// remove inequality:  x1 != x2
void symbolic_state::remove_ne(object_id x1, object_id x2)
{
    throw "symbolic_state::remove_neq: todo";
}
#endif

/////////////////////////////////////////////////////////////////////////////
// list segment
/////////////////////////////////////////////////////////////////////////////

// create new slist_segment defined by type and pointer variable
// new_sls:
//   type: 0+ 1+
//   end pointer value
//   struct type
//   next pointer field
// returns: id of new listsegment
object_id symbolic_state::sls_create(list_segment_length t, value_id next, type_id st, field_id nextfield)
{
    if(!val_is_pointer(next))
        throw "symbolic_state::create_sls: next value is not a pointer";
    // TODO: check target type

    object_id i = allocate_object(); // allocate space
    slist_segment * p = slist_segment::create(i, t, next, st, nextfield); // TODO: add type
    objects.create(i,p); // add to container

    return i;
}

// create sls LS1P at given position
object_id symbolic_state::sls1p_create_at(object_id i, value_id next, type_id st, field_id nextfield)
{
    if(objects.exist(i))
        throw "symbolic_state::sls1p_create_at: object position already used";
    if(!val_is_pointer(next))
        throw "symbolic_state::sls1p_create_at: next value is not a pointer";
    // TODO: check target type
    slist_segment * p = slist_segment::create(i, LS1P, next, st, nextfield); // TODO: add type
    objects.create(i,p); // add to container
    return i;
}

// erase
void symbolic_state::sls_delete(object_id ls)
{
    std::cout << "symbolic_state::sls_delete(" << ls << ") \n";
    // TODO: check if unlinked, _writable, ...
    if(!obj_is_sls(ls))
        throw "symbolic_state::remove_sls: not slist segment";
    if(obj_pointed_by(ls)!=0)
        throw "symbolic_state::remove_sls: used slist segment removed";
    object * p = obj(ls);
    // remove pointer from container, update refcount
    objects.erase(ls);
    if(p->_symbolic_state_refcount()==0)      // last pointer (refcount==0)
        delete p;                             // free object
}

/////////////////////////////////////////////////////////////////////////////
// algorithms
/////////////////////////////////////////////////////////////////////////////

/// RULES, abstract, check, split, ...

/////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// OPERATIONAL SYMBOLIC EXECUTION RULES
/////////////////////////////////////////////////////////////////////////////

// ASSIGN: x1 := x2;
// TODO add typecheck
void symbolic_state::assign_v_v(object_id x1, object_id x2) {
    // TODO: check type - variables should be type compatible  [[[depends on preprocessor]]]
    if(x1==x2)
        return; // a=a is ignored   TODO:should never be used (eliminated by preprocessing)

    if(var_get_value(x1) == var_get_value(x2)) {
        // if both variables have the same value / point to the same location
        //TODO if(is_neq(x1,x2)) throw "inconsistency";
        // nothing to do
        return;
    }

    // TODO is neq really needed ?
    //    if(is_neq(x1,x2))        // there is   x1 != x2
    //        remove_ne(x1,x2);

    var_assign_value(x1,var_get_value(x2));
}

// LOOKUP:    x := E->f
// get the target of pointer variable
// x1 = x2->f

// x1=x2->f  OR  x1=x2.f
// TODO: split both cases?
void symbolic_state::assign_v_m(object_id x1, object_id m, field_id f)               //  x1 = x2->f
{
    CHECK_VAR_ID(x1);
    CHECK_VAR_ID(m);
    // assign
    value_id mval  = var_get_value(m);
    if(mval==undefined_value || mval==nil_value)
        throw "symbolic_state::assign_v_m: nil pointer dereference";

    if(var_is_pointer(m)) { // x1 = m->f
        m = val_points_to(mval);      // *m
        mval = var_get_value(m);      // concretized struct value
    }
    // x1 = m.f
    // CHECK
    if(obj_is_sls(m))
        throw "assign_v_m: should be concretized";
    value_id h = val_get_field_value(mval,f);  // m.f
    var_assign_value(x1,h);
}

// MUTATE
// write variable contents to target of pointer variable
// E->f := F

void symbolic_state::assign_m_v(object_id m, field_id f, object_id x2)      //  m->f = x2;
{
    CHECK_VAR_ID(m);
    CHECK_VAR_ID(x2);
    // assign
    value_id mval = var_get_value(m);
    if(mval==undefined_value || mval==nil_value)
        throw "symbolic_state::assign_m_v: nil pointer dereference";
    if(var_is_pointer(m)) { // m->f = x2
        m = val_points_to(mval);      // *m
        mval = var_get_value(m);      // concretized struct value
    }
    // m.f = x2
    // CHECK
    if(obj_is_sls(m))
        throw "assign_v_m: should be concretized";
    value_id x2val = var_get_value(x2);
    // prepare new struct value
    value_id h = val_update_field(mval,f,x2val);
    // assign
    var_assign_value(m,h);
}

// concretize slist segment -- create structure on beginning
// possibilities:
//   sls0+   ERR | struct+sls0+
//   sls1+   struct+sls0+
void symbolic_state::sls_concretize(object_id ls) {
    std::cout << "symbolic_state::sls_concretize(" << ls << ") START\n";
    ASSERT(sls_length(ls)==LS1P);

    obj_COW(ls); // make writable copy of list segment
    // move listsegment
    slist_segment *p = sls(ls);
    // free old position, update target refcount (to zero)
    objects.erase(ls);
    object_id i = allocate_object();    // allocate space
    p->this_object = i;                 // = change position/identity
    objects.create(i,p);                // add to container, update refcount
    p->set_length(LS0P);                // LS1P->LS0P

    // partial val_unlink_obj, pointer value unchanged
    p->set_pointed_by(undefined_value); // moved object is not a target
    /// we have moved listsegment

    type_id stype = p->target_struct_type();
    field_id field = p->get_nextfield();

    // relink nextptr
    value_id nextptrval = p->next();
    val_COW(nextptrval); // will write to value
    val_refcount_down(nextptrval,ls);   // remove backlink from old ls value
    val_refcount_up(nextptrval,i);      // add backlink from new ls value

    /// create new struct variable
    var_new_at(ls,stype); // create variable of struct type at old position of ls

    /// create new struct value
    value_id sval = val_new_struct(stype); // UNDEFINED fields for now

    // create new next-pointer value
    value_id link = val_new_ptr(i);      // address of shortened listsegment
    sval=val_update_field(sval, field, link); // set nextptr in structure value

    /// link variable<->value
    var_assign_value(ls,sval);

//    val(sval)->print();

    // END: ptr->slsX+  ===> ptr->struct->sls0+
    std::cout << "symbolic_state::sls_concretize END\n";
}

/////////////////////////////////////////////////////////////////////////////
// utility

void symbolic_state::print() const {
    std::cout << "[symbolic_state#" << num << ", cloned from #" << cloned_from << "]";
    if(done)
        std::cout << " DONE" << std::endl;
    std::cout << std::endl;
    std::cout << "Objects:\n";
//    std::cout << "  var0: *undefined*\n"; // not stored
    for(so_container<object_id,object>::const_iterator i=objects.begin(); i!=objects.end(); ++i) {
        const object *p = i->second;
        std::cout << "  ";
        p->print();
        std::cout << std::endl;
    }

    std::cout << "Values:\n";
//    std::cout << "  val#0: *undefined*\n"; // not stored
    for(so_container<value_id,value>::const_iterator i=values.begin(); i!=values.end(); ++i) {
        const value *p = i->second;
        std::cout << "  ";
        p->print();
        std::cout << std::endl;
    }

    // TODO stack, etc
    //    std::cout << "\n";
}

// module conditions.cc
/////////////////////////////////////////////////////////////////////////////
/// Precondition is the list of symbolic heaps
class Precondition {
    typedef std::list<symbolic_state*> list_t;  // TODO: profiling, change to vector?
    list_t l;
  public:
    Precondition() {}
    Precondition(symbolic_state *s) { push_back(s); }
#if 0
    ~Precondition() {
        // delete all
        for(iterator i=begin(); i!=end(); ++i) {
            symbolic_state *p = *i;
            delete p;
            erase(i);
        }
    }
#endif
    // iterator over symbolic state list
    typedef list_t::iterator iterator;
    iterator begin() { return l.begin(); }
    iterator end()   { return l.end(); }
    // const_iterator over symbolic state list
    typedef list_t::const_iterator const_iterator;
    const_iterator begin() const { return l.begin(); }
    const_iterator end()   const { return l.end(); }

    // add SH to list for line
    void push_back(symbolic_state *sh) {
        l.push_back(sh); //empty list
    }
    // remove symbolic state from list
    void erase(iterator ptr) {
        l.erase(ptr);
    }

    // number of states inside
    size_t size() const {
        return l.size();
    }

    // print all symbolic states
    void print() const {
        std::cout << "==================== Precondition ====================\n";
        for(const_iterator i=begin(); i!=end(); ++i) {
            if(i!=begin())
                std::cout << "------------------------------------------------------\n";
            (*i)->print();
        }
        std::cout << "======================================================\n";
    }
};


// /////////////////////////////////////////////////////////////////////////////
// /// Postcondition is the vector of Precondition pointers
// class Postcondition {
//     typedef std::vector<Precondition *> list_t;
//     list_t l;
//   public:
//     Postcondition() {}
//     void add(Precondition *sh) {
//         l.push_back(sh);
//     }
//     Precondition &operator[] (int i) { return *l.at(i); }
//     int size() { return l.size(); }
// };


/////////////////////////////////////////////////////////////////////////////
/// abstract state for all program lines
/// hash? (line,symb-heap)
class AbstractProgramState {
    typedef Precondition::iterator iterator;

    std::map<line_id,Precondition> m; // map line/command->precondition

    // TODO: add stack state to precondition elements
  public:
    Precondition & operator [] (line_id i) { return m[i]; }
    // add SH to list for line
    void add(line_id line, symbolic_state *sh) {
        m[line].push_back(sh); //list
    }
    // remove SH from list
    void erase(line_id line, iterator &ptr) {
        m[line].erase(ptr);
    }
    // iterator over SymbolicHeapList
    iterator begin(line_id line) {
        return m[line].begin();
    }
    iterator end(line_id line) {
        return m[line].end();
    }
};

// TODO:
// abstract source code description
// command(args) ===  a=x, a=[x], [a] = x; new a, delete a;  if, while, goto?
// algorithm: start, for each line, loops, ...

} // namespace SepLog





/////////////////////////////////////////////////////////////////////////////
// TEST
/////////////////////////////////////////////////////////////////////////////
using namespace SepLog;

// code interpretation
// a) create GRAPH (or interface to GCC data structures)
// b) interpret GRAPH
//
// commands identified by line numbers?
//
// FUNCTION(line,name,signature?)
// DEF_VAR(line,var)
// ASSIGN(line,var,expr)
// IF(line,condition,line1,line2)
// WHILE(line,condition,line1,line2)
// RETURN(line,var)
//


// module code.cc
// interface to preprocessed program code

/////////////////////////////////////////////////////////////////////////////
// program representation
// ======================
//
// simple command (TODO: convert from preprocessing unit)
enum Operation {
    UNDEFINED_OP=0,
    ASSIGN_V_V, ASSIGN_M_V, ASSIGN_V_M,
    IFEQ, IFNEQ,
    GOTO,
    // SWITCH,
    CALL, RETURN,
    EXIT,
    DEFVAR,
    MAX_OP
};
const char *OperationName[MAX_OP] = {
    "undefined operation",
    "assign_v_v", "assign_m_v", "assign_v_m",
    "if_eq", "if_neq",
    "goto",
    "call", "return",
    "EXIT",
    "DEFVAR",
};

typedef std::pair<line_id, symbolic_state*> execution_continuation_t;
typedef std::list< execution_continuation_t > execution_result_t;

// TODO: class hierarchy + virtual  postvector execute(pre)
// wrapper for any code model implementation
class Command {
  public:
    mutable unsigned count;         // executed count times
  private:
    Operation op;
    union {
    object_id var1;
    name_id varname;
    };
    object_id var2;
    field_id f_id;      // var?->next or var.next (depends on type)

    // TODO: graph?
    line_id line[3];   // command, next command, next2 command positions
 public:
    /// constructors:
    // v++, call v
    Command(Operation o, object_id v1): count(0), op(o), var1(v1) {}
    // IFEQ(c==v) goto nextl
    Command(Operation o, object_id c, object_id v, line_id nextl):count(0),  op(o), var1(c) { line[2]=nextl;}
    // goto nextl, CALL nextl
    Command(Operation o, line_id nextl):count(0),  op(o) {line[2]=nextl;}
    // v1 = v2
    Command(Operation o, object_id v1, object_id v2):count(0),  op(o), var1(v1), var2(v2) {}
    // v1->field = v2, v1.field = v2, and reverse
    Command(Operation o, object_id v1, object_id v2, field_id i):count(0),  op(o), var1(v1), var2(v2), f_id(i) {}
    // defvar
    Command(Operation o, name_id nm): count(0), op(o), varname(nm) {}
    // exit
    Command(Operation o): count(0), op(o) {}
    // unknown/uninitialized/terminate_exec
    Command(): op(UNDEFINED_OP) {}

    // TODO assert
    void set_line(int i, line_id l) { line[i] = l; }
    line_id get_line(int i) const { return line[i]; }

  public:
    // interface

    Operation get_op() const { return op; }
    // return variables used as command arguments
    object_id get_v1() const { return var1; }
    object_id get_v2() const { return var2; }
    // return struct field_id identification ( s->ITEM | s.ITEM )
    field_id get_field_id() const {
        return f_id;
    }

    // return next line(s) id
    line_id get_next(int i=1) const {
        // encoding:
        // i==1 next command (ALWAYS: ASSIGN, GOTO, ...)
        // i==2 next if branch (IF, CALL)
        //NOT-YET i>=3 goto switch variant (SWITCH)
        if(i>2) throw "TODO Command:get_next(i>1)";
        return line[i];
    }

    execution_result_t execute(const symbolic_state *pre) const;
    virtual void print() const {
        std::cout << std::setw(6) << line[0] << ": ";
        std::cout << "COMMAND(" << op << "=" ;
        std::cout << std::setw(10) << OperationName[op];
        if(op!=EXIT) {
        std::cout << ", " << std::setw(4) << var1;
        std::cout << ", " << std::setw(4) << var2;
        if(op==ASSIGN_V_M || op==ASSIGN_M_V)
            std::cout << ", f" << std::setfill('0') << std::setw(3) << f_id << std::setfill(' ');
        else
            std::cout << ",     ";
        std::cout << ", nextcmd=" << line[1];
        if(op==IFEQ||op==IFNEQ||op==CALL)
            std::cout << ", nextcmdIFtrue=" << line[2];
        }
        std::cout << ") ";
        std::cout << std::endl;
    }
}; // Command

// sample code to execute in text form
const char * sample_code =
" f: l                  \n"  // function: +list of parameters(by value) ??
"    var r;             \n"  // new stack variable (has name)
"    r = null;          \n"  // v1 = v2; v2==special_predefined_const_var
" L: if (l==null) goto L2; \n"  // ifnot(c) goto L2
"    var p;             \n"  // new stack variable (has name)
"    p=l;               \n"  // v1 = v2
"    l = l->next;       \n"  // v1 = v2->item
"    p->next = r;       \n"  // v1->item = v2
"    r = p;             \n"  // v1 = v2
"    goto L;            \n"  // goto
" L2:                   \n"  // nop ??????????????? eliminate
"    return r;          \n"  // return v1
;


// program representation TODO: facade on GCC tables/graphs
class Code {
    // static representation of program
    std::vector<Command> program;
  public:
    Code() {
        // TODO: read code from source text
        // TODO: types

        // variables already defined - expected
        object_id null=nil;
        object_id r=static_cast<object_id>(2);
        object_id l=static_cast<object_id>(3);
        object_id p=static_cast<object_id>(5);
        field_id next=static_cast<field_id>(1); // check

        add_line(Command(ASSIGN_V_V,r,null));
        line_id L = add_line(Command(IFEQ,l,null /*,next2unknown*/));
        add_line(Command(ASSIGN_V_V,p,l));
        add_line(Command(ASSIGN_V_M,l,l,next));
        add_line(Command(ASSIGN_M_V,p,r,next));
        line_id ll = add_line(Command(ASSIGN_V_V,r,p));
        program[ll].set_line(1,L); // GOTO L
        line_id ln = add_line(Command(EXIT));
        program[L].set_line(2,ln);
    } // code

    // const access
    const Command &operator[](line_id i) const { return program[i]; }

    // fill:
    line_id add_line(Command c) {
        line_id ln = static_cast<line_id>(program.size());
        c.set_line(0,ln);
        c.set_line(1,line_id(ln+1));
        program.push_back(c); // add next line
        return ln;
    }
    void print() const {
        std::cout << "Program:" << std::endl;
        for(unsigned i=0; i<program.size(); ++i) {
            program[i].print();
        }
        std::cout << std::endl;
    }
};

// simple program form for standalone debugging
// TODO: code = set of modules, module = vector of lines, line_id=number (1..N=module1,N+1..M=module2,...+module_map)
// total max = INT_MAX lines (32bit >2G lines)
/// TODO: line->module_name ?





// module: avm.cc
/////////////////////////////////////////////////////////////////////////////
/// Abstract Virtual Machine
// ========================
//
// uses code+initial state (precondition) to evolve
// status = pointer to symbolic states
// if problem: exception of char* type  (TODO: improve)
//
struct AVM {
    const Code *code;           // code abstraction: vector of lines/commands (shared)
    AbstractProgramState precondition; // state of execution for all commands
    std::queue<line_id> todo;   // queue of lines/commands to analyze
    line_id start;

    AVM(): code(0), start(line_id(0)) {}
    void set_program(const Code*c)   {
        code  = c;
    }
    void set_initial_state(line_id l, Precondition p)  {
        start = l;
        precondition[start] = p;
    }

    // execution from start line:
    bool symbolic_execution();

    //...

//    bool entailment(Postcondition &p, const Postcondition &diff);
//    bool simplify(Postcondition &p);
};

/////////////////////////////////////////////////////////////////////////////
// symbolic execution
// ==================

/////////////////////////////////////////////////////////////////////////////
// execute simple command
// input: single symbolic state from precondition
// output: possible postconditions (maybe simplified)
// returns true if O.K.
//

// TODO: virtual result execute(pre) and command hierarchy?

execution_result_t Command::execute(const symbolic_state *pre) const {

    std::cout << "EXECUTE: ";
    print();
    std::cout << "\n";

    execution_result_t result;       // empty list

    // check number of command executions
    count++;                         // statistics: command processed count times
    // (infinite loop check)
    if(count>10)
        throw "EXEC: too many loops"; // TODO: add info

    // 1) clone precondition
    // 2) update cloned state by command
    // 3) reduce state space (entailment,join)
    object_id v1 = get_v1();
    object_id v2 = get_v2();

    switch(get_op()) {
        case EXIT: return result;
        case DEFVAR:
        {
            throw "todo: exec defvar";
            //symbolic_state *s = pre->clone(); // new symbolic state, sharing all
            //s->new_var(get_type(),get_name());
        } break;
        case ASSIGN_V_V:    // a = b;
        {
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
            s->assign_v_v(v1,v2);
            result.push_back(execution_continuation_t(get_next(),s));
        } break;
        case ASSIGN_V_M:    // v1 = v2->field; OR v1 = v2.field;
        {
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all

            ASSERT(s->obj_is_var(v1));

            if(s->var_is_pointer(v2)) { // v2->f
                object_id target = s->var_points_to(v2);
                if(target==nil) {
                    std::cout << "*** NULL->f pointer dereference\n";
                    std::cout << "at " << v2 << "\n";
                    s->print();
                    std::cout << "\n";
                } else if(s->obj_is_sls(target)) { // if pointer target is abstract
                    object_id ls = target;
                    if(s->sls_length(ls)==LS0P) { // one possibility: empty ls
                        value_id next = s->sls_get_next(ls);
                        if(next==nil_value) {
                            std::cout << "\n";
                            std::cout << "*** possible NULL pointer dereference at listsegment0+";
                            std::cout << "at " << v2 << "\n";
                            s->print();
                            std::cout << "\n";
                        } else {
                            throw "TODO: assign_V_M v, ls0p->nonnil ";
                        }
                    }
                    // both cases ??
                    s->sls_concretize(ls);
                }
                s->assign_v_m(v1,target,get_field_id());
            } else { // not pointer: v2.f
                s->assign_v_m(v1,v2,get_field_id());
            }
            result.push_back(execution_continuation_t(get_next(),s));
        } break;
        case ASSIGN_M_V:    // a->item = b;
        {
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
            ASSERT(s->obj_is_var(v2)); // TODO: check

            if(s->var_is_pointer(v1)) { // v1->f
                object_id target = s->var_points_to(v1);
                if(target==nil) {
                    std::cout << "*** NULL->f pointer dereference\n";
                    std::cout << "at " << v2 << "\n";
                    s->print();
                    std::cout << "\n";
                } else if(s->obj_is_sls(target)) { // if pointer target is abstract
                    object_id ls = target;
                    if(s->sls_length(ls)==LS0P) { // one possibility: empty ls
                        value_id next = s->sls_get_next(ls);
                        if(next==nil_value) {
                            std::cout << "\n";
                            std::cout << "*** possible NULL pointer dereference at listsegment0+";
                            std::cout << "at " << v2 << "\n";
                            s->print();
                            std::cout << "\n";
                        } else {
                            throw "TODO: assign_V_M v, ls0p->nonnil ";
                        }
                    }
                    // both cases ??
                    s->sls_concretize(ls);
                }
                s->assign_m_v(target,get_field_id(),v2);
            } else { // not pointer: v2.f
                s->assign_m_v(v1,get_field_id(),v2);
            }
            result.push_back(execution_continuation_t(get_next(),s));
        } break;
        case IFEQ:          // if(a==b)
        {
            // TODO: add check for loop fixed point???????????????????
            //eval_condition(); // for now only var=var (and pointers)
            if(pre->var_is_equal(v1,v2)) { // same value ==> condition always true
                std::cout << "if(true) // v1==v2\n";
                // known exactly, go only single way
                symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                // add true to s is not needed, v1=v2 already holds
                result.push_back(execution_continuation_t(get_next(2),s));  // if true
            }
            else // values are not the same ==> maybe nondeterministic:
            {    // condition false or true - we don't know
                std::cout << "if(v1!=v2)\n";
                if(pre->var_is_pointer(v1) && pre->var_is_pointer(v2)) {
                    std::cout << "if(ptr1 ?=? ptr2)\n";
                    // both pointers
                    object_id target1 = pre->var_points_to(v1);
                    object_id target2 = pre->var_points_to(v2);
                    if(pre->obj_is_nonempty(target1) && pre->obj_is_nonempty(target2)) {
                        std::cout << "if(ptr1!=ptr2) deterministic\n";
                        // values different
                        // v1!=v2
                        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                        // add NEQ(v1,v2) ?    is for speed only?
                        result.push_back(execution_continuation_t(get_next(),s)); // if false
                    } else if (pre->obj_is_nonempty(target1) && !pre->obj_is_nonempty(target2)) {
                        std::cout << "if(ptr!=ptr2pe) nondet\n";
                        //TODO: wrap to function
                        // target2 is abstract: possibly empty segment
                        value_id t2next = pre->sls_get_next(target2); // this value should be shared
                        if(pre->val_points_to(t2next) == target1) { // if possibly empty next is equal
                            std::cout << " -- nondet A == empty listsegment\n";
                            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                            // ensure pe => empty
                            s->sls_eliminate_empty(target2); // remove LS0P
                            result.push_back(execution_continuation_t(get_next(2),s));  // if true
                        }
                        std::cout << " -- nondet B == nonempty listsegment\n";
                        // and possibly non-true // v1!=v2
                        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                        // add NEQ(v1,v2) ?
                        s->sls_set_length(target2,LS1P);
                        result.push_back(execution_continuation_t(get_next(),s)); // if false
                    } else if (!pre->obj_is_nonempty(target1) && pre->obj_is_nonempty(target2)) {
                        std::cout << "if(ptr2pe!=ptr)  nondet\n";
                        //TODO: wrap to function
                        // target1 is possibly empty segment
                        // a) empty variant
                        value_id t1next = pre->sls_get_next(target1); // this value should be shared
                        if(pre->val_points_to(t1next) == target2) { // if possibly empty next is equal
                            std::cout << " -- nondet A == empty listsegment\n";
                            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                            // ensure pe => empty
                            s->sls_eliminate_empty(target1); // remove LS0P
                            result.push_back(execution_continuation_t(get_next(2),s));  // if true
                        }
                        std::cout << " -- nondet B == nonempty listsegment\n";
                        // b) nonempty variant
                        // v1!=v2
                        symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                        // add NEQ(v1,v2) ?    is for speed only?
                        s->sls_set_length(target1,LS1P);
                        result.push_back(execution_continuation_t(get_next(),s)); // if false
                    } else if (!pre->obj_is_nonempty(target1) && !pre->obj_is_nonempty(target2)) {
                        std::cout << "if(ptr2pe!=ptr2pe)\n";
                        // both targets possibly empty
                        throw "EXEC: TODO: pe == pe ";
                    }
                } else {
                    std::cout << "if(nonptr!=nonptr)\n";
                    throw "EXEC:  TODO: non-pointers";
                }
            }

        } break;
        case CALL:          // TODO: arguments->parameters, new stackframe
        {
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all
                            s->stack_push(get_next()); // return address
                            result.push_back(execution_continuation_t(get_next(2),s));
                            break;

        } break;
        case RETURN:        // TODO: return value, delete stackframe
        {
            symbolic_state *s = pre->clone(); // new symbolic state, sharing all

                            result.push_back(execution_continuation_t(s->stack_top(),s)); // go to return address
                            s->stack_pop(); // remove return address
                            break;

        } break;
        default: // internal error
        {
                            std::string s;
                            std::ostringstream os(s);
                            os << "*** Bad command code: " << __FILE__ << ":" << __LINE__;
                            throw s.c_str();
        } break;
    } // switch
    std::cout << "- comamnd executed\n";

    // 3) check/simplify
    // TODO: part of entailment -- special cases only



    return result; // return all possible continuations
}

/////////////////////////////////////////////////////////////////////////////
// execute code, search for fixed point if there are loops
// input: initialized AVM-state = program, precondition, start_line
// output: postconditions for each line (stored in AbstractProgramState)
bool AVM::symbolic_execution() {
    unsigned long counter = 0;

    std::cout << "\n";
    std::cout << "***** Execution start *****\n";
    std::cout << "\n";

    todo.push(start);              // start on this line
    while(!todo.empty()) {
        counter++;
        // get next line number to execute
        line_id line = todo.front(); todo.pop();
        // execute the line command
        const Command &cmd = (*code)[line];  // get description

        Precondition &pre =  precondition[line]; // OR list of symbolic states
        pre.print();

        // for all precondition parts
        std::set<line_id> already_scheduled; // to eliminate duplicate scheduling of continuations
        unsigned sscount=0;
        for(Precondition::const_iterator i=pre.begin(); i!=pre.end(); ++i) {
            // for all components (= symbolic states) of precondition compute postconditions
            const symbolic_state *ss = *i; // next symbolic state in precondition
            std::cout << "precondition[" << line << "][" << sscount++ << "] ";
            std::cout << "= state#" << ss->state_number();
            if(ss->is_done()) {
                std::cout << " DONE\n";
                continue;
            }
            std::cout << "\n";

            execution_result_t r = cmd.execute(ss); // execute command

            ss->mark_done(); // state is executed

            // result is the container of pairs (next line, symbolic state)
            while(r.size()>0) {
                // get and parse single continuation
                execution_continuation_t ec = r.front(); r.pop_front();

                line_id next_cmd = ec.first;
                symbolic_state *p = ec.second;

                // =======================================================
                std::cout << "Result[line#" << next_cmd << "]: ";
                p->print_short();

                // merge with existing states (entailment,join,etc)
                // TODO

                // =======================================================
                // for now we add unchanged
                precondition[next_cmd].push_back(p);    // add part of postcondition to next preconditions
                // if unchanged do nothing = continue
                if(already_scheduled.count(next_cmd)==0) {
                    todo.push(next_cmd);                // analyze continuation later (TODO: FIFO or LIFO ?)
                    already_scheduled.insert(next_cmd);
                }
            } // while result item
        } // for precondition item
    } // while nonempty todo-list

    // ? fixed point found
    std::cout << "***** Execution ended successfully after " << counter << " commands executed" << std::endl;

    return true;
} // symbolic_execution


// /////////////////////////////////////////////////////////////////////////////
// // reduce and add newly computed postcondition diff (in-place)
// bool AVM::entailment(Postcondition &p, const Postcondition &diff) {
//     // a) simplify
//     // b) add
//     // c) simplify
//     return true;
// }
//
// // simplify postcondition (in-place)
// bool AVM::simplify(Postcondition &p) {
//     // a) remove duplicity
//     // b) ...
//     // return
//     return true;
// }


// module test.cc
/////////////////////////////////////////////////////////////////////////////
// Test program
//
int main() try {

// TODO: move to Code abstraction?
    // fill type table
    type_id stype = types.add_structure("structX",2);
    type_id ptype = types.add_pointer_to(stype);
                      types.add_structure_field(stype,"data",ptype);
    field_id nextid = types.add_structure_field(stype,"next",ptype);
    if(nextid!=1) throw "err";

    // TODO: wrap to function
    symbolic_state *s = symbolic_state::create();   // empty initial symbolic_state
    s->var_new(ptype,"r");
    object_id ll = s->var_new(ptype,"l");
#if 1
    object_id ls = s->sls_create(LS0P,nil_value,stype,nextid); // ls(_,nil,lambda)
    s->var_assign_pointer_at(ll,ls);
#else
    s->var_assign_value(ll,nil);
#endif
    s->var_new(ptype,"p");
    std::cout <<  "Global initial state\n";
    s->print();
    Precondition p(s);  // initial precondition

    AVM avm;            // create virtual machine
    Code c;             // TODO:  Code(file|fileset)
    avm.set_program(&c);
    line_id     startline = line_id(0);  // TODO
    avm.set_initial_state(startline,p);

    c.print();

    // run analysis
    avm.symbolic_execution();
    // execution ended in fixed point

    // get postcondition on any program position
    // print

}
catch(const char *s) {
    std::cout << "Exception: " << s << '\n' ;
    std::cerr << "Exception: " << s << '\n' ;
}
/////////////////////////////////////////////////////////////////////////////
