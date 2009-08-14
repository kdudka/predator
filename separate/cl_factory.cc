#include "cl_factory.hh"
#include "cl_pp.hh"

#include <map>
#include <sstream>
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

    std::ostringstream str;
    str << __FUNCTION__ << ": file format not found: " << fmt;
    cl_warn(str.str().c_str());

    return 0;
}
