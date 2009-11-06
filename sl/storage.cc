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

namespace Storage {

// /////////////////////////////////////////////////////////////////////////////
// Var implementation
Var::Var(EVar, const struct cl_operand *op) {
    // TODO
}

Var::~Var() {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// VarDb implementation
struct VarDb::Private {
    // TODO
};

VarDb::VarDb():
    d(new Private)
{
    // TODO
}

VarDb::~VarDb() {
    delete d;
}

void VarDb::insert(const struct cl_operand *op) {
    // TODO
}

const Var& VarDb::operator[](int idx) const {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// TypeDb implementation
struct TypeDb::Private {
    // TODO
};

TypeDb::TypeDb():
    d(new Private)
{
    // TODO
}

TypeDb::~TypeDb() {
    delete d;
}

void TypeDb::insert(const struct cl_type *clt) {
    // TODO
}

const struct cl_type* TypeDb::operator[](int idx) const {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// Insn implementation
Insn::Insn(const struct cl_insn *cli) {
    // TODO
}

Insn::~Insn() {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// ControlFlow implementation
ControlFlow::ControlFlow() {
    // TODO
}

ControlFlow::~ControlFlow() {
    // TODO
}

void ControlFlow::insert(Block *bb) {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// FncMap implementation
struct FncMap::Private {
    // TODO
};

FncMap::FncMap():
    d(new Private)
{
    // TODO
}

FncMap::~FncMap() {
    delete d;
}

Fnc& FncMap::operator[](int uid) {
    // TODO
}

const Fnc& FncMap::operator[](int uid) const {
    // TODO
}


// /////////////////////////////////////////////////////////////////////////////
// FileMap implementation
struct FileMap::Private {
    // TODO
};

FileMap::FileMap():
    d(new Private)
{
    // TODO
}

FileMap::~FileMap() {
    delete d;
}

File& FileMap::operator[](const char *name) {
    // TODO
}

const File& FileMap::operator[](const char *name) const {
    // TODO
}


} // namespace Storage
