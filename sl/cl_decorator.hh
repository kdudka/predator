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
            struct cl_location      *loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
        {
            slave_->fnc_open(loc, fnc_name, scope);
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
            struct cl_location      *loc,
            const char              *label)
        {
            slave_->insn_jmp(loc, label);
        }

        virtual void insn_cond(
            struct cl_location      *loc,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
        {
            slave_->insn_cond(loc, src, label_true, label_false);
        }

        virtual void insn_ret(
            struct cl_location      *loc,
            struct cl_operand       *src)
        {
            slave_->insn_ret(loc, src);
        }

        virtual void insn_unop(
            struct cl_location      *loc,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
        {
            slave_->insn_unop(loc, type, dst, src);
        }

        virtual void insn_binop(
            struct cl_location      *loc,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
        {
            slave_->insn_binop(loc, type, dst, src1, src2);
        }

        virtual void insn_call_open(
            struct cl_location      *loc,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
        {
            slave_->insn_call_open(loc, dst, fnc);
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
