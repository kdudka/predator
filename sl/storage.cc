/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

#include "storage.hh"

#include "cl_private.hh"

#include <map>
#include <stack>

#include <boost/filesystem/path.hpp>

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
    uid(op->data.var.id)
{
    switch (op->code) {
        case CL_OPERAND_VAR:
            if (op->data.var.name)
                name = op->data.var.name;
            // fall through!

        case CL_OPERAND_REG:
            break;

        default:
            // unexpected operand type
            TRAP;
    }

    // dig type of variable
    clt = digVarType(op);
    if (!clt)
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
        case VAR_REG:
            if (CL_SCOPE_FUNCTION == op->scope)
                break;
            // fall through!

        case VAR_VOID:
            TRAP;
    }
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
    d(new Private(*ref.d))
{
}

VarDb::~VarDb() {
    delete d;
}

VarDb& VarDb::operator=(const VarDb &ref) {
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
    typedef std::map<cl_type_uid_t, const struct cl_type *> TMap;
    TMap db;
};

TypeDb::TypeDb():
    d(new Private)
{
}

TypeDb::~TypeDb() {
    delete d;
}

bool TypeDb::insert(const struct cl_type *clt) {
    if (!clt) {
        CL_DEBUG("TypeDb::insert() got a NULL pointer");
        return false;
    }
    cl_type_uid_t uid = clt->uid;

    typedef Private::TMap TDb;
    TDb &db = d->db;
    TDb::iterator iter = db.find(uid);
    if (db.end() != iter)
        return false;

    // insert type into db
    db[uid] = clt;
    return true;
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
namespace {
    // TODO: move the implementation to code_listener.h
    bool isTermInsn(enum cl_insn_e code) {
        switch (code) {
            case CL_INSN_JMP:
            case CL_INSN_COND:
            case CL_INSN_RET:
            case CL_INSN_ABORT:
            case CL_INSN_SWITCH:
                return true;

            case CL_INSN_NOP:
            case CL_INSN_UNOP:
            case CL_INSN_BINOP:
            case CL_INSN_CALL:
            default:
                return false;
        }
    }
}

void Block::append(const Insn *insn) {
    if (!insns_.empty()) {
        // check insn sequence
        const Insn *last = insns_[insns_.size() - 1];
        if (isTermInsn(last->code))
            // invalid insn sequence
            TRAP;
    }

    insns_.push_back(insn);
}

const TTargetList& Block::targets() const {
    if (insns_.empty())
        // Oops, we are asked for targets without any insn inside. We
        // can still return a reference to an empty vector in such
        // cases, but is it actually useful?
        TRAP;

    const Insn *last = insns_[insns_.size() - 1];
    if (!isTermInsn(last->code))
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
    d(new Private(*ref.d))
{
}

ControlFlow::~ControlFlow() {
    delete d;
}

ControlFlow& ControlFlow::operator=(const ControlFlow &ref) {
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
const char* nameOf(const Fnc &fnc) {
    const struct cl_operand &op = fnc.def;
    if (CL_OPERAND_CST != op.code)
        TRAP;

    const struct cl_cst &cst = op.data.cst;
    if (CL_TYPE_FNC != cst.code)
        TRAP;

    return cst.data.cst_fnc.name;
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
    d(new Private(*ref.d))
{
}

FncDb::~FncDb() {
    delete d;
}

FncDb& FncDb::operator=(const FncDb &ref) {
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


// /////////////////////////////////////////////////////////////////////////////
// FileDb implementation
struct FileDb::Private {
    typedef std::map<std::string, unsigned> TMap;
    TMap db;
};

FileDb::FileDb():
    d(new Private)
{
}

FileDb::FileDb(const FileDb &ref):
    d(new Private(*ref.d))
{
}

FileDb::~FileDb() {
    delete d;
}

FileDb& FileDb::operator=(const FileDb &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
}

File*& FileDb::operator[](const char *name) {
    boost::filesystem::path filePath(name);
    filePath.normalize();
    const std::string &canonName = filePath.string();

    File* &ref = dbLookup(d->db, files_, canonName, 0);
    if (!ref)
        // XXX: the object will be NOT destroyed by File
        ref = new File(canonName);

    return ref;
}

const File* FileDb::operator[](const char *name) const {
    boost::filesystem::path filePath(name);
    filePath.normalize();
    return dbConstLookup(d->db, files_, filePath.string());
}

} // namespace CodeStorage
