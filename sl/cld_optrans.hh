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

class CldOpTransBase: public ClDecoratorBase {
    public:
        CldOpTransBase(ICodeListener *slave);

    protected:
        /**
         * @note design pattern @b template @b method
         */
        virtual void modifyOperand(struct cl_operand *) = 0;

    public:
        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            struct cl_operand local_src = *arg_src;
            CldOpTransBase::cloneAccessor(&local_src);

            this->traverseOperand(&local_src);

            ClDecoratorBase::fnc_arg_decl(arg_id, &local_src);
            CldOpTransBase::freeClonedAccessor(&local_src);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            struct cl_insn local_cli = *cli;

            switch (cli->code) {
                case CL_INSN_COND: {
                        struct cl_operand src = *(cli->data.insn_cond.src);
                        CldOpTransBase::cloneAccessor(&src);

                        this->traverseOperand(&src);
                        local_cli.data.insn_cond.src = &src;

                        ClDecoratorBase::insn(&local_cli);
                        CldOpTransBase::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_RET: {
                        struct cl_operand src = *(cli->data.insn_ret.src);
                        CldOpTransBase::cloneAccessor(&src);

                        this->traverseOperand(&src);
                        local_cli.data.insn_ret.src = &src;

                        ClDecoratorBase::insn(&local_cli);
                        CldOpTransBase::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_UNOP: {
                        struct cl_operand dst = *(cli->data.insn_unop.dst);
                        struct cl_operand src = *(cli->data.insn_unop.src);

                        CldOpTransBase::cloneAccessor(&dst);
                        CldOpTransBase::cloneAccessor(&src);

                        this->traverseOperand(&dst);
                        this->traverseOperand(&src);

                        local_cli.data.insn_unop.dst = &dst;
                        local_cli.data.insn_unop.src = &src;
                        ClDecoratorBase::insn(&local_cli);

                        CldOpTransBase::freeClonedAccessor(&dst);
                        CldOpTransBase::freeClonedAccessor(&src);
                    }
                    break;

                case CL_INSN_BINOP: {
                        struct cl_operand dst = *(cli->data.insn_binop.dst);
                        struct cl_operand src1 = *(cli->data.insn_binop.src1);
                        struct cl_operand src2 = *(cli->data.insn_binop.src2);

                        CldOpTransBase::cloneAccessor(&dst);
                        CldOpTransBase::cloneAccessor(&src1);
                        CldOpTransBase::cloneAccessor(&src2);

                        this->traverseOperand(&dst);
                        this->traverseOperand(&src1);
                        this->traverseOperand(&src2);

                        local_cli.data.insn_binop.dst = &dst;
                        local_cli.data.insn_binop.src1 = &src1;
                        local_cli.data.insn_binop.src2 = &src2;
                        ClDecoratorBase::insn(&local_cli);

                        CldOpTransBase::freeClonedAccessor(&dst);
                        CldOpTransBase::freeClonedAccessor(&src1);
                        CldOpTransBase::freeClonedAccessor(&src2);
                    }
                    break;

                default:
                    ClDecoratorBase::insn(cli);
                    break;
            }
        }

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            struct cl_operand local_dst = *dst;
            struct cl_operand local_fnc = *fnc;

            CldOpTransBase::cloneAccessor(&local_dst);
            CldOpTransBase::cloneAccessor(&local_fnc);

            this->traverseOperand(&local_dst);
            this->traverseOperand(&local_fnc);

            ClDecoratorBase::insn_call_open(loc, &local_dst, &local_fnc);

            CldOpTransBase::freeClonedAccessor(&local_dst);
            CldOpTransBase::freeClonedAccessor(&local_fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            struct cl_operand local_src = *arg_src;
            CldOpTransBase::cloneAccessor(&local_src);

            this->traverseOperand(&local_src);

            ClDecoratorBase::insn_call_arg(arg_id, &local_src);
            CldOpTransBase::freeClonedAccessor(&local_src);
        }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            struct cl_operand local_src = *src;
            CldOpTransBase::cloneAccessor(&local_src);

            this->traverseOperand(&local_src);

            ClDecoratorBase::insn_switch_open(loc, &local_src);
            CldOpTransBase::freeClonedAccessor(&local_src);
        }

        virtual void finalize() { }

    private:
        static void cloneAccessor(struct cl_operand *);
        static void freeClonedAccessor(struct cl_operand *);

    private:
        void traverseOperand(struct cl_operand *op);
};
