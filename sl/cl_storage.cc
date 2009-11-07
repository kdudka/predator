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
    // TODO
};

ClStorageBuilder::ClStorageBuilder():
    d(new Private)
{
}

ClStorageBuilder::~ClStorageBuilder() {
    // TODO: this->run(...)
    delete d;
}

void file_open(const char *file_name) {
    // TODO
}

void file_close() {
    // TODO
}

void fnc_open(const struct cl_operand *fnc) {
    // TODO
}

void fnc_arg_decl(int arg_id, const struct cl_operand *arg_src) {
    // TODO
}

void fnc_close() {
    // TODO
}

void bb_open(const char *bb_name) {
    // TODO
}

void insn(const struct cl_insn *cli) {
    // TODO
}

void insn_call_open(
    const struct cl_location*loc,
    const struct cl_operand *dst,
    const struct cl_operand *fnc)
{
    // TODO
}

void insn_call_arg(int arg_id, const struct cl_operand *arg_src) {
    // TODO
}

void insn_call_close() {
    // TODO
}

void insn_switch_open(
    const struct cl_location*loc,
    const struct cl_operand *src)
{
    // TODO
}

void insn_switch_case(
    const struct cl_location*loc,
    const struct cl_operand *val_lo,
    const struct cl_operand *val_hi,
    const char              *label)
{
    // TODO
}

void insn_switch_close() {
    // TODO
}
