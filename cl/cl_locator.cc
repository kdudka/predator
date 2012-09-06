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

#include "cl_locator.hh"

#include <cl/cl_msg.hh>

#include "cl.hh"
#include "cl_private.hh"

#include <iostream>

class ClLocator: public ICodeListener {
    public:
        ClLocator();

        virtual void file_open(const char *file_name) {
            CL_LOC_SET_FILE(lastLoc_, file_name);
        }

        virtual void file_close() {
            lastLoc_ = cl_loc_unknown;
        }

        virtual void fnc_open(const struct cl_operand *fnc) {
            const struct cl_loc &loc = fnc->data.cst.data.cst_fnc.loc;
            this->printLocation(&loc);
            lastLoc_ = loc;
        }

        virtual void fnc_arg_decl(int, const struct cl_operand *) { }
        virtual void fnc_close() { }
        virtual void bb_open(const char *) { }

        virtual void insn(const struct cl_insn *cli) {
            this->printLocation(&cli->loc);
            CL_LOC_SETIF(lastLoc_, &cli->loc);
        }

        virtual void insn_call_open(const struct cl_loc     *loc,
                                    const struct cl_operand *,
                                    const struct cl_operand *)
        {
            this->printLocation(loc);
            CL_LOC_SETIF(lastLoc_, loc);
        }

        virtual void insn_call_arg(int, const struct cl_operand *) { }
        virtual void insn_call_close() { }

        virtual void insn_switch_open(const struct cl_loc     *loc,
                                      const struct cl_operand *)
        {
            this->printLocation(loc);
            CL_LOC_SETIF(lastLoc_, loc);
        }

        virtual void insn_switch_case(const struct cl_loc     *loc,
                                      const struct cl_operand *,
                                      const struct cl_operand *, const char *)
        {
            this->printLocation(loc);
            CL_LOC_SETIF(lastLoc_, loc);
        }

        virtual void insn_switch_close() { }

        virtual void acknowledge() { }

    private:
        std::ostream            &out_;
        struct cl_loc           lastLoc_;

    private:
        void printLocation(const struct cl_loc *);
};

// /////////////////////////////////////////////////////////////////////////////
// ClLocator implementation
ClLocator::ClLocator():
    out_(std::cout)
{
}

void ClLocator::printLocation(const struct cl_loc *loc)
{
    CL_DEBUG_MSG(cl_loc_fallback(loc, &lastLoc_),
            "linearized code follows...");
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_locator.hh for more details
ICodeListener* createClLocator(const char *)
{
    return new ClLocator;
}
