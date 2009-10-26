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
#include "cld_optrans.hh"

#include <map>
#include <string>

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
            fnc_    = fnc->data.cst_fnc.name;
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
        CL_MSG_STREAM(cl_error, LocationWriter(0, &fncLoc_) << "error: "
                << "argument #" << arg_id
                << " of function '" << fnc_ << "'"
                << " is already declared as '"
                << i->second.varName << "'");
        return;
    }

    if (arg_src->code != CL_OPERAND_VAR)
        TRAP;

    ArgDecl &ad = map_[arg_id];
    ad.varId    = arg_src->data.var.id;
    ad.varName  = arg_src->data.var.name;
}

void CldArgSubst::modifyOperand(struct cl_operand *op) {
    if (CL_OPERAND_ARG != op->code)
        return;

    int arg = op->data.arg.id;
    TMap::iterator i = map_.find(arg);
    if (map_.end() == i) {
        CL_MSG_STREAM(cl_error, LocationWriter(0, &loc_) << "error: "
                << "argument #" << arg
                << " of function '" << fnc_ << "'"
                << " was not declared");
        CL_MSG_STREAM(cl_note, LocationWriter(0, &fncLoc_) << "note: "
                << "function '" << fnc_ << "'"
                << " was declarede here");
        return;
    }

    const ArgDecl &ad = i->second;
    op->data.var.id     = ad.varId;
    op->data.var.name   = ad.varName.c_str();

    op->code            = CL_OPERAND_VAR;
    op->scope           = CL_SCOPE_FUNCTION;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldArgSubst(ICodeListener *slave) {
    return new CldArgSubst(slave);
}
