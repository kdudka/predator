#include "code_listener.h"
#include "cl_factory.hh"
#include "cl_private.hh"

#include <cstring>

static const char *app_name = "<cl uninitialized>";
static bool app_name_allocated = false;

static void no_debug(const char *)
{
}

static void def_debug(const char *msg)
{
    fprintf(stderr, "%s: %s\n", app_name, msg);
}

static void def_warn(const char *msg)
{
    fprintf(stderr, "%s: warning: %s\n", app_name, msg);
}

static void def_error(const char *msg)
{
    fprintf(stderr, "%s: error: %s\n", app_name, msg);
}

static void def_note(const char *msg)
{
    fprintf(stderr, "%s: note: %s\n", app_name, msg);
}

static void def_die(const char *msg)
{
    def_error(msg);
    exit(EXIT_FAILURE);
}

static struct cl_init_data init_data = {
    def_debug,          // .debug
    def_warn,           // .warn
    def_error,          // .error
    def_note,           // .error
    def_die             // .die
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
    app_name = strdup(name);
    if (app_name)
        app_name_allocated = true;
    else
        cl_die("strdup failed");

    if (verbose)
        init_data.debug = def_debug;
    else
        init_data.debug = no_debug;

    init_data.warn   = def_warn;
    init_data.error  = def_error;
    init_data.note   = def_note;
    init_data.die    = def_die;
}

void cl_global_cleanup(void)
{
    if (app_name_allocated)
        free((char *)app_name);
}

struct ClWrapData {
    ICodeListener                   *listener;              ///< wrapped object
    FILE                            *output;
    bool                            close_file_on_destroy;
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
        cl_die("uncaught exception in CL_WRAP"); \
    } \
} while (0)

// TODO: merge this together with the previous macro definition
#define CL_WRAP_VA(fnc, ...) do { \
    try { \
        ICodeListener *listener = cl_obtain_from_wrap(self); \
        listener->fnc(__VA_ARGS__); \
    } \
    catch (...) { \
        cl_die("uncaught exception in CL_WRAP"); \
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
            const char              *fnc_name,
            int                     line,
            enum cl_scope_e         scope)
{
    CL_WRAP_VA(fnc_open, fnc_name, line, scope);
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

static void cl_wrap_insn_call(
            struct cl_code_listener *self,
            int                     line,
            struct cl_operand       *dst,
            const char              *fnc_name)
{
    CL_WRAP_VA(insn_call, line, dst, fnc_name);
}

static void cl_wrap_insn_call_arg(
            struct cl_code_listener *self,
            int                     pos,
            struct cl_operand       *src)
{
    CL_WRAP_VA(insn_call_arg, pos, src);
}

static void cl_wrap_destroy(struct cl_code_listener *self)
{
    ClWrapData *data = static_cast<ClWrapData *>(self->data);
    delete data->listener;

    if (data->output && data->close_file_on_destroy)
        fclose(data->output);

    delete data;
    delete self;
}

struct cl_code_listener* cl_create_listener_wrap(ICodeListener *listener)
{
    ClWrapData *data = new ClWrapData;
    data->listener              = listener;
    data->output                = NULL;
    data->close_file_on_destroy = false;

    struct cl_code_listener *wrap = new cl_code_listener;
    wrap->data          = data;
    wrap->file_open     = cl_wrap_file_open;
    wrap->file_close    = cl_wrap_file_close;
    wrap->fnc_open      = cl_wrap_fnc_open;
    wrap->fnc_arg_decl  = cl_wrap_fnc_arg_decl;
    wrap->fnc_close     = cl_wrap_fnc_close;
    wrap->bb_open       = cl_wrap_bb_open;
    wrap->insn_jmp      = cl_wrap_insn_jmp;
    wrap->insn_cond     = cl_wrap_insn_cond;
    wrap->insn_ret      = cl_wrap_insn_ret;
    wrap->insn_unop     = cl_wrap_insn_unop;
    wrap->insn_binop    = cl_wrap_insn_binop;
    wrap->insn_call     = cl_wrap_insn_call;
    wrap->insn_call_arg = cl_wrap_insn_call_arg;
    wrap->destroy       = cl_wrap_destroy;

    return wrap;
}

struct cl_code_listener* cl_writer_create(
        const char              *fmt,
        FILE                    *output,
        bool                    close_file_on_destroy)
{
    try {
        ClFactory factory;
        ICodeListener *listener = factory.create(fmt, output);
        if (!listener) {
            cl_error("failed to create cl_code_listener");
            return NULL;
        }

        struct cl_code_listener *wrap = cl_create_listener_wrap(listener);
        ClWrapData *data = static_cast<ClWrapData *>(wrap->data);
        data->output                = output;
        data->close_file_on_destroy = close_file_on_destroy;

        return wrap;
    }
    catch (...) {
        CL_DIE("uncaught exception in cl_writer_create");
    }
}
