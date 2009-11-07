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

#include <cstring>
#include <map>
#include <stack>

#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

namespace Storage {

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
    clt(op->type),
    uid(op->data.var.id)
{
    if (CL_OPERAND_VAR != op->code)
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

VarDb::~VarDb() {
    delete d;
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

void TypeDb::insert(const struct cl_type *clt) {
    if (!clt) {
        CL_DEBUG("TypeDb::insert() got a NULL pointer");
        return;
    }
    cl_type_uid_t uid = clt->uid;

    typedef Private::TMap TDb;
    TDb &db = d->db;
    TDb::iterator iter = db.find(uid);
    if (db.end() == iter) {
        // insert type into db
        db[uid] = clt;
        return;
    }

    if (uid != iter->second->uid) {
        CL_DEBUG("TypeDb::insert() has detected attempt to redefine cl_type");
        TRAP;
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
// Insn implementation
namespace {
    void dupString(const char *&str) {
        str = strdup(str);
    }

    void freeString(const char *str) {
        free(const_cast<char *>(str));
    }

    template <typename TFnc>
    void handleCstStrings(TFnc fnc, struct cl_cst &cst) {
        enum cl_type_e code = cst.code;
        switch (code) {
            case CL_TYPE_FNC:
                fnc(cst.data.cst_fnc.name);
                break;

            case CL_TYPE_STRING:
                fnc(cst.data.cst_string.value);
                break;

            default:
                break;
        }
    }

    template <typename TFnc>
    void handleOperandStrings(TFnc fnc, struct cl_operand *op) {
        enum cl_operand_e code = op->code;
        switch (code) {
            case CL_OPERAND_VAR:
                fnc(op->data.var.name);
                break;

            case CL_OPERAND_CST:
                handleCstStrings(fnc, op->data.cst);
                break;

            default:
                break;
        }
    }

    template <class TStack>
    void cloneAccessor(struct cl_accessor **dst, const struct cl_accessor *src,
                       TStack &opStack)
    {
        while (src) {
            // clone current cl_accessor object
            *dst = new struct cl_accessor(*src);

            if (CL_ACCESSOR_DEREF_ARRAY == src->code) {
                // clone array index
                struct cl_operand const *idxSrc = src->data.array.index;
                struct cl_operand *idxDst = new struct cl_operand(*idxSrc);
                (*dst)->data.array.index = idxDst;

                // schedule array index as an operand for the next wheel
                opStack.push(idxDst, idxSrc);
            }

            // move to next cl_accessor object (if any)
            src = src->next;
            dst = &(*dst)->next;
        }
    }

    // FIXME: I guess this will need a debugger first :-)
    void storeOperand(struct cl_operand &dst, const struct cl_operand *src) {
        // shallow copy
        dst = *src;

        // clone objects recursively using std::stack
        typedef std::pair<struct cl_operand *, const struct cl_operand *> TPair;
        typedef std::stack<TPair> TStack;
        TStack opStack;
        opStack.push(&dst, src);
        while (!opStack.empty()) {
            struct cl_operand *cDst;
            struct cl_operand const *cSrc;
            boost::tie(cDst, cSrc) = opStack.top();
            opStack.pop();

            // clone list of cl_accessor objects
            // and schedule all array indexes for the next wheel eventually
            cloneAccessor(&cDst->accessor, cSrc->accessor, opStack);

            // duplicate all strings
            handleOperandStrings(dupString, cDst);
        }
    }

    void releaseOperand(struct cl_operand &ref) {
        // use std::stack to avoid recursion
        std::stack<struct cl_operand *> opStack;
        opStack.push(&ref);
        while (!opStack.empty()) {
            struct cl_operand *op = opStack.top();
            opStack.pop();

            // remove all duplicated strings
            handleOperandStrings(freeString, op);

            // destroy cl_accessor objects recursively
            struct cl_accessor *ac = op->accessor;
            while (ac) {
                struct cl_accessor *next = ac->next;
                if (CL_ACCESSOR_DEREF_ARRAY == ac->code)
                    opStack.push(ac->data.array.index);

                delete ac;
                ac = next;
            }

            if (op != &ref)
                // free cloned array index
                delete op;
        }

        // all accessors freed
        ref.accessor = 0;
    }
}

Insn::Insn(enum cl_insn_e code_, const struct cl_location &loc_):
    code(code_),
    loc(loc_)
{
    switch (code) {
        case CL_INSN_CALL:
        case CL_INSN_SWITCH:
            break;

        default:
            // wrong constructor used
            TRAP;
    }
}

Insn::Insn(const struct cl_insn *cli, ControlFlow &cfg):
    code(cli->code),
    loc(cli->loc)
{
    switch (code) {
        case CL_INSN_NOP:
            TRAP;
            break;

        case CL_INSN_JMP:
            targets.push_back(&cfg[cli->data.insn_jmp.label]);
            break;

        case CL_INSN_COND:
            operands.resize(1);
            storeOperand(operands[0], cli->data.insn_cond.src);

            targets.resize(2);
            targets[0] = &cfg[cli->data.insn_cond.then_label];
            targets[1] = &cfg[cli->data.insn_cond.else_label];
            break;

        case CL_INSN_RET:
            operands.resize(1);
            storeOperand(operands[0], cli->data.insn_ret.src);
            // fall through!

        case CL_INSN_ABORT:
            break;

        case CL_INSN_UNOP:
            this->subCode = static_cast<int> (cli->data.insn_unop.code);
            operands.resize(2);
            storeOperand(operands[0], cli->data.insn_unop.dst);
            storeOperand(operands[1], cli->data.insn_unop.src);
            break;

        case CL_INSN_BINOP:
            this->subCode = static_cast<int> (cli->data.insn_binop.code);
            operands.resize(3);
            storeOperand(operands[0], cli->data.insn_binop.dst);
            storeOperand(operands[1], cli->data.insn_binop.src1);
            storeOperand(operands[2], cli->data.insn_binop.src2);
            break;

        case CL_INSN_CALL:
        case CL_INSN_SWITCH:
            // wrong constructor used
            TRAP;
    }
}

Insn::~Insn() {
    BOOST_FOREACH(struct cl_operand &op, operands) {
        releaseOperand(op);
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

ControlFlow::~ControlFlow() {
    delete d;
}

Block& ControlFlow::operator[](const char *name) {
    return dbLookup(d->db, bbs_, name, Block(this, name));
}

const Block& ControlFlow::operator[](const char *name) const {
    return dbConstLookup(d->db, bbs_, name);
}


// /////////////////////////////////////////////////////////////////////////////
// FncMap implementation
struct FncMap::Private {
    typedef std::map<int, unsigned> TMap;
    TMap db;
};

FncMap::FncMap():
    d(new Private)
{
}

FncMap::~FncMap() {
    delete d;
}

Fnc& FncMap::operator[](int uid) {
    return dbLookup(d->db, fncs_, uid);
}

const Fnc& FncMap::operator[](int uid) const {
    return dbConstLookup(d->db, fncs_, uid);
}


// /////////////////////////////////////////////////////////////////////////////
// FileMap implementation
struct FileMap::Private {
    typedef std::map<std::string, unsigned> TMap;
    TMap db;
};

FileMap::FileMap():
    d(new Private)
{
}

FileMap::~FileMap() {
    delete d;
}

File& FileMap::operator[](const char *name) {
    boost::filesystem::path filePath(name);
    filePath.normalize();
    const std::string &canonName = filePath.string();
    return dbLookup(d->db, files_, canonName, File(canonName));
}

const File& FileMap::operator[](const char *name) const {
    boost::filesystem::path filePath(name);
    filePath.normalize();
    return dbConstLookup(d->db, files_, filePath.string());
}


} // namespace Storage
