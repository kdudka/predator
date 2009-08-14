/**
 * @file cl_chain.cc
 * @attention not tested yet
 */

#include "cl.h"
#include "cl_private.hh"

#include <boost/foreach.hpp>

#include <vector>

/// local IClListener implementation
class ClChain: public IClListener {
    public:
        virtual ~ClChain();

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn_jmp(
            int                     line,
            const char              *label);

        virtual void insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false);

        virtual void insn_ret(
            int                     line,
            struct cl_operand       *src);

        virtual void insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src);

        virtual void insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2);

        virtual void insn_call(
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name);

        virtual void insn_call_arg(
            int                     pos,
            struct cl_operand       *src);

    public:
        void append(cl_code_listener *);

    private:
        std::vector<cl_code_listener *> list_;
};

// /////////////////////////////////////////////////////////////////////////////
// ClChain implementation
#define CL_CHAIN_FOREACH(fnc) do { \
    BOOST_FOREACH(cl_code_listener *item, list_) { \
        item->fnc(item); \
    } \
} while (0)

#define CL_CHAIN_FOREACH_VA(fnc, ...) do { \
    BOOST_FOREACH(cl_code_listener *item, list_) { \
        item->fnc(item, __VA_ARGS__); \
    } \
} while (0)

ClChain::~ClChain() {
    CL_CHAIN_FOREACH(destroy);
}

void ClChain::append(cl_code_listener *item) {
    list_.push_back(item);
}

void ClChain::file_open(
            const char              *file_name)
{
    CL_CHAIN_FOREACH_VA(file_open, file_name);
}

void ClChain::file_close()
{
    CL_CHAIN_FOREACH(file_close);
}

void ClChain::fnc_open(
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope)
{
    CL_CHAIN_FOREACH_VA(fnc_open, fnc_name, line, scope);
}

void ClChain::fnc_arg_decl(
            int                     arg_pos,
            const char              *arg_name)
{
    CL_CHAIN_FOREACH_VA(fnc_arg_decl, arg_pos, arg_name);
}

void ClChain::fnc_close()
{
    CL_CHAIN_FOREACH(fnc_close);
}

void ClChain::bb_open(
            const char              *bb_name)
{
    CL_CHAIN_FOREACH_VA(bb_open, bb_name);
}

void ClChain::insn_jmp(
            int                     line,
            const char              *label)
{
    CL_CHAIN_FOREACH_VA(insn_jmp, line, label);
}

void ClChain::insn_cond(
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
{
    CL_CHAIN_FOREACH_VA(insn_cond, line, src, label_true, label_false);
}

void ClChain::insn_ret(
            int                     line,
            struct cl_operand       *src)
{
    CL_CHAIN_FOREACH_VA(insn_ret, line, src);
}

void ClChain::insn_unop(
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
{
    CL_CHAIN_FOREACH_VA(insn_unop, line, type, dst, src);
}

void ClChain::insn_binop(
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
{
    CL_CHAIN_FOREACH_VA(insn_binop, line, type, dst, src1, src2);
}

void ClChain::insn_call(
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name)
{
    CL_CHAIN_FOREACH_VA(insn_call, line, dst, fnc_name);
}

void ClChain::insn_call_arg(
            int                     pos,
            struct cl_operand       *src)
{
    CL_CHAIN_FOREACH_VA(insn_call_arg, pos, src);
}

// /////////////////////////////////////////////////////////////////////////////
// public interface, see code_listener.h for more details
struct cl_code_listener* cl_chain_create(void)
{
    try {
        return cl_create_listener_wrap(new ClChain);
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_chain_create");
    }
}

void cl_chain_append(
        struct cl_code_listener      *self,
        struct cl_code_listener      *item)
{
    try {
        IClListener *listener = cl_code_listener_from_wrap(self);
        ClChain *chain = dynamic_cast<ClChain *>(listener);
        if (!chain)
            CL_DIE("failed to downcast IClListener to ClChain");

        chain->append(item);
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_chain_append");
    }
}
