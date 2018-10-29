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
#include <cl/cl_msg.hh>
#include <cl/code_listener.h>
#include <cl/memdebug.hh>

#include "cl.hh"
#include "cl_factory.hh"
#include "cl_private.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#if CL_MSG_SQUEEZE_REPEATS
    static std::string last_msg;

#   define CHK_LAST(text, filter) do {              \
       if ((filter) && !last_msg.compare(text))     \
           return;                                  \
       last_msg = (text);                           \
    } while (0)

#else // CL_MSG_SQUEEZE_REPEATS
#   define CHK_LAST(text, filter) do { } while (0)
#endif

const struct cl_loc cl_loc_unknown = {
    0,  // .file
    0,  // .line
    0,  // .column
    0,  // .sysp
};

const struct cl_loc* cl_loc_fallback(
        const struct cl_loc     *loc,
        const struct cl_loc     *fallback)
{
    return (loc->file)
        ? loc
        : fallback;
}

static const char *app_name = "<cl uninitialized>";
static bool app_name_allocated = false;

static void cl_no_msg(const char *)
{
}

static void cl_def_msg(const char *msg)
{
    if (app_name)
        fprintf(stderr, "%s: %s\n", app_name, msg);
    else
        fprintf(stderr, "%s\n", msg);
}

static void cl_def_die(const char *msg)
{
    cl_def_msg(msg);
    exit(EXIT_FAILURE);
}

static struct cl_init_data init_data = {
    cl_def_msg,            // .debug
    cl_def_msg,            // .warn
    cl_def_msg,            // .error
    cl_def_msg,            // .note
    cl_def_die,            // .die
    0                      // .debug_level
};

void cl_debug(const char *msg)
{
    init_data.debug(msg);
}

void cl_warn(const char *msg)
{
    CHK_LAST(msg, /* filter */ true);
    init_data.warn(msg);
}

void cl_error(const char *msg)
{
    CHK_LAST(msg, /* filter */ true);
    init_data.error(msg);
}

void cl_note(const char *msg)
{
    CHK_LAST(msg, /* filter */ false);
    init_data.note(msg);
}

void cl_die(const char *msg)
{
    // this call should never return (TODO: annotation?)
    init_data.die(msg);
    abort();
}

int cl_debug_level(void)
{
    return init_data.debug_level;
}

void cl_global_init(struct cl_init_data *data)
{
    initMemDrift();
    init_data = *data;
}

void cl_global_init_defaults(const char *name, int debug_level)
{
    initMemDrift();
    if (app_name_allocated)
        free((char *) app_name);

    app_name_allocated = false;

    if (name) {
        app_name = strdup(name);
        if (app_name)
            app_name_allocated = true;
        else
            CL_DIE("strdup failed");
    } else {
        app_name = NULL;
    }

    if (debug_level)
        init_data.debug = cl_def_msg;
    else
        init_data.debug = cl_no_msg;

    init_data.warn          = cl_def_msg;
    init_data.error         = cl_def_msg;
    init_data.note          = cl_def_msg;
    init_data.die           = cl_def_die;
    init_data.debug_level   = debug_level;
}

void cl_global_cleanup(void)
{
    if (app_name_allocated)
        free((char *)app_name);
}

bool cl_is_term_insn(enum cl_insn_e code)
{
    switch (code) {
        case CL_INSN_JMP:
        case CL_INSN_COND:
        case CL_INSN_RET:
        case CL_INSN_ABORT:
        case CL_INSN_SWITCH:
            return true;

        case CL_INSN_CLOBBER:
        case CL_INSN_NOP:
        case CL_INSN_UNOP:
        case CL_INSN_BINOP:
        case CL_INSN_CALL:
        case CL_INSN_LABEL:
            return false;
    }

    CL_BREAK_IF("cl_is_term_insn() got something special");
    return false;
}

ICodeListener* cl_obtain_from_wrap(struct cl_code_listener *wrap)
{
    return static_cast<ICodeListener *>(wrap->data);
}

// do not throw an exception through the pure C interface
#define CL_WRAP(fnc) do { \
    try { \
        ICodeListener *listener = cl_obtain_from_wrap(self); \
        listener->fnc(); \
    } \
    catch (...) { \
        CL_DIE("uncaught exception in " #fnc "()"); \
    } \
} while (0)

