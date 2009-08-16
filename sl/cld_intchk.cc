#include "cld_intchk.hh"
#include "cl_decorator.hh"

#include <map>
#include <string>

class CldCbSeqChk: public ClDecoratorBase {
    public:
        CldCbSeqChk(ICodeListener *slave);

        virtual void file_open(
            const char              *file_name)
        {
            file_ = file_name;
            this->setState(S_FILE_LEVEL);
            ClDecoratorBase::file_open(file_name);
        }

        virtual void file_close() {
            this->setState(S_INIT);
            ClDecoratorBase::file_close();
        }

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            line_ = line;
            this->setState(S_FNC_DECL);
            ClDecoratorBase::fnc_open(line, fnc_name, scope);
        }

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
        {
            this->chkArgDecl();
            ClDecoratorBase::fnc_arg_decl(arg_pos, arg_name);
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

        virtual void insn_jmp(
            int                     line,
            const char              *label)
        {
            line_ = line;
            this->chkInsnJmp();
            ClDecoratorBase::insn_jmp(line, label);
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            line_ = line;
            this->chkInsnCond();
            ClDecoratorBase::insn_cond(line, src, label_true, label_false);
        }

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src)
        {
            line_ = line;
            this->chkInsnRet();
            ClDecoratorBase::insn_ret(line, src);
        }

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            line_ = line;
            this->chkInsnUnop();
            ClDecoratorBase::insn_unop(line, type, dst, src);
        }

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            line_ = line;
            this->chkInsnBinop();
            ClDecoratorBase::insn_binop(line, type, dst, src1, src2);
        }

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            line_ = line;
            this->setState(S_INSN_CALL);
            ClDecoratorBase::insn_call_open(line, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
        {
            this->chkInsnCallArg();
            ClDecoratorBase::insn_call_arg(arg_pos, arg_src);
        }

        virtual void insn_call_close() {
            this->setCallClose();
            ClDecoratorBase::insn_call_close();
        }

    private:
        enum EState {
            S_INIT,
            S_FILE_LEVEL,
            S_FNC_DECL,
            S_FNC_BODY,
            S_BLOCK_LEVEL,
            S_INSN_CALL
        };

        EState                      state_;
        std::string                 file_;
        int                         line_;


    private:
        static const char* toString(EState);
        void emitUnexpected(const char *);
        void emitUnexpected(EState);
        void setState(EState);
        void chkArgDecl();
        void chkInsnJmp();
        void chkInsnCond();
        void chkInsnRet();
        void chkInsnUnop();
        void chkInsnBinop();
        void chkInsnCallArg();
        void setCallClose();
};

class CldLabelChk: public ClDecoratorBase {
    public:
        CldLabelChk(ICodeListener *slave);

        virtual void file_open(
            const char              *file_name)
        {
            file_ = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            line_ = line;
            this->reset();
            ClDecoratorBase::fnc_open(line, fnc_name, scope);
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

        virtual void insn_jmp(
            int                     line,
            const char              *label)
        {
            this->reqLabel(label);
            ClDecoratorBase::insn_jmp(line, label);
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            this->reqLabel(label_true);
            this->reqLabel(label_false);
            ClDecoratorBase::insn_cond(line, src, label_true, label_false);
        }

    private:
        struct LabelState {
            bool defined;
            bool reachable;
            int  line;

            LabelState(): defined(false), reachable(false), line(-1) { }
        };

        typedef std::map<std::string, LabelState> TMap;

        TMap                map_;
        std::string         file_;
        int                 line_;

    private:
        void reset();
        void defineLabel(const char *);
        void reqLabel(const char *);
        void emitWarnings();
};

// TODO: go through CFG and report all unused/initialized?
class CldRegUsageChk: public ClDecoratorBase {
    public:
        CldRegUsageChk(ICodeListener *slave);

        virtual void file_open(
            const char              *file_name)
        {
            file_ = file_name;
            ClDecoratorBase::file_open(file_name);
        }

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            line_ = line;
            this->reset();
            ClDecoratorBase::fnc_open(line, fnc_name, scope);
        }

        virtual void fnc_close() {
            this->emitWarnings();
            ClDecoratorBase::fnc_close();
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            line_ = line;
            this->handleSrc(src);
            ClDecoratorBase::insn_cond(line, src, label_true, label_false);
        }

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src)
        {
            line_ = line;
            this->handleSrc(src);
            ClDecoratorBase::insn_ret(line, src);
        }

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            line_ = line;
            this->handleDst(dst);
            this->handleSrc(src);
            ClDecoratorBase::insn_unop(line, type, dst, src);
        }

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            line_ = line;
            this->handleDst(dst);
            this->handleSrc(src1);
            this->handleSrc(src2);
            ClDecoratorBase::insn_binop(line, type, dst, src1, src2);
        }

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            line_ = line;
            this->handleDst(dst);
            this->handleSrc(fnc);
            ClDecoratorBase::insn_call_open(line, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
        {
            this->handleSrc(arg_src);
            ClDecoratorBase::insn_call_arg(arg_pos, arg_src);
        }

    private:
        struct Usage {
            bool read;
            bool written;
            int  line;

            Usage(): read(false), written(false), line(-1) { }
        };

        typedef std::map<int, Usage> TMap;

        TMap                map_;
        std::string         file_;
        int                 line_;

    private:
        void reset();
        void handleDst(struct cl_operand *);
        void handleSrc(struct cl_operand *);
        void emitWarnings();
};

