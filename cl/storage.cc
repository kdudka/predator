/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#include "config_cl.h"

#include <cl/storage.hh>
#include <cl/cl_msg.hh>

#include "util.hh"

#include <map>
#include <stack>

#include <boost/tuple/tuple.hpp>

bool operator==(const struct cl_type &a, const struct cl_type &b) {
    // go through the given types recursively and match UIDs etc.
    typedef std::pair<const struct cl_type *, const struct cl_type *> TItem;
    std::stack<TItem> todo;
    push(todo, &a, &b);
    while (!todo.empty()) {
        const struct cl_type *cltA, *cltB;
        boost::tie(cltA, cltB) = todo.top();
        todo.pop();

        if (cltA->uid == cltB->uid)
            // UID matched, go next
            continue;

        const enum cl_type_e code = cltA->code;
        if (cltB->code != code)
            // code mismatch
            return false;

        const int cnt = cltA->item_cnt;
        if (cltB->item_cnt != cnt)
            // mismatch in the count of sub-types
            return false;

        switch (code) {
            case CL_TYPE_VOID:
            case CL_TYPE_INT:
            case CL_TYPE_CHAR:
            case CL_TYPE_BOOL:
            case CL_TYPE_REAL:
                if (cltA->name && cltB->name && STREQ(cltA->name, cltB->name))
                    // FIXME: we simply ignore differences that gcc seems
                    //        important!
                    return true;

                // fall through!

            case CL_TYPE_ENUM:
            case CL_TYPE_UNKNOWN:
                return false;

            case CL_TYPE_STRING:
                // should be used only by cl_cst, see the dox
                TRAP;
                return false;

            case CL_TYPE_PTR:
            case CL_TYPE_FNC:
            case CL_TYPE_STRUCT:
            case CL_TYPE_UNION:
            case CL_TYPE_ARRAY:
                // nest into types
                for (int i = 0; i < cnt; ++i) {
                    const struct cl_type_item *ciA = cltA->items + i;
                    const struct cl_type_item *ciB = cltB->items + i;
                    if (ciA->name && ciB->name && !STREQ(ciA->name, ciB->name))
                        return false;

                    push(todo, ciA->type, ciB->type);
                }
        }
    }

    // all OK
    return true;
}

namespace CodeStorage {

namespace {
    /**
     * Look for an existing value, create a new one if not found.
     * @param db Mapping from key to index.
     * @param idxTab Table of values accessible by the index.
     * @param key Unique ID used as a key to look for.
     * @param tpl Object template used to create a new object.
     */
    template <class TDb, class TTab, class TKey>
    typename TTab::value_type&
    dbLookup(TDb &db, TTab &idxTab, TKey key,
             const typename TTab::value_type &tpl
                 = typename TTab::value_type())
    {
        typename TDb::iterator iter = db.find(key);
        if (db.end() != iter)
            // key found
            return idxTab[iter->second];

        // allocate a new item
        unsigned idx = idxTab.size();
        db[key] = idx;
        idxTab.push_back(tpl);
        return idxTab[idx];
    }

