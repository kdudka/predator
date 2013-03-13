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

#include "cl_storage.hh"
#include "util.hh"

#include <map>
#include <stack>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

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
        if (db.end() == iter) {
            CL_BREAK_IF("can't insert anything into const object");
            return idxTab.front();
        }

        return idxTab[iter->second];
    }
}

// /////////////////////////////////////////////////////////////////////////////
// Var implementation
namespace {
    const struct cl_type* digVarType(const struct cl_operand *op) {
        const struct cl_accessor *ac = op->accessor;
        return (ac)
            ? ac->type
            : op->type;
    }
}

Var::Var():
    code(VAR_VOID)
{
}

Var::~Var()
{
}

Var::Var(EVar code_, const struct cl_operand *op):
    code(code_),
    loc(op->data.var->loc),
    uid(op->data.var->uid),
    initialized(op->data.var->initialized),
    isExtern(op->data.var->is_extern),
    mayBePointed(false)
{
    CL_BREAK_IF(CL_OPERAND_VAR != op->code);
    if (op->data.var->name)
        name = op->data.var->name;

    // dig type of variable
    this->type = digVarType(op);
    CL_BREAK_IF(!this->type);

    // check for eventual scope mismatch
    switch (code) {
        case VAR_GL:
            if (CL_SCOPE_GLOBAL == op->scope || CL_SCOPE_STATIC == op->scope)
                return;

            // invalid scope
            break;

        case VAR_LC:
        case VAR_FNC_ARG:
            if (CL_SCOPE_FUNCTION == op->scope)
                return;
            // fall through!

        case VAR_VOID:
            // invalid scope
            break;
    }

    CL_BREAK_IF("attempt to create invalid CodeStorage::Var object");
}

