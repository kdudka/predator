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

#include <cl/location.hh>

#include "cl.hh"

#include <iostream>

class ClLocator: public ICodeListener {
    public:
        ClLocator();

        virtual void file_open(const char *file_name) {
            lastLoc_.currentFile = file_name;
        }

        virtual void file_close() {
            lastLoc_.currentFile.clear();
        }

        virtual void fnc_open(const struct cl_operand *fnc) {
            this->printLocation(&fnc->loc);
            lastLoc_ = &fnc->loc;
        }

        virtual void fnc_arg_decl(int, const struct cl_operand *) { }
        virtual void fnc_close() { }
        virtual void bb_open(const char *) { }

        virtual void insn(const struct cl_insn *cli) {
            if (CL_INSN_JMP != cli->code)
                this->printLocation(&cli->loc);
            lastLoc_ = &cli->loc;
        }

        virtual void insn_call_open(const struct cl_location *loc,
                                    const struct cl_operand *,
                                    const struct cl_operand *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_call_arg(int, const struct cl_operand *) { }
        virtual void insn_call_close() { }

        virtual void insn_switch_open(const struct cl_location *loc,
                                      const struct cl_operand *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_switch_case(const struct cl_location *loc,
                                      const struct cl_operand *,
                                      const struct cl_operand *, const char *)
        {
            this->printLocation(loc);
            lastLoc_ = loc;
        }

        virtual void insn_switch_close() { }

        virtual void finalize() { }

    private:
        std::ostream            &out_;
        Location                lastLoc_;

    private:
        void printLocation(const struct cl_location *);
};

// /////////////////////////////////////////////////////////////////////////////
// ClLocator implementation
// TODO: remove idents of unused args
ClLocator::ClLocator():
    out_(std::cout)
{
}

void ClLocator::printLocation(const struct cl_location *loc) {
    out_ << LocationWriter(loc, &lastLoc_)
        << "linearized code follows..."
        << std::endl;
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see cl_locator.hh for more details
ICodeListener* createClLocator(const char *) {
    return new ClLocator;
}
