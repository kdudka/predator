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
#include "clf_intchk.hh"

#include "cl_filter.hh"
#include "cl_private.hh"
#include "clf_opchk.hh"
#include "usagechk.hh"

#include <map>
#include <string>

#ifndef CLF_BYPASS_USAGE_CHK
#   define CLF_BYPASS_USAGE_CHK 0
#endif

class ClfCbSeqChk: public ClFilterBase {
    public:
        ClfCbSeqChk(ICodeListener *slave);
        virtual ~ClfCbSeqChk() {
            this->setState(S_DESTROYED);
        }

        virtual void file_open(
            const char              *file_name)
        {
            CL_LOC_SET_FILE(loc_, file_name);
            this->setState(S_FILE_LEVEL);
            ClFilterBase::file_open(file_name);
        }

        virtual void file_close() {
            loc_ = cl_loc_unknown;
            this->setState(S_INIT);
            ClFilterBase::file_close();
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            loc_ = fnc->data.cst.data.cst_fnc.loc;
            this->setState(S_FNC_DECL);
            ClFilterBase::fnc_open(fnc);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->chkArgDecl();
            ClFilterBase::fnc_arg_decl(arg_id, arg_src);
        }

        virtual void fnc_close() {
            this->setState(S_FILE_LEVEL);
            ClFilterBase::fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            this->setState(S_BLOCK_LEVEL);
            ClFilterBase::bb_open(bb_name);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            loc_ = cli->loc;

            switch (cli->code) {
                case CL_INSN_NOP:
                    this->chkInsnNop();
                    break;

                case CL_INSN_JMP:
                    this->chkInsnJmp();
                    break;

                case CL_INSN_COND:
                    this->chkInsnCond();
                    break;

                case CL_INSN_RET:
                    this->chkInsnRet();
                    break;

                case CL_INSN_ABORT:
                    this->chkInsnAbort();
                    break;

                case CL_INSN_UNOP:
                    this->chkInsnUnop();
                    break;

                case CL_INSN_BINOP:
                    this->chkInsnBinop();
                    break;

                case CL_INSN_CALL:
                    CL_BREAK_IF("ClfCbSeqChk::insn() got CL_INSN_CALL, why?");
                    break;

                case CL_INSN_SWITCH:
                    CL_BREAK_IF("ClfCbSeqChk::insn() got CL_INSN_SWITCH, why?");
                    break;

                case CL_INSN_LABEL:
                    this->chkInsnLabel();
                    break;
            }

            ClFilterBase::insn(cli);
        }

        virtual void insn_call_open(
            const struct cl_loc     *loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            CL_LOC_SETIF(loc_, loc);
            this->setState(S_INSN_CALL);
            ClFilterBase::insn_call_open(loc, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->chkInsnCallArg();
            ClFilterBase::insn_call_arg(arg_id, arg_src);
        }

        virtual void insn_call_close() {
            this->setCallClose();
            ClFilterBase::insn_call_close();
        }

        virtual void insn_switch_open(
            const struct cl_loc     *loc,
            const struct cl_operand *src)
        {
            CL_LOC_SETIF(loc_, loc);
            this->setState(S_INSN_SWITCH);
            ClFilterBase::insn_switch_open(loc, src);
        }

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            CL_LOC_SETIF(loc_, loc);
            this->chkInsnSwitchCase();
            ClFilterBase::insn_switch_case(loc, val_lo, val_hi, label);
        }

        virtual void insn_switch_close()
        {
            this->setSwitchClose();
            ClFilterBase::insn_switch_close();
        }

        virtual void acknowledge()
        {
            this->setState(S_ACKNOWLEDGE);
            ClFilterBase::acknowledge();
        }

    private:
        enum EState {
            S_INIT,
            S_FILE_LEVEL,
            S_FNC_DECL,
            S_FNC_BODY,
            S_BLOCK_LEVEL,
            S_INSN_CALL,
            S_INSN_SWITCH,
            S_ACKNOWLEDGE,
            S_DESTROYED
        };

        EState                      state_;
        struct cl_loc               loc_;


    private:
        static const char* toString(EState);
        void emitUnexpected(const char *);
        void emitUnexpected(EState);
        void setState(EState);
        void chkArgDecl();
        void chkInsnNop();
        void chkInsnJmp();
        void chkInsnCond();
        void chkInsnRet();
        void chkInsnAbort();
        void chkInsnUnop();
        void chkInsnBinop();
        void chkInsnCallArg();
        void setCallClose();
        void chkInsnSwitchCase();
        void setSwitchClose();
        void chkInsnLabel();
};

class ClfLabelChk: public ClFilterBase {
    public:
        ClfLabelChk(ICodeListener *slave);

