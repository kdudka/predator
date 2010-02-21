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

#include "config.h"
#include "cld_intchk.hh"

#include "cl_decorator.hh"
#include "cld_opchk.hh"
#include "usagechk.hh"

#include <map>
#include <string>

class CldCbSeqChk: public ClDecoratorBase {
    public:
        CldCbSeqChk(ICodeListener *slave);
        virtual ~CldCbSeqChk() {
            this->setState(S_DESTROYED);
        }

        virtual void file_open(
            const char              *file_name)
        {
            loc_.currentFile = file_name;
            this->setState(S_FILE_LEVEL);
            ClDecoratorBase::file_open(file_name);
        }

        virtual void file_close() {
            this->setState(S_INIT);
            ClDecoratorBase::file_close();
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            loc_ = &fnc->loc;
            this->setState(S_FNC_DECL);
            ClDecoratorBase::fnc_open(fnc);
        }

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->chkArgDecl();
            ClDecoratorBase::fnc_arg_decl(arg_id, arg_src);
        }

        virtual void fnc_close() {
            this->setState(S_FILE_LEVEL);
            ClDecoratorBase::fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            this->setState(S_BLOCK_LEVEL);
            ClDecoratorBase::bb_open(bb_name);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            loc_ = &cli->loc;

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
                case CL_INSN_SWITCH:
                    TRAP;
            }

            ClDecoratorBase::insn(cli);
        }

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
        {
            loc_ = loc;
            this->setState(S_INSN_CALL);
            ClDecoratorBase::insn_call_open(loc, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
        {
            this->chkInsnCallArg();
            ClDecoratorBase::insn_call_arg(arg_id, arg_src);
        }

        virtual void insn_call_close() {
            this->setCallClose();
            ClDecoratorBase::insn_call_close();
        }

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
        {
            loc_ = loc;
            this->setState(S_INSN_SWITCH);
            ClDecoratorBase::insn_switch_open(loc, src);
        }

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            loc_ = loc;
            this->chkInsnSwitchCase();
            ClDecoratorBase::insn_switch_case(loc, val_lo, val_hi, label);
        }

        virtual void insn_switch_close()
        {
            this->setSwitchClose();
            ClDecoratorBase::insn_switch_close();
        }

        virtual void finalize()
        {
            this->setState(S_DONE);
            ClDecoratorBase::finalize();
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
            S_DONE,
            S_DESTROYED
        };

        EState                      state_;
        Location                    loc_;


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
};

class CldLabelChk: public ClDecoratorBase {
    public:
        CldLabelChk(ICodeListener *slave);

        virtual void file_open(
            const char              *file_name)
        {
            loc_.currentFile = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            const struct cl_operand *fnc)
        {
            loc_ = &fnc->loc;
            this->reset();
            ClDecoratorBase::fnc_open(fnc);
        }

        virtual void fnc_close() {
            this->emitWarnings();
            ClDecoratorBase::fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            this->defineLabel(bb_name);
            ClDecoratorBase::bb_open(bb_name);
        }

        virtual void insn(
            const struct cl_insn    *cli)
        {
            loc_ = &cli->loc;

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

            ClDecoratorBase::insn(cli);
        }

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
        {
            loc_ = loc;
            this->reqLabel(label);
            ClDecoratorBase::insn_switch_case(loc, val_lo, val_hi, label);
        }

    private:
        struct LabelState {
            bool                    defined;
            bool                    reachable;
            Location                loc;

            LabelState(): defined(false), reachable(false) { }
        };

        typedef std::map<std::string, LabelState> TMap;

        TMap                map_;
        Location            loc_;

    private:
        void reset();
        void defineLabel(const char *);
        void reqLabel(const char *);
        void emitWarnings();
};

class CldRegUsageChk: public CldOpCheckerBase {
    public:
        CldRegUsageChk(ICodeListener *slave):
            CldOpCheckerBase(slave),
            usageChecker_("register %r")
        {
        }

        virtual void fnc_close() {
            usageChecker_.emitPendingMessages(CldOpCheckerBase::lastLocation());
            usageChecker_.reset();
            CldOpCheckerBase::fnc_close();
        }

    protected:
        virtual void checkDstOperand(const struct cl_operand *op) {
            if (CL_OPERAND_REG != op->code)
                return;

            int id = op->data.reg.id;
            usageChecker_.write(id, id, CldOpCheckerBase::lastLocation());
        }

        virtual void checkSrcOperand(const struct cl_operand *op) {
            if (CL_OPERAND_REG != op->code)
                return;

            int id = op->data.reg.id;
            usageChecker_.read(id, id, CldOpCheckerBase::lastLocation());
        }

    private:
        UsageChecker<int, int> usageChecker_;
};

class CldLcVarUsageChk: public CldOpCheckerBase {
    public:
        CldLcVarUsageChk(ICodeListener *slave):
            CldOpCheckerBase(slave),
            usageChecker_("local variable ")
        {
        }

        virtual void fnc_close() {
            usageChecker_.emitPendingMessages(CldOpCheckerBase::lastLocation());
            usageChecker_.reset();
            CldOpCheckerBase::fnc_close();
        }

