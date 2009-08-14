#include "cl_factory.hh"

#include <map>
#include <sstream>
#include <string>

using std::string;

// /////////////////////////////////////////////////////////////////////////////
// ClFactory implementation
struct ClFactory::Private {
    typedef ICodeListener* (*TCreateFnc)(FILE *);
    typedef std::map<string, TCreateFnc> TMap;
    TMap map;
};

ClFactory::ClFactory():
    d(new Private)
{
#if 0
    d->map["cgt"] = &createCltFileWriter;
#endif
}

ClFactory::~ClFactory() {
    delete d;
}

ICodeListener* ClFactory::create(const char *fmt, FILE *output) {
    Private::TMap::iterator i = d->map.find(fmt);
    if (i != d->map.end())
        return (i->second)(output);

    std::ostringstream str;
    str << __FUNCTION__ << ": file format not found: " << fmt;
    cl_warn(str.str().c_str());

    return 0;
}
