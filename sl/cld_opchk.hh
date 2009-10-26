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

#include "cl_decorator.hh"

class CldOpCheckerBase: public ClDecoratorBase {
    public:
        CldOpCheckerBase(ICodeListener *slave);

    protected:
        const Location& lastLocation() const {
            return loc_;
        }

    protected:
        /**
         * @note design pattern @b template @b method
         */
        virtual void checkDstOperand(const struct cl_operand *) = 0;

        /**
         * @note design pattern @b template @b method
         */
        virtual void checkSrcOperand(const struct cl_operand *) = 0;

    public:
        virtual void file_open(
            const char              *file_name)
        {
            loc_.currentFile = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            loc_ = &fnc->loc;
            ClDecoratorBase::fnc_open(fnc);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->handleDstSrc(arg_src);
            ClDecoratorBase::fnc_arg_decl(arg_id, arg_src);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            loc_ = &cli->loc;

            switch (cli->code) {
                case CL_INSN_COND:
                    this->handleSrc(cli->data.insn_cond.src);
                    break;

                case CL_INSN_RET:
                    this->handleSrc(cli->data.insn_ret.src);
                    break;

                case CL_INSN_UNOP:
                    this->handleDstSrc(cli->data.insn_unop.dst);
                    this->handleSrc(cli->data.insn_unop.src);
                    break;

                case CL_INSN_BINOP:
                    this->handleDstSrc(cli->data.insn_binop.dst);
                    this->handleSrc(cli->data.insn_binop.src1);
                    this->handleSrc(cli->data.insn_binop.src2);
                    break;

                default:
                    break;
            }

            ClDecoratorBase::insn(cli);
        }

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            loc_ = loc;
            this->handleDstSrc(dst);
            this->handleSrc(fnc);
            ClDecoratorBase::insn_call_open(loc, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->handleSrc(arg_src);
            ClDecoratorBase::insn_call_arg(arg_id, arg_src);
        }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            loc_ = loc;
            this->handleSrc(src);
            ClDecoratorBase::insn_switch_open(loc, src);
        }

    private:
        Location            loc_;

    private:
        void handleArrayIdx(const struct cl_operand *);
        void handleSrc(const struct cl_operand *);
        void handleDstSrc(const struct cl_operand *);
};
