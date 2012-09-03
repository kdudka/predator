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

#include "clf_unilabel.hh"

#include <cl/cl_msg.hh>

#include "cl_filter.hh"

#include <map>
#include <sstream>
#include <string>

class ClfUniLabel: public ClFilterBase {
    public:
        ClfUniLabel(ICodeListener *slave, cl_scope_e scope);

        virtual void file_open(const char *file_name) {
            if (CL_SCOPE_STATIC == scope_)
                this->reset();
            ClFilterBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            if (CL_SCOPE_FUNCTION == scope_)
                this->reset();
            ClFilterBase::fnc_open(fnc);
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            std::string resolved(this->resolveLabel(bb_name));
            ClFilterBase::bb_open(resolved.c_str());
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            struct cl_insn local_cli = *cli;
            switch (cli->code) {
                case CL_INSN_JMP: {
                        std::string resolved(this->resolveLabel(
                                    cli->data.insn_jmp.label));

                        local_cli.data.insn_jmp.label = resolved.c_str();
                        ClFilterBase::insn(&local_cli);
                    }
                    break;

                case CL_INSN_COND: {
                        std::string resolved1(this->resolveLabel(
                                    cli->data.insn_cond.then_label));

                        std::string resolved2(this->resolveLabel(
                                    cli->data.insn_cond.else_label));

                        local_cli.data.insn_cond.then_label = resolved1.c_str();
                        local_cli.data.insn_cond.else_label = resolved2.c_str();
                        ClFilterBase::insn(&local_cli);
                    }
                    break;

                default:
                    ClFilterBase::insn(cli);
                    break;
            }
        }

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            std::string resolved(this->resolveLabel(label));
            ClFilterBase::insn_switch_case(loc, val_lo, val_hi,
                    resolved.c_str());
        }


    private:
        typedef std::map<std::string, int> TMap;

        cl_scope_e      scope_;
        TMap            map_;
        int             last_;

    private:
        std::string resolveLabel(const char *);
        int labelLookup(const char *);
        void reset();
};

ClfUniLabel::ClfUniLabel(ICodeListener *slave, cl_scope_e scope):
    ClFilterBase(slave),
    scope_(scope),
    last_(0)
{
    switch (scope) {
        case CL_SCOPE_GLOBAL:
        case CL_SCOPE_STATIC:
        case CL_SCOPE_FUNCTION:
            break;

        default:
            CL_DIE("invalid scope for ClfUniLabel filter");
    }
}

std::string ClfUniLabel::resolveLabel(const char *label)
{
    std::ostringstream str;
    str << "L" << this->labelLookup(label);
    return str.str();
}

int ClfUniLabel::labelLookup(const char *label)
{
    std::string str(label);

    TMap::iterator i = map_.find(str);
    if (map_.end() != i)
        return i->second;

    map_[str] = ++last_;
    return last_;
}

void ClfUniLabel::reset()
{
    map_.clear();
    last_ = 0;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see clf_unilabel.hh for more details
ICodeListener* createClfUniLabel(ICodeListener *slave, cl_scope_e scope)
{
    return new ClfUniLabel(slave, scope);
}
