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

class IStateRewriter {
    public:
        virtual ~IStateRewriter() { }
        virtual void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *) = 0;
        virtual void replaceInsn(TLocIdx at, GenericInsn *insn) = 0;
        virtual void dropInsn(TLocIdx at) = 0;
};

class MultiRewriter: public IStateRewriter {
    public:
        virtual void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn);
        virtual void replaceInsn(TLocIdx at, GenericInsn *insn);
        virtual void dropInsn(TLocIdx at);

        /// does NOT take ownership of writer
        void appendWriter(IStateRewriter &slave);

    private:
        std::vector<IStateRewriter *> slaveList_;
};

class RecordRewriter: public IStateRewriter {
    public:
        RecordRewriter();
        virtual ~RecordRewriter();

        virtual void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn);
        virtual void replaceInsn(TLocIdx at, GenericInsn *insn);
        virtual void dropInsn(TLocIdx at);

        // apply the changes in the same order as they came; and drop them
        void flush(IStateRewriter *pConsumer);

        bool empty() const;

    private:
        RecordRewriter(const RecordRewriter &);
        RecordRewriter& operator=(const RecordRewriter &);

    private:
        struct Private;
        Private *d;
};

class StateRewriter: public IStateRewriter {
    public:
        /// *pState has to be valid till the destruction of StateRewriter
        StateRewriter(GlobalState *pState):
            state_(*pState)
        {
        }

        /// takes ownership of *insn
        virtual void insertInsn(TLocIdx src, TLocIdx dst, GenericInsn *insn);

        /// takes ownership of *insn
        virtual void replaceInsn(TLocIdx at, GenericInsn *insn);

        virtual void dropInsn(TLocIdx at);

        bool /* any change */ dedupOutgoingEdges(TLocIdx at);

    private:
        GlobalState                &state_;
};

enum EVarLevel {
    VL_NONE,
    VL_CODE_LISTENER,
    VL_CONTAINER_VAR
};

struct GenericVar {
    EVarLevel           code;
    int                 uid;

    GenericVar():
        code(VL_NONE),
        uid(0)
    {
    }

    GenericVar(const EVarLevel code_, const int uid_):
        code(code_),
        uid(uid_)
    {
    }
};

/// required by std::set<GenericVar>
inline bool operator<(const GenericVar &a, const GenericVar &b)
{
    RETURN_IF_COMPARED(a, b, code);
    RETURN_IF_COMPARED(a, b, uid);
    return false;
}

typedef std::set<GenericVar>                        TGenericVarSet;

class AnnotatedInsn: public GenericInsn {
    public:
        virtual const TGenericVarSet& liveVars() const = 0;
        virtual const TGenericVarSet& killVars() const = 0;
};

class ClInsn: public AnnotatedInsn {
    public:
        ClInsn(TInsn insn):
            insn_(insn),
            done_(false)
        {
        }

        virtual void writeToStream(std::ostream &str) const;

        virtual const TGenericVarSet& liveVars() const;
        virtual const TGenericVarSet& killVars() const;

        virtual TInsn clInsn() const
        {
            return insn_;
        }

    private:
        TInsn                       insn_;
        mutable TGenericVarSet      live_;
        mutable TGenericVarSet      kill_;
        mutable bool                done_;

        virtual GenericInsn* doClone() const
        {
            return new ClInsn(*this);
        }

        void lazyInit() const;
};

class TextInsn: public AnnotatedInsn {
    public:
        TextInsn(
                const std::string      &text,
                const TGenericVarSet   &live,
                const TGenericVarSet   &kill):
            text_(text),
            live_(live),
            kill_(kill)
        {
        }

        virtual void writeToStream(std::ostream &str) const;

        virtual const TGenericVarSet& liveVars() const
        {
            return live_;
        }

        virtual const TGenericVarSet& killVars() const
        {
            return kill_;
        }

        virtual TInsn clInsn() const
        {
            return 0;
        }

    private:
        const std::string           text_;
        const TGenericVarSet        live_;
        const TGenericVarSet        kill_;

        virtual GenericInsn *doClone() const
        {
            return new TextInsn(*this);
        }
};

} // namespace FixedPoint

#endif /* H_GUARD_FIXED_POINT_REWRITE_H */
