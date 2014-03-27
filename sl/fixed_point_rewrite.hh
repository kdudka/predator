/*
 * Copyright (C) 2014 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_FIXED_POINT_REWRITE_H
#define H_GUARD_FIXED_POINT_REWRITE_H

#include "fixed_point.hh"

namespace FixedPoint {

class StateRewriter {
    public:
        /// *pState has to be valid till the destruction of StateRewriter
        StateRewriter(GlobalState *pState):
            state_(*pState)
        {
        }

        /// takes ownership of *insn
        void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn);

        /// takes ownership of *insn
        void replaceInsn(TLocIdx at, GenericInsn *insn);

        void dropInsn(TLocIdx at);

        bool /* any change */ dedupOutgoingEdges(TLocIdx at);

    private:
        GlobalState                &state_;
};

class ClInsn: public GenericInsn {
    public:
        ClInsn(TInsn insn):
            insn_(insn)
        {
        }

        virtual void writeToStream(std::ostream &str) const;

        virtual TInsn clInsn() const
        {
            return insn_;
        }

    private:
        TInsn insn_;

        virtual GenericInsn* doClone() const
        {
            return new ClInsn(*this);
        }
};

class TextInsn: public GenericInsn {
    public:
        TextInsn(const std::string &text):
            text_(text)
        {
        }

        virtual void writeToStream(std::ostream &str) const;

        virtual TInsn clInsn() const
        {
            return 0;
        }

    private:
        std::string text_;

        virtual GenericInsn *doClone() const
        {
            return new TextInsn(*this);
        }
};

} // namespace FixedPoint

#endif /* H_GUARD_FIXED_POINT_REWRITE_H */