    /**
     * Look for an existing value, trap to debugger if not found.
     * @param db Mapping from key to index.
     * @param idxTab Table of values accessible by the index.
     * @param key Unique ID used as a key to look for.
     */
    template <class TDb, class TTab, class TKey>
    const typename TTab::value_type&
    dbConstLookup(const TDb &db, const TTab &idxTab, TKey key)
    {
        typename TDb::const_iterator iter = db.find(key);
        if (db.end() == iter)
            // can't insert anything into const object
            TRAP;

        return idxTab[iter->second];
    }
}

// /////////////////////////////////////////////////////////////////////////////
// Var implementation
namespace {
    const struct cl_type* digVarType(const struct cl_operand *op) {
        const struct cl_accessor *ac = op->accessor;

        // FIXME: this hasn't been tested very well for all types of accessors
        return (ac)
            ? ac->type
            : op->type;
    }
}

Var::Var():
    code(VAR_VOID)
{
}

Var::~Var() {
    // nothing to cleanup for now
}

Var::Var(EVar code_, const struct cl_operand *op):
    code(code_),
    loc(op->loc),
    uid(op->data.var->uid),
    initial(op->data.var->initial)
{
    SE_BREAK_IF(CL_OPERAND_VAR != op->code);
    if (op->data.var->name)
        name = op->data.var->name;

    // dig type of variable
    this->clt = digVarType(op);
    if (!this->clt)
        TRAP;

    // check for eventual scope mismatch
    switch (code) {
        case VAR_GL:
            if (CL_SCOPE_GLOBAL != op->scope
                    && CL_SCOPE_STATIC != op->scope)
                TRAP;
            break;

        case VAR_LC:
        case VAR_FNC_ARG:
            if (CL_SCOPE_FUNCTION == op->scope)
                break;
            // fall through!

        case VAR_VOID:
            TRAP;
    }
}

bool isOnStack(const Var &var) {
    const EVar code = var.code;
    switch (code) {
        case VAR_FNC_ARG:
        case VAR_LC:
            return true;

        case VAR_VOID:
        case VAR_GL:
            return false;
    }

    TRAP;
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// VarDb implementation
struct VarDb::Private {
    typedef std::map<int, unsigned> TMap;
    TMap db;
};

VarDb::VarDb():
    d(new Private)
{
}

VarDb::VarDb(const VarDb &ref):
    vars_(ref.vars_),
    d(new Private(*ref.d))
{
}

VarDb::~VarDb() {
    delete d;
}

VarDb& VarDb::operator=(const VarDb &ref) {
    vars_ = ref.vars_;
    delete d;
    d = new Private(*ref.d);
    return *this;
}

Var& VarDb::operator[](int uid) {
    return dbLookup(d->db, vars_, uid);
}

const Var& VarDb::operator[](int uid) const {
    return dbConstLookup(d->db, vars_, uid);
}


// /////////////////////////////////////////////////////////////////////////////
// TypeDb implementation
struct TypeDb::Private {
    typedef std::map<int, const struct cl_type *> TMap;
    TMap db;

    int codePtrSizeof;
    int dataPtrSizeof;

    Private():
        codePtrSizeof(-1),
        dataPtrSizeof(-1)
    {
    }

    void updatePtrSizeof(int size, int *pField);
    void digPtrSizeof(const struct cl_type *);
};

TypeDb::TypeDb():
    d(new Private)
{
}

TypeDb::~TypeDb() {
    delete d;
}

void TypeDb::Private::updatePtrSizeof(int size, int *pField) {
    SE_BREAK_IF(size <= 0);
    SE_BREAK_IF(-1 != *pField && *pField != size);
    *pField = size;
}

void TypeDb::Private::digPtrSizeof(const struct cl_type *clt) {
    if (CL_TYPE_PTR != clt->code)
        return;

    SE_BREAK_IF(1 != clt->item_cnt);
    const struct cl_type *next = clt->items[0].type;
    this->updatePtrSizeof(clt->size, (CL_TYPE_FNC == next->code)
            ? &this->codePtrSizeof
            : &this->dataPtrSizeof);
}

bool TypeDb::insert(const struct cl_type *clt) {
    if (!clt) {
        CL_DEBUG("TypeDb::insert() got a NULL pointer");
        return false;
    }
    const int uid = clt->uid;

    typedef Private::TMap TDb;
    TDb &db = d->db;
    TDb::iterator iter = db.find(uid);
    if (db.end() != iter)
        return false;

    // insert type into db
    db[uid] = clt;
    d->digPtrSizeof(clt);
    return true;
}

int TypeDb::codePtrSizeof() const {
    return d->codePtrSizeof;
}

int TypeDb::dataPtrSizeof() const {
    return d->dataPtrSizeof;
}

void readTypeTree(TypeDb &db, const struct cl_type *clt) {
    if (!clt) {
        CL_DEBUG("readTypeTree() got a NULL pointer");
        return;
    }

    // DFS through the type graph
    std::stack<const struct cl_type *> typeStack;
    typeStack.push(clt);
    while (!typeStack.empty()) {
        clt = typeStack.top();
#if 0
        std::cout << "--- " << clt->uid
            << "(code = " << clt->code
            << ", item_cnt = " << clt->item_cnt
            << ")\n";
#endif
        typeStack.pop();
        if (db.insert(clt)) {
            const int max = (CL_TYPE_ARRAY == clt->code) ? 1
                : clt->item_cnt;
            const struct cl_type_item *items = clt->items;
            for (int i = 0; i < max; ++i)
                typeStack.push(items[i].type);
        }
    }
}

const struct cl_type* TypeDb::operator[](int uid) const {
    typedef Private::TMap TDb;
    TDb &db = d->db;
    TDb::iterator iter = db.find(uid);
    if (db.end() == iter) {
        CL_DEBUG("TypeDb::insert() is unable to find the required cl_type: #"
                << uid);

        // we'll probably have to crash anyway
        TRAP;
        return 0;
    }

    return iter->second;
}


// /////////////////////////////////////////////////////////////////////////////
// Block implementation
void Block::append(const Insn *insn) {
    if (!insns_.empty()) {
        // check insn sequence
        const Insn *last = insns_[insns_.size() - 1];
        if (cl_is_term_insn(last->code))
            // invalid insn sequence
            TRAP;
    }

    insns_.push_back(insn);
}

const Insn* Block::front() const {
    if (insns_.empty())
        // no instructions here, sorry
        TRAP;

    return insns_.front();
}

const Insn* Block::back() const {
    if (insns_.empty())
        // no instructions here, sorry
        TRAP;

    return insns_.back();
}

const TTargetList& Block::targets() const {
    const Insn *last = this->back();
    if (!cl_is_term_insn(last->code))
        // no chance to get targets without any terminal insn
        TRAP;

    return last->targets;
}

// /////////////////////////////////////////////////////////////////////////////
// ControlFlow implementation
struct ControlFlow::Private {
    typedef std::map<std::string, unsigned> TMap;
    TMap db;
};

ControlFlow::ControlFlow():
    d(new Private)
{
}

ControlFlow::ControlFlow(const ControlFlow &ref):
    bbs_(ref.bbs_),
    d(new Private(*ref.d))
{
}

ControlFlow::~ControlFlow() {
    delete d;
}

ControlFlow& ControlFlow::operator=(const ControlFlow &ref) {
    bbs_ = ref.bbs_;
    delete d;
    d = new Private(*ref.d);
    return *this;
}

const Block* ControlFlow::entry() const {
    if (bbs_.empty())
        TRAP;

    return bbs_[0];
}

Block*& ControlFlow::operator[](const char *name) {
    Block* &ref = dbLookup(d->db, bbs_, name, 0);
    if (!ref)
        // XXX: the object will be NOT destroyed by ControlFlow
        ref = new Block(this, name);

    return ref;
}

const Block* ControlFlow::operator[](const char *name) const {
    return dbConstLookup(d->db, bbs_, name);
}


// /////////////////////////////////////////////////////////////////////////////
// Fnc implementation
namespace {
    const struct cl_cst& cstFromFnc(const Fnc &fnc) {
        const struct cl_operand &op = fnc.def;
        if (CL_OPERAND_CST != op.code)
            TRAP;

        const struct cl_cst &cst = op.data.cst;
        if (CL_TYPE_FNC != cst.code)
            TRAP;

        return cst;
    }
}

const char* nameOf(const Fnc &fnc) {
    const struct cl_cst &cst = cstFromFnc(fnc);
    return cst.data.cst_fnc.name;
}

int uidOf(const Fnc &fnc) {
    const struct cl_cst &cst = cstFromFnc(fnc);
    return cst.data.cst_fnc.uid;
}

bool isDefined(const Fnc &fnc) {
    return CL_OPERAND_CST == fnc.def.code
        && !cstFromFnc(fnc).data.cst_fnc.is_extern;
}

// /////////////////////////////////////////////////////////////////////////////
// FncDb implementation
struct FncDb::Private {
    typedef std::map<int, unsigned> TMap;
    TMap db;
};

FncDb::FncDb():
    d(new Private)
{
}

FncDb::FncDb(const FncDb &ref):
    fncs_(ref.fncs_),
    d(new Private(*ref.d))
{
}

FncDb::~FncDb() {
    delete d;
}

FncDb& FncDb::operator=(const FncDb &ref) {
    fncs_ = ref.fncs_;
    delete d;
    d = new Private(*ref.d);
    return *this;
}

Fnc*& FncDb::operator[](int uid) {
    Fnc* &ref = dbLookup(d->db, fncs_, uid, 0);
    if (!ref)
        // XXX: the object will be NOT destroyed by FncDb
        ref = new Fnc;

    return ref;
}

const Fnc* FncDb::operator[](int uid) const {
    return dbConstLookup(d->db, fncs_, uid);
}

} // namespace CodeStorage
