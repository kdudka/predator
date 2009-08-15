#include "cl_factory.hh"
#include "cl_pp.hh"

#include <map>
#include <string>

using std::string;

// /////////////////////////////////////////////////////////////////////////////
// ClFactory implementation
struct ClFactory::Private {
    typedef ICodeListener* (*TCreateFnc)(int);
    typedef std::map<string, TCreateFnc> TMap;
    TMap map;
};

ClFactory::ClFactory():
    d(new Private)
{
    d->map["pp"] = &createClPrettyPrint;
}

ClFactory::~ClFactory() {
    delete d;
}

ICodeListener* ClFactory::create(const char *fmt, int fd_out) {
    Private::TMap::iterator i = d->map.find(fmt);
    if (i != d->map.end())
        return (i->second)(fd_out);

    CL_MSG_STREAM(cl_warn, __FILE__ << ":" << __LINE__ << ": warning: "
            << "file format '" << fmt << "' not found [internal location]");

    return 0;
}
