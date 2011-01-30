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

#ifndef H_GUARD_CL_FILTER_H
#define H_GUARD_CL_FILTER_H

/**
 * @file cl_filter.hh
 * ClFilterBase - base class for all code listener @b filters
 */

#include "cl.hh"

/**
 * base class for all ICodeListener filters
 *
 * The class itself can't be instantiated.  You always need to inherit the
 * class.  The only thing which has to be implemented in the derived class is
 * a constructor, which calls the parent constructor with an instance of
 * ICodeListener as the argument.  All other methods which are not overridden
 * will be forwarded to that instance of ICodeListener.
 * @note design pattern @b filter
 */
class ClFilterBase: public ICodeListener {
    public:
        /**
         * @attention the slave ICodeListener will be deleted automatically on
         * the object's destruction
         */
        virtual ~ClFilterBase() {
            delete slave_;
        }

        virtual void file_open(
            const char              *file_name)
        {
            slave_->file_open(file_name);
        }

        virtual void file_close() {
            slave_->file_close();
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            slave_->fnc_open(fnc);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            slave_->fnc_arg_decl(arg_id, arg_src);
        }

        virtual void fnc_close() {
            slave_->fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            slave_->bb_open(bb_name);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            slave_->insn(cli);
        }

        virtual void insn_call_open(
            const struct cl_loc     *loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            slave_->insn_call_open(loc, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            slave_->insn_call_arg(arg_id, arg_src);
        }

        virtual void insn_call_close() {
            slave_->insn_call_close();
        }

        virtual void insn_switch_open(
            const struct cl_loc     *loc,
            const struct cl_operand *src)
        {
            slave_->insn_switch_open(loc, src);
        }

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            slave_->insn_switch_case(loc, val_lo, val_hi, label);
        }

        virtual void insn_switch_close() {
            slave_->insn_switch_close();
        }

        virtual void acknowledge() {
            slave_->acknowledge();
        }

    protected:
        /**
         * @param slave An instance of ICodeListener.  All methods which are not
         * overridden in the derived class will be forwarded to this instance.
         * @copydoc ~ClFilterBase()
         */
        ClFilterBase(ICodeListener *slave):
            slave_(slave)
        {
        }

    private:
        ICodeListener *slave_;
};

#endif /* H_GUARD_CL_FILTER_H */
