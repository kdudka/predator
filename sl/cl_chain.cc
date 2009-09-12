/**
 * @file cl_chain.cc
 * @attention not tested yet
 */

#include "code_listener.h"
#include "cl_private.hh"

#include <boost/foreach.hpp>

#include <vector>

/// local ICodeListener implementation
class ClChain: public ICodeListener {
    public:
        virtual ~ClChain();

        virtual void reg_type_db(
            cl_get_type_fnc_t       fnc,
            void                    *user_data);

        virtual void file_open(
            const char              *file_name);

        virtual void file_close();

        virtual void fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope);

        virtual void fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void fnc_close();

        virtual void bb_open(
            const char              *bb_name);

        virtual void insn(
            const struct cl_insn    *cli);

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc);

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src);

        virtual void insn_call_close();

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src);

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label);

        virtual void insn_switch_close();

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

void ClChain::reg_type_db(
            cl_get_type_fnc_t       fnc,
            void                    *user_data)
{
    CL_CHAIN_FOREACH_VA(reg_type_db, fnc, user_data);
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
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
{
    CL_CHAIN_FOREACH_VA(fnc_open, loc, fnc_name, scope);
}

void ClChain::fnc_arg_decl(
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    CL_CHAIN_FOREACH_VA(fnc_arg_decl, arg_id, arg_src);
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

void ClChain::insn(
            const struct cl_insn    *cli)
{
    CL_CHAIN_FOREACH_VA(insn, cli);
}

void ClChain::insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
{
    CL_CHAIN_FOREACH_VA(insn_call_open, loc, dst, fnc);
}

void ClChain::insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    CL_CHAIN_FOREACH_VA(insn_call_arg, arg_id, arg_src);
}

void ClChain::insn_call_close()
{
    CL_CHAIN_FOREACH(insn_call_close);
}

void ClChain::insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
{
    CL_CHAIN_FOREACH_VA(insn_switch_open, loc, src);
}

void ClChain::insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    CL_CHAIN_FOREACH_VA(insn_switch_case, loc, val_lo, val_hi, label);
}

void ClChain::insn_switch_close()
{
    CL_CHAIN_FOREACH(insn_switch_close);
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
        ICodeListener *listener = cl_obtain_from_wrap(self);
        ClChain *chain = dynamic_cast<ClChain *>(listener);
        if (!chain)
            CL_DIE("failed to downcast ICodeListener to ClChain");

        chain->append(item);
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_chain_append");
    }
}
