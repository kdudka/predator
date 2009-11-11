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

#include <map>

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

FncMap::FncMap(const FncMap &ref):
    d(new Private(*ref.d))
{
}

FncMap::~FncMap() {
    delete d;
}

FncMap& FncMap::operator=(const FncMap &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
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

FileMap::FileMap(const FileMap &ref):
    d(new Private(*ref.d))
{
}

FileMap::~FileMap() {
    delete d;
}

FileMap& FileMap::operator=(const FileMap &ref) {
    delete d;
    d = new Private(*ref.d);
    return *this;
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


} // namespace CodeStorage