        virtual void file_open(
            const char              *file_name)
        {
            CL_LOC_SET_FILE(loc_, file_name);
            ClFilterBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            loc_ = fnc->data.cst.data.cst_fnc.loc;
            this->reset();
            ClFilterBase::fnc_open(fnc);
        }

        virtual void fnc_close() {
            this->emitWarnings();
            ClFilterBase::fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            this->defineLabel(bb_name);
            ClFilterBase::bb_open(bb_name);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            loc_ = cli->loc;

            switch (cli->code) {
                case CL_INSN_JMP:
                    this->reqLabel(cli->data.insn_jmp.label);
                    break;

                case CL_INSN_COND:
                    this->reqLabel(cli->data.insn_cond.then_label);
                    this->reqLabel(cli->data.insn_cond.else_label);
                    break;

                default:
                    break;
            }

            ClFilterBase::insn(cli);
        }

        virtual void insn_switch_case(
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            CL_LOC_SETIF(loc_, loc);
            this->reqLabel(label);
            ClFilterBase::insn_switch_case(loc, val_lo, val_hi, label);
        }

    private:
        struct LabelState {
            bool                    defined;
            bool                    reachable;
            struct cl_loc           loc;

            LabelState():
                defined(false),
                reachable(false),
                loc(cl_loc_unknown)
            {
            }
        };

        typedef std::map<std::string, LabelState> TMap;

        TMap                map_;
        struct cl_loc       loc_;

    private:
        void reset();
        void defineLabel(const char *);
        void reqLabel(const char *);
        void emitWarnings();
};

class ClfRegUsageChk: public ClfOpCheckerBase {
    public:
        ClfRegUsageChk(ICodeListener *slave):
            ClfOpCheckerBase(slave),
            usageChecker_("register %r")
        {
        }

        virtual void fnc_close() {
            usageChecker_.emitPendingMessages(ClfOpCheckerBase::lastLocation());
            usageChecker_.reset();
            ClfOpCheckerBase::fnc_close();
        }

    protected:
        virtual void checkDstOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code)
                return;

            int id = op->data.var->uid;
            usageChecker_.write(id, id, ClfOpCheckerBase::lastLocation());
        }

        virtual void checkSrcOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code)
                return;

            int id = op->data.var->uid;
            usageChecker_.read(id, id, ClfOpCheckerBase::lastLocation());
        }

    private:
        UsageChecker<int, int> usageChecker_;
};

class ClfLcVarUsageChk: public ClfOpCheckerBase {
    public:
        ClfLcVarUsageChk(ICodeListener *slave):
            ClfOpCheckerBase(slave),
            usageChecker_("local variable ")
        {
        }

        virtual void fnc_close() {
            usageChecker_.emitPendingMessages(ClfOpCheckerBase::lastLocation());
            usageChecker_.reset();
            ClfOpCheckerBase::fnc_close();
        }

    protected:
        virtual void checkDstOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code || CL_SCOPE_FUNCTION != op->scope)
                return;

            usageChecker_.write(op->data.var->uid,
                                std::string("'") + op->data.var->name +"'",
                                ClfOpCheckerBase::lastLocation());
        }

        virtual void checkSrcOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code || CL_SCOPE_FUNCTION != op->scope)
                return;

            usageChecker_.read(op->data.var->uid,
                               std::string("'") + op->data.var->name +"'",
                               ClfOpCheckerBase::lastLocation());
        }

    private:
        UsageChecker<int, std::string> usageChecker_;
};

// /////////////////////////////////////////////////////////////////////////////
// ClfCbSeqChk implementation
ClfCbSeqChk::ClfCbSeqChk(ICodeListener *slave):
    ClFilterBase(slave),
    state_(S_INIT),
    loc_(cl_loc_unknown)
{
}

const char* ClfCbSeqChk::toString(EState state)
{
#define CASE_TO_STRING(state) case state: return #state;
    switch (state) {
        CASE_TO_STRING(S_INIT)
        CASE_TO_STRING(S_FILE_LEVEL)
        CASE_TO_STRING(S_FNC_DECL)
        CASE_TO_STRING(S_FNC_BODY)
        CASE_TO_STRING(S_BLOCK_LEVEL)
        CASE_TO_STRING(S_INSN_CALL)
        CASE_TO_STRING(S_INSN_SWITCH)
        CASE_TO_STRING(S_ACKNOWLEDGE)
        CASE_TO_STRING(S_DESTROYED)
        default:
            CL_DIE("ClfCbSeqChk::toString");
            return NULL;
    }
}

void ClfCbSeqChk::emitUnexpected(const char *what)
{
    CL_ERROR_MSG(&loc_, "unexpected callback in state "
            << toString(state_) << " (" << what << ")");
}

void ClfCbSeqChk::emitUnexpected(EState state)
{
    this->emitUnexpected(toString(state));
}