bool isOnStack(const Var &var)
{
    const EVar code = var.code;
    switch (code) {
        case VAR_FNC_ARG:
        case VAR_LC:
            return true;

        case VAR_VOID:
        case VAR_GL:
            return false;
    }

    CL_BREAK_IF("CodeStorage::isOnStack() got invalid CodeStorage::Var object");
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

VarDb::~VarDb()
{
    BOOST_FOREACH(const Var &var, vars_)
        BOOST_FOREACH(const Insn *insn, var.initials)
            destroyInsn(const_cast<Insn *>(insn));

    delete d;
}

Var& VarDb::operator[](int uid)
{
    return dbLookup(d->db, vars_, uid);
}

const Var& VarDb::operator[](int uid) const
{
    return dbConstLookup(d->db, vars_, uid);
}


// /////////////////////////////////////////////////////////////////////////////
// TypeDb implementation
struct TypeDb::Private {
    typedef std::map<int, const struct cl_type *> TMap;
    TMap db;

    int codePtrSizeof;
    int dataPtrSizeof;
    const struct cl_type *genericDataPtr;

    Private():
        codePtrSizeof(-1),
        dataPtrSizeof(-1),
        genericDataPtr(0)
    {
    }

    static void updatePtrSizeof(int size, int *pField);
    void digPtrSizeof(const struct cl_type *);
};

TypeDb::TypeDb():
    d(new Private)
{
}

TypeDb::~TypeDb()
{
    delete d;
}

void TypeDb::Private::updatePtrSizeof(int size, int *pField)
{
    CL_BREAK_IF(size <= 0);
    CL_BREAK_IF(-1 != *pField && *pField != size);
    *pField = size;
}

void TypeDb::Private::digPtrSizeof(const struct cl_type *clt)
{
    if (CL_TYPE_PTR != clt->code)
        return;

    CL_BREAK_IF(1 != clt->item_cnt);
    const struct cl_type *next = clt->items[0].type;
    const bool isCodePtr = (CL_TYPE_FNC == next->code);
    this->updatePtrSizeof(clt->size, (isCodePtr)
            ? &this->codePtrSizeof
            : &this->dataPtrSizeof);

    if (isCodePtr)
        return;

    if (!this->genericDataPtr || CL_TYPE_VOID == clt->code)
        this->genericDataPtr = clt;
}

bool TypeDb::insert(const struct cl_type *clt)
{
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
    types_.push_back(clt);

    d->digPtrSizeof(clt);
    return true;
}

int TypeDb::codePtrSizeof() const
{
    return d->codePtrSizeof;
}

int TypeDb::dataPtrSizeof() const
{
    return d->dataPtrSizeof;
}

const struct cl_type* TypeDb::genericDataPtr() const
{
    return d->genericDataPtr;
}

void readTypeTree(TypeDb &db, const struct cl_type *clt)
{
    if (!clt) {
#if 0
        CL_DEBUG("readTypeTree() got a NULL pointer");
#endif
        return;
    }

    // DFS through the type graph
    std::stack<const struct cl_type *> typeStack;
    typeStack.push(clt);
    while (!typeStack.empty()) {
        clt = typeStack.top();
        typeStack.pop();
        if (db.insert(clt)) {
            const int max = clt->item_cnt;
            const struct cl_type_item *items = clt->items;
            for (int i = 0; i < max; ++i)
                typeStack.push(items[i].type);
        }
    }
}

const struct cl_type* TypeDb::operator[](int uid) const
{
    typedef Private::TMap TDb;
    TDb &db = d->db;
    TDb::iterator iter = db.find(uid);
    if (db.end() == iter) {
        CL_DEBUG("TypeDb::insert() is unable to find the required cl_type: #"
                << uid);

        // we'll probably have to crash anyway
        CL_BREAK_IF("invalid call of TypeDb::operator[](int) const");
        return 0;
    }

    return iter->second;
}


// /////////////////////////////////////////////////////////////////////////////
// Block implementation
void Block::append(Insn *insn)
{
#ifndef NDEBUG
    if (!insns_.empty()) {
        // check insn sequence
        const Insn *last = insns_[insns_.size() - 1];
        CL_BREAK_IF(cl_is_term_insn(last->code));
    }
#endif
    insn->bb = this;
    insns_.push_back(insn);
}

void Block::appendPredecessor(Block *pred)
{
    inbound_.push_back(pred);
}

const Insn* Block::front() const
{
    CL_BREAK_IF(insns_.empty());
    return insns_.front();
}

const Insn* Block::back() const
{
    CL_BREAK_IF(insns_.empty());
    return insns_.back();
}

const TTargetList& Block::targets() const
{
    const Insn *last = this->back();
    CL_BREAK_IF(!cl_is_term_insn(last->code));

    return last->targets;
}

bool Block::isLoopEntry() const
{
    BOOST_FOREACH(const Block *ref, inbound_) {
        const Insn *term = ref->back();
        const TTargetList &tList = ref->targets();

        // go through loop closing edges and check their target
        BOOST_FOREACH(const unsigned idx, term->loopClosingTargets)
            if (tList[idx] == this)
                return true;
    }

    // no loop entry found
    return false;
}


// /////////////////////////////////////////////////////////////////////////////
// PointsTo implementation

namespace PointsTo {

bool isDead(const GlobalData &data) {
    return data.dead;
}

Item::Item() :
    code(PT_ITEM_VAR)
{
    data.var = 0;
}

Item::Item(ItemCodeE code_) :
    code(code_)
{
}

Item::Item(const Var *v) :
    code(PT_ITEM_VAR)
{
    data.var = v;
}

bool Item::isGlobal() const
{
    switch (code) {
        case PT_ITEM_MALLOC:
            return true;
        case PT_ITEM_VAR:
            return data.var->code == VAR_GL;
        default:
            return false;
    }
}

int Item::uid() const
{
    switch (code) {
        case PT_ITEM_VAR:
            return data.var->uid;
        case PT_ITEM_RET:
            // Just for now! -- this should be safe
            return data.fnc->def.data.cst.data.cst_fnc.uid;
        case PT_ITEM_MALLOC:
            return data.mallocId;
    }
    return 0;
}

const char *Item::name() const
{
    std::stringstream name;
    switch (code) {
        case PT_ITEM_VAR:
            name << data.var->uid << ":" << data.var->name;
            break;
        case PT_ITEM_RET:
            name << "fnc:" << nameOf(*data.fnc);
            break;
        case PT_ITEM_MALLOC:
            name << "heap_" << -data.mallocId;
    }

#define __nameRet_MAX_LEN 1024
    static char nameRet[__nameRet_MAX_LEN];
    nameRet[__nameRet_MAX_LEN - 1] = 0;
    strncpy(nameRet, name.str().c_str(), __nameRet_MAX_LEN -1);

    return nameRet;
}

const Var *Item::var() const
{
    if (code != PT_ITEM_VAR)
        return 0;

    return data.var;
}

Node::Node():
    isBlackHole(false)
{
}

Node::~Node()
{
}

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

ControlFlow::~ControlFlow()
{
    delete d;
}

ControlFlow& ControlFlow::operator=(const ControlFlow &ref)
{
    bbs_ = ref.bbs_;
    delete d;
    d = new Private(*ref.d);
    return *this;
}

const Block* ControlFlow::entry() const
{
    CL_BREAK_IF(bbs_.empty());
    return bbs_[0];
}

Block*& ControlFlow::operator[](const char *name)
{
    Block* &ref = dbLookup(d->db, bbs_, name, 0);
    if (!ref)
        // the object will be NOT destroyed by ControlFlow
        ref = new Block(this, name);

    return ref;
}

const Block* ControlFlow::operator[](const char *name) const
{
    return dbConstLookup(d->db, bbs_, name);
}


// /////////////////////////////////////////////////////////////////////////////
// Fnc implementation
inline const struct cl_cst& cstFromFnc(const Fnc &fnc)
{
    const struct cl_operand &op = fnc.def;
    CL_BREAK_IF(CL_OPERAND_CST != op.code);

    const struct cl_cst &cst = op.data.cst;
    CL_BREAK_IF(CL_TYPE_FNC != cst.code);

    return cst;
}

const char* nameOf(const Fnc &fnc)
{
    const struct cl_cst &cst = cstFromFnc(fnc);
    return cst.data.cst_fnc.name;
}

const struct cl_loc* locationOf(const Fnc &fnc)
{
    const struct cl_cst &cst = cstFromFnc(fnc);
    return &cst.data.cst_fnc.loc;
}

int uidOf(const Fnc &fnc)
{
    const struct cl_cst &cst = cstFromFnc(fnc);
    return cst.data.cst_fnc.uid;
}

bool isDefined(const Fnc &fnc)
{
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

FncDb::~FncDb()
{
    delete d;
}

FncDb& FncDb::operator=(const FncDb &ref)
{
    fncs_ = ref.fncs_;
    delete d;
    d = new Private(*ref.d);
    return *this;
}

Fnc*& FncDb::operator[](int uid)
{
    Fnc* &ref = dbLookup(d->db, fncs_, uid, 0);
    if (!ref)
        // the object will be NOT destroyed by FncDb
        ref = new Fnc;

    return ref;
}

const Fnc* FncDb::operator[](int uid) const
{
    return dbConstLookup(d->db, fncs_, uid);
}

const Fnc* fncByCfg(const ControlFlow *pCfg)
{
    const char *ptr = reinterpret_cast<const char *>(pCfg);
    ptr -= offsetof(Fnc, cfg);
    return reinterpret_cast<const Fnc *>(ptr);
}

} // namespace CodeStorage