// TODO: merge this together with the previous macro definition
#define CL_WRAP_VA(fnc, ...) do { \
    try { \
        ICodeListener *listener = cl_obtain_from_wrap(self); \
        listener->fnc(__VA_ARGS__); \
    } \
    catch (...) { \
        CL_DIE("uncaught exception in " #fnc "()"); \
    } \
} while (0)

static void cl_wrap_file_open(
            struct cl_code_listener *self,
            const char              *file_name)
{
    CL_WRAP_VA(file_open, file_name);
}

static void cl_wrap_file_close(
            struct cl_code_listener *self)
{
    CL_WRAP(file_close);
}

static void cl_wrap_fnc_open(
            struct cl_code_listener *self,
            const struct cl_operand *fnc)
{
    CL_WRAP_VA(fnc_open, fnc);
}

static void cl_wrap_fnc_arg_decl(
            struct cl_code_listener *self,
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    CL_WRAP_VA(fnc_arg_decl, arg_id, arg_src);
}

static void cl_wrap_fnc_close(
            struct cl_code_listener *self)
{
    CL_WRAP(fnc_close);
}

static void cl_wrap_bb_open(
            struct cl_code_listener *self,
            const char              *bb_name)
{
    CL_WRAP_VA(bb_open, bb_name);
}

static void cl_wrap_insn(
            struct cl_code_listener *self,
            const struct cl_insn    *cli)
{
    CL_WRAP_VA(insn, cli);
}

static void cl_wrap_insn_call_open(
            struct cl_code_listener *self,
            const struct cl_loc     *loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
{
    CL_WRAP_VA(insn_call_open, loc, dst, fnc);
}

static void cl_wrap_insn_call_arg(
            struct cl_code_listener *self,
            int                     arg_id,
            const struct cl_operand *arg_src)
{
    CL_WRAP_VA(insn_call_arg, arg_id, arg_src);
}

static void cl_wrap_insn_call_close(
            struct cl_code_listener *self)
{
    CL_WRAP(insn_call_close);
}

static void cl_wrap_insn_switch_open(
            struct cl_code_listener *self,
            const struct cl_loc     *loc,
            const struct cl_operand *src)
{
    CL_WRAP_VA(insn_switch_open, loc, src);
}

static void cl_wrap_insn_switch_case(
            struct cl_code_listener *self,
            const struct cl_loc     *loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
{
    CL_WRAP_VA(insn_switch_case, loc, val_lo, val_hi, label);
}

static void cl_wrap_insn_switch_close(
            struct cl_code_listener *self)
{
    CL_WRAP(insn_switch_close);
}

static void cl_wrap_acknowledge(
            struct cl_code_listener *self)
{
    CL_WRAP(acknowledge);
}

static void cl_wrap_destroy(struct cl_code_listener *self)
{
    ICodeListener *cl = static_cast<ICodeListener *>(self->data);
    delete cl;
    delete self;
}

struct cl_code_listener* cl_create_listener_wrap(ICodeListener *listener)
{
    struct cl_code_listener *wrap = new cl_code_listener;
    wrap->data              = listener;

    // pure C callback to C++ object wrappers
    wrap->file_open         = cl_wrap_file_open;
    wrap->file_close        = cl_wrap_file_close;
    wrap->fnc_open          = cl_wrap_fnc_open;
    wrap->fnc_arg_decl      = cl_wrap_fnc_arg_decl;
    wrap->fnc_close         = cl_wrap_fnc_close;
    wrap->bb_open           = cl_wrap_bb_open;
    wrap->insn              = cl_wrap_insn;
    wrap->insn_call_open    = cl_wrap_insn_call_open;
    wrap->insn_call_arg     = cl_wrap_insn_call_arg;
    wrap->insn_call_close   = cl_wrap_insn_call_close;
    wrap->insn_switch_open  = cl_wrap_insn_switch_open;
    wrap->insn_switch_case  = cl_wrap_insn_switch_case;
    wrap->insn_switch_close = cl_wrap_insn_switch_close;
    wrap->acknowledge       = cl_wrap_acknowledge;
    wrap->destroy           = cl_wrap_destroy;

    return wrap;
}

struct cl_code_listener* cl_code_listener_create(const char *config_string)
{
    try {
        ClFactory factory;
        ICodeListener *listener = factory.create(config_string);
        if (!listener) {
            CL_ERROR("failed to create cl_code_listener");
            return NULL;
        }

        return cl_create_listener_wrap(listener);
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_code_listener_create()");
    }
}
