#include "code_listener.h"
#include "cl_factory.hh"
#include "cl_private.hh"

#include <cstdio>
#include <cstring>

#include <unistd.h>

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
    cl_def_msg,            // .error
    cl_def_die             // .die
};

void cl_debug(const char *msg)
{
    init_data.debug(msg);
}

void cl_warn(const char *msg)
{
    init_data.warn(msg);
}

void cl_error(const char *msg)
{
    init_data.error(msg);
}

void cl_note(const char *msg)
{
    init_data.note(msg);
}

void cl_die(const char *msg)
{
    // this call should never return (TODO: annotation?)
    init_data.die(msg);
    abort();
}

void cl_global_init(struct cl_init_data *data)
{
    init_data = *data;
}

void cl_global_init_defaults(const char *name, bool verbose)
{
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

    if (verbose)
        init_data.debug = cl_def_msg;
    else
        init_data.debug = cl_no_msg;

    init_data.warn   = cl_def_msg;
    init_data.error  = cl_def_msg;
    init_data.note   = cl_def_msg;
    init_data.die    = cl_def_die;
}

void cl_global_cleanup(void)
{
    if (app_name_allocated)
        free((char *)app_name);
}

struct ClWrapData {
    ICodeListener                   *listener;              ///< wrapped object
    int                             fd_out;
    bool                            close_fd_on_destroy;
};

ICodeListener* cl_obtain_from_wrap(struct cl_code_listener *wrap)
{
    ClWrapData *data = static_cast<ClWrapData *>(wrap->data);
    return data->listener;
}

// do not throw an exception through the pure C interface
#define CL_WRAP(fnc) do { \
    try { \
        ICodeListener *listener = cl_obtain_from_wrap(self); \
        listener->fnc(); \
    } \
    catch (...) { \
        CL_DIE("uncaught exception in CL_WRAP"); \
    } \
} while (0)

// TODO: merge this together with the previous macro definition
#define CL_WRAP_VA(fnc, ...) do { \
    try { \
        ICodeListener *listener = cl_obtain_from_wrap(self); \
        listener->fnc(__VA_ARGS__); \
    } \
    catch (...) { \
        CL_DIE("uncaught exception in CL_WRAP"); \
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
            int                     line,
            const char              *fnc_name,
            enum cl_scope_e         scope)
{
    CL_WRAP_VA(fnc_open, line, fnc_name, scope);
}

static void cl_wrap_fnc_arg_decl(
            struct cl_code_listener *self,
            int                     arg_pos,
            const char              *arg_name)
{
    CL_WRAP_VA(fnc_arg_decl, arg_pos, arg_name);
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

static void cl_wrap_insn_jmp(
            struct cl_code_listener *self,
            int                     line,
            const char              *label)
{
    CL_WRAP_VA(insn_jmp, line, label);
}

static void cl_wrap_insn_cond(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *src,
            const char              *label_true,
            const char              *label_false)
{
    CL_WRAP_VA(insn_cond, line, src, label_true, label_false);
}

static void cl_wrap_insn_ret(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *src)
{
    CL_WRAP_VA(insn_ret, line, src);
}

static void cl_wrap_insn_unop(
            struct cl_code_listener *self,
            int                     line,
            enum cl_unop_e          type,
            struct cl_operand       *dst,
            struct cl_operand       *src)
{
    CL_WRAP_VA(insn_unop, line, type, dst, src);
}

static void cl_wrap_insn_binop(
            struct cl_code_listener *self,
            int                     line,
            enum cl_binop_e         type,
            struct cl_operand       *dst,
            struct cl_operand       *src1,
            struct cl_operand       *src2)
{
    CL_WRAP_VA(insn_binop, line, type, dst, src1, src2);
}

static void cl_wrap_insn_call_open(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *dst,
            struct cl_operand       *fnc)
{
    CL_WRAP_VA(insn_call_open, line, dst, fnc);
}

static void cl_wrap_insn_call_arg(
            struct cl_code_listener *self,
            int                     arg_pos,
            struct cl_operand       *arg_src)
{
    CL_WRAP_VA(insn_call_arg, arg_pos, arg_src);
}

static void cl_wrap_insn_call_close(
            struct cl_code_listener *self)
{
    CL_WRAP(insn_call_close);
}

static void cl_wrap_destroy(struct cl_code_listener *self)
{
    ClWrapData *data = static_cast<ClWrapData *>(self->data);
    delete data->listener;

    if (data->close_fd_on_destroy)
        close(data->fd_out);

    delete data;
    delete self;
}

struct cl_code_listener* cl_create_listener_wrap(ICodeListener *listener)
{
    ClWrapData *data = new ClWrapData;
    data->listener              = listener;
    data->fd_out                = -1;
    data->close_fd_on_destroy   = false;

    struct cl_code_listener *wrap = new cl_code_listener;
    wrap->data              = data;
    wrap->file_open         = cl_wrap_file_open;
    wrap->file_close        = cl_wrap_file_close;
    wrap->fnc_open          = cl_wrap_fnc_open;
    wrap->fnc_arg_decl      = cl_wrap_fnc_arg_decl;
    wrap->fnc_close         = cl_wrap_fnc_close;
    wrap->bb_open           = cl_wrap_bb_open;
    wrap->insn_jmp          = cl_wrap_insn_jmp;
    wrap->insn_cond         = cl_wrap_insn_cond;
    wrap->insn_ret          = cl_wrap_insn_ret;
    wrap->insn_unop         = cl_wrap_insn_unop;
    wrap->insn_binop        = cl_wrap_insn_binop;
    wrap->insn_call_open    = cl_wrap_insn_call_open;
    wrap->insn_call_arg     = cl_wrap_insn_call_arg;
    wrap->insn_call_close   = cl_wrap_insn_call_close;
    wrap->destroy           = cl_wrap_destroy;

    return wrap;
}

struct cl_code_listener* cl_code_listener_create(
        const char              *fmt,
        int                     fd_out,
        bool                    close_fd_on_destroy)
{
    try {
        ClFactory factory;
        ICodeListener *listener = factory.create(fmt, fd_out);
        if (!listener) {
            CL_MSG_STREAM(cl_error, __FILE__ << ":" << __LINE__ << " error: "
                    << "failed to create cl_code_listener");

            return NULL;
        }

        struct cl_code_listener *wrap = cl_create_listener_wrap(listener);
        ClWrapData *data = static_cast<ClWrapData *>(wrap->data);
        data->fd_out                = fd_out;
        data->close_fd_on_destroy   = close_fd_on_destroy;

        return wrap;
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_writer_create");
    }
}