void ClfCbSeqChk::setState(EState newState)
{
    switch (state_) {
        case S_INIT:
            switch (newState) {
                case S_FILE_LEVEL:
                case S_ACKNOWLEDGE:
                case S_DESTROYED:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_FILE_LEVEL:
            switch (newState) {
                case S_INIT:
                case S_FNC_DECL:
                case S_DESTROYED:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_FNC_DECL:
            this->emitUnexpected(newState);
            break;

        case S_FNC_BODY:
            switch (newState) {
                case S_BLOCK_LEVEL:
                case S_FILE_LEVEL:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_BLOCK_LEVEL:
            switch (newState) {
                case S_INSN_CALL:
                case S_INSN_SWITCH:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_INSN_CALL:
        case S_INSN_SWITCH:
            this->emitUnexpected(newState);
            break;

        case S_ACKNOWLEDGE:
            if (S_DESTROYED != newState)
                this->emitUnexpected(newState);
            break;

        case S_DESTROYED:
            // this should never happen
            this->emitUnexpected(newState);
            CL_TRAP;
    }

    state_ = newState;
}

void ClfCbSeqChk::chkArgDecl()
{
    if (S_FNC_DECL != state_)
        this->emitUnexpected("fnc_arg_decl");
}

void ClfCbSeqChk::chkInsnNop()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_NOP");
}

void ClfCbSeqChk::chkInsnJmp()
{
    switch (state_) {
        case S_FNC_DECL:
        case S_BLOCK_LEVEL:
            break;

        default:
            this->emitUnexpected("CL_INSN_JMP");
    }

    state_ = S_FNC_BODY;
}

void ClfCbSeqChk::chkInsnCond()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_COND");

    state_ = S_FNC_BODY;
}

void ClfCbSeqChk::chkInsnRet()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_RET");

    state_ = S_FNC_BODY;
}

void ClfCbSeqChk::chkInsnAbort()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_ABORT");

    state_ = S_FNC_BODY;
}

void ClfCbSeqChk::chkInsnUnop()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_UNOP");
}

void ClfCbSeqChk::chkInsnBinop()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_BINOP");
}

void ClfCbSeqChk::chkInsnLabel()
{
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_LABEL");
}

void ClfCbSeqChk::chkInsnCallArg()
{
    if (S_INSN_CALL != state_)
        this->emitUnexpected("insn_call_arg");
}

void ClfCbSeqChk::setCallClose()
{
    if (S_INSN_CALL != state_)
        this->emitUnexpected("insn_call_close");

    state_ = S_BLOCK_LEVEL;
}

void ClfCbSeqChk::chkInsnSwitchCase()
{
    if (S_INSN_SWITCH != state_)
        this->emitUnexpected("insn_switch_case");
}

void ClfCbSeqChk::setSwitchClose()
{
    if (S_INSN_SWITCH != state_)
        this->emitUnexpected("insn_switch_close");

    state_ = S_FNC_BODY;
}


// /////////////////////////////////////////////////////////////////////////////
// ClfLabelChk implementation
ClfLabelChk::ClfLabelChk(ICodeListener *slave):
    ClFilterBase(slave),
    loc_(cl_loc_unknown)
{
}

void ClfLabelChk::reset()
{
    map_.clear();
}

void ClfLabelChk::defineLabel(const char *label)
{
    LabelState &ls = map_[label];
    if (ls.defined) {
        CL_ERROR_MSG(&loc_, "redefinition of label '" << label << "'");
        CL_NOTE_MSG(&ls.loc, "originally defined here");
    }
    ls.defined = true;
    if (!ls.loc.file)
        ls.loc = loc_;
}

void ClfLabelChk::reqLabel(const char *label)
{
    LabelState &ls = map_[label];
    ls.reachable = true;
    if (!ls.loc.file)
        ls.loc = loc_;
}

void ClfLabelChk::emitWarnings()
{
    TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        const std::string label = i->first;
        const LabelState &ls = i->second;
        const struct cl_loc *loc = cl_loc_fallback(&ls.loc, &loc_);

        if (!ls.defined)
            CL_ERROR_MSG(loc, "jump to undefined label '" << label << "'");

        if (!ls.reachable)
            CL_WARN_MSG(loc, "unreachable label '" << label << "'");
    }
}

namespace {
    ICodeListener* usageChk(ICodeListener *slave) {
#if CLF_BYPASS_USAGE_CHK
        return slave;
#else
        return
            new ClfLcVarUsageChk(
            new ClfRegUsageChk(slave));
#endif
    }
} // namespace


// /////////////////////////////////////////////////////////////////////////////
// public interface, see clf_intchk.hh for more details
ICodeListener* createClfIntegrityChk(ICodeListener *slave)
{
    return usageChk(
        new ClfLabelChk(
        new ClfCbSeqChk(slave)));
}
