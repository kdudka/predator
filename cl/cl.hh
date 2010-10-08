/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_CL_H
#define H_GUARD_CL_H

/**
 * @file cl.hh
 * ICodeListener interface
 */

#include <cl/code_listener.h>

/**
 * C++ interface for listener objects. It can be wrapped to struct
 * cl_code_listener object when exposing to pure C world.
 *
 * @copydoc cl_code_listener
 */
class ICodeListener {
    public:
        virtual ~ICodeListener() { }

        /// See cl_code_listener::file_open
        virtual void file_open(
            const char              *file_name)
            = 0;

        /// See cl_code_listener::file_close
        virtual void file_close()
            = 0;

        /// See cl_code_listener::fnc_open
        virtual void fnc_open(
            const struct cl_operand *fnc)
            = 0;

        /// See cl_code_listener::fnc_arg_decl
        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
            = 0;

        /// See cl_code_listener::fnc_close
        virtual void fnc_close()
            = 0;

        /// See cl_code_listener::bb_open
        virtual void bb_open(
            const char              *bb_name)
            = 0;

        /// See cl_code_listener::insn
        virtual void insn(
            const struct cl_insn    *cli)
            = 0;

        /// See cl_code_listener::insn_call_open
        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
            = 0;

        /// See cl_code_listener::insn_call_arg
        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
            = 0;

        /// See cl_code_listener::insn_call_close
        virtual void insn_call_close()
            = 0;

        /// See cl_code_listener::insn_switch_open
        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
            = 0;

        /// See cl_code_listener::insn_switch_case
        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
            = 0;

        /// See cl_code_listener::insn_switch_close
        virtual void insn_switch_close()
            = 0;

        /// See cl_code_listener::acknowledge
        virtual void acknowledge()
            = 0;
};

#endif /* H_GUARD_CL_H */
