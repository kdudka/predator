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

#include "cld_argsub.hh"
#include "cl_decorator.hh"

#include <map>
#include <string>

class CldArgSubst: public ClDecoratorBase {
    public:
        CldArgSubst(ICodeListener *slave);

        virtual void file_open(const char *file_name) {
            loc_.currentFile    = file_name;
            fncLoc_.currentFile = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            fnc_    = fnc_name;
            fncLoc_ = loc;
            this->reset();
            ClDecoratorBase::fnc_open(loc, fnc_name, scope);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->regArg(arg_id, arg_src);
            ClDecoratorBase::fnc_arg_decl(arg_id, arg_src);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            struct cl_insn local_cli = *cli;
            loc_ = &cli->loc;

            switch (cli->code) {
                case CL_INSN_COND: {
                        struct cl_operand src = *(cli->data.insn_cond.src);
                        this->substArg(&src);
                        local_cli.data.insn_cond.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_RET: {
                        struct cl_operand src = *(cli->data.insn_ret.src);
                        this->substArg(&src);
                        local_cli.data.insn_ret.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_UNOP: {
                        struct cl_operand dst = *(cli->data.insn_unop.dst);
                        struct cl_operand src = *(cli->data.insn_unop.src);
                        this->substArg(&dst);
                        this->substArg(&src);
                        local_cli.data.insn_unop.dst = &dst;
                        local_cli.data.insn_unop.src = &src;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_BINOP: {
                        struct cl_operand dst = *(cli->data.insn_binop.dst);
                        struct cl_operand src1 = *(cli->data.insn_binop.src1);
                        struct cl_operand src2 = *(cli->data.insn_binop.src2);
                        this->substArg(&dst);
                        this->substArg(&src1);
                        this->substArg(&src2);
                        local_cli.data.insn_binop.dst = &dst;
                        local_cli.data.insn_binop.src1 = &src1;
                        local_cli.data.insn_binop.src2 = &src2;
                        ClDecoratorBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_NOP:
                case CL_INSN_JMP:
                case CL_INSN_ABORT:
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
            loc_ = loc;

            this->substArg(&local_dst);
            this->substArg(&local_fnc);

            ClDecoratorBase::insn_call_open(loc, &local_dst, &local_fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            struct cl_operand local_arg_src = *arg_src;
            this->substArg(&local_arg_src);
            ClDecoratorBase::insn_call_arg(arg_id, &local_arg_src);
        }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            struct cl_operand local_src = *src;
            loc_ = loc;

            this->substArg(&local_src);

            ClDecoratorBase::insn_switch_open(loc, &local_src);
        }

    private:
        // we use map because some arg_id positions may be omitted
        typedef std::map<int, std::string> TMap;

        std::string             fnc_;
        Location                fncLoc_;
        Location                loc_;

        TMap                    map_;
        int                     last_;

    private:
        void reset();
        void regArg(int arg_id, const struct cl_operand *arg_src);

        // we do not return reference to string because we return NULL
        // when arg position is not found
        const char* argLookup(int);

        void substArg(struct cl_operand *op);
};

CldArgSubst::CldArgSubst(ICodeListener *slave):
    ClDecoratorBase(slave),
    last_(0)
{
}

void CldArgSubst::reset() {
    map_.clear();
    last_ = 0;
}

void CldArgSubst::regArg(int arg_id, const struct cl_operand *arg_src) {
    TMap::iterator i = map_.find(arg_id);
    if (map_.end() != i) {
        CL_MSG_STREAM(cl_error, LocationWriter(0, &fncLoc_) << "error: "
                << "argument #" << arg_id
                << " of function '" << fnc_ << "'"
                << " is already declared as '"
                << i->second << "'");
        return;
    }

    if (arg_src->code != CL_OPERAND_VAR)
        TRAP;

    map_[arg_id] = arg_src->data.var.name;
}

const char* CldArgSubst::argLookup(int arg) {
    TMap::iterator i = map_.find(arg);
    if (map_.end() == i) {
        CL_MSG_STREAM(cl_error, LocationWriter(0, &loc_) << "error: "
                << "argument #" << arg
                << " of function '" << fnc_ << "'"
                << " was not declared");
        CL_MSG_STREAM(cl_note, LocationWriter(0, &fncLoc_) << "note: "
                << "function '" << fnc_ << "'"
                << " was declarede here");
        return NULL;
    }

    return i->second.c_str();
}

void CldArgSubst::substArg(struct cl_operand *op) {
    if (CL_OPERAND_ARG != op->code)
        return;

    op->code = CL_OPERAND_VAR;
    op->data.var.name = this->argLookup(op->data.arg.id);
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldArgSubst(ICodeListener *slave) {
    return new CldArgSubst(slave);
}
