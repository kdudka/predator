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

#ifndef H_GUARD_CL_STORAGE_H
#define H_GUARD_CL_STORAGE_H

/**
 * @file cl_storage.hh
 * Definition of ClStorageBuilder base class
 */

#include "cl.hh"

namespace CodeStorage {
    struct Storage;
}

/**
 * base class for all CodeStorage based code listeners, see run()
 * @note design pattern @b template @b method
 */
class ClStorageBuilder: public ICodeListener {
    protected:
        ClStorageBuilder();

        /**
         * once the Storage model is built this @b template @b method is called
         * to process its content, run some analysis, etc.
         */
        virtual void run(CodeStorage::Storage &) = 0;

    public:
        virtual ~ClStorageBuilder();

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            const struct cl_operand *fnc);

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn(
            const struct cl_insn    *cli);

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc);

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void insn_call_close();

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src);

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label);

        virtual void insn_switch_close();

        virtual void finalize();

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_CL_STORAGE_H */