// /////////////////////////////////////////////////////////////////////////////
// CldCbSeqChk implementation
CldCbSeqChk::CldCbSeqChk(ICodeListener *slave):
    ClDecoratorBase(slave),
    state_(S_INIT),
    line_(-1)
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
        default:
            CL_DIE("CldCbSeqChk::toString");
            return NULL;
    }
}

void CldCbSeqChk::emitUnexpected(const char *what) {
    CL_MSG_STREAM(cl_error, file_ << ":" << line_ << ": error: "
            << "unexpected callback in state "
            << toString(state_) << " (" << what << ")");
}

void CldCbSeqChk::emitUnexpected(EState state) {
    this->emitUnexpected(toString(state));
}

void CldCbSeqChk::setState(EState newState) {
    switch (state_) {
        case S_INIT:
            if (S_FILE_LEVEL != newState)
                this->emitUnexpected(newState);
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
            if (S_BLOCK_LEVEL != newState)
                this->emitUnexpected(newState);
            break;

        case S_BLOCK_LEVEL:
            switch (newState) {
                case S_FILE_LEVEL:
                case S_BLOCK_LEVEL:
                case S_INSN_CALL:
                    break;
                default:
                    this->emitUnexpected(newState);
            }
            break;

        case S_INSN_CALL:
            this->emitUnexpected(newState);
            break;
    }

    state_ = newState;
}

void CldCbSeqChk::chkArgDecl() {
    if (S_FNC_DECL != state_)
        this->emitUnexpected("fnc_arg_decl");
}

void CldCbSeqChk::chkInsnJmp() {
    if (S_FNC_DECL == state_) {
        state_ = S_FNC_BODY;
        return;
    }

    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("insn_jmp");
}

void CldCbSeqChk::chkInsnCond() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("insn_cond");
}

void CldCbSeqChk::chkInsnRet() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("insn_ret");
}

void CldCbSeqChk::chkInsnUnop() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("insn_unop");
}

void CldCbSeqChk::chkInsnBinop() {
    if (S_BLOCK_LEVEL != state_)
        this->emitUnexpected("insn_binop");
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


// /////////////////////////////////////////////////////////////////////////////
// CldLabelChk implementation
CldLabelChk::CldLabelChk(ICodeListener *slave):
    ClDecoratorBase(slave),
    line_(-1)
{
}

void CldLabelChk::reset() {
    map_.clear();
}

void CldLabelChk::defineLabel(const char *label) {
    LabelState &ls = map_[label];
    ls.defined = true;
    if (ls.line < 0)
        ls.line = line_;
}

void CldLabelChk::reqLabel(const char *label) {
    LabelState &ls = map_[label];
    ls.reachable = true;
    if (ls.line < 0)
        ls.line = line_;
}

void CldLabelChk::emitWarnings() {
    TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        const std::string label = i->first;
        const LabelState &ls = i->second;

        if (!ls.defined) {
            CL_MSG_STREAM(cl_error, file_ << ":" << ls.line << ": error: "
                    << "jump to undefined label '" << label << "'");
        }

        if (!ls.reachable) {
            CL_MSG_STREAM(cl_warn, file_ << ":" << ls.line << ": warning: "
                    << "unreachable label '" << label << "'");
        }
    }
}


// /////////////////////////////////////////////////////////////////////////////
// CldRegUsageChk implementation
CldRegUsageChk::CldRegUsageChk(ICodeListener *slave):
    ClDecoratorBase(slave),
    line_(-1)
{
}

void CldRegUsageChk::reset() {
    map_.clear();
}

void CldRegUsageChk::handleDst(struct cl_operand *op) {
    if (CL_OPERAND_REG != op->type)
        return;

    Usage &u = map_[op->value.reg_id];
    u.written = true;
    if (u.line < 0)
        u.line = line_;
}

void CldRegUsageChk::handleSrc(struct cl_operand *op) {
    if (CL_OPERAND_REG != op->type)
        return;

    Usage &u = map_[op->value.reg_id];
    u.read = true;
    if (u.line < 0)
        u.line = line_;
}

void CldRegUsageChk::emitWarnings() {
    TMap::iterator i;
    for (i = map_.begin(); i != map_.end(); ++i) {
        int reg = i->first;
        const Usage &u = i->second;

        if (!u.read) {
            CL_MSG_STREAM(cl_warn, file_ << ":" << u.line << ": warning: "
                    << "unused register %r" << reg);
        }

        if (!u.written) {
            CL_MSG_STREAM(cl_error, file_ << ":" << u.line << ": error: "
                    << "uninitialized register %r" << reg);
        }
    }
}


// /////////////////////////////////////////////////////////////////////////////
// public interface, see cld_unilabel.hh for more details
ICodeListener* createCldIntegrityChk(ICodeListener *slave) {
    return
        new CldRegUsageChk(
        new CldLabelChk(
        new CldCbSeqChk(slave)
        ));
}
