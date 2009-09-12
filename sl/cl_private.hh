#ifndef H_GUARD_CL_PRIVATE_H
#define H_GUARD_CL_PRIVATE_H

#include "code_listener.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <signal.h>
#define TRAP raise(SIGTRAP)

// pull in __attribute__ ((__noreturn__))
#define CL_DIE(msg) do { \
    cl_die("fatal error: " msg); \
    abort(); \
} while (0)

#define CL_MSG_STREAM(fnc, to_stream) do { \
    std::ostringstream str; \
    str << to_stream; \
    fnc(str.str().c_str()); \
} while (0)

#define CL_MSG_STREAM_INTERNAL(fnc, to_stream) \
    CL_MSG_STREAM(fnc, __FILE__ << ":" << __LINE__ \
            << ": " << to_stream << " [internal location]")

#define CL_INTERNAL_ERROR(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_error, "internal error: " << to_stream)

#define CL_DEBUG(to_stream) \
    CL_MSG_STREAM_INTERNAL(cl_debug, "debug: " << to_stream)

void cl_debug(const char *msg);
void cl_warn(const char *msg);
void cl_error(const char *msg);
void cl_note(const char *msg);
void cl_die(const char *msg);

/**
 * C++ interface for listener objects. It can be wrapped to struct code_listener
 * object when exposing to pure C world. See code_listener for details about
 * methods and their parameters.
 */
class ICodeListener {
    public:
        virtual ~ICodeListener() { }

        virtual void reg_type_db(
            cl_get_type_fnc_t       fnc,
            void                    *user_data)
            = 0;

        virtual void file_open(
            const char              *file_name)
            = 0;

        virtual void file_close()
            = 0;

        virtual void fnc_open(
            const struct cl_location*loc,
            const char              *fnc_name,
            enum cl_scope_e         scope)
            = 0;

        virtual void fnc_arg_decl(
            int                     arg_id,
            const char              *arg_name)
            = 0;

        virtual void fnc_close()
            = 0;

        virtual void bb_open(
            const char              *bb_name)
            = 0;

        virtual void insn(
            const struct cl_insn    *cli)
            = 0;

        virtual void insn_call_open(
            const struct cl_location*loc,
            const struct cl_operand *dst,
            const struct cl_operand *fnc)
            = 0;

        virtual void insn_call_arg(
            int                     arg_id,
            const struct cl_operand *arg_src)
            = 0;

        virtual void insn_call_close()
            = 0;

        virtual void insn_switch_open(
            const struct cl_location*loc,
            const struct cl_operand *src)
            = 0;

        virtual void insn_switch_case(
            const struct cl_location*loc,
            const struct cl_operand *val_lo,
            const struct cl_operand *val_hi,
            const char              *label)
            = 0;

        virtual void insn_switch_close()
            = 0;
};

class AbstractCodeListener: public ICodeListener {
    public:
        AbstractCodeListener():
            getTypeFnc_(0)
        {
        }

        virtual void reg_type_db(
            cl_get_type_fnc_t       fnc,
            void                    *user_data)
        {
            getTypeFnc_             = fnc;
            getTypeFncData_         = user_data;
        }

    protected:
        struct cl_type* getType(cl_type_uid_t uid) {
            if (!getTypeFnc_)
                CL_DIE("call of uninitialized AbstractCodeListener::getType()");

            return getTypeFnc_(uid, getTypeFncData_);
        }

    private:
        cl_get_type_fnc_t           getTypeFnc_;
        void                        *getTypeFncData_;
};

/**
 * wrap ICodeListener object so that it can be exposed to pure C world
 */
struct cl_code_listener* cl_create_listener_wrap(ICodeListener *);

/**
 * retrieve wrapped ICodeListener object
 */
ICodeListener* cl_obtain_from_wrap(struct cl_code_listener *);

struct Location {
    std::string currentFile;
    std::string locFile;
    int         locLine;
    int         locColumn;

    Location():
        locLine(-1),
        locColumn(-1)
    {
    }

    Location(const Location *loc):
        locLine(-1),
        locColumn(-1)
    {
        if (loc) {
            currentFile     = loc->currentFile;
            locFile         = loc->locFile;
            locLine         = loc->locLine;
            locColumn       = loc->locColumn;
        }
    }

    Location(const struct cl_location *loc):
        locLine(-1),
        locColumn(-1)
    {
        if (loc)
            this->operator=(loc);
    }

    Location& operator=(const struct cl_location *loc) {
        if (loc->file)
            locFile = loc->file;
        else
            locFile.clear();

        locLine = loc->line;
        locColumn = loc->column;

        return *this;
    }
};

struct LocationWriter {
    Location                        loc;
    Location                        last;

    LocationWriter(const Location &loc_, const Location *last_ = 0):
        loc(loc_),
        last(last_)
    {
    }

    LocationWriter(const struct cl_location *loc_, const Location *last_ = 0):
        loc(loc_),
        last(last_)
    {
    }
};
std::ostream& operator<<(std::ostream &, const LocationWriter &);


template <typename TCont>
bool hasKey(const TCont &cont, const typename TCont::key_type &key) {
    return cont.end() != cont.find(key);
}

#endif /* H_GUARD_CL_PRIVATE_H */
