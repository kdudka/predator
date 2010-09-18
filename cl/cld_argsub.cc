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

#include "config_cl.h"
#include "cld_argsub.hh"

#include <cl/cl_msg.hh>
#include <cl/location.hh>

#include "cld_optrans.hh"

#include <map>
#include <string>

#ifndef CLD_ARG_SUBST_KEEP_TYPE_REF
#   define CLD_ARG_SUBST_KEEP_TYPE_REF 0
#endif

class CldArgSubst: public CldOpTransBase {
    public:
        CldArgSubst(ICodeListener *slave);

        virtual void file_open(const char *file_name) {
            loc_.currentFile    = file_name;
            fncLoc_.currentFile = file_name;
            CldOpTransBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            fnc_    = fnc->data.cst.data.cst_fnc.name;
            fncLoc_ = &fnc->loc;
            map_.clear();
            CldOpTransBase::fnc_open(fnc);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->regArg(arg_id, arg_src);
            CldOpTransBase::fnc_arg_decl(arg_id, arg_src);
        }

    private:
        struct ArgDecl {
            int             varId;
            std::string     varName;
#if CLD_ARG_SUBST_KEEP_TYPE_REF
            struct cl_type  *type;
#endif
        };

        // we use map because some arg_id positions may be omitted
        typedef std::map<int, ArgDecl> TMap;

        std::string             fnc_;
        Location                fncLoc_;
        Location                loc_;

        TMap                    map_;

    private:
        void regArg(int arg_id, const struct cl_operand *arg_src);

    protected:
        virtual void modifyOperand(struct cl_operand *);
};

CldArgSubst::CldArgSubst(ICodeListener *slave):
    CldOpTransBase(slave)
{
}

void CldArgSubst::regArg(int arg_id, const struct cl_operand *arg_src) {
    TMap::iterator i = map_.find(arg_id);
    if (map_.end() != i) {
        CL_ERROR_MSG(LocationWriter(0, &fncLoc_), "argument #"
                << arg_id << " of function '" << fnc_ << "'"
                << " is already declared as '"
                << i->second.varName << "'");
        return;
    }

    if (arg_src->code != CL_OPERAND_VAR)
        TRAP;

    ArgDecl &ad = map_[arg_id];
    ad.varId    = arg_src->data.var.id;
    ad.varName  = arg_src->data.var.name;
#if CLD_ARG_SUBST_KEEP_TYPE_REF
    ad.type     = arg_src->type;
#endif
}

void CldArgSubst::modifyOperand(struct cl_operand *op) {
    if (CL_OPERAND_ARG != op->code)
        return;

    int arg = op->data.arg.id;
    TMap::iterator i = map_.find(arg);
    if (map_.end() == i) {
        CL_ERROR_MSG(LocationWriter(0, &loc_), "argument #"
                << arg << " of function '" << fnc_ << "'"
                << " was not declared");
        CL_NOTE_MSG(LocationWriter(0, &fncLoc_), "function '"
                << fnc_ << "'" << " was declarede here");
        return;
    }

    const ArgDecl &ad = i->second;
    op->data.var.id     = ad.varId;
    op->data.var.name   = ad.varName.c_str();

    op->code            = CL_OPERAND_VAR;
    op->scope           = CL_SCOPE_FUNCTION;
#if CLD_ARG_SUBST_KEEP_TYPE_REF
    op->type            = ad.type;
#endif
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_argsub.hh for more details
ICodeListener* createCldArgSubst(ICodeListener *slave) {
    return new CldArgSubst(slave);
}
