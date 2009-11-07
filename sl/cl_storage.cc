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

#include "cl_storage.hh"
#include "storage.hh"

using namespace Storage;

struct ClStorageBuilder::Private {
    // FIXME: Oops, the identifiers are awkward, thus we need the 'struct' here
    struct Storage stor;
};

ClStorageBuilder::ClStorageBuilder():
    d(new Private)
{
}

ClStorageBuilder::~ClStorageBuilder() {
    delete d;
}

void ClStorageBuilder::finalize() {
    this->run(d->stor);
}

void ClStorageBuilder::file_open(const char *file_name) {
    // TODO
}

void ClStorageBuilder::file_close() {
    // TODO
}

void ClStorageBuilder::fnc_open(const struct cl_operand *fnc) {
    // TODO
}

void ClStorageBuilder::fnc_arg_decl(int arg_id,
                                    const struct cl_operand *arg_src)
{
    // TODO
}

void ClStorageBuilder::fnc_close() {
    // TODO
}

void ClStorageBuilder::bb_open(const char *bb_name) {
    // TODO
}

void ClStorageBuilder::insn(const struct cl_insn *cli) {
    // TODO
}

void ClStorageBuilder::insn_call_open(
    const struct cl_location*loc,
    const struct cl_operand *dst,
    const struct cl_operand *fnc)
{
    // TODO
}

void ClStorageBuilder::insn_call_arg(int arg_id,
                                     const struct cl_operand *arg_src)
{
    // TODO
}

void ClStorageBuilder::insn_call_close() {
    // TODO
}

void ClStorageBuilder::insn_switch_open(
    const struct cl_location*loc,
    const struct cl_operand *src)
{
    // TODO
}

void ClStorageBuilder::insn_switch_case(
    const struct cl_location*loc,
    const struct cl_operand *val_lo,
    const struct cl_operand *val_hi,
    const char              *label)
{
    // TODO
}

void ClStorageBuilder::insn_switch_close() {
    // TODO
}