    protected:
        virtual void checkDstOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code || CL_SCOPE_FUNCTION != op->scope)
                return;

            usageChecker_.write(op->data.var.id,
                                std::string("'") + op->data.var.name +"'",
                                CldOpCheckerBase::lastLocation());
        }

        virtual void checkSrcOperand(const struct cl_operand *op) {
            if (CL_OPERAND_VAR != op->code || CL_SCOPE_FUNCTION != op->scope)
                return;

            usageChecker_.read(op->data.var.id,
                               std::string("'") + op->data.var.name +"'",
                               CldOpCheckerBase::lastLocation());
        }

    private:
        UsageChecker<int, std::string> usageChecker_;
};

// /////////////////////////////////////////////////////////////////////////////
// CldCbSeqChk implementation
CldCbSeqChk::CldCbSeqChk(ICodeListener *slave):
    ClDecoratorBase(slave),
    state_(S_INIT)
{
}

const char* CldCbSeqChk::toString(EState state) {
#define CASE_TO_STRING(state) case state: return #state;
    switch (state) {
        CASE_TO_STRING(S_INIT)
        CASE_TO_STRING(S_FILE_LEVEL)
        CASE_TO_STRING(S_FNC_DECL)
        CASE_TO_STRING(S_FNC_BODY)
        CASE_TO_STRING(S_BLOCK_LEVEL)
        CASE_TO_STRING(S_INSN_CALL)
        CASE_TO_STRING(S_INSN_SWITCH)
        CASE_TO_STRING(S_DONE)
        CASE_TO_STRING(S_DESTROYED)
        default:
            CL_DIE("CldCbSeqChk::toString");
            return NULL;
    }
}

void CldCbSeqChk::emitUnexpected(const char *what) {
    CL_ERROR_MSG(LocationWriter(0, &loc_), "unexpected callback in state "
            << toString(state_) << " (" << what << ")");
}

void CldCbSeqChk::emitUnexpected(EState state) {
    this->emitUnexpected(toString(state));
}

void CldCbSeqChk::setState(EState newState) {
    switch (state_) {
        case S_INIT:
            switch (newState) {
                case S_FILE_LEVEL:
                case S_DONE:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_FILE_LEVEL:
            switch (newState) {
                case S_INIT:
                case S_FNC_DECL:
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

        case S_DONE:
            if (S_DESTROYED != newState)
                this->emitUnexpected(newState);
            break;

        case S_DESTROYED:
            // this should never happen
            this->emitUnexpected(newState);
            TRAP;
    }

    state_ = newState;
}

void CldCbSeqChk::chkArgDecl() {
    if (S_FNC_DECL != state_)
        this->emitUnexpected("fnc_arg_decl");
}

void CldCbSeqChk::chkInsnNop() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_NOP");
}

void CldCbSeqChk::chkInsnJmp() {
    switch (state_) {
        case S_FNC_DECL:
        case S_BLOCK_LEVEL:
            break;

        default:
            this->emitUnexpected("CL_INSN_JMP");
    }

    state_ = S_FNC_BODY;
}

void CldCbSeqChk::chkInsnCond() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_COND");

    state_ = S_FNC_BODY;
}

void CldCbSeqChk::chkInsnRet() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_RET");

    state_ = S_FNC_BODY;
}

void CldCbSeqChk::chkInsnAbort() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_ABORT");

    state_ = S_FNC_BODY;
}

void CldCbSeqChk::chkInsnUnop() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_UNOP");
}

void CldCbSeqChk::chkInsnBinop() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("CL_INSN_BINOP");
}

void CldCbSeqChk::chkInsnCallArg() {
    if (S_INSN_CALL != state_)
        this->emitUnexpected("insn_call_arg");
}

void CldCbSeqChk::setCallClose() {
    if (S_INSN_CALL != state_)
        this->emitUnexpected("insn_call_close");

    state_ = S_BLOCK_LEVEL;
}

void CldCbSeqChk::chkInsnSwitchCase() {
    if (S_INSN_SWITCH != state_)
        this->emitUnexpected("insn_switch_case");
}

void CldCbSeqChk::setSwitchClose() {
    if (S_INSN_SWITCH != state_)
        this->emitUnexpected("insn_switch_close");

    state_ = S_FNC_BODY;
}


// /////////////////////////////////////////////////////////////////////////////
// CldLabelChk implementation
CldLabelChk::CldLabelChk(ICodeListener *slave):
    ClDecoratorBase(slave)
{
}

void CldLabelChk::reset() {
    map_.clear();
}

void CldLabelChk::defineLabel(const char *label) {
    LabelState &ls = map_[label];
    if (ls.defined) {
        CL_ERROR_MSG(LocationWriter(loc_), "redefinition of label '"
                << label << "'");
        CL_NOTE_MSG(LocationWriter(ls.loc), "originally defined here");
    }
    ls.defined = true;
    if (ls.loc.locLine < 0)
        ls.loc = loc_;
}

void CldLabelChk::reqLabel(const char *label) {
    LabelState &ls = map_[label];
    ls.reachable = true;
    if (ls.loc.locLine < 0)
        ls.loc = loc_;
}

void CldLabelChk::emitWarnings() {
    TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        const std::string label = i->first;
        const LabelState &ls = i->second;
        const LocationWriter lw(ls.loc, &loc_);

        if (!ls.defined)
            CL_ERROR_MSG(lw, "jump to undefined label '" << label << "'");

        if (!ls.reachable)
            CL_WARN_MSG(lw, "unreachable label '" << label << "'");
    }
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_intchk.hh for more details
ICodeListener* createCldIntegrityChk(ICodeListener *slave) {
    return
        new CldLcVarUsageChk(
        new CldRegUsageChk(
        new CldLabelChk(
        new CldCbSeqChk(slave)
        )));
}
