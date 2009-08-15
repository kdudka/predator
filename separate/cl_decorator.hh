#ifndef H_GUARD_CL_DECORATOR_H
#define H_GUARD_CL_DECORATOR_H

#include "cl_private.hh"

class ClDecoratorBase: public ICodeListener {
    public:
        virtual ~ClDecoratorBase() {
            delete slave_;
        }

        virtual void file_open(
            const char              *file_name)
        {
            slave_->file_open(file_name);
        }

        virtual void file_close() {
            slave_->file_close();
        }

        virtual void fnc_open(
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            slave_->fnc_open(line, fnc_name, scope);
        }

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
        {
            slave_->fnc_arg_decl(arg_pos, arg_name);
        }

        virtual void fnc_close() {
            slave_->fnc_close();
        }

        virtual void bb_open(
            const char              *bb_name)
        {
            slave_->bb_open(bb_name);
        }

        virtual void insn_jmp(
            int                     line,
            const char              *label)
        {
            slave_->insn_jmp(line, label);
        }

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            slave_->insn_cond(line, src, label_true, label_false);
        }

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src)
        {
            slave_->insn_ret(line, src);
        }

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            slave_->insn_unop(line, type, dst, src);
        }

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            slave_->insn_binop(line, type, dst, src1, src2);
        }

        virtual void insn_call_open(
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            slave_->insn_call_open(line, dst, fnc);
        }

        virtual void insn_call_arg(
            int                     arg_pos,
            struct cl_operand       *arg_src)
        {
            slave_->insn_call_arg(arg_pos, arg_src);
        }

        virtual void insn_call_close() {
            slave_->insn_call_close();
        }

    protected:
        ClDecoratorBase(ICodeListener *slave):
            slave_(slave)
        {
        }

    private:
        ICodeListener *slave_;
};

#endif /* H_GUARD_CL_DECORATOR_H */
